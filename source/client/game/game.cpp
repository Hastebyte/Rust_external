#include "game.h"

bool rust_manager::verify_game( )
{
	// get the hooked function

	this->m_driver_control = kernel_control_function( );

	if ( !this->m_driver_control )
		return false;

	// get process id

	this->m_pid = get_process_id( _xorstr( "RustClient.exe" ) );

	if ( !this->m_pid )
		return false;

	// get base address

	this->m_base = call_driver_control( ID_GET_PROCESS_BASE, this->m_pid );

	if ( !this->m_base )
		return false;

	// get unity module - moved to only run once
	// const wchar_t* module_name = L"UnityPlayer.dll";
	// m_unity_base = call_driver_control( ID_GET_PROCESS_MODULE, this->m_pid, module_name );

	return true;
}

uint64_t rust_manager::get_module( const wchar_t* module_name )
{
	return call_driver_control( ID_GET_PROCESS_MODULE, this->m_pid, module_name );
}

bool rust_manager::validate_class( uint64_t class_instance, uint64_t vtable_offset )
{
	if ( !class_instance || !vtable_offset )
		return false;

	uint64_t vtable = read<uint64_t>( class_instance );
	uint64_t offset = vtable - this->m_unity_base;

	if ( vtable_offset != offset )
		return false;

	return true;
}

bool rust_manager::get_objects( )
{
	for ( auto p : m_players )
		delete p;

	m_players.clear( );
	m_local_player = nullptr;

	uint64_t game_object_manager = read<uint64_t>( this->m_unity_base + OFFSET_GAME_OBJECT_MANAGER );
	
	// printf( "[+] unity_base: %llx game_object_manager: %llx\n", m_unity_base, game_object_manager );

	if ( !game_object_manager )
		return false;

	uint64_t last_tagged_object = read<uint64_t>( game_object_manager + 0x00 );
	uint64_t tagged_object		= read<uint64_t>( game_object_manager + 0x08 );
	uint64_t last_active_object = read<uint64_t>( game_object_manager + 0x10 );
	uint64_t active_object		= read<uint64_t>( game_object_manager + 0x18 );

	//printf( "    last_tagged_object:\t %llx\n",	last_tagged_object );
	//printf( "    tagged_object:\t %llx\n",		tagged_object );
	//printf( "    last_active_object:\t %llx\n",	last_active_object );
	//printf( "    active_object:\t %llx\n",		active_object );

	if ( !_VALID( last_tagged_object ) || !_VALID( tagged_object ) )
		return false;

	if ( !_VALID( last_active_object ) || !_VALID( active_object ) )
		return false;

	const unsigned long max_entity_count = 30000000;
	unsigned long counter = 0;

	// lambda parser

	auto parse_object_type = [this,counter]( uint16_t tag, uint64_t game_object )
	{
		if ( tag )
		{
			// if ( tag < 2000 )
			// 	printf( "[%02d] obj: %llx tag:%d\n", counter, game_object, tag );

			switch ( tag )
			{
			case ID_TAG_PLAYER:
				// printf( "[%02d] obj: %llx tag:%d\n", counter, game_object, tag );
				parse_player( game_object );
				break;
			case ID_TAG_CAMERA:
				parse_camera( game_object );
				break;
			case ID_TAG_SKY:
				parse_sky( game_object );
				break;
			default:
				break;
			}
		}
	};

	while ( tagged_object != last_tagged_object )
	{
		counter++;

		uint64_t game_object = read<uint64_t>( tagged_object + 0x10 );

		if ( !_VALID( game_object ) )
		{
			// printf( "[!] bad game object\n" );
			continue;
		}

		// parse object type

		uint16_t tag	= read<uint16_t>( game_object + OFFSET_OBJECT_TAG );
		uint16_t ulayer	= read<uint16_t>( game_object + OFFSET_OBJECT_LAYER );

		//if ( tag == 6 )
		//{
		//	printf( "[+] tag is player %llx\n", game_object );
		//}

		parse_object_type( tag, game_object );

		// iterate to next object

		tagged_object = read<uint64_t>( tagged_object + 0x08 );

		if ( !_VALID( tagged_object ) )
		{
			printf( _xorstr( "<invalid object break>\n" ) );
			break;
		}

		// limit object iteration

		//if ( counter > max_entity_count )
		//{
		//	printf( _xorstr( "<limit reached break>\n" ) );
		//	break;
		//}
	}

	return true;

}

