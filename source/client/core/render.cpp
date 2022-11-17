#include "render.h"

temporary_window::temporary_window( )
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

temporary_window::~temporary_window( )
{
	if ( this->window )
		DestroyWindow( this->window );

	UnregisterClass( WindowClassName, GetModuleHandle( nullptr ) );
}

HWND temporary_window::handle( ) const
{
	return window;
}

bool composition_renderer::build_swapchain( HWND game_window )
{
	RECT window_rect;

	if ( !GetClientRect( game_window, &window_rect ) )
	{
		printf( "[!] Invalid window\n" );
		return false;
	}

	this->target_window = game_window;

	HRESULT result =
		D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, &d3d11_device, nullptr, nullptr );

	if ( FAILED( result ) )
	{
		printf( "[!] Failed to create device, result=%lx\n", result );
		return false;
	}

	d3d11_device->QueryInterface( IID_PPV_ARGS( &dxgi_device ) );

	result = CreateDXGIFactory2( NULL, __uuidof( dxgi_factory ), reinterpret_cast<void**>( &dxgi_factory ) );

	if ( FAILED( result ) )
	{
		printf( " [!] Failed to create DXGI factory, result=%lx\n", result );
		return false;
	}

	DXGI_SWAP_CHAIN_DESC1 description = {};
	description.Format		= DXGI_FORMAT_B8G8R8A8_UNORM;
	description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	description.SwapEffect	= DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	description.BufferCount = 2;
	description.SampleDesc.Count = 1;
	description.AlphaMode	= DXGI_ALPHA_MODE_PREMULTIPLIED;
	description.Width		= screen_width  = window_rect.right  - window_rect.left;
	description.Height		= screen_height = window_rect.bottom - window_rect.top;

	printf( "[+] renderer using %dx%d\n", screen_width, screen_height );

	// This call can fail if a previous composition has not been cleaned up for this window

	result = dxgi_factory->CreateSwapChainForComposition( d3d11_device, &description, nullptr, &dxgi_swapchain );

	if ( FAILED( result ) )
	{
		printf( "[!] Failed to create SwapChain, result=0x%lx\n", result );
		return false;
	}

	D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory );
	result = d2d_factory->CreateDevice( dxgi_device, &d2d_device );

	if ( FAILED( result ) )
	{
		printf( "[!] Failed to create ID2D1Device, result=0x%lx\n", result );
		return false;
	}

	result = d2d_device->CreateDeviceContext( D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_device_context );

	if ( FAILED( result ) )
	{
		printf( "[!] Failed to create ID2DDeviceContext, result=0x%lx\n", result );
		return false;
	}

	dxgi_swapchain->GetBuffer( 0, __uuidof( render_surface ), reinterpret_cast<void**>( &render_surface ) );

	// Render target

	D2D1_BITMAP_PROPERTIES1 properties = {};
	properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

	d2d_device_context->CreateBitmapFromDxgiSurface( render_surface, properties, &render_bitmap );
	d2d_device_context->SetTarget( render_bitmap );
	d2d_device_context->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );

	// Objects used to draw

	const auto color = D2D1::ColorF( 0.f, 0.f, 0.f );
	d2d_device_context->CreateSolidColorBrush( color, &d2d_solid_brush );
	DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( write_factory ), (IUnknown**)&write_factory );
	write_factory->CreateTextFormat( L"tahoma", nullptr, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10.f, L"en-us", &write_text_format );

	return true;
}

bool composition_renderer::build_composition( HWND hTest )
{
	HRESULT result = DCompositionCreateDevice( dxgi_device, __uuidof( composition_device ), reinterpret_cast<void**>( &composition_device ) );

	if ( FAILED( result ) )
	{
		printf( "[!] CDevice creation failed" );
		return false;
	}

	result = composition_device->CreateTargetForHwnd( hTest, true, &composition_target );

	if ( FAILED( result ) )
	{
		printf( "[!] Create target failed, %lx\n", result );
		return false;
	}

	result = composition_device->CreateVisual( &composition_visual ); if ( FAILED( result ) ) { printf( "er11\n" ); };
	result = composition_visual->SetContent( dxgi_swapchain ); if ( FAILED( result ) ) { printf( "er11\n" ); };
	result = composition_target->SetRoot( composition_visual ); if ( FAILED( result ) ) { printf( "er11\n" ); };
	result = composition_device->Commit( ); if ( FAILED( result ) ) { printf( "er11\n" ); };
	result = composition_device->WaitForCommitCompletion( ); if ( FAILED( result ) ) { printf( "er11\n" ); };

	return true;
}

