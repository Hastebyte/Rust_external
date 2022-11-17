#pragma once

#include <windows.h>
#include <cstdint>
#include <cstdlib>

#include "..\util\shared.h"
#include "..\util\xorstr.h"

enum E_COMMAND_CODE
{
	ID_NULL					= 0,	//
	ID_READ_PROCESS_MEMORY	= 5,	// 
	ID_READ_KERNEL_MEMORY	= 6,	// 
	ID_WRITE_PROCESS_MEMORY = 7,	//
	ID_GET_PROCESS			= 10,	//
	ID_GET_PROCESS_BASE		= 11,	//
	ID_GET_PROCESS_MODULE	= 12,	//
	ID_GET_WND_THREAD		= 15,	//
	ID_SET_WND_THREAD		= 16,	//
	ID_GET_DRIVER_BASE		= 20,	//
	ID_REMOVE_CACHE_IMAGE	= 25,	//
	ID_CHANGE_HDD_SERIALS	= 100,	//
	ID_DISABLE_SMART_ID		= 101,	//
};

typedef struct _MEMORY_STRUCT
{
	uint64_t	process_id;
	void*		address;
	uint64_t	size;
	uint64_t	size_copied;
	void*		buffer;
	uint64_t	struct_value;
} MEMORY_STRUCT, *PMEMORY_STRUCT;

template<typename ... A>
uint64_t call_driver_control( void* control_function, const A ... arguments )
{
	if ( !control_function )
		return 0;

	using tFunction = uint64_t( __stdcall* )( A... );
	const auto control = static_cast<tFunction>( control_function );

	return control( arguments ... );
}

template<typename T>
T read_virtual_memory( void* ctrl, uint64_t pid, uint64_t address )
{
	T buffer = NULL;

	if ( !pid )
		return buffer;

	MEMORY_STRUCT memory_struct	= {0};
	memory_struct.process_id	= pid;
	memory_struct.address		= reinterpret_cast<void*>( address );
	memory_struct.size			= sizeof( T );
	memory_struct.buffer		= &buffer;

	NTSTATUS result
		= ( NTSTATUS )( call_driver_control( ctrl, ID_READ_PROCESS_MEMORY, &memory_struct ) );

	if ( result != 0 )
		return NULL;

	return buffer;
}

template<typename T>
bool write_virtual_memory( void* ctrl, uint64_t pid, uint64_t address, T buffer )
{
	MEMORY_STRUCT memory_struct	= {0};
	memory_struct.process_id	= pid;
	memory_struct.address		= reinterpret_cast<void*>( address );
	memory_struct.size			= sizeof( T );
	memory_struct.buffer		= &buffer;

	NTSTATUS result
		= ( NTSTATUS )( call_driver_control( ctrl, ID_WRITE_PROCESS_MEMORY, &memory_struct ) );

	if ( result != 0 )
		return false;

	return true;
}

void* kernel_control_function( );
uint64_t read_kernel( void* control_function, uint64_t address, void* buffer, std::size_t size );