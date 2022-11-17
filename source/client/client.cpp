#include "client.h"

void update( rust_manager* game )
{
	VMProtectBeginMutation( "update_thread" );

	while ( game->m_renderer_created )
	{
		std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
		std::lock_guard<std::mutex> guard( game->mut );
		game->get_objects( );

		char title_buffer[256] = {0};
		sprintf_s( title_buffer, "c:%llu", game->m_players.size( ) );
		SetConsoleTitle( title_buffer );
	}

	VMProtectEnd( );
}

void render( rust_manager* game )
{
	VMProtectBeginMutation( "render_thread" );

	game->m_screen_width  = game->m_renderer->screen_width;
	game->m_screen_height = game->m_renderer->screen_height;

	while ( game->m_renderer_created )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		std::lock_guard<std::mutex> guard( game->mut );

		game->m_renderer->begin_draw( );

		if ( game->m_aimbot_on )
		{
			game->m_renderer->draw_text( 10.f, 10.f, D2D1::ColorF( D2D1::ColorF::Red, 1.0f ), "Aimbot On [right click to lock]" );
			game->m_renderer->draw_circle( game->m_screen_width / 2.0f, game->m_screen_height / 2.0f, 200, D2D1::ColorF( D2D1::ColorF::LightBlue, 1.0f ), 2.0f );
		}

		if ( game->m_force_day_time_on )
		{
			game->force_daytime( 12.0f );
		}

		// printf( "[+] render\n" );

		if ( !game->m_local_player )
		{
			//printf( "[!] invalid local player\n" );
			game->m_renderer->end_draw( );
			continue;
		}

		if ( game->m_players.empty( ) )
		{
			//printf( "[!] no players to render\n" );
			game->m_renderer->end_draw( );
			continue;
		}

		//printf( "[+] local player: %llx\n", game->m_local_player->m_object );
		//printf( "[+] target list:\n" );

		for ( game::player* player : game->m_players )
		{
			vector3 local_position;
			game->get_player_position( game->m_local_player->m_object_class, local_position );

			vector3 target_position;
			game->get_player_position( player->m_object_class, target_position );

			float distance = local_position.Distance( target_position );

			//printf( "   -> %llx (%2.1f %2.1f %2.1f)\n", player->m_object,
			//	player->m_world_position.x,
			//	player->m_world_position.y,
			//	player->m_world_position.z );

			D3DXMATRIX view_matrix = game->read<D3DXMATRIX>( game->m_camera + OFFSET_CAMERA_VIEW_MATRIX );

			vector2 screen_feet_position;
			
			if ( !game->world_to_screen( view_matrix, target_position, &screen_feet_position ) )
				continue;

			screen_feet_position.x = std::round( screen_feet_position.x );
			screen_feet_position.y = std::round( screen_feet_position.y );

			D2D1_COLOR_F draw_color = D2D1::ColorF( D2D1::ColorF::CadetBlue, 1.0f );

			bool visible = game->is_visible( player->m_base_entity );

			if ( visible )
				draw_color = D2D1::ColorF( D2D1::ColorF::Red, 1.0f );
	
			float health = game->get_health( player->m_base_entity );

			if ( health == 0 )
				continue;

			game->m_renderer->draw_text( screen_feet_position.x, screen_feet_position.y, draw_color,
				"player [%1.0f m] H:%1.0f", distance, health );

			game->draw_health_bar( screen_feet_position.x, screen_feet_position.y - 10.f, health );

			// get head and draw box 

			vector2 screen_head_position;
			vector3 world_head_position = game->get_bone_pos( game->get_entity_bone( player->m_base_entity, head ) );
			
			if ( !game->world_to_screen( view_matrix, world_head_position, &screen_head_position ) )
				continue;

			screen_head_position.x = std::round( screen_head_position.x );
			screen_head_position.y = std::round( screen_head_position.y );

			if ( game->m_aimbot_on && nt_key_state( VK_RBUTTON ) )
			{
				if ( visible && health > 0.f && game->inside_aim_radius( screen_head_position, 200.f ) )
				{
					game->update_camera_position( );
					game->aim_at( world_head_position );
				}
			}


			// exit here if target is dead

			//game->m_renderer->draw_text( screen_head_position.x, screen_head_position.y,
			//	draw_color,
			//	"[head]", distance );
	
			float h	 = screen_feet_position.y - screen_head_position.y;
			float w  = h / 4.5f;

			game->draw_corner_box(	screen_head_position.x - w,
									screen_head_position.y,
									w * 2, h, draw_color );

			//float h2 = vPos.y - vHead.y;



			/*printf( "   screen: (%2.1f %2.1f)\n",
				screen_position.x,
				screen_position.y );*/
		}


		game->m_renderer->end_draw( );
		//Sleep( 1 );
	}
	
	VMProtectEnd( );
}

