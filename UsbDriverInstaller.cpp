#include "Shell.hpp"
#include "FindPnputil.hpp"
#include "Device.hpp"
#include "ConfigParser.hpp"
#include "deps/cxxopts.hpp"

#include <iostream>
#include <vector>
#include <fstream>

#include <filesystem>

#include <ShlObj.h>

using namespace std::string_literals;

std::string getTargetDirectory(const InputParams& opts)
{
	if (opts.tempFilesPath.empty() || opts.tempFilesPath == ".")
	{
		return getCurrentDirectory();
	}
	else
	{
		return opts.tempFilesPath;
	}
}

int prepareDriver(Device& device, const std::string& infName, const std::string& driverDir, const std::string& vendor)
{
	auto infNameArr = strToArr(infName);
	auto driverDirArr = strToArr(driverDir);
	auto vendorArr = strToArr(vendor);

	wdi_options_prepare_driver driverOptions{};
	driverOptions.vendor_name = vendorArr.get();
	driverOptions.driver_type = WDI_LIBUSB0;
	driverOptions.use_wcid_driver = false;

	std::cout << "Preparing driver " << infName << std::endl;
	auto status = wdi_prepare_driver(&device.wdiDevice, driverDirArr.get(), infNameArr.get(), &driverOptions);
	if (status != WDI_SUCCESS)
	{
		throw std::runtime_error("Driver preparation failed");
	}
	return status;
}

void installDeviceFilter(const std::string& driverDir, const std::string& fullInfPath)
{
	bool is64 = isX64();

	std::string filterPath = std::string(is64 ? "amd64" : "x86") + "\\install-filter.exe";
	std::string filterArgs = " install --inf=" + quote(fullInfPath);
	auto status = executeAndWait(filterPath, filterArgs, driverDir);
	if (status != STATUS_OK)
	{
		throw std::runtime_error("Installing a device filter with install-filter failed");
	}
}

void cleanUp(const std::string& dir)
{
	try
	{
		std::filesystem::remove_all(dir);
	}
	catch (std::exception)
	{
		std::cout << "Failed to remove directory " << dir << " - but it don't affect installation." << std::endl;
	}
}

void installDevice(Device& device, const std::string& vendor, const InputParams& opts)
{
	auto rootDir = getTargetDirectory(opts);
	auto driverDir = getDriverDir(device, rootDir);
	auto infName = getInfName(device);
	auto fullInfPath = joinPath(driverDir, infName);

	std::cout << "Create and install driver " << fullInfPath << std::endl;

	prepareDriver(device, infName, driverDir, vendor);

	auto scriptFile = createPnputilFinder(driverDir);
	runPnputilFinder(scriptFile, driverDir);
	auto pnputilPath = findPnputilPath(driverDir);
	if (opts.forceDriverInstall || not checkDeviceIsInstalled(device, pnputilPath, driverDir))
	{
		installInf(pnputilPath, fullInfPath);
	}
	installDeviceFilter(driverDir, fullInfPath);

	if (opts.deleteTempFiles)
	{
		cleanUp(driverDir);
	}

	std::cout << "Device " << device.deviceName << " successfully installed" << std::endl;
}

int main(int argc, char *argv[])
{
	int status = 0;
	auto opts = parseOptions(argc, argv);
	if (opts.showHelp)
	{
		return 0;
	}
	if (opts.error)
	{
		return 3;
	}

	if (not IsUserAnAdmin())
	{
		std::cout << "Must be run with administartor privileges" << std::endl;
		status = 2;
	}
	else
	{
		try
		{
			auto thisPath = getThisPath();
			DeviceConfig config = parse(joinPath(thisPath, "devices.json"));

			for (auto& d : config.devices)
			{
				installDevice(d, config.vendor, opts);
			}
		}
		catch (const std::runtime_error& e)
		{
			std::cout << e.what() << std::endl;
			status = 1;
		}
	}

	std::cout << "Press key to exit." << std::endl;
	std::cin.peek();

	return status;
}
