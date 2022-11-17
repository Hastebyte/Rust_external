#include <windows.h>
#include <chrono>
#include <random>

#include "render_concept.h"

#include "..\game\game2.h"
#include "..\..\kelib\util\process.h"

std::random_device rd;
std::mt19937 mt( rd( ) );

// draw some text and 100 rectangles
 
void boxes_and_text( CompositionRenderer& renderer, uint64_t last_frame_duration, uint64_t frames_rendered )
{
	wchar_t buffer[128] = {0};

	swprintf( buffer, 128,
		L"Last frame duration: %llu microseconds\nFrames rendered: %llu",
		last_frame_duration,
		frames_rendered );

	// add some random colors and movements

	//std::uniform_real_distribution<float> distribution( 0.5, 1.0 );
	//const D2D1::ColorF random_color = D2D1::ColorF( distribution( mt ), distribution( mt ), 1.0f, 1.0f );
	const float box_size = 25;

	renderer.draw_text( buffer, 20, 20, 24, D2D1::ColorF( 0.5f, 1.0f, 1.0f, 1.0f ) );

	for ( int i = 0; i < 5; i++ )
	{
		//
		
		for ( int n = 0; n < 10; n++ )
			renderer.draw_rect( 50 + ( i * box_size ) + ( n * box_size ), 100 + ( i * box_size ), box_size, box_size, 2, D2D1::ColorF( 0.5f, 1.0f, 1.0f, 1.0f ) );
	}
}

void render_loop( CompositionRenderer& renderer )
{


	uint64_t last_frame_duration = 0;
	uint64_t frames_rendered = 0;

	while ( ( GetAsyncKeyState( VK_F12 ) & 1 ) == 0 )
	{
		auto start_time = std::chrono::steady_clock::now( );

		// render something

		renderer.begin_draw( );
		boxes_and_text( renderer, last_frame_duration, frames_rendered );
		renderer.end_draw( );

		// calculate time it took to render

		auto end_time = std::chrono::steady_clock::now( );
		last_frame_duration = std::chrono::duration_cast<std::chrono::microseconds>( end_time - start_time ).count( );
		frames_rendered++;
	}
}

void test( )
{
	// Step 1. get target window

	DWORD target_pid	= kelib::GetProcessId( "RainbowSix.exe" );
	HWND  target_window	= kelib::GetProcessWindow( target_pid );

	if ( !target_window )
	{
		printf( "[!] window not found\n" );
		return;
	}

	// Step 2. build local d3d device

	CompositionRenderer renderer;
	renderer.build_swapchain( target_window );

	printf( "[+] creating\n" );
	printf( "     pid:%lx\n", target_pid );
	printf( "     window:%llx\n", reinterpret_cast<uint64_t>( target_window ) );


	// Step 3. create temporary window and get kernel thread info for each window

	temporary_window window;
	uint64_t wnd_thread_info_local = call_driver_control( kernel_control_function( ), ID_GET_WND_THREAD, window.handle( ) );
	uint64_t wnd_thread_info_target = call_driver_control( kernel_control_function( ), ID_GET_WND_THREAD, target_window );

	printf( "[+] kernel pointers\n" );
	printf( "     local: %llx\n", wnd_thread_info_local );
	printf( "     target:%llx\n", wnd_thread_info_target );

	// Step 4. spoof ownership temporarily, change the game's window struct to have our temporary thread info pointer
	// kernel will check this pointer when making the composition surface

	if ( call_driver_control( kernel_control_function( ), ID_SET_WND_THREAD, target_window, wnd_thread_info_local ) == 0 )
	{
		// printf( "[+] ownership changed\n" );

		if ( renderer.build_composition( target_window ) )
			printf( "[+] built\n" );
		else
			printf( "[!] destroyed\n" );

		// Test for:
		// 0x88980800 - DCOMPOSITION_ERROR_WINDOW_ALREADY_COMPOSED
		// 0x88980801 - DCOMPOSITION_ERROR_SURFACE_BEING_RENDERED
		// 0x88980802 - DCOMPOSITION_ERROR_SURFACE_NOT_BEING_RENDERED
	}

	// Step 5. restore

	call_driver_control( kernel_control_function( ), ID_SET_WND_THREAD, target_window, wnd_thread_info_target );

	// Step 6. render test

	render_loop( renderer );

	// Step 7. destroy renderer

	renderer.destroy_composition( );

};