bool rust_manager::get_active_objects( )
{
	// m_local_player = nullptr;

	uint64_t game_object_manager = read<uint64_t>( this->m_unity_base + OFFSET_GAME_OBJECT_MANAGER );

	// printf( "[+] unity_base: %llx game_object_manager: %llx\n", m_unity_base, game_object_manager );

	if ( !game_object_manager )
		return false;

	uint64_t last_tagged_object		= read<uint64_t>( game_object_manager + 0x00 );
	uint64_t tagged_object			= read<uint64_t>( game_object_manager + 0x08 );
	uint64_t last_active_object		= read<uint64_t>( game_object_manager + 0x10 );
	uint64_t active_object			= read<uint64_t>( game_object_manager + 0x18 );

	//printf( "    last_tagged_object:\t %llx\n",	last_tagged_object );
	//printf( "    tagged_object:\t %llx\n",		tagged_object );
	//printf( "    last_active_object:\t %llx\n",	last_active_object );
	//printf( "    active_object:\t %llx\n",		active_object );

	if ( !_VALID( last_tagged_object ) || !_VALID( tagged_object ) )
		return false;

	if ( !_VALID( last_active_object ) || !_VALID( active_object ) )
		return false;

	const unsigned long max_entity_count = 30000000;
	unsigned long counter = 0;

	// lambda parser

	auto parse_object_type = [this, counter] ( uint16_t tag, uint64_t game_object )
	{
		if ( tag )
		{
			// if ( tag < 2000 )
			// 	printf( "[%02d] obj: %llx tag:%d\n", counter, game_object, tag );

			switch ( tag )
			{
			case ID_TAG_PLAYER:
				// printf( "[%02d] obj: %llx tag:%d\n", counter, game_object, tag );
				parse_player( game_object );
				break;
			case ID_TAG_CAMERA:
				parse_camera( game_object );
				break;
			case ID_TAG_SKY:
				parse_sky( game_object );
				break;
			default:
				break;
			}
		}
	};

	// iterate active objects

	// printf( "[+] iterating active..\n" );

	while ( active_object != last_active_object )
	{
		counter++;

		uint64_t game_object = read<uint64_t>( active_object + 0x10 );

		// doesn't seem to ever be invalid

		if ( !_VALID( game_object ) )
			continue;

		// parse object type

		uint16_t tag = read<uint16_t>( game_object + OFFSET_OBJECT_TAG );
		uint16_t ulayer = read<uint16_t>( game_object + OFFSET_OBJECT_LAYER );

		// if ( tag == 6 )
		// {
			// printf( "[+] active tag is player %llx\n", game_object );
		// }

		parse_object_type( tag, game_object );

		// iterate to next object

		active_object = read<uint64_t>( active_object + 0x08 );

		if ( !_VALID( active_object ) )
			break;
	}

	return true;

}

void rust_manager::parse_player( uint64_t object )
{
	//printf( "[+] player_object: %llx\n", object );

	uint64_t object_class = read<uint64_t>( object + OFFSET_CORRESPONDING_OBJECT );

	if ( !_VALID( object_class ) )
		return;

	// get entity details (this should go above position)

	uint64_t entity = read<uint64_t>( object_class + OFFSET_ENTITY_REF );

	if ( !_VALID( entity ) )
		return;

	// printf( "[+] entity: %llx\n", entity );

	uint64_t base_entity = read<uint64_t>( entity + OFFSET_BASE_ENTITY );

	if ( !_VALID( base_entity ) )
		return;

	// printf( "[+] base_entity: %llx\n", base_entity );


	int local_team	= read<int>( base_entity + OFFSET_BASE_PLAYER_TEAM_NUMBER );



	// not valid for other players?
	// uint64_t player_movement = read<uint64_t>( base_entity + OFFSET_BASE_PLAYER_MOVEMENT );

	// printf( "[+] current team: %d local team %d\n", current_team_id, local_team );
	// printf( "[+] player_movement %llx\n", player_movement );

	// get position

	//uint64_t transform_class = read<uint64_t>( object_class + OFFSET_ENTITY_TRANSFORM );

	//if ( !_VALID( transform_class ) )
	//	return;

	//uint64_t visual_state = read<uint64_t>( transform_class + OFFSET_TRANSFORM_VISUALSTATE );

	//if ( !_VALID( visual_state ) )
	//	return;

	// name for local player check (revise this)
	// enemy players have some asset_prefab path for name

	uint64_t name_buffer = read<uint64_t>( object + OFFSET_OBJECT_NAME );

	if ( !_VALID( name_buffer ) )
		return;

	char name[32] = {0};
	read( name_buffer, name, sizeof( name ) - 1 );

	// construct internal player object

	game::player* player = new game::player( );

	player->m_object		= object;
	player->m_object_class  = object_class;
	player->m_entity		= entity;
	player->m_base_entity	= base_entity;
	player->m_team			= local_team;
	
	// used for recoil?

	

	//read( visual_state + OFFSET_VISUALSTATE_POSITION, &player->m_world_position, sizeof( vector3 ) );

	// printf( "[+] visual state: %2.1f %2.1f %2.1f\n", player->m_world_position.x, player->m_world_position.y, player->m_world_position.z );

	if ( strcmp( name, _xorstr( "LocalPlayer" ) ) == 0 )
	{
		m_local_player = player;
		m_current_item_id = read<int>( base_entity + OFFSET_ACTIVE_ITEM_ID );
	} else {
		m_players.push_back( player );
	}

	// [+] player_object: 2beaa58bd60
	// [+] entity: 2beaa58c010
	// [+] base_entity: 2be9d01a800
	// [+] current team: 0 local team 0
	// [+] player_movement 2be3d9a1cc0
	// [+] visual state: 32.7 0.0 95.1

	// [+] player_object: 2beba19bc60
	// [+] entity: 2beba19bf10
	// [+] base_entity: 2be9d510000
	// [+] current team: 0 local team 0
	// [+] player_movement 0
	// [+] visual state: 0.0 -4.0 21.8
}

