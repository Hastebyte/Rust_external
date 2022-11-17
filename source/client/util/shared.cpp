#include "shared.h"

unsigned long nt_key_state( int key )
{
	typedef DWORD( NTAPI* tNtUserGetAsyncKeyState )( DWORD key );
	tNtUserGetAsyncKeyState lpNtUsernt_key_state = reinterpret_cast<tNtUserGetAsyncKeyState>( GetProcAddress( GetModuleHandle( _xorstr( "win32u.dll" ) ), _xorstr( "NtUserGetAsyncKeyState" ) ) );

	if ( !lpNtUsernt_key_state )
		return 0;

	return lpNtUsernt_key_state( key );
}

unsigned long get_process_id( const char* lpExeName )
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

	if ( hSnapShot == INVALID_HANDLE_VALUE )
		return NULL;

	PROCESSENTRY32 pe = {0};
	pe.dwSize = sizeof( PROCESSENTRY32 );

	for ( BOOL	success = Process32First( hSnapShot, &pe );
		success == TRUE;
		success = Process32Next( hSnapShot, &pe ) )
	{
		if ( strcmp( lpExeName, pe.szExeFile ) == 0 )
		{
			CloseHandle( hSnapShot );
			return pe.th32ProcessID;
		}
	}

	CloseHandle( hSnapShot );
	return NULL;
}

HWND get_process_window( unsigned long pid )
{
	std::pair<HWND, unsigned long> params = {0, pid};

	BOOL bResult = EnumWindows( [] ( HWND hwnd, LPARAM lParam ) -> BOOL
	{
		auto pParams = ( std::pair<HWND, unsigned long>* )( lParam );

		unsigned long processId;

		if ( GetWindowThreadProcessId( hwnd, &processId ) && processId == pParams->second )
		{
			SetLastError( (unsigned long)-1 );
			pParams->first = hwnd;
			return FALSE;
		}

		return TRUE;

	}, (LPARAM)&params );

	if ( !bResult && GetLastError( ) == -1 && params.first )
		return params.first;

	return NULL;
}

std::string get_key_name( unsigned int virtual_key )
{
	char buffer[1024] = {0};

	unsigned long scan_code = MapVirtualKey( virtual_key, MAPVK_VK_TO_VSC );
	unsigned long param = ( scan_code << 16 );

	int result = GetKeyNameText( param, buffer, 1024 );

	if ( !result )
		return std::string( );

	return buffer;
}

const std::string get_directory_file( const std::string& file )
{
	char buffer[MAX_PATH] = {0};
	GetModuleFileName( NULL, buffer, MAX_PATH );
	std::string::size_type pos = std::string( buffer ).find_last_of( "\\/" );

	return std::string( buffer ).substr( 0, pos ) + "\\" + file;
}

bool file_exists( const std::string& path )
{
	return ( GetFileAttributes( path.c_str( ) ) != 0xFFFFFFFF );
}

const std::string random_string( size_t length )
{
	auto randchar = [] ( ) -> char
	{
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = ( sizeof( charset ) - 1 );
		return charset[rand( ) % max_index];
	};
	std::string str( length, 0 );
	std::generate_n( str.begin( ), length, randchar );
	return str;
}

std::string& ltrim( std::string& s )
{
	s.erase( s.begin( ), std::find_if( s.begin( ), s.end( ), [] ( int c ) { return !std::isspace( c ); } ) );
	return s;
}

std::string& rtrim( std::string& s )
{
	s.erase( std::find_if( s.rbegin( ), s.rend( ), [] ( int c ) { return !std::isspace( c ); } ).base( ), s.end( ) );
	return s;
}

std::string& trim( std::string & s )
{
	return ltrim( rtrim( s ) );
}