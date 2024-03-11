#include "driver.h"
#include <handleapi.h>
#include <fileapi.h>
#include <ioapiset.h>


bool driver_c::get_driver_handle() {
	driver_handle = CreateFile("\\\\.\\\pymodule", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (!driver_handle || (driver_handle == INVALID_HANDLE_VALUE))
		return false;

	return true;
}
void driver_c::read_memory(ULONGLONG address, PVOID buffer, DWORD size) {
	t_virtual arguments = { 0 };

	arguments.security_code = code_security;
	arguments.address = (ULONGLONG)address;
	arguments.buffer = (ULONGLONG)buffer;
	arguments.size = size;
	arguments.process_id = process_id;
	arguments.virtual_mode = 0xfc6;

	DeviceIoControl(driver_handle, code_control, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}
void driver_c::read_memory1(void* address, void* buffer, const std::size_t size) {
	t_virtual arguments = { 0 };

	arguments.security_code = code_security;
	arguments.address = (ULONGLONG)address;
	arguments.buffer = (ULONGLONG)buffer;
	arguments.size = size;
	arguments.process_id = process_id;
	arguments.virtual_mode = 0xfc6;

	DeviceIoControl(driver_handle, code_control, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}
void driver_c::write_memory(PVOID address, PVOID buffer, DWORD size) {
	t_virtual arguments = { 0 };

	arguments.security_code = code_security;
	arguments.address = (ULONGLONG)address;
	arguments.buffer = (ULONGLONG)buffer;
	arguments.size = size;
	arguments.process_id = process_id;
	arguments.virtual_mode = 0x359;

	DeviceIoControl(driver_handle, code_control, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}
uintptr_t driver_c::find_image() {
	uintptr_t image_address = { NULL };
	t_image arguments = { NULL };

	arguments.security_code = code_security;
	arguments.process_id = process_id;
	arguments.address = (ULONGLONG*)&image_address;

	DeviceIoControl(driver_handle, code_image, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);

	return image_address;
}
