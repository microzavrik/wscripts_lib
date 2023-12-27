#pragma once

#include <atlsafe.h>
#include <Windows.h>
#include <atlimage.h>

#include <DXGI.h>
#include <D3D11.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include <cstdint>
#include <utility>
#include <vector>
#include <fstream>
#include <string>

#if 0
#include "utils.h"
#endif

struct adapter_info
{
	WCHAR* videocard_description;
	SIZE_T dedicated_video_memory;
	SIZE_T system_video_memory;
	SIZE_T shared_video_memory;
};

struct cpuid_regs
{
	DWORD Eax;
	DWORD Ebx;
	DWORD Ecx;
	DWORD Edx;
};

class wscript
{
public:
	float get_cpu_load();
	DWORDLONG get_current_used_physical_memory();
	VOID take_screen_shot(const std::string& path);
	std::string get_cpu_vendor_string();
private:
	static float calculate_cpu_load(uint64_t idle_ticks, uint64_t total_ticks);
	static uint64_t file_time_to_int64(const FILETIME& ft);
	VOID get_system_info();
	std::pair<float, float> get_ram_info();
	VOID get_videocard_info();
	std::string split_into_chars(DWORD value);
	std::string get_cpu_vendor_substring(DWORD Eax);
private:
	float total_physmemory_in_gb;
	float total_physmemory_in_mb;
	adapter_info video_info;
};