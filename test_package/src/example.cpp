#include "stratoceph/stratoceph.h"
#include <vector>
#include <string>

// Hack - make both 'The Elm Architecture' available for test and refactor
#include "stratoceph/ncurses/html_msg.hpp" // HTML -> ncurses GUI
#include "stratoceph/imgui/html_msg.hpp" // HTML -> imgui / open_gl GU

#include <iostream>
#include <map>
#include <memory>
#include <ncurses.h>
#include <stack>
#include <functional>
#include <queue>
#include <filesystem>
#include <cmath>  // std::pow,...
#include <immer/vector.hpp>

namespace first {

    // Splits a size_t range into mod10 sub-ranges
    struct Mod10View {
      using Range = std::pair<size_t,size_t>;
      Range m_range;
      size_t m_subrange_size;
  
      Mod10View(Range range)
        :  m_range{range}
          ,m_subrange_size{static_cast<size_t>(std::pow(10,std::ceil(std::log10(range.second-range.first))-1))} {}
  
      template <class T>
      Mod10View(T const& container) : Mod10View(Range(0,container.size())) {}
  
      // return vector of [begin,end[
      std::vector<Range> subranges() {
        std::vector<std::pair<size_t,size_t>> result{};
        for (size_t i=m_range.first;i<m_range.second;i += m_subrange_size) {
          result.push_back(std::make_pair(i,std::min(i+m_subrange_size,m_range.second)));
        }
        return result;
      }
    };
  
    // ----------------------------------
    // Begin: Forward State
    // ----------------------------------
  
    struct StateImpl; // Forward
    using State = std::shared_ptr<StateImpl>;
  
    // ----------------------------------
    // End: Forward State
    // ----------------------------------
  
    // ----------------------------------
    // Begin: Message
    // ----------------------------------
  
    struct MsgImpl {
      virtual ~MsgImpl() = default;
    };
  
    using Msg = std::shared_ptr<MsgImpl>;
  
    struct NCursesKey : public MsgImpl {
      int key;
      NCursesKey(int key) : key{key}, MsgImpl{} {}
    };
  
    struct Quit : public MsgImpl {};
  
    struct PushStateMsg : public MsgImpl {
      State m_parent{};
      State m_state{};
      PushStateMsg(State const& parent,State const& state)
        :  m_parent{parent}
          ,m_state{state} {}
    };
  
    Msg const QUIT_MSG{std::make_shared<Quit>()};
  
    // ----------------------------------
    // END: Message
    // ----------------------------------
  
    // ----------------------------------
    // Begin: Subscription
    // ----------------------------------
  
    std::optional<Msg> onKey(Event event) {
      if (event.contains("Key")) {
        return Msg{std::make_shared<NCursesKey>(std::stoi(event["Key"]))};
      }
      return std::nullopt;
    }
  
    // ----------------------------------
    // End: Subscription
    // ----------------------------------
  
    // ----------------------------------
    // Begin: Command
    // ----------------------------------
  
    using Cmd = std::function<std::optional<Msg>()>;
  
    std::optional<Msg> Nop() {
      return std::nullopt;
    };
  
    std::optional<Msg> DO_QUIT() {
      return QUIT_MSG;
    };
  
    // ----------------------------------
    // End: Command
    // ----------------------------------
  
    using StateFactory = std::function<State()>;
  
    // ----------------------------------
    // Begin: Model
    // ----------------------------------
  
    struct StateImpl {
    private:
    public:
      using UX = std::vector<std::string>;
      using Options = std::map<char,std::pair<std::string,StateFactory>>;
      UX m_ux;
      Options m_options;
      StateImpl(UX const& ux) : m_ux{ux},m_options{} {}
      void add_option(char ch,std::pair<std::string,StateFactory> option) {
        m_options[ch] = option;
      }
      UX const& ux() const {return m_ux;}
      UX& ux() {return m_ux;}
      Options const& options() const {return m_options;}
      virtual std::pair<std::optional<State>,Cmd> update(Msg const& msg) {
        return {std::nullopt,Nop}; // Default - no StateImpl mutation
      }
    };
     
