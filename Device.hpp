#pragma once

#define no_init_all deprecated
#include <libwdi/libwdi.h>
#include <string>
#include <memory>

struct Device
{
	unsigned short vid;
	unsigned short pid;
	unsigned short rev;
	std::string deviceName;
	std::string description;

	wdi_device_info wdiDevice;

	Device(unsigned short vid,
		   unsigned short pid,
		   unsigned short rev,
		   const std::string& name,
		   const std::string& desc);

private:
	std::unique_ptr<char[]> wdiDesc;
};

std::string getInfName(const Device& d);

std::string getDriverDir(const Device& d, const std::string& root);

std::string getHardwareId(const Device& d, bool addRev = true);

std::unique_ptr<char[]> strToArr(const std::string& s);
