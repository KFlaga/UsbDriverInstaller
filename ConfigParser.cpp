#include "ConfigParser.hpp"
#include "Shell.hpp"
#include <Windows.h>
#include "deps/json.hpp"
#include "deps/cxxopts.hpp"
#include <fstream>
#include <streambuf>

DeviceConfig parse(const std::string& filePath)
{
	std::ifstream file{ filePath };
	if (not file.is_open())
	{
		throw std::runtime_error("Couldn't open device list file " + filePath);
	}

	DeviceConfig config;

	nlohmann::json json;
	file >> json;

	config.vendor = json["vendor"].get<std::string>();
	auto devices = json["devices"];

	for (auto& device : devices)
	{
		config.devices.emplace_back(
			device["pid"].get<int>(),
			device["vid"].get<int>(),
			device["rev"].get<int>(),
			device["name"].get<std::string>(),
			device["desc"].get<std::string>()
		);
	}

	return config;
}

std::string getTargetDirectory(const std::string& param)
{
	if (param == ".")
	{
		return getCurrentDirectory();
	}
	else if (param.empty() || param == "<TEMP>")
	{
		return getTempDirectory();
	}
	else
	{
		return param;
	}
}

InputParams parseOptions(int argc, char* argv[])
{
	cxxopts::Options options("UsbDriverInstaller",
		"Installs libusb driver using libwdi to generate and self-sign dirver, pnputil to install it and finishes with install-filter. Must be run with administartor privileges.");

	options.add_options()
		("d, dir", "Directory where driver files will be created", cxxopts::value<std::string>()->default_value("<TEMP>"))
		("n, no_remove", "Leave created driver files after install", cxxopts::value<bool>()->default_value("false"))
		("f, force", "Install driver even when it is found to be installed", cxxopts::value<bool>()->default_value("false"))
		("h,help", "Print usage")
	;

	InputParams input{};

	try
	{
		auto result = options.parse(argc, argv);
		if (result.count("help"))
		{
			std::cout << options.help() << std::endl;
			input.showHelp = true;
		}
		else
		{
			input.tempFilesPath = getTargetDirectory(result["dir"].as<std::string>());
			input.deleteTempFiles = !result["no_remove"].as<bool>();
			input.forceDriverInstall = result["force"].as<bool>();
		}
	}
	catch (const cxxopts::OptionParseException& ex)
	{
		std::cout << "Invalid input parameters. Details: " << ex.what() << std::endl;
		input.error = true;
	}

	return input;
}
