#pragma once

#include "Headers.h"

class DirectXHook
{
public:
	static DWORD D_OnLoad();
	static DWORD D_OnUnLoad();
};

extern DirectXHook D3D;