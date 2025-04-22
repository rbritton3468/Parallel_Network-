#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

// Runs a executable and get its standard output which is stored in a string and returned

std::string runExecutableAndCaptureOutput(const std::string& command);

#endif