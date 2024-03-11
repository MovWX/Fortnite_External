#pragma once
#include <cstdint>
#include <string>

#define code_control CTL_CODE(FILE_DEVICE_UNKNOWN, 0x269, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_image CTL_CODE(FILE_DEVICE_UNKNOWN, 0xfee, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_security 0xcabd5e5
#include <basetsd.h>
#include <Windows.h>

typedef struct t_virtual {
	INT32 security_code;
	INT32 process_id;
	INT32 virtual_mode;
	ULONGLONG address;
	ULONGLONG buffer;
	ULONGLONG size;
} e_virtual, * s_virtual;
typedef struct t_image {
	INT32 security_code;
	INT32 process_id;
	ULONGLONG* address;
} e_image, * s_image;

class driver_c {
public:
	uintptr_t process_id;
	uintptr_t base_address;


	bool get_driver_handle();
	void read_memory(ULONGLONG address, PVOID buffer, DWORD size);
	void read_memory1(void* address, void* buffer, const std::size_t size);
	void write_memory(PVOID address, PVOID buffer, DWORD size);
	uintptr_t find_image();
private:
	HANDLE driver_handle;
};
inline driver_c* driver = new driver_c;

template <typename T>
T read(uint64_t address) {
	T buffer{ };
	driver->read_memory(address, &buffer, sizeof(T));
	return buffer;
}

template<typename T>
void write(uint64_t address, T buffer) {
	driver->write_memory((PVOID)address, &buffer, sizeof(T));
}
inline std::string read_string(uintptr_t Address, void* Buffer, SIZE_T Size)
{
	driver->read_memory(Address, Buffer, Size);
	char name[255] = { 0 };
	memcpy(&name, Buffer, Size);
	return std::string(name);
}
inline void global_read_memory(uintptr_t Address, void* Buffer, SIZE_T Size)
{
	driver->read_memory(Address, Buffer, Size);
}