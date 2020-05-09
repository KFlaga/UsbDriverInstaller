#pragma once

#include <string>

struct Device;

std::string createPnputilFinder(const std::string& driverDir);

void runPnputilFinder(const std::string& scriptFile, const std::string& driverDir);

std::string findPnputilPath(const std::string& driverDir);

bool checkDeviceIsInstalled(const Device& device, const std::string& pnputilPath, const std::string& driverDir);

void installInf(const std::string& pnputilPath, const std::string& fullInfPath);
