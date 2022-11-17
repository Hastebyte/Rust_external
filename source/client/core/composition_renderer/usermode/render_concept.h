#ifndef RENDER_CONCEPT_H
#define RENDER_CONCEPT_H

#pragma once
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
	temporary_window( )
	{
		WNDCLASS wc{};

		wc.lpszClassName = WindowClassName;
		wc.hInstance = GetModuleHandle( nullptr );
		wc.lpfnWndProc = [] ( const HWND window, const UINT message, const WPARAM wparam, const LPARAM lparam ) -> LRESULT
		{
			return DefWindowProc( window, message, wparam, lparam );
		};

		RegisterClass( &wc );

		this->window = CreateWindow( WindowClassName, WindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, GetModuleHandle( nullptr ), nullptr );
	}

	~temporary_window( )
	{
		if ( this->window )
			DestroyWindow( this->window );

		UnregisterClass( WindowClassName, GetModuleHandle( nullptr ) );
	}

	HWND handle( ) const
	{
		return window;
	}
};

class CompositionRenderer
{
public:
	CompositionRenderer( ) { }
	CompositionRenderer( const CompositionRenderer& ) = delete;
   ~CompositionRenderer( ) { }

   bool build_swapchain( HWND game_window )
   {
		RECT window_rect;

		if ( !GetClientRect( game_window, &window_rect ) )
		{
			printf( "[!] Invalid window\n" );
			return false;
		}

		this->targetWindow = game_window;

		HRESULT result =
			D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, &d3d11Device, nullptr, nullptr );

		if ( FAILED( result ) )
		{
			printf( "[!] Failed to create device, result=%lx\n", result );
			return false;
		}
 
		d3d11Device->QueryInterface( IID_PPV_ARGS( &dxgiDevice ) );

		result = CreateDXGIFactory2( NULL, __uuidof( dxgiFactory ), reinterpret_cast<void**>( &dxgiFactory ) );

		if ( FAILED( result ) )
		{
			printf(" [!] Failed to create DXGI factory, result=%lx\n", result );
			return false;
		}

		DXGI_SWAP_CHAIN_DESC1 description = { };
		description.Format				= DXGI_FORMAT_B8G8R8A8_UNORM;
		description.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		description.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		description.BufferCount			= 2;
		description.SampleDesc.Count	= 1;
		description.AlphaMode			= DXGI_ALPHA_MODE_PREMULTIPLIED;
		description.Width				= screenWidth = window_rect.right - window_rect.left;
		description.Height				= screenHeight = window_rect.bottom - window_rect.top;

		// This call can fail if a previous composition has not been cleaned up for this window

		result = dxgiFactory->CreateSwapChainForComposition( d3d11Device, &description, nullptr, &dxgiSwapChain );

		if ( FAILED( result ) )
		{
			printf( "[!] Failed to create SwapChain, result=0x%lx\n", result );
			return false;
		}

