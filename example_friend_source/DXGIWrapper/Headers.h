#pragma once

#pragma warning (disable:4005)
#pragma warning (disable:4101)
#pragma warning (disable:4102)
#pragma warning (disable:4244)
#pragma warning (disable:4267)
#pragma warning (disable:4302)
#pragma warning (disable:4305)
#pragma warning (disable:4307)
#pragma warning (disable:4309)
#pragma warning (disable:4311)
#pragma warning (disable:4312)
#pragma warning (disable:4996)

#pragma execution_character_set("utf-8")

#include <Windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string>
#include <shlobj.h>
#include <mutex>
#include <utility>
#include <shlwapi.h>
#include <psapi.h> 
#include <time.h>
#include <dwmapi.h>
#include <TlHelp32.h>
#include <DbgHelp.h>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include <map>
#include <functional>
#include <list>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <algorithm>   
#include <cctype> 
#include <winternl.h>
#include <winioctl.h>
#include <stdlib.h>
#include <unordered_map> 

#include <d3d9.h>
#include <d3dx9.h>
#include <d3d11_1.h>
#include <d3dx11.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <D3DX10math.h>
#include <d3d9types.h>

#include <directxmath.h>
#include <DirectXColors.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "dbghelp.lib")

#include "pDevice.h"
#include "Process.h"
#include "Renderer.h"
#include "Render.h"

typedef DWORD64 QWORD;

using namespace std;
