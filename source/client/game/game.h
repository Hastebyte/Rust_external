#pragma once

#include <windows.h>
#include <vector>

#include <mutex>
#include <thread>

#include <d3d9.h>
#include <d3dx9math.h>

#include "offsets.h"
#include "vector.h"

#include "..\core\kernel.h"
#include "..\core\render.h"
#include "..\util\shared.h"

#include "VMProtectSDK.h"

#pragma comment( lib, "d3dx10.lib" )

namespace game
{
	struct player
	{
		uint64_t	m_object;
		uint64_t	m_object_class;
		uint64_t	m_entity;
		uint64_t	m_base_entity;
		uint64_t	m_player_movement;
		vector3		m_world_position;
		int			m_active_team;
		int			m_team;

		//char m_name[18];
		//vector3 m_screen_position;
		//float m_distance;
		//UINT m_health;
	};

	struct gadget
	{
		unsigned long m_type2;
		unsigned long m_type;
		unsigned long m_state;
		float m_distance;
		bool m_visible;
		vector3 m_world_position;
		vector3 m_screen_position;
	};
}

class rust_manager
{
private:
	void* m_driver_control;				// driver control function
	uint64_t m_base;					// game base (as integer for arithmetic)

public:
	unsigned long m_pid;				// process id moved to public
	uint64_t m_unity_base;				// module base for UnityPlayer.dll moved to public
	bool m_renderer_created;			// status of the internal renderer
	composition_renderer* m_renderer;	// our internal renderer

	bool m_aimbot_on;
	bool m_force_day_time_on;

	int m_screen_height;				// screen height
	int m_screen_width;					// screen width

	uint64_t m_camera;					// world to screen
	uint64_t m_sky_dome;				// change time
	uint32_t m_current_item_id;			// used for recoil?

	vector3 m_camera_position;			// aimbot

	std::vector<game::player*> m_players;
	std::vector<game::gadget*> m_gadgets;
	game::player* m_local_player;
	std::mutex mut;

	~rust_manager( ) { }
	 rust_manager( ) { }

	template<typename ... A>
	uint64_t call_driver_control( const A ... arguments )
	{
		if ( !m_driver_control )
			return 0;

		using tFunction = uint64_t( __stdcall* )( A... );
		const auto control = static_cast<tFunction>( m_driver_control );

		return control( arguments ... );
	}

	template<typename T>
	T read( uint64_t address )
	{
		T buffer;
		RtlSecureZeroMemory( &buffer, sizeof( T ) );

		if ( !this->m_pid )
			return buffer;

		MEMORY_STRUCT memory_struct	= { 0 };
		memory_struct.process_id	= this->m_pid;
		memory_struct.address		= reinterpret_cast<void*>( address );
		memory_struct.size			= sizeof( T );
		memory_struct.buffer		= &buffer;

		this->call_driver_control( ID_READ_PROCESS_MEMORY, &memory_struct );
		return buffer;
	}
	
	bool read( uint64_t address, void* buffer, size_t size )
	{
		if ( !this->m_pid )
			return false;

		MEMORY_STRUCT memory_struct	= { 0 };
		memory_struct.process_id	= this->m_pid;
		memory_struct.address		= reinterpret_cast<void*>( address );
		memory_struct.size			= size;
		memory_struct.buffer		= buffer;

		uint64_t result = this->call_driver_control( ID_READ_PROCESS_MEMORY, &memory_struct );

		if ( result != 0 )
			return false;

		return true;
	}

	template<typename T>
	bool write( uint64_t address, T buffer )
	{
		MEMORY_STRUCT memory_struct	= { 0 };
		memory_struct.process_id	= this->m_pid;
		memory_struct.address		= reinterpret_cast<void*>( address );
		memory_struct.size			= sizeof( T );
		memory_struct.buffer		= &buffer;

		uint64_t result = this->call_driver_control( ID_WRITE_PROCESS_MEMORY, &memory_struct );

		if ( result != 0 )
			return false;

		return true;
	}