void rust_manager::parse_camera( uint64_t object )
{
	int64_t object_class = read<uint64_t>( object + OFFSET_CAMERA_OBJECT );

	if ( !_VALID( object_class ) )
		return;

	uint64_t camera = read<uint64_t>( object_class + OFFSET_MAIN_CAMERA );

	if ( !_VALID( camera ) )
		return;

	vector3 camera_position = read<vector3>( camera + OFFSET_CAMERA_POSITION );

	// vector3 camera_position;
	// read( camera + OFFSET_CAMERA_POSITION, &camera_position, sizeof( vector3 ) );

	if ( camera_position.x == 0.0f || camera_position.y == 0.0f )
	{
		// printf( "[!] bad camera\n" );
		return;
	}

	m_camera_position = camera_position;
	m_camera = camera;

	//printf( "[+] camera: %llx { %2.1f %2.1f %2.1f }\n",
	//	camera,
	//	camera_position.x,
	//	camera_position.y,
	//	camera_position.z );

	// printf( "[+] cam saved, view.41 { %2.1f %2.1f %2.1f }\n", m_view_matrix._41, m_view_matrix._42, m_view_matrix._43 );
}

void rust_manager::update_camera_position( )
{
	vector3 camera_position = read<vector3>( m_camera + OFFSET_CAMERA_POSITION );

	if ( camera_position.x == 0.0f || camera_position.y == 0.0f )
	{
		// printf( "[!] bad camera\n" );
		return;
	}

	m_camera_position = camera_position;
}

void rust_manager::parse_sky( uint64_t object )
{
	m_sky_dome = object;
}

bool rust_manager::get_player_position( uint64_t object_class, vector3& position )
{
	uint64_t transform_class = read<uint64_t>( object_class + OFFSET_ENTITY_TRANSFORM );

	if ( !_VALID( transform_class ) )
		return false;

	uint64_t visual_state = read<uint64_t>( transform_class + OFFSET_TRANSFORM_VISUALSTATE );

	if ( !_VALID( visual_state ) )
		return false;

	return read( visual_state + OFFSET_VISUALSTATE_POSITION, &position, sizeof( vector3 ) );
}

bool rust_manager::is_visible( uint64_t base_entity )
{
	uint64_t player_model = read<uint64_t>( base_entity + OFFSET_BASE_PLAYER_PLAYER_MODEL );

	if ( !player_model )
		return false;

	// printf( "[+] player model: %llx\n", player_model );

	if ( read<byte>( player_model + OFFSET_PLAYER_MODEL_VISIBLE ) == 1 )
	{
		return true;
	} else {
		return false;
	}
}

float rust_manager::get_health( uint64_t base_entity )
{
	return read<float>( base_entity + OFFSET_BASE_COMBAT_ENTITY_HEALTH );
}