DWORD WINAPI key_thread( LPVOID param )
{
	VMProtectBeginMutation( "key_thread" );

	printf( "[+] Push F2 now to start renderer\n" );
	printf( "[F2/F4] - start/stop renderer\n" );
	printf( "[F5]  - aimbot on (right mouse click to lock)\n" );
	printf( "[F6]  - aimbot off\n" );
	printf( "[F9]  - set no recoil for current weapon\n" );
	printf( "[F10] - force daylight on (expiremental)\n" );
	printf( "[F11] - force daylight off\n" );

	rust_manager game;

	while ( true )
	{
		if ( !game.verify_game( ) )
		{
			Sleep( 5000 );
			continue;
		}

		if ( nt_key_state( VK_F5 ) )
		{
			printf( _xorstr( "[+] aimbot on\n" ) );
			game.m_aimbot_on = true;
			Sleep( 150 );
			continue;
		}

		if ( nt_key_state( VK_F6 ) )
		{
			printf( _xorstr( "[+] aimbot off\n" ) );
			game.m_aimbot_on = false;
			Sleep( 150 );
			continue;
		}

		if ( nt_key_state( VK_F9 ) )
		{
			printf( _xorstr( "[+] set no recoil for current weapon\n" ) );
			game.get_objects( );
			game.no_recoil( );
			Sleep( 150 );
			continue;
		}

		if ( nt_key_state( VK_F10 ) )
		{
			printf( _xorstr( "[+] force daytime on\n" ) );
			game.m_force_day_time_on = true;
			Sleep( 150 );
			continue;
		}

		if ( nt_key_state( VK_F11 ) )
		{
			printf( _xorstr( "[+] force daytime off\n" ) );
			game.m_force_day_time_on = false;
			Sleep( 150 );
			continue;
		}

		if ( nt_key_state( VK_F12 ) )
		{
			printf( _xorstr( "[+] no fall damage\n" ) );
			game.no_fall_damage( );
			Sleep( 150 );
			continue;
		}

		if ( nt_key_state( VK_F2 ) )
		{
			// let's make sure local player, camera and other objects
			// are available to the renderer from the start

			game.get_objects( );
			game.get_active_objects( );
			// game.get_objects( );

			printf( _xorstr( "[+] Creating renderer\n" ) );

			game.build_renderer( );

			//if ( game.build_renderer( ) )
			//{
				game.m_renderer_created = true;

				std::thread thread_update( update, &game );
				std::thread thread_render( render, &game );

				thread_update.detach( );
				thread_render.detach( );
			//}

			Sleep( 150 );


			const wchar_t* module_name = L"UnityPlayer.dll";
			game.m_unity_base = call_driver_control( kernel_control_function( ), ID_GET_PROCESS_MODULE, game.m_pid, module_name );
			printf( "[+] unity: %llx\n", game.m_unity_base );

			continue;
		} 

		if ( nt_key_state( VK_F4 ) )
		{
			printf( _xorstr( "[+] Destroying renderer..\n" ) );

			Sleep( 200 );

			if ( game.m_renderer_created )
			{
				game.m_renderer_created = false;
				Sleep( 500 );
				game.m_renderer->erase_visuals( );
				Sleep( 500 );
				game.destroy_renderer( );
				Sleep( 150 );

				printf( _xorstr( "[+] Destroyed\n" ) );

				// delete m_renderer;
			}
		}

		Sleep( 10 );
	}

	VMProtectEnd( );
	return 0;
};



int main( void )
{
	VMProtectBeginMutation( "main" );

	// look at win32kbase!HMChangeOwnerThread+0x103

	SetConsoleTitle( random_string( 20 ).c_str( ) );
	LoadLibrary( "user32.dll" );

	uint64_t result = call_driver_control( kernel_control_function( ), 1338 );

	if ( result == 0xC0000005 )
	{
		MessageBox( HWND_DESKTOP, _xorstr( "Program not properly started" ), _xorstr( "Error" ), MB_ICONERROR );
		ExitProcess( 0 );
		return 0;
	}

	//HANDLE mutex = CreateMutex( 0, 0, "UAPMvWT0aYHLRILIaSFI4iNnZVsB72kduCm6soaQ" );

	//if ( mutex != NULL )
	//{
	//	MessageBox( HWND_DESKTOP, _xorstr( "Program not properly started" ), _xorstr( "Error" ), MB_ICONERROR );
	//	ExitProcess( 0 );
	//	return 0;
	//}

	printf( _xorstr( "[+] Push F1 when you've spawned in game to start\n" ) );

	while ( !nt_key_state( VK_F1 ) )
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

	key_thread( nullptr );

	while ( ( nt_key_state( VK_F12 ) & 1 ) == 0 )
		Sleep( 100 );

	VMProtectEnd( );
	return 0;
}
