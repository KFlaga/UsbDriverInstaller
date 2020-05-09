#include "Device.hpp"
#include "Shell.hpp"
#include <sstream>

std::unique_ptr<char[]> strToArr(const std::string& s)
{
	auto ptr = std::make_unique<char[]>(s.size() + 1);
	std::copy(s.begin(), s.end(), ptr.get());
	ptr[s.size()] = 0;
	return std::move(ptr);
}

Device::Device(unsigned short vid, unsigned short pid, unsigned short rev, const std::string& name, const std::string& desc) :
	vid{ vid },
	pid{ pid },
	rev{ rev },
	deviceName{ name },
	description{ desc }
{
	wdiDesc = strToArr(desc);
	wdiDevice.desc = wdiDesc.get();
	wdiDevice.vid = vid;
	wdiDevice.pid = pid;

	wdiDevice.device_id = 0;
	wdiDevice.compatible_id = 0;
	wdiDevice.mi = 0;
	wdiDevice.is_composite = false;
	wdiDevice.driver = 0;
}

std::string getInfName(const Device& d)
{
	std::string infName = d.deviceName;
	// Replace spaces with underscores
	for (int i = 0; i < infName.size(); ++i)
	{
		if (infName[i] == ' ')
		{
			infName[i] = '_';
		}
	}
	return infName + ".inf";
}

std::string getDriverDir(const Device& d, const std::string& root)
{
	return joinPath(root, d.deviceName);
}

std::string getHardwareId(const Device& d, bool addRev)
{
	std::stringstream ss{};
	ss << "USB\\VID_" << std::hex << d.vid << "&PID_" << d.pid;
	if (addRev)
	{
		ss << "&REV_" << d.rev;
	}
	return ss.str();
}
