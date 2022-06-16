#pragma once

#include <vector>
#include <fstream>

#include <windows.h>
#include <windef.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>  
#include <string>
#include <Psapi.h>
#include "mmsystem.h"
#include "Model_x.h"
#include "vector3.h"
#include "mesh.h"
#include "vertex.h"


extern LPD3DXFONT gProgressFont;

// Random number generator
int			RandomNumber(int iMin, int iMax);
float		RandomNumber(float fMin, float fMax);
D3DXVECTOR3	RandomNumber(D3DXVECTOR3 vMin, D3DXVECTOR3 vMax);
D3DXCOLOR	RandomNumber(D3DXCOLOR Min, D3DXCOLOR Max);

// String conversion helpers
char			*ConvertString( char *out, WCHAR *in );
WCHAR			*ConvertString( WCHAR *out, char *in );
WCHAR			*ConvertString( char *in );

// Dump user hardware properties
void LogOSAndAdapter();
HRESULT GetVideoMemoryViaDirectDraw( HMONITOR hMonitor, DWORD* pdwAvailableVidMem );
HRESULT GetVideoMemoryViaD3D9( HMONITOR hMonitor, UINT* pdwAvailableTextureMem );
HRESULT GetVideoMemoryViaWMI( HMONITOR hMonitor, DWORD* pdwAdapterRam );

// Miscellaneous functions
HRESULT CreateGlobalVariables(IDirect3DDevice9* pd3dDevice);
HRESULT ResetGlobalVariables(IDirect3DDevice9* pd3dDevice);
void LostGlobalVariables();
void DestroyGlobalVariables();
void Progress( std::string text, float prc );
char * Commify( double f );

class Point
{
	public:
		Point(){x = y = 0;}
		Point(int _x, int _y){Set(_x,_y);}

		void operator=(const POINT rhs){x = rhs.x;y = rhs.y;}
		bool operator==(const Point rhs){return rhs.x == x && rhs.y == y;}
		bool operator!=(const Point rhs){return rhs.x != x || rhs.y != y;}
		void operator+=(const Point rhs){x += rhs.x; y += rhs.y;}
		void operator/=(const int rhs){x /= rhs; y /= rhs;}
		Point operator/(const Point rhs){return Point(x / rhs.x, y / rhs.y);}
		Point operator/(const int d){return Point(x / d, y / d);}
		Point operator-(const Point &rhs){return Point(x - rhs.x, y - rhs.y);}
		Point operator+(const Point &rhs){return Point(x + rhs.x, y + rhs.y);}
		Point operator-(const int &rhs){return Point(x - rhs, y - rhs);}
		Point operator+(const int &rhs){return Point(x + rhs, y + rhs);}

		float Distance(Point p) { return sqrtf( (float)((p.x-x)*(p.x-x)+(p.y-y)*(p.y-y)) ); }
		bool inRect(RECT r){if(x < r.left || x > r.right || y < r.top || y > r.bottom)return false;else return true;}

		void Set(int _x, int _y){x = _x; y = _y;}
		int x,y;
};
