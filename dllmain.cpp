#include "stdafx.h"
#include "stdio.h"
#include "..\includes\injector\injector.hpp"
#include "..\includes\IniReader.h"
#include <windows.h>
#include <cstdint>
#include <d3d9.h>
#include <sstream>
#include <vector>
#include <iostream>
#include <string>

DWORD WINAPI Thing(LPVOID);

bool IsRacing, Tranny, CarMatch, ShouldRun = true;
int ThreadDelay;
char* IntroMovieName, * PlayerName, CurrentCar, * CCar;
std::string CurCar;
std::string CarInput;
std::vector<std::string> carList;


unsigned int* GetPtr(unsigned int* offsets, int count)
{
	auto ptr = (unsigned int*)offsets[0];
	for (int i = 1, size = count; i < size; i++)
	{
		if (ptr == NULL || *ptr == 0)
		{
			return 0;
		}

		ptr = (unsigned int*)(*ptr + offsets[i]);
	}

	return ptr;
}

std::string GetCarName()
{
	unsigned int offsets[] = { 0x00ACE110, 0x54, 0x20, 0x14, 0x28, 0x40, 0x3F4, 0x40 };
	auto ptr = (char*)GetPtr(offsets, 8);
	if (ptr)
	{
		return ptr;
	}
	return std::string();
}


void SplitString(std::string stringIn)
{
	std::stringstream ss(stringIn);
	std::string tmp;
	while (std::getline(ss, tmp, ','))
	{
		carList.push_back(tmp);
	}
}

void Init()
{
	// Read values from .ini
	CIniReader iniReader("ProStreetGearWhine.ini");
	CarInput = iniReader.ReadString("Gameplay", "CarList", CarInput);
	Tranny = iniReader.ReadInteger("Gameplay", "Tranny", Tranny) == 0;
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&Thing, NULL, 0, NULL);
	SplitString(CarInput);

	if (CarInput == std::string("ALL"))
	{
		injector::WriteMemory<unsigned char>(0xAB09B8, 1, true);
		ShouldRun = false;
	}
}

bool ValidateCar()
{
	for (auto it = carList.begin(); it != carList.end(); ++it)
	{
		if (CCar == (*it))
		{
			CarMatch = true;
		}
	}
	return CarMatch;
}

DWORD WINAPI Thing(LPVOID)
{
	while (ShouldRun)
	{
		Sleep(ThreadDelay);

		IsRacing = *(bool*)0xAACF5F;

		if (!IsRacing)
		{
			CarMatch = false;
			injector::WriteMemory<unsigned char>(0xAB09B8, 0, true);
			Sleep(5000);
		}

		if (IsRacing)
		{
			CIniReader iniReader("ProStreetGearWhine.ini");
			CurCar = GetCarName();
			CCar = &CurCar[0];
		}

		if ((IsRacing == true) && ValidateCar() == true)
		{
			injector::WriteMemory<unsigned char>(0xAB09B8, 1, true);
		}
	}
	return 0;
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);

		if (strstr((const char*)(base + (0xA49742 - base)), "ProStreet08Release.exe"))
			Init();

		else
		{
			MessageBoxA(NULL, "This .exe is not supported.\nPlease use a NOCD v1.1 NFS.exe.", "ProStreetWhineMod", MB_ICONERROR);
			return FALSE;
		}
	}
	return TRUE;

}

