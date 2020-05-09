#include "Shell.hpp"

#include <ShlObj.h>
#include <Windows.h>
#include <locale>
#include <codecvt>
#include <iostream>
#include <algorithm>

status_t executeAndWait(const std::string& file, const std::string& args, const std::string& workingDir)
{
	SHELLEXECUTEINFOA info;
	info.cbSize = sizeof(SHELLEXECUTEINFOA);
	info.hwnd = nullptr;
	info.lpVerb = "open";
	info.lpFile = file.c_str();
	info.lpParameters = args.empty() ? nullptr : args.c_str();
	info.lpDirectory = workingDir.c_str();
	info.fMask = SEE_MASK_NOASYNC | SEE_MASK_NO_CONSOLE | SEE_MASK_NOCLOSEPROCESS;

	std::cout << "Executing command " << workingDir << "\\" << file << " " << args << std::endl;
	ShellExecuteExA(&info);
	DWORD status = PROCESS_NOT_RUN;
	if (info.hProcess != NULL)
	{
		do
		{
			GetExitCodeProcess(info.hProcess, &status);
		} while (status == STILL_ACTIVE);
		CloseHandle(info.hProcess);
	}
	return (status_t)status;
}

std::string toString(TCHAR* path, DWORD length)
{
	std::wstring wpath( path, length );
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.to_bytes(wpath);
}

std::string getCurrentDirectory()
{
	TCHAR currentDir[MAX_PATH];
	auto length = GetCurrentDirectory(MAX_PATH, currentDir);
	return toString(currentDir, length);
}

std::string getWindowsDirectory()
{
	TCHAR winDir[MAX_PATH];
	auto length = GetWindowsDirectory(winDir, MAX_PATH);
	return toString(winDir, length);
}

bool isX64()
{
	BOOL x;
	if (sizeof(uintptr_t) < 8)
	{
		IsWow64Process(GetCurrentProcess(), &x);
	}
	else
	{
		x = TRUE;
	}
	return x;
}

std::string getThisPath()
{
	TCHAR temp[MAX_PATH];
	auto length = GetModuleFileName(NULL, temp, MAX_PATH);
	auto path = toString(temp, length);
	auto offset = path.find_last_of('\\');
	return path.substr(0, offset);
}

std::string joinPath(const std::string& a, const std::string& b)
{
	if (a.empty())
	{
		return b;
	}
	else if (b.empty())
	{
		return a;
	}
	else if (b.front() == '\\' || b.front() == '/')
	{
		return b;
	}
	else if (a.back() == '\\' || a.back() == '/')
	{
		return a + b;
	}
	else
	{
		return a + '\\' + b;
	}
}

std::string quote(const std::string& s)
{
	return "\"" + s + "\"";
}

std::string removeQuotes(const std::string& s)
{
	int starts = (s.size() > 0 && s.front() == '"') ? 1 : 0;
	int ends = (s.size() > 0 && s.back() == '"') ? 1 : 0;
	return s.substr(starts, s.size() - ends - starts);
}

std::string getTempDirectory()
{
	char buf[MAX_PATH + 1];
	auto size = GetTempPathA(MAX_PATH, buf);
	return std::string(buf, size);
}
