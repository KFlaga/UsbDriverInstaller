#include "FindPnputil.hpp"
#include "Device.hpp"
#include "Shell.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std::string_literals;

const char* dirsFile = "dirs.txt";
const char* enumDevicesFile = "devices.txt";

std::string createPnputilFinder(const std::string& driverDir)
{
	auto scriptFile = "find_pnputil.bat";
	auto fullPath = joinPath(driverDir, scriptFile);

	const char* scriptContent = R"(
@echo off
SET my_dir="%~dp0"

echo Running find_pnputil.bat
cd /D %SystemDrive%
cd %SystemDrive%\Windows
dir pnputil.exe /S > "%my_dir%dirs.txt"
)";

	std::ofstream file{ fullPath, std::ios::out | std::ios::trunc };
	file << scriptContent;
	file.flush();
	file.close();

	return scriptFile;
}

std::string createDriversEnumerator(const std::string& driverDir, const std::string& pnputilPath)
{
	auto scriptFile = "enum_drivers.bat";
	auto fullPath = joinPath(driverDir, scriptFile);

	std::string pnputilCommand = joinPath(pnputilPath, "pnputil.exe");
	std::string scriptContent = R"(
@echo off
SET my_dir="%~dp0"
SET pnputil_path=")"s + pnputilCommand + R"("

echo Running enum_drivers.bat
%pnputil_path% -e > "%my_dir%)"s + enumDevicesFile + R"("
)"s;

	std::ofstream file{ fullPath, std::ios::out | std::ios::trunc };
	file << scriptContent;
	file.flush();
	file.close();

	return scriptFile;
}

void runPnputilFinder(const std::string& scriptFile, const std::string& driverDir)
{
	std::cout << "Finding pnputil.exe (it may take a while)" << std::endl;
	auto status = executeAndWait(scriptFile, "", driverDir);
	if (status != STATUS_OK)
	{
		throw std::runtime_error("Execution of script "s + scriptFile + " was unsuccessfull"s);
	}
}

std::string findPnputilPath(const std::string& driverDir)
{
	std::ifstream dirs{ joinPath(driverDir, dirsFile) };
	if (dirs.is_open())
	{
		std::cout << "Results of pnputli search:" << std::endl;
		std::vector<std::string> candidates{};
		while (true)
		{
			std::string line;
			std::getline(dirs, line);
			if (dirs.fail())
				break;

			std::cout << line << std::endl;

			auto offset = line.find("Windows");
			if (offset != std::string::npos)
			{
				offset -= 3; // For C:/
				candidates.push_back(line.substr(offset));
			}
		}
		dirs.close();

		auto pnputilPath = std::find_if(candidates.begin(), candidates.end(),
			[](const std::string& p) { return p.find("System32") == std::string::npos; });
		if (pnputilPath != candidates.end())
		{
			std::cout << "Found pnputil : " << *pnputilPath << "\\" << "pnputil.exe" << std::endl;
			return *pnputilPath;
		}
		else
		{
			throw std::runtime_error("Path to pnputil not found in "s + dirsFile);
		}
	}
	else
	{
		throw std::runtime_error("Cannot open "s + dirsFile + " - it was supposed to be prepared by previous step"s);
	}
}

bool checkDeviceIsInstalled(const Device& device, const std::string& pnputilPath, const std::string& driverDir)
{
	std::string deviceId = getHardwareId(device, false);
	std::string enumeratorScript = createDriversEnumerator(driverDir, pnputilPath);
	auto status = executeAndWait(enumeratorScript, "", driverDir);
	if (status == STATUS_OK)
	{
		std::string targetPath = joinPath(driverDir, enumDevicesFile);
		std::ifstream devices{ targetPath };
		if (devices.is_open())
		{
			while (true)
			{
				std::string line;
				std::getline(devices, line);
				if (devices.fail())
					break;

				if (line.find(deviceId) != std::string::npos)
				{
					std::cout << "Driver for device " << deviceId << " is already installed." << std::endl;
					devices.close();
					return true;
				}
			}
			devices.close();
		}
	}
	std::cout << "Driver for device " << deviceId << " wasn't found." << std::endl;
	return false;
}

void installInf(const std::string& pnputilPath, const std::string& fullInfPath)
{
	std::string args = " -a " + quote(fullInfPath);
	auto status = executeAndWait("pnputil.exe", args, pnputilPath);
	if (status != STATUS_OK)
	{
		throw std::runtime_error("Installing a driver with pnputil failed");
	}
}