		D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory );
		result = d2dFactory->CreateDevice( dxgiDevice, &d2dDevice );

		if ( FAILED( result ) )
		{
			printf( "[!] Failed to create ID2D1Device, result=0x%lx\n", result );
			return false;
		}

		result = d2dDevice->CreateDeviceContext( D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dDeviceContext );

		if ( FAILED( result ) )
		{
			printf( "[!] Failed to create ID2DDeviceContext, result=0x%lx\n", result );
			return false;
		}

		dxgiSwapChain->GetBuffer( 0, __uuidof( renderSurface ), reinterpret_cast<void**>( &renderSurface ) );

		// Render target

		D2D1_BITMAP_PROPERTIES1 properties	= { };
		properties.pixelFormat.alphaMode	= D2D1_ALPHA_MODE_PREMULTIPLIED;
		properties.pixelFormat.format		= DXGI_FORMAT_B8G8R8A8_UNORM;
		properties.bitmapOptions			= D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

		d2dDeviceContext->CreateBitmapFromDxgiSurface( renderSurface, properties, &renderBitmap );
		d2dDeviceContext->SetTarget( renderBitmap );
		d2dDeviceContext->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );

		// Objects used to draw

		const auto color = D2D1::ColorF( 0.f, 0.f, 0.f );
		d2dDeviceContext->CreateSolidColorBrush( color, &d2dSolidBrush );
		DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( writeFactory ), (IUnknown**)&writeFactory );
		writeFactory->CreateTextFormat( L"tahoma", nullptr, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10.f, L"en-us", &writeTextFormat );
	
		return true;
   }

   bool build_composition( HWND hTest )
   {
		HRESULT result = DCompositionCreateDevice( dxgiDevice, __uuidof( compositionDevice ), reinterpret_cast<void**>( &compositionDevice ) );

		if ( FAILED( result ) )
		{
			printf( "[!] DCompositionCreateDevice failed" );
			return false;
		}

		result = compositionDevice->CreateTargetForHwnd( hTest, true, &compositionTarget );

		if ( FAILED( result ) )
		{
			printf( "[!] CreateTargetForHwnd( ) failed, %lx\n", result );
			return false;
		}

		result = compositionDevice->CreateVisual( &compositionVisual ); if ( FAILED( result ) ) { printf( "er11\n" ); };
		result = compositionVisual->SetContent( dxgiSwapChain ); if ( FAILED( result ) ) { printf( "er11\n" ); };
		result = compositionTarget->SetRoot( compositionVisual ); if ( FAILED( result ) ) { printf( "er11\n" ); };
		result = compositionDevice->Commit( ); if ( FAILED( result ) ) { printf( "er11\n" ); };
		result = compositionDevice->WaitForCommitCompletion( ); if ( FAILED( result ) ) { printf( "er11\n" ); };

		return true;
   }

	void destroy_composition( )
	{
		compositionVisual->SetContent( nullptr );
		compositionTarget->SetRoot( nullptr );
		compositionDevice->Release( );
		compositionVisual->Release( );
		compositionTarget->Release( );
	}

	void begin_draw( )
	{
		d2dDeviceContext->BeginDraw( );
		d2dDeviceContext->Clear( );
	}

	void end_draw( )
	{
		d2dDeviceContext->EndDraw( );

		HRESULT hPresentResult = dxgiSwapChain->Present( 0, 0 );

		if ( FAILED( hPresentResult ) )
		{
			printf( "> present( ) failed = %X\n", hPresentResult );
		}
	}

	void draw_rect( float x, float y, float w, float h, float t, const D2D1_COLOR_F& color )
	{
		d2dSolidBrush->SetColor( color );
		d2dDeviceContext->DrawRectangle( D2D1::RectF( x, y, x + w, y + h ), d2dSolidBrush, t );
	}

	bool draw_text( const std::wstring& text, float x, float y, float s, const D2D1_COLOR_F& color )
	{
		IDWriteTextLayout* layout;
		writeFactory->CreateTextLayout( text.c_str( ), UINT32( text.length( ) + 1 ), writeTextFormat, float( screenWidth ), float( screenHeight ), &layout );

		const DWRITE_TEXT_RANGE range { 0, UINT32( text.length( ) ) };

		layout->SetFontSize( s, range );

		// draw black background

		d2dSolidBrush->SetColor( D2D1::ColorF( 0.f, 0.f, 0.f ) );
		d2dDeviceContext->DrawTextLayout( D2D1::Point2F( x - 1, y ), layout, d2dSolidBrush );
		d2dDeviceContext->DrawTextLayout( D2D1::Point2F( x + 1, y ), layout, d2dSolidBrush );
		d2dDeviceContext->DrawTextLayout( D2D1::Point2F( x, y - 1 ), layout, d2dSolidBrush );
		d2dDeviceContext->DrawTextLayout( D2D1::Point2F( x, y + 1 ), layout, d2dSolidBrush );

		// draw color text

		d2dSolidBrush->SetColor( color );
		d2dDeviceContext->DrawTextLayout( D2D1::Point2F( x, y ), layout, d2dSolidBrush );

		return true;
	}


private:
	HWND targetWindow;
	int screenWidth;
	int screenHeight;

	ID3D11Device* d3d11Device;
	IDXGIDevice* dxgiDevice;
	IDXGIFactory2* dxgiFactory;
	IDXGISwapChain1* dxgiSwapChain;
	IDXGISurface2* renderSurface;
	ID2D1Bitmap1* renderBitmap;
	ID2D1Factory2* d2dFactory;
	ID2D1Device1* d2dDevice;
	ID2D1DeviceContext1* d2dDeviceContext;
	ID2D1SolidColorBrush* d2dSolidBrush;
	IDWriteFactory* writeFactory;
	IDWriteTextFormat* writeTextFormat;
	IDCompositionDevice* compositionDevice;
	IDCompositionTarget* compositionTarget;
	IDCompositionVisual* compositionVisual;
};

#endif