#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <stdio.h>

#include <string>
#include <utility>

#include <random>
#include <cctype>
#include <locale>

#include <functional>
#include <algorithm>

#include "../util/xorstr.h"

unsigned long nt_key_state( int key );
unsigned long get_process_id( const char* lpExeName );
HWND get_process_window( unsigned long pid );
std::string get_key_name( unsigned int virtual_key );

bool file_exists( const std::string& path );
const std::string get_directory_file( const std::string& file );
const std::string random_string( size_t length );

std::string& ltrim( std::string& s );
std::string& rtrim( std::string& s );
std::string& trim ( std::string& s );