    struct RBDState : public StateImpl {
      StateFactory SIE_factory = []() {
        auto SIE_ux = StateImpl::UX{
          "RBD to SIE UX goes here"
        };
        return std::make_shared<StateImpl>(SIE_ux);
      };
      using RBD = std::string;
      RBD m_rbd;
      RBDState(RBD rbd) : m_rbd{rbd} ,StateImpl({}) {
        ux().clear();
        ux().push_back(rbd);
        this->add_option('0',{"RBD -> SIE",SIE_factory});
      }
    };
  
    struct RBDsState : public StateImpl {
  
      using RBDs = std::vector<std::string>;
      RBDsState::RBDs m_all_rbds;
      Mod10View m_mod10_view;
  
      struct RBDs_subrange_factory {
        // RBD subrange StateImpl factory
        RBDsState::RBDs m_all_rbds{};
        Mod10View m_mod10_view;
  
        auto operator()() {return std::make_shared<RBDsState>(m_all_rbds,m_mod10_view);}
  
        RBDs_subrange_factory(RBDsState::RBDs all_rbds, Mod10View mod10_view)
          :  m_mod10_view{mod10_view}            
            ,m_all_rbds{all_rbds} {} 
      };
  
      RBDsState(RBDs all_rbds,Mod10View mod10_view)
        :  m_mod10_view{mod10_view}
          ,m_all_rbds{all_rbds}
          ,StateImpl({}) {
  
        auto subranges = m_mod10_view.subranges();
        for (size_t i=0;i<subranges.size();++i) {
          auto const subrange = subranges[i];
          auto const& [begin,end] = subrange;
          auto caption = std::to_string(begin);
          if (end-begin==1) {
            // Single RBD in range option
            this->add_option(static_cast<char>('0'+i),{caption,[rbd=m_all_rbds[begin]](){
              // Single RBT factory
              auto RBD_ux = StateImpl::UX{
                "RBD UX goes here"
              };
              return std::make_shared<RBDState>(rbd);
            }});
          }
          else {
            caption += " .. ";
            caption += std::to_string(end-1);
            this->add_option(static_cast<char>('0'+i),{caption,RBDs_subrange_factory(m_all_rbds,subrange)});
          }
        }
  
        // Initiate view UX
        for (size_t i=m_mod10_view.m_range.first;i<m_mod10_view.m_range.second;++i) {
          auto entry = std::to_string(i);
          entry += ". ";
          entry += m_all_rbds[i];
          this->ux().push_back(entry);
        }
      }
      RBDsState(RBDs all_rbds) : RBDsState(all_rbds,Mod10View(all_rbds)) {}
  
    };
  
    struct May2AprilState : public StateImpl {
      StateFactory RBDs_factory = []() {
        auto  all_rbds = RBDsState::RBDs{
           "RBD #0"
          ,"RBD #1"
          ,"RBD #2"
          ,"RBD #3"
          ,"RBD #4"
          ,"RBD #5"
          ,"RBD #6"
          ,"RBD #7"
          ,"RBD #8"
          ,"RBD #9"
          ,"RBD #10"
          ,"RBD #11"
          ,"RBD #12"
          ,"RBD #13"
          ,"RBD #14"
          ,"RBD #15"
          ,"RBD #16"
          ,"RBD #17"
          ,"RBD #18"
          ,"RBD #19"
          ,"RBD #20"
          ,"RBD #21"
          ,"RBD #22"
          ,"RBD #23"
        };        
        return std::make_shared<RBDsState>(all_rbds);
      };
      May2AprilState(StateImpl::UX ux) : StateImpl{ux} {
        this->add_option('0',{"RBD:s",RBDs_factory});
      }
    };
  
    struct VATReturnsState : public StateImpl {
      VATReturnsState(StateImpl::UX ux) : StateImpl{ux} {}
    };
  
    struct Q1State : public StateImpl {
      StateFactory VATReturns_factory = []() {
        auto VATReturns_ux = StateImpl::UX{
          "VAT Returns UX goes here"
        };
        return std::make_shared<VATReturnsState>(VATReturns_ux);
      };
      Q1State(StateImpl::UX ux) : StateImpl{ux} {
        this->add_option('0',{"VAT Returns",VATReturns_factory});
      }
    };
  
    struct ProjectState : public StateImpl {
      StateFactory may2april_factory = []() {
        auto may2april_ux = StateImpl::UX{
          "May to April"
        };
        return std::make_shared<May2AprilState>(may2april_ux);
      };
  
      StateFactory q1_factory = []() {
        auto q1_ux = StateImpl::UX{
          "Q1 UX goes here"
        };
        return std::make_shared<Q1State>(q1_ux);
      };
  
      ProjectState(StateImpl::UX ux) : StateImpl{ux} {
        this->add_option('0',{"May to April",may2april_factory});
        this->add_option('1',{"Q1",q1_factory});
      }
    };
  
    struct WorkspaceState : public StateImpl {
      StateFactory itfied_factory = []() {
        auto itfied_ux = StateImpl::UX{
          "ITfied UX"
        };
        return std::make_shared<ProjectState>(itfied_ux);
      };
  
      StateFactory orx_x_factory = []() {
        auto org_x_ux = StateImpl::UX{
          "Other Organisation UX"
        };
        return std::make_shared<ProjectState>(org_x_ux);
      };
  
      WorkspaceState(StateImpl::UX ux) : StateImpl{ux} {
        this->add_option('0',{"ITfied AB",itfied_factory});        
        this->add_option('1',{"Org x",orx_x_factory});        
      }
    }; // Workspace StateImpl
  
    struct FrameworkState : public StateImpl {
      StateFactory workspace_0_factory = []() {
        auto workspace_0_ux = StateImpl::UX{
          "Workspace UX"
        };
        return std::make_shared<WorkspaceState>(workspace_0_ux);
      };
  
      FrameworkState(StateImpl::UX ux) : StateImpl{ux} {
        this->add_option('0',{"Workspace x",workspace_0_factory});        
      }
  
      virtual std::pair<std::optional<State>,Cmd> update(Msg const& msg) {
        std::optional<State> new_state{};
        auto key_msg_ptr = std::dynamic_pointer_cast<NCursesKey>(msg);
        if (key_msg_ptr != nullptr) {
          auto ch = key_msg_ptr->key;
          if (ch == '+') {
            this->m_ux.back().push_back('+');
            new_state = std::make_shared<FrameworkState>(*this);          
          }
        }
        return {new_state,Nop};
      }
  
    };
  
    auto framework_state_factory = []() {
      auto framework_ux = StateImpl::UX{
        "Framework UX"
      };
      return std::make_shared<FrameworkState>(framework_ux);
    };
  
    struct Model {
      std::string top_content;
      std::string main_content;
      std::string user_input;
      /*
      The stack contains the 'path of states' the user has navigated to.
      */
      std::stack<State> stack{};
    };
  
    // ----------------------------------
    // Begin: Model
    // ----------------------------------
  
    bool is_quit_msg(Msg const& msg) {
      // std::cout << "\nis_quit_msg sais Hello" << std::flush;
      return msg == QUIT_MSG;
    }
  
    // ----------------------------------
    // Begin: init,update,view
    // ----------------------------------
  
    std::tuple<Model,runtime::IsQuit<Msg>,Cmd> init() {
      // std::cout << "\ninit sais Hello :)" << std::flush;
      Model model = { "Welcome to the top section"
                     ,"This is the main content area"
                     ,""};
  
      model.stack.push(framework_state_factory());
      return {model,is_quit_msg,Nop};
    }
  
    std::pair<Model,Cmd> update(Model model, Msg msg) {
  
      Cmd cmd = Nop;
      std::optional<State> new_state{};
      if (model.stack.size()>0) {
        auto pp = model.stack.top()->update(msg);
        new_state = pp.first;
        cmd = pp.second;
      }
      if (new_state) {
        // Let 'StateImpl' update itself
        model.stack.top() = *new_state; // mutate
      }
      else {
        // Process StateImpl transition or user input
        if (auto key_msg_ptr = std::dynamic_pointer_cast<NCursesKey>(msg);key_msg_ptr != nullptr) {
          auto ch = key_msg_ptr->key; 
          if (ch == KEY_BACKSPACE || ch == 127) { // Handle backspace
            if (!model.user_input.empty()) {
              model.user_input.pop_back();
            }
          } 
          else if (ch == '\n') {
            // User pressed Enter: process command (optional)
            model.user_input.clear(); // Reset input after submission
          } 
          else {
            if (model.user_input.empty() and ch == 'q' or model.stack.size()==0) {
              // std::cout << "\nTime to QUIT!" << std::flush;
              cmd = DO_QUIT;
            }
            else if (model.user_input.empty() and model.stack.size() > 0) {
              if (ch == '-') {
                // (1) Transition back to old StateImpl
                model.stack.pop();
              } 
              else if (model.stack.top()->options().contains(ch)) {
                // (2) Transition to new StateImpl
                cmd = [ch,parent = model.stack.top()]() -> Msg {
                  State new_state = parent->options().at(ch).second();
                  auto msg = std::make_shared<PushStateMsg>(parent,new_state);
                  return msg;
                };
              }
              else {
                model.user_input += ch; // Append typed character
              }
            }
            else {
                model.user_input += ch; // Append typed character
            }
          }
        }    
        else if (auto pimpl = std::dynamic_pointer_cast<PushStateMsg>(msg);pimpl != nullptr) {
          if (model.stack.top() == pimpl->m_parent) {
            // The transition matches
            model.stack.push(pimpl->m_state);
          }
          else {
            model.user_input.push_back('?');
          }
        }    
      }
      // Update UX
      if (model.stack.size() > 0) {
        // StateImpl UX (top window)
        model.top_content.clear();
        for (std::size_t i=0;i<model.stack.top()->ux().size();++i) {
          if (i>0) model.top_content.push_back('\n');
          model.top_content += model.stack.top()->ux()[i];
        }
        // StateImpl transition UX (Midle window)
        model.main_content.clear();
        for (auto const &[ch, option] : model.stack.top()->options()) {
          std::string entry{};
          entry.push_back(ch);
          entry.append(" - ");
          entry.append(option.first);
          entry.push_back('\n');
          model.main_content.append(entry);
        }
      }
  
      return {model,cmd}; // Return updated model
    }
  
    Html_Msg<Msg> view(const Model &model) {
      // std::cout << "\nview sais Hello :)" << std::flush;
  
      // Create a new pugi document
      Html_Msg<Msg> ui{};
      auto& doc = ui.doc;
  
      // Note: HTML doc may be tested for validity at:
      // https://www.w3schools.com/html/tryit.asp?filename=tryhtml_intro
  
      // Create the root HTML element
      pugi::xml_node html = doc.append_child("html");
  
      // Create the body
      pugi::xml_node body = html.append_child("body");
  
      // Create the top section with class "content"
      pugi::xml_node top = body.append_child("div");
      top.append_attribute("class") = "content";
      top.text().set(model.top_content.c_str());
  
      // Create the main section with class "content"
      pugi::xml_node main = body.append_child("div");
      main.append_attribute("class") = "content";
      main.text().set(model.main_content.c_str());
  
      // Create the user prompt section with class "user-prompt"
      pugi::xml_node prompt = body.append_child("div");
      prompt.append_attribute("class") = "user-prompt";
      // Add a label element for the prompt text
      pugi::xml_node label = prompt.append_child("label");
      label.text().set((">" + model.user_input).c_str());
  
      // Make prompt 'html-correct' (even though render does not care for now)
      pugi::xml_node input = prompt.append_child("input");
      input.append_attribute("type") = "text";
      input.append_attribute("id") = "command";
      input.append_attribute("name") = "command";
  
      ui.event_handlers["OnKey"] = onKey;
      return ui;
    }
  
    // ----------------------------------
    // End: init,update,view
    // ----------------------------------
  
    int main(int argc, char *argv[]) {
      Runtime<Model,Msg,Cmd> app(init,view,update);
      return app.run(argc,argv);
    }
  } // namespace first
  

int main(int argc, char *argv[]) {

    if (true) {
        // conan template generated code
        stratoceph();
        std::vector<std::string> vec;
        vec.push_back("test_package");
        stratoceph_print_vector(vec);
    }

    // Hack - code to refactor into client an stratoceph lib code
    return first::main(argc,argv);
}