	bool write( uint64_t address, void* buffer, size_t size )
	{
		MEMORY_STRUCT memory_struct	= { 0 };
		memory_struct.process_id	= this->m_pid;
		memory_struct.address		= reinterpret_cast<void*>( address );
		memory_struct.size			= size;
		memory_struct.buffer		= buffer;

		uint64_t result = this->call_driver_control( ID_WRITE_PROCESS_MEMORY, &memory_struct );

		if ( result != 0 )
			return false;

		return true;
	}

	bool verify_game( );
	uint64_t get_module( const wchar_t* module_name );
	bool validate_class( uint64_t class_instance, uint64_t vtable_offset );
	bool get_objects( );
	bool get_active_objects( );
	void parse_player( uint64_t object );
	void parse_camera( uint64_t object );
	void update_camera_position( );
	void parse_sky( uint64_t object );

	bool get_player_position( uint64_t object_class, vector3& position );
	bool is_visible( uint64_t base_entity );
	float get_health( uint64_t base_entity );
	uint64_t get_entity_bone( uint64_t base_entity, int bone_index );
	float get_vector_by_index( __m128 v, int index );
	vector3 get_bone_pos( uint64_t pTransform );
	bool world_to_screen( D3DXMATRIX& view_matrix, vector3& origin, vector2* out );
	bool inside_aim_radius( vector2& screen_position, float aim_circum );
	bool force_daytime( float hour );


	const uint64_t cActItemID = 0x5A8;
	const uint64_t PlayerInventory = 0x460;
	const uint64_t ItemContainer = 0x28;
	const uint64_t list_item = 0x20;
	const uint64_t item_id = 0x78;
	const uint64_t item_Defintion = 0x10;
	const uint64_t list_item_count = 0x18;
	const uint64_t Phrase = 0x20;
	const uint64_t ItemName = 0x18;
	const uint64_t Name_len = 0x10;
	const uint64_t Name_char = 0x14;

	bool no_recoil( )
	{
		if ( !m_local_player )
		{
			printf( "[!] local player is not valid\n" );
		}

		uint64_t pPlayerInventory = read<uint64_t>( m_local_player->m_base_entity + PlayerInventory );
		
		if ( !pPlayerInventory )
			return false;

		uint64_t pItemContainer = read<uint64_t>( pPlayerInventory + ItemContainer );
		
		if ( !pItemContainer )
			return false;

		uint64_t plist_item = read<uint64_t>( pItemContainer + list_item );
		
		if ( !plist_item )
			return false;
			
		uint64_t plist = read<uint64_t>( plist_item + 0x10 );
		int itemCount = read<int>( plist_item + 0x18 );

		if ( !plist || itemCount <= 0 )
			return false;

		// printf( "[+] item count: %d\n", itemCount );

		for ( int k = 0; k < itemCount; k++ )
		{
			uint64_t a_item = read<uint64_t>( plist + 0x20 + 0x8 * k );

			if ( !a_item )
				continue;

			int uid = read<int>( a_item + 0x78 );

			// printf( "[+] holding uid: %d currentid: %d\n", uid, m_current_item_id );

			//item holding

			if ( uid != 0 && m_current_item_id == uid )	
			{
				uint64_t pItemDefinition = read<int>( a_item + 0x10 );
				if ( pItemDefinition )
				{
					uint64_t pcName = read<uint64_t>( pItemDefinition + 0x20 );

					if ( pcName )
					{
						uint64_t pDisplayName = read<uint64_t>( pcName + 0x18 );

						int nameLen = read<int>( pDisplayName + Name_len );

						wchar_t wname[255] = {NULL};
						if ( read( pDisplayName + Name_char, &wname, sizeof( wname ) ) )
						{
							char aname[255];
							sprintf_s( aname, "%ws", wname );
							// printf( "[+] item: %s\n", aname );
						}
					}

					int dwItemCategory = read<int>( pItemDefinition + 0xBC );

					if ( dwItemCategory == 0 )	//holding a weapon
					{
						uint64_t pProjectile = read<uint64_t>( a_item + 0x58 );

						// printf( "[+] projectile: %llx\n", pProjectile );

						if ( pProjectile )
						{
							uint64_t pRecoil = read<uint64_t>( pProjectile + 0x248 );
							if ( pRecoil )
							{
								if ( true )	//No recoil
								{
									write<float>( pProjectile + 0x290, 0.0f );	//aimSway
									write<float>( pProjectile + 0x294, 0.0f );	//aimSwaySpeed

									write<float>( pRecoil + 0x28, 0.0f );	//recoilYawMin
									write<float>( pRecoil + 0x2C, 0.0f );
									write<float>( pRecoil + 0x30, 0.0f );
									write<float>( pRecoil + 0x34, 0.0f );
									write<float>( pRecoil + 0x3C, 0.0f );
								}
							}
						}
					}
				}
			}
		}

		return true;
	}

