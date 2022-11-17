#include "kernel.h"

void* kernel_control_function( )
{
	HMODULE hModule = LoadLibrary( _xorstr( "win32u.dll" ) );

	if ( !hModule )
		return nullptr;

	return reinterpret_cast<void*>( GetProcAddress( hModule, _xorstr( "NtTokenManagerCreateFlipObjectReturnTokenHandle" ) ) );
}

uint64_t read_kernel( void* control_function, uint64_t address, void* buffer, std::size_t size )
{
	return call_driver_control( control_function, ID_READ_KERNEL_MEMORY, address, buffer, size );
}