uint64_t rust_manager::get_entity_bone( uint64_t base_entity, int bone_index )
{
	uint64_t player_model = read<uint64_t>( base_entity + OFFSET_BASE_PLAYER_PLAYER_MODEL );

	if ( !player_model ) {
		return 0;
	}

	uint64_t multi_mesh = read<uint64_t>( player_model + OFFSET_PLAYER_MODEL_SKIN );
	if ( !multi_mesh ) {
		return 0;
	}

	uint64_t bone_dictionary = read<uint64_t>( multi_mesh + OFFSET_BONE_DICTIONARY );

	if ( !bone_dictionary ) {
		return 0;
	}

	uint64_t bone_values = read<uint64_t>( bone_dictionary + OFFSET_BONE_DICTIONARY_ENTRY );
	if ( !bone_values ) {
		return 0;
	}

	uint64_t entity_bone = read<uint64_t>( bone_values + ( OFFSET_ENTRY_BONE_VALUE + ( ( bone_index - 1 ) * 0x8 ) ) );
	if ( !entity_bone ) {
		return 0;
	}

	uint64_t bone_transform = read<uint64_t>( entity_bone + OFFSET_BONE_VALUE_TRANS );

	if ( !bone_transform ) {
		return 0;
	}

	return bone_transform;
}


float rust_manager::get_vector_by_index( __m128 v, int index )
{
	union
	{
		__m128 v;
		float a[4];
	} converter;
	converter.v = v;
	return converter.a[index];
}

vector3 rust_manager::get_bone_pos( uint64_t pTransform )
{
	if ( !pTransform ) {
		return vector3( 0, 0, 0 );
	}

	__m128 result;

	const __m128 mulVec0 = { -2.000,  2.000, -2.000, 0.000 };
	const __m128 mulVec1 = {  2.000, -2.000, -2.000, 0.000 };
	const __m128 mulVec2 = { -2.000, -2.000,  2.000, 0.000 };

	transform_access_ready_only ptransform_access_ready_only = read <transform_access_ready_only>( pTransform + 0x38 );

	unsigned int index = read<unsigned int>( pTransform + 0x40 );
	transform_data transformData = read <transform_data>( ptransform_access_ready_only.transform_data + 0x18 );

	if ( transformData.transform_array && transformData.transform_indices )
	{
		result = read <__m128>( transformData.transform_array + 0x30 * index );
		int transformIndex = read<int>( transformData.transform_indices + 0x4 * index );
		int pSafe = 0;
		while ( transformIndex >= 0 && pSafe++ < 200 )
		{
			matrix_34 matrix34 = read<matrix_34>( transformData.transform_array + 0x30 * transformIndex );

			__m128 xxxx = _mm_castsi128_ps( _mm_shuffle_epi32( *( __m128i* )( &matrix34.vec1 ), 0x00 ) );	// xxxx
			__m128 yyyy = _mm_castsi128_ps( _mm_shuffle_epi32( *( __m128i* )( &matrix34.vec1 ), 0x55 ) );	// yyyy
			__m128 zwxy = _mm_castsi128_ps( _mm_shuffle_epi32( *( __m128i* )( &matrix34.vec1 ), 0x8E ) );	// zwxy
			__m128 wzyw = _mm_castsi128_ps( _mm_shuffle_epi32( *( __m128i* )( &matrix34.vec1 ), 0xDB ) );	// wzyw
			__m128 zzzz = _mm_castsi128_ps( _mm_shuffle_epi32( *( __m128i* )( &matrix34.vec1 ), 0xAA ) );	// zzzz
			__m128 yxwy = _mm_castsi128_ps( _mm_shuffle_epi32( *( __m128i* )( &matrix34.vec1 ), 0x71 ) );	// yxwy
			__m128 tmp7 = _mm_mul_ps( *( __m128* )( &matrix34.vec2 ), result );

			result = _mm_add_ps(
				_mm_add_ps(
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps( _mm_mul_ps( xxxx, mulVec1 ), zwxy ),
								_mm_mul_ps( _mm_mul_ps( yyyy, mulVec2 ), wzyw ) ),
							_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( tmp7 ), 0xAA ) ) ),
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps( _mm_mul_ps( zzzz, mulVec2 ), wzyw ),
								_mm_mul_ps( _mm_mul_ps( xxxx, mulVec0 ), yxwy ) ),
							_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( tmp7 ), 0x55 ) ) ) ),
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps( _mm_mul_ps( yyyy, mulVec0 ), yxwy ),
								_mm_mul_ps( _mm_mul_ps( zzzz, mulVec1 ), zwxy ) ),
							_mm_castsi128_ps( _mm_shuffle_epi32( _mm_castps_si128( tmp7 ), 0x00 ) ) ),
						tmp7 ) ), *( __m128* )( &matrix34.vec0 ) );

			transformIndex = read<int>( transformData.transform_indices + 0x4 * transformIndex );
		}
	}

	return vector3( get_vector_by_index( result, 0 ), get_vector_by_index( result, 1 ), get_vector_by_index( result, 2 ) );
}

