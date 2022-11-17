#pragma once

#include "Headers.h"

class CRender
{
public:
	static void DrawPresent(IDXGISwapChain *m_pSwapChain, Renderer* pRender);
	static void DrawPMenu(IDXGISwapChain *m_pSwapChain, Renderer* pRender);
};

extern CRender cRender;