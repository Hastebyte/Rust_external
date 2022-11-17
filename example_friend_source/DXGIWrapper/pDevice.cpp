#include "Headers.h"

bool inited = false;

bool OnPresent(IDXGISwapChain *swap, UINT syncInterval, UINT flags) 
{
	Renderer* pRender = Renderer::GetInstance();
	if (pRender != nullptr)
	{
		pRender->BeginScene();
		CRender::DrawPresent(swap, pRender);
		pRender->EndScene();
	}

	return true;
}

void FreeDXGICapture()
{
}

DWORD DirectXHook::D_OnLoad()
{

	return 0;
}

DWORD DirectXHook::D_OnUnLoad()
{
	FreeDXGICapture();

	return 0;
}