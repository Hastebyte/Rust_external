#include "Headers.h"
#include "Process.h"

Process* process = new Process();

Process::Process() {
	
}

Process::~Process() {
}

#ifdef _AMD64_
#define _PTR_MAX_VALUE ((uint64_t)0x000F000000000000)
#else
#define _PTR_MAX_VALUE ((DWORD)0xFFE00000)
#endif

bool Process::IsValid(uint64_t Ptr)
{
	return (Ptr >= (uint64_t)0x10000) && (Ptr < _PTR_MAX_VALUE);
}

bool Process::Read(uint64_t dwAddress, LPVOID lpBuffer, uint64_t dwSize) {

	if (IsValid(dwAddress) && !IsBadReadPtr((LPVOID)dwAddress, dwSize) && !IsBadReadPtr((LPVOID)lpBuffer, dwSize))
	{
		memcpy_s(lpBuffer, dwSize, (LPVOID)dwAddress, dwSize);

		return true;
	}

	return false;
}

bool Process::Write(uint64_t dwAddress, LPCVOID lpBuffer, uint64_t dwSize) {
	if (IsValid(dwAddress) && !IsBadReadPtr((LPVOID)dwAddress, dwSize) && !IsBadReadPtr((LPVOID)lpBuffer, dwSize))
	{
		memcpy_s((LPVOID)dwAddress, dwSize, lpBuffer, dwSize);

		return true;
	}

	return false;
}
