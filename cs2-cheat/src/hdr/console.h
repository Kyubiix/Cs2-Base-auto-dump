#pragma once

#include <windows.h>
#include <vector>

FILE* CreateConsole()
{
	FILE* fp;

	AllocConsole();
	freopen_s(&fp, "CONOUT$", "w", stdout);

	HWND consoleWindow = GetConsoleWindow();
	if (consoleWindow != nullptr)
	{
		DWORD consoleProcessId;
		GetWindowThreadProcessId(consoleWindow, &consoleProcessId);
		if (consoleProcessId != GetCurrentProcessId())
		{
			FreeConsole();
			AttachConsole(consoleProcessId);
			freopen_s(&fp, "CONOUT$", "w", stdout);
		}
	}
	return fp;
}