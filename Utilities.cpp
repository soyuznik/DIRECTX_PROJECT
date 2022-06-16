#include "Utilities.h"

#include <vector>
#include <fstream>
#include "vertex.h"

#define COLLISION_RADIUS 0.0f
LPD3DXFONT gProgressFont = NULL;

#define LOG(m) 
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

/****************************************************************************

RandomNumber: returns a random number between iMin and iMax.

****************************************************************************/
int RandomNumber(int iMin, int iMax)
{
	if (iMin == iMax) return(iMin);
	return((rand() % (abs(iMax-iMin)+1))+iMin);
}

float RandomNumber(float fMin, float fMax)
{
	if (fMin == fMax) return(fMin);
	float fRandom = (float)rand() / (float)RAND_MAX;
	return((fRandom * (float)fabs(fMax-fMin))+fMin);
}

D3DXVECTOR3 RandomNumber(D3DXVECTOR3 vMin, D3DXVECTOR3 vMax)
{
	float x = RandomNumber(vMin.x, vMax.x);
	float y = RandomNumber(vMin.y, vMax.y);
	float z = RandomNumber(vMin.z, vMax.z);
	return(D3DXVECTOR3(x,y,z));
}

D3DXCOLOR RandomNumber(D3DXCOLOR Min, D3DXCOLOR Max)
{
	float r = RandomNumber(Min.r, Max.r);
	float g = RandomNumber(Min.g, Max.g);
	float b = RandomNumber(Min.b, Max.b);
	float a = RandomNumber(Min.a, Max.a);
	return(D3DXCOLOR(r,g,b,a));
}

char * ConvertString( char *out, WCHAR *in )
{
	size_t convertedChars = 0;
	size_t origsize = wcslen(in) + 1;

	wcstombs_s(&convertedChars, out, origsize, in, _TRUNCATE);
	return out;
}

WCHAR * ConvertString( WCHAR *out, char *in )
{
	size_t convertedChars = 0;
	size_t origsize = strlen(in) + 1;

	mbstowcs_s(&convertedChars, out, origsize, in, _TRUNCATE);
	return out;
}

WCHAR * ConvertString( char *in )
{
	static WCHAR out[512];

	size_t convertedChars = 0;
	size_t origsize = strlen(in) + 1;

	mbstowcs_s(&convertedChars, out, origsize, in, _TRUNCATE);
	return out;
}


HRESULT ResetGlobalVariables(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;
	
	(gProgressFont->OnResetDevice());

	return S_OK;
}

void LostGlobalVariables()
{
	gProgressFont->OnLostDevice();
}

void DestroyGlobalVariables()
{	
	SAFE_RELEASE( gProgressFont );
}



char * Commify(double f)
{
	static char s[1000];
	char *p;
	sprintf_s(s,"%f",f);
	p = s+strlen(s);
	while( *--p=='0' ){ *p='\0'; };
	if( *p=='.' ){ *p='\0'; }
	p = strrchr(s,'.');
	if( p == 0 ){ p = s+strlen(s); }
	while( (p-=3) > s ){
		memmove(p+1,p,strlen(p));
		*p=',';
	}
	return s;
}


#include <ddraw.h>

typedef HRESULT ( WINAPI* LPDIRECTDRAWCREATE )( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );

struct DDRAW_MATCH
{
	GUID guid;
	HMONITOR hMonitor;
	CHAR strDriverName[512];
	bool bFound;
};


//-----------------------------------------------------------------------------
BOOL WINAPI DDEnumCallbackEx( GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm )
{
	DDRAW_MATCH* pDDMatch = ( DDRAW_MATCH* ) lpContext;
	if( pDDMatch->hMonitor == hm )
	{
		pDDMatch->bFound = true;
		strcpy_s( pDDMatch->strDriverName, 512, lpDriverName );
		memcpy( &pDDMatch->guid, lpGUID, sizeof( GUID ) );
	}
	return TRUE;
}


//-----------------------------------------------------------------------------

HRESULT GetVideoMemoryViaD3D9( HMONITOR hMonitor, UINT* pdwAvailableTextureMem )
{
    HRESULT hr;
    bool bGotMemory = false;
    *pdwAvailableTextureMem = 0;

    IDirect3D9* pD3D9 = NULL;
    pD3D9 = Direct3DCreate9( D3D_SDK_VERSION );
    if( pD3D9 )
    {
        UINT dwAdapterCount = pD3D9->GetAdapterCount();
        for( UINT iAdapter = 0; iAdapter < dwAdapterCount; iAdapter++ )
        {
            IDirect3DDevice9* pd3dDevice = NULL;

            HMONITOR hAdapterMonitor = pD3D9->GetAdapterMonitor( iAdapter );
            if( hMonitor != hAdapterMonitor )
                continue;

            HWND hWnd = GetDesktopWindow();

            D3DPRESENT_PARAMETERS pp;
            ZeroMemory( &pp, sizeof( D3DPRESENT_PARAMETERS ) );
            pp.BackBufferWidth = 800;
            pp.BackBufferHeight = 600;
            pp.BackBufferFormat = D3DFMT_R5G6B5;
            pp.BackBufferCount = 1;
            pp.MultiSampleType = D3DMULTISAMPLE_NONE;
            pp.MultiSampleQuality = 0;
            pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            pp.hDeviceWindow = hWnd;
            pp.Windowed = TRUE;

            pp.EnableAutoDepthStencil = FALSE;
            pp.Flags = 0;
            pp.FullScreen_RefreshRateInHz = 0;
            pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

            hr = pD3D9->CreateDevice( iAdapter, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &pd3dDevice );
            if( SUCCEEDED( hr ) )
            {
                *pdwAvailableTextureMem = pd3dDevice->GetAvailableTextureMem();
                bGotMemory = true;
                SAFE_RELEASE( pd3dDevice );
            }
        }

        SAFE_RELEASE( pD3D9 );
    }

    if( bGotMemory )
        return S_OK;
    else
        return E_FAIL;
}



#include <wbemidl.h>

typedef BOOL ( WINAPI* PfnCoSetProxyBlanket )( IUnknown* pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc,
                                               OLECHAR* pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel,
                                               RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities );

