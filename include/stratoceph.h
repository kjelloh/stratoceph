#pragma once

#include <vector>
#include <string>


#ifdef _WIN32
  #define STRATOCEPH_EXPORT __declspec(dllexport)
#else
  #define STRATOCEPH_EXPORT
#endif

STRATOCEPH_EXPORT void stratoceph();
STRATOCEPH_EXPORT void stratoceph_print_vector(const std::vector<std::string> &strings);