bool rust_manager::world_to_screen( D3DXMATRIX& view_matrix, vector3& origin, vector2* out )
{
	D3DXMATRIX temp;

	D3DXMatrixTranspose( &temp, &view_matrix );

	vector3 translation	= vector3( temp._41, temp._42, temp._43 );
	vector3 up			= vector3( temp._21, temp._22, temp._23 );
	vector3 right		= vector3( temp._11, temp._12, temp._13 );

	float w = translation.Dot( origin ) + temp._44;

	if ( w < 0.098f )
		return false;

	float y = up.Dot( origin ) + temp._24;
	float x = right.Dot( origin ) + temp._14;

	out->x = ( m_screen_width  / 2 ) * ( 1.f + x / w );
	out->y = ( m_screen_height / 2 ) * ( 1.f - y / w );

	return true;
}

bool rust_manager::inside_aim_radius( vector2& screen_position, float aim_circum )
{
	float xhair_distance = sqrt(
		  pow( screen_position.x - ( m_screen_width  / 2.f ), 2.f )
		+ pow( screen_position.y - ( m_screen_height / 2.f ), 2.f ) );

	// printf( "[+] xhair distance: %1.0f\n", xhair_distance );

	if ( xhair_distance > aim_circum /*/ 2.0f*/ )
		return false;

	return true;
}

bool rust_manager::force_daytime( float hour )
{
	if ( !m_sky_dome )
	{
		printf( _xorstr( "[!] sky dome not found\n" ) );
		return false;
	}

	uint64_t objectClass = read<uint64_t>( m_sky_dome + OFFSET_CORRESPONDING_OBJECT );

	if ( !objectClass )
		return false;

	uint64_t entity_ptr = read<uint64_t>( objectClass + OFFSET_ENTITY_REF );

	if ( !entity_ptr )
		return false;

	uint64_t base_entity = read<uint64_t>( entity_ptr + OFFSET_BASE_ENTITY );

	if ( !base_entity )
		return false;

	uint64_t cycle_params = read<uint64_t>( base_entity + OFFSET_TOD_CYCLE_PARAMETERS );

	if ( !cycle_params )
		return false;

	write<float>( cycle_params + OFFSET_TOD_HOUR, hour );

	return true;
}

void rust_manager::draw_corner_box( float x, float y, float w, float h, const D2D1_COLOR_F& color )
{
	const float length = h / 10;

	// Box: Upper Left Corner 
	m_renderer->draw_line( x, y, x, y + length, color, 2.0f );					// Top To Bottom
	m_renderer->draw_line( x, y, x + length, y, color, 2.0f );					// Left To Right 

	//  Box: Upper Right Corner     
	m_renderer->draw_line( x + w, y, x + w - length, y, color, 2.0f );			// Right To Left     
	m_renderer->draw_line( x + w, y, x + w, y + length, color, 2.0f );			// Top To Bottom   

	// Box: Bottom Left Corner
	m_renderer->draw_line( x, y + h, x + length, y + h, color, 2.0f );			// Right To Left   
	m_renderer->draw_line( x, y + h, x, y + h - length, color, 2.0f );			// Bottom To Top  

	// Box: Bottom Right Corner
	m_renderer->draw_line( x + w, y + h, x + w - length, y + h, color, 2.0f );	// Right To Left         
	m_renderer->draw_line( x + w, y + h, x + w, y + h - length, color, 2.0f );	// Bottom To Top
}

void rust_manager::draw_health_bar( float x, float y, float health )
{
	float r = min( ( 510 * ( 100 - health ) ) / 100, 255 );
	float g = min( ( 510 * health ) / 100, 255 );
	float b = 0;
	float a = 255;

	D2D1::ColorF health_color = D2D1::ColorF( round( r * 255.0f ), round( g * 255.0f ), round( b * 255.0f ) );
	D2D1::ColorF black_color  = D2D1::ColorF( 0.f, 0.f, 0.f );

	m_renderer->draw_filled_rect( x + 1.f, y + 38.f, health / 2.f, 6.f, health_color );
	m_renderer->draw_rect( x + 1, y + 38.f, 50.f, 6.f, 1.f, black_color );
}