	bool no_fall_damage( )
	{
		if ( !m_local_player )
		{
			printf( "[!] invalid local player\n" );
			return false;
		}

		// no fall damage

		write<vector3>( m_local_player->m_player_movement + 0xB4, vector3( 0.0f, 0.0f, 0.0f ) );

		// gravity 1.0 - 2.5

		write<float>( m_local_player->m_player_movement + 0x7C, 1.0 );

		return true;
	}

	void clamp_angles( vector3& angle )
	{
		if ( angle.x > 180 )
			angle.x -= 360;
		else if ( angle.x < -180 )
			angle.x += 360;

		if ( angle.y > 180 )
			angle.y -= 360;
		else if ( angle.y < -180 )
			angle.y += 360;

		if ( angle.x < -74.9f )
			angle.x = -74.9f;
		if ( angle.x > 74.9f )
			angle.x = 74.9f;

		while ( angle.y < -180.0f )
			angle.y += 360.0f;
		while ( angle.y > 180.0f )
			angle.y -= 360.0f;

		angle.z = 0.0f;
	}


	bool aim_at( const vector3& aim_position )
	{
		if ( !m_local_player )
		{
			printf( "[!] invalid local player\n" );
			return false;
		}

		float screen_center_x = m_screen_width  * 0.5f;
		float screen_center_y = m_screen_height * 0.5f;

		vector3 vec_ang;
		vector3 vec_delta = m_camera_position - aim_position;
		float magnitude = vec_delta.Length( );

		vec_ang.x = ( asinf( vec_delta.y / magnitude )  * 180.0f / 3.14159265f );
		vec_ang.y = ( atan2( vec_delta.x, vec_delta.z ) * 180.0f / 3.14159265f ) + 180.0f;
		vec_ang.z = 0.0f;

		// clamp_angles( vec_ang );

		uint64_t player_input = read<uint64_t>( m_local_player->m_base_entity + OFFSET_BASE_PLAYER_INPUT );

		if ( !player_input )
		{
			printf( "[!] player input is not valid\n" );
			return false;
		}

		write<vector3>( player_input + 0x44, vec_ang );
		return true;
	}

	//void draw_corner_box( float x, float y, float w, float h, const D2D1_COLOR_F& color )
	//{
	//	float w  = w;
	//	float h = h;
	//	
	//	// Box: Upper Left Corner 
	//	m_renderer->draw_line( x, y, x, y + ( h / 5 ), color, 2.0f );	// Top To Bottom
	//	m_renderer->draw_line( x, y, x + ( w / 5 ), y, color, 2.0f );	// Left To Right 

	//	//  Box: Upper Right Corner     
	//	m_renderer->draw_line( x + w, y, x + w - ( w / 5 ), y, color, 2.0f );		// Right To Left     
	//	m_renderer->draw_line( x + w, y, x + w, y + ( h / 5 ), color, 2.0f );		// Top To Bottom   

