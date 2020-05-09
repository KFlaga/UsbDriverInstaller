#pragma once

#include "Device.hpp"
#include <string>
#include <vector>

struct DeviceConfig
{
	std::string vendor;
	std::vector<Device> devices;
};

DeviceConfig parse(const std::string& filePath);

struct InputParams
{
	std::string tempFilesPath = ".";
	bool deleteTempFiles = true;
	bool forceDriverInstall = false;

	bool showHelp = false;
	bool error = false;
};

InputParams parseOptions(int argc, char* argv[]);
