#pragma once

#include <string>

using status_t = unsigned long;
constexpr status_t PROCESS_NOT_RUN = 0xFFFF;
constexpr status_t STATUS_OK = 0;

status_t executeAndWait(const std::string& file, const std::string& args, const std::string& workingDir);

std::string getCurrentDirectory();

std::string getWindowsDirectory();
std::string getTempDirectory();

std::string getThisPath();

bool isX64();

std::string joinPath(const std::string& a, const std::string& b);

std::string quote(const std::string& s);
std::string removeQuotes(const std::string& s);