void composition_renderer::erase_visuals( )
{
	composition_visual->SetContent( nullptr );
	composition_target->SetRoot( nullptr );
}

void composition_renderer::destroy_composition( )
{
	// composition_visual->SetContent( nullptr );
	// composition_target->SetRoot( nullptr );

	composition_visual->Release( );
	composition_target->Release( );
	composition_device->Release( );
}

void composition_renderer::begin_draw( )
{
	d2d_device_context->BeginDraw( );
	d2d_device_context->Clear( );
}

void composition_renderer::end_draw( )
{
	d2d_device_context->EndDraw( );

	HRESULT hPresentResult = dxgi_swapchain->Present( 0, 0 );

	if ( FAILED( hPresentResult ) )
	{
		printf( "> present( ) failed = %X\n", hPresentResult );
	}
}

void composition_renderer::draw_rect( float x, float y, float w, float h, float t, const D2D1_COLOR_F& color )
{
	d2d_solid_brush->SetColor( color );
	d2d_device_context->DrawRectangle( D2D1::RectF( x, y, x + w, y + h ), d2d_solid_brush, t );
}

void composition_renderer::draw_filled_rect( const float x, const float y, const float w, const float h, const D2D1_COLOR_F& color )
{
	d2d_solid_brush->SetColor( color );
	d2d_device_context->FillRectangle( D2D1::RectF( x, y, x + w, y + h ), d2d_solid_brush );
}

void composition_renderer::draw_line( const float x1, const float y1, const float x2, const float y2, const D2D1_COLOR_F& color, const float t )
{
	d2d_solid_brush->SetColor( color );
	d2d_device_context->DrawLine( D2D1::Point2F( x1, y1 ), D2D1::Point2F( x2, y2 ), d2d_solid_brush, t );
}

void composition_renderer::draw_circle( const float	x, const float y, const float r, const D2D1_COLOR_F& color, const float t )
{
	d2d_solid_brush->SetColor( color );
	d2d_device_context->DrawEllipse( D2D1::Ellipse( D2D1::Point2F( x, y ), r, r ), d2d_solid_brush, t );
}

void composition_renderer::draw_filled_circle( const float x, const float y, const float r, const D2D1_COLOR_F& color )
{
	d2d_solid_brush->SetColor( color );
	d2d_device_context->FillEllipse( D2D1::Ellipse( D2D1::Point2F( x, y ), r, r ), d2d_solid_brush );
}

bool composition_renderer::draw_text( const std::wstring& text, float x, float y, float s, const D2D1_COLOR_F& color )
{
	IDWriteTextLayout* layout;
	write_factory->CreateTextLayout( text.c_str( ), UINT32( text.length( ) + 1 ), write_text_format, float( screen_width ), float( screen_height ), &layout );

	//if ( centered )
	//{
	//	DWRITE_TEXT_METRICS metrics;
	//	check_result( text_layout->GetMetrics( &metrics ) );

	//	x -= metrics.width / 2.f;
	//}

	const DWRITE_TEXT_RANGE range{0, UINT32( text.length( ) )};

	layout->SetFontSize( s, range );

	// draw black background

	d2d_solid_brush->SetColor( D2D1::ColorF( 0.f, 0.f, 0.f ) );
	d2d_device_context->DrawTextLayout( D2D1::Point2F( x - 1, y ), layout, d2d_solid_brush );
	d2d_device_context->DrawTextLayout( D2D1::Point2F( x + 1, y ), layout, d2d_solid_brush );
	d2d_device_context->DrawTextLayout( D2D1::Point2F( x, y - 1 ), layout, d2d_solid_brush );
	d2d_device_context->DrawTextLayout( D2D1::Point2F( x, y + 1 ), layout, d2d_solid_brush );

	// draw color text

	d2d_solid_brush->SetColor( color );
	d2d_device_context->DrawTextLayout( D2D1::Point2F( x, y ), layout, d2d_solid_brush );

	layout->Release( );

	return true;
}

bool composition_renderer::draw_text( const float x, const float y, const D2D1_COLOR_F& color, const char* text, ... )
{
	char buffer[2048] = {0};
	wchar_t wbuffer[8192] = {0};

	va_list argptr;

	va_start( argptr, text );
	_vsnprintf_s( buffer, sizeof( buffer ), text, argptr );
	va_end( argptr );

	MultiByteToWideChar( CP_ACP, 0, buffer, static_cast<int>( strlen( buffer ) ), wbuffer, ARRAYSIZE( wbuffer ) );

	return draw_text( wbuffer, x, y, 15.f, color );
}
