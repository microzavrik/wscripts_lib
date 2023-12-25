#include "wscript.h"

float wscript::calculate_cpu_load(uint64_t idle_ticks, uint64_t total_ticks)
{
	static uint64_t _previous_total_ticks = 0;
	static uint64_t _previous_idle_ticks = 0;

	uint64_t total_ticks_since_last_time = total_ticks - _previous_total_ticks;
	uint64_t idle_ticks_since_last_time = idle_ticks - _previous_idle_ticks;

	float ret = 1.0f - ((total_ticks_since_last_time > 0) ?
		((float)idle_ticks_since_last_time) / total_ticks_since_last_time : 0);

	_previous_total_ticks = total_ticks;
	_previous_idle_ticks = idle_ticks;

	return ret;
}

uint64_t wscript::file_time_to_int64(const FILETIME& ft)
{
	return (((uint64_t)(ft.dwHighDateTime)) << 32) | ((uint64_t)ft.dwLowDateTime);
}

float wscript::get_cpu_load()
{
	FILETIME idle_time, kernel_time, user_time;
	return GetSystemTimes(&idle_time, &kernel_time, &user_time)
		? calculate_cpu_load(file_time_to_int64(idle_time), file_time_to_int64(kernel_time)
			+ file_time_to_int64(user_time)) : -1.0f;
}

VOID wscript::get_system_info()
{
	auto [mem_gb, mem_mb] = get_ram_info();
	total_physmemory_in_gb = mem_gb;
	total_physmemory_in_mb = mem_mb;
}

[[nodiscard]] std::pair<float, float> wscript::get_ram_info()
{
	MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&mem_info);
	DWORDLONG total_phys_memory = mem_info.ullTotalPhys;

	float total_phys_memory_in_gb = static_cast<float>(total_phys_memory) / (1024 * 1024 * 1024);
	float total_phys_memory_in_mb = static_cast<float>(total_phys_memory) / (1024 * 1024);

	return std::make_pair(total_phys_memory_in_gb, total_phys_memory_in_mb);
}

VOID wscript::get_videocard_info()
{
	CoInitialize(nullptr);

	IDXGIFactory* p_factory = nullptr;
	CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&p_factory));

	IDXGIAdapter* p_adapter = nullptr;
	p_factory->EnumAdapters(0, &p_adapter);

	DXGI_ADAPTER_DESC adapter_desciption;
	p_adapter->GetDesc(&adapter_desciption);

	video_info.videocard_description = adapter_desciption.Description;
	video_info.dedicated_video_memory = adapter_desciption.DedicatedVideoMemory / (1024 * 1024);
	video_info.system_video_memory = adapter_desciption.DedicatedSystemMemory / (1024 * 1024);
	video_info.shared_video_memory = adapter_desciption.SharedSystemMemory / (1024 * 1024);

	p_adapter->Release();
	p_factory->Release();
}

DWORDLONG wscript::get_current_used_physical_memory()
{
	static MEMORYSTATUSEX mem_info;
	mem_info.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&mem_info);
	DWORDLONG total_phys_memory = mem_info.ullTotalPhys - mem_info.ullAvailPhys;

	return total_phys_memory / (1024 * 1024);
}

VOID wscript::take_screen_shot(const std::string& path)
{
	keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY, 0);
	keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

	HBITMAP h_bitmap;
	Sleep(500);
	OpenClipboard(nullptr);
	h_bitmap = (HBITMAP)GetClipboardData(CF_BITMAP);
	CloseClipboard();

	std::vector<BYTE> buf;
	IStream* stream = NULL;
	HRESULT hr = CreateStreamOnHGlobal(0, TRUE, &stream);
	CImage image;
	ULARGE_INTEGER li_size;

	image.Attach(h_bitmap);
	image.Save(stream, Gdiplus::ImageFormatJPEG);
	IStream_Size(stream, &li_size);
	DWORD len = li_size.LowPart;
	IStream_Reset(stream);
	buf.resize(len);
	IStream_Read(stream, &buf[0], len);
	stream->Release();
	
	std::fstream fi;
	fi.open(path, std::fstream::binary | std::fstream::out);
	fi.write(reinterpret_cast<const char*>(&buf[0]), buf.size() * sizeof(BYTE));
	fi.close();
}