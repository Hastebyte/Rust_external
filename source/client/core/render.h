#ifndef RENDER_H
#define RENDER_H

#include <wrl.h>
#include <dxgi.h>
#include <d2d1.h>
#include <d3d11.h>
#include <d2d1_2.h>
#include <dcomp.h>
#include <exception>
#include <string>
#include <dwrite.h>

#pragma comment( lib, "dxgi" )
#pragma comment( lib, "d2d1" )
#pragma comment( lib, "d3d11" )
#pragma comment( lib, "dcomp" )
#pragma comment( lib, "dwrite" )

#pragma comment( lib, "d2d1.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "dcomp.lib" )
#pragma comment( lib, "d3d11.lib" )

class temporary_window
{
	constexpr static auto WindowClassName = "AppWindowClass";
	constexpr static auto WindowTitle = "";
	HWND window;
public:
	 temporary_window( );
	~temporary_window( );
	HWND handle( ) const;
};

class composition_renderer
{
public:
	composition_renderer( ) { }
	composition_renderer( const composition_renderer& ) = delete;
   ~composition_renderer( ) { }

	bool build_swapchain( HWND game_window );
	bool build_composition( HWND hTest );
	void erase_visuals( );
	void destroy_composition( );
	void begin_draw( );
	void end_draw( );
	void draw_rect( float x, float y, float w, float h, float t, const D2D1_COLOR_F& color );
	void draw_filled_rect( const float x, const float y, const float w, const float h, const D2D1_COLOR_F& color );
	void draw_line( const float x1, const float y1, const float x2, const float y2, const D2D1_COLOR_F& color, const float t );
	void draw_circle( const float x, const float y, const float r, const D2D1_COLOR_F& color, const float t );
	void draw_filled_circle( const float x, const float y, const float r, const D2D1_COLOR_F& color );
	bool draw_text( const std::wstring& text, float x, float y, float s, const D2D1_COLOR_F& color );
	bool draw_text( const float x, const float y, const D2D1_COLOR_F& color, const char* text, ... );

	void draw_bitmap( const float x, const float y, const float r, const D2D1_COLOR_F& color )
	{
		d2d_device_context->DrawBitmap( menu_bitmap );
	}


	int screen_width;
	int screen_height;

private:
	HWND target_window;

	ID3D11Device* d3d11_device;
	IDXGIDevice* dxgi_device;
	IDXGIFactory2* dxgi_factory;
	IDXGISwapChain1* dxgi_swapchain;
	IDXGISurface2* render_surface;
	ID2D1Bitmap1* render_bitmap;
	ID2D1Bitmap* menu_bitmap;
	ID2D1Factory2* d2d_factory;
	ID2D1Device1* d2d_device;
	ID2D1DeviceContext1* d2d_device_context;
	ID2D1SolidColorBrush* d2d_solid_brush;
	IDWriteFactory* write_factory;
	IDWriteTextFormat* write_text_format;
	IDCompositionDevice* composition_device;
	IDCompositionTarget* composition_target;
	IDCompositionVisual* composition_visual;
};

#endif