	//	// Box: Bottom Left Corner
	//	m_renderer->draw_line( x, y + h, x + ( w / 5 ), y + h, color, 2.0f );		// Right To Left   
	//	m_renderer->draw_line( x, y + h, x, y + h - ( h / 5 ), color, 2.0f );		// Bottom To Top  
	//	
	//	// Box: Bottom Right Corner
	//	m_renderer->draw_line( x + w, y + h, x + w - ( w / 5 ), y + h, color, 2.0f );	// Right To Left         
	//	m_renderer->draw_line( x + w, y + h, x + w, y + h - ( h / 5 ), color, 2.0f );	// Bottom To Top
	//}

	void draw_corner_box( float x, float y, float w, float h, const D2D1_COLOR_F& color );
	void draw_health_bar( float x, float y, float health );

	bool build_renderer( )
	{
		VMProtectBeginMutation( "build_renderer" );

		// Step 1. get target window

		HWND target_window = get_process_window( m_pid );

		if ( !target_window )
		{
			printf( "[!] window not found\n" );
			return false;
		}

		// Step 2. build local d3d device

		m_renderer = new composition_renderer( );

		if ( !m_renderer->build_swapchain( target_window ) )
		{
			printf( "[!] failed to build swapchain\n" );
			return false;
		}

		//printf( "[+] creating\n" );
		//printf( "     pid:%lx\n", target_pid );
		//printf( "     window:%llx\n", reinterpret_cast<uint64_t>( target_window ) );


		// Step 3. create temporary window and get kernel thread info for both our window and target window

		temporary_window window;
		uint64_t wnd_thread_info_local  = call_driver_control( ID_GET_WND_THREAD, window.handle( ) );
		uint64_t wnd_thread_info_target = call_driver_control( ID_GET_WND_THREAD, target_window );

		// printf( "[+] kernel pointers\n" );
		// printf( "     local:  \t%llx\n", wnd_thread_info_local );
		// printf( "     target: \t%llx\n", wnd_thread_info_target );

		// Step 4. spoof ownership temporarily, change the game's window struct to have our temporary thread info pointer
		// kernel will check this pointer when making the composition surface

		call_driver_control( ID_SET_WND_THREAD, target_window, wnd_thread_info_local );

		printf( "[+] building directx surface\n" );

		if ( !m_renderer->build_composition( target_window ) )
		{
			printf( "[!] failed to start renderer\n" );

			//call_driver_control( ID_SET_WND_THREAD, target_window, wnd_thread_info_target );
			//delete m_renderer;
			//return false;
		}

		 //test for:
		 // 0x88980800 - DCOMPOSITION_ERROR_WINDOW_ALREADY_COMPOSED
		 // 0x88980801 - DCOMPOSITION_ERROR_SURFACE_BEING_RENDERED
		 // 0x88980802 - DCOMPOSITION_ERROR_SURFACE_NOT_BEING_RENDERED

		// Step 5. restore owner

		call_driver_control( ID_SET_WND_THREAD, target_window, wnd_thread_info_target );

		VMProtectEnd( );
		return true;
	};

	void destroy_renderer( )
	{	
		VMProtectBeginMutation( "destroy_renderer" );

		// Step 1. get target window

		HWND target_window = get_process_window( m_pid );

		if ( !target_window )
		{
			printf( "[!] window not found\n" );
			return;
		}

		// Step 2. create temporary window and get kernel thread info for both our window and target window

		temporary_window window;
		uint64_t wnd_thread_info_local  = call_driver_control( ID_GET_WND_THREAD, window.handle( ) );
		uint64_t wnd_thread_info_target = call_driver_control( ID_GET_WND_THREAD, target_window );

		// Step 3. change owner

		call_driver_control( ID_SET_WND_THREAD, target_window, wnd_thread_info_local );

		m_renderer->destroy_composition( );

		// Step 4. restore owner

		call_driver_control( ID_SET_WND_THREAD, target_window, wnd_thread_info_target );

		// delete m_renderer;

		VMProtectEnd( );
	}
};