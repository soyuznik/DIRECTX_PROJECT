//-----------------------------------------------------------------------------
//           Name: dx9_texture.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to texture geometry with 
//                 Direct3D.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>  
#include <string>
#include <Psapi.h>
#include "mmsystem.h"


#define SHOWERROR(s,f,l)	char buf[1024]; sprintf( buf, "File: %s\nLine: %d\n%s",f,l,s); MessageBox( 0, buf, "Error", 0 );
#define SAFE_DELETE(x)		if( x ) { delete(x); (x) = NULL; }
#define SAFE_RELEASE(x) if( x ) { (x)->Release(); (x) = NULL; }
#define SAFE_DELETE_ARRAY(x) if( x ) { delete [] (x); (x) = NULL; }

using namespace std;

LPD3DXFONT m_font;
string str_hud ;
  
  
#define PI 3.14159265

LPDIRECT3DDEVICE9       g_pd3dDevice    = NULL;

#define ___DIRECTX 
#define  GLuint UINT
#define  GLdouble double
#define  GLubyte byte

#include "timer.h"
#include "vector3.h"

#include "functions.h"
#include "maptree.h"
#include "octree.h"
#pragma comment(lib, "legacy_stdio_definitions.lib")
void InitResources( void );

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HANDLE					hProcess		= OpenProcess(PROCESS_QUERY_INFORMATION |PROCESS_VM_READ, FALSE, GetCurrentProcessId() );
HWND                    g_hWnd          = NULL;
LPDIRECT3D9             g_pD3D          = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL;
LPDIRECT3DTEXTURE9      g_pTexture      = NULL;
LPDIRECT3DTEXTURE9      g_pTexture_Particle      = NULL;
LPD3DXEFFECT            g_pEffect       = NULL;

LPD3DXMESH				g_pTeapotMesh = NULL;
D3DMATERIAL9			g_teapotMtrl ;

D3DXMATRIX g_matWorld;
D3DXMATRIX g_matView;
D3DXMATRIX g_matProj;
D3DLIGHT9    g_pLight0,g_pLight1;
int gWindowScreenX , gWindowScreenY ; 

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

int curr_shad = 0;

ID3DXMatrixStack * stack = NULL;


#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_TEX1 )


#include "Model_x.h"
#include "xAnimator\XfileEntity.h"

#include "particles.h"
#include "Maths.h"
#include "objects.h"

#include "resourceManager.h"

#include "player.h"
Player *player;

#include "enemy.h"
vector<Enemy*>enems;

Gun gun;
bool gKeys[256];
bool gSpKeys[256];
bool gMouseButton[5];

Enemy *enem;

Triangle *worldMap; int totalMapTris;

float lx=0.0f,lz=-1.0f;
float x=0.0f, z=0.0f,y=0.0f;

LPDIRECT3DTEXTURE9      g_pTextureCrosshair      = NULL;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void init(void);
void shutDown(void);
void render(void);

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
	WNDCLASSEX winClass; 
	MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));

	gWindowScreenX =  GetSystemMetrics( SM_CXSCREEN );
	gWindowScreenY =  GetSystemMetrics( SM_CYSCREEN );
    
	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = LoadIcon(hInstance, 0);
    winClass.hIconSm	   = LoadIcon(hInstance, 0);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                             "Direct3D (DX9) - Texturing",
						      WS_POPUP,
					         0, 0, gWindowScreenX, gWindowScreenY, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
		    render();
	}

	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return uMsg.wParam;
}

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
	static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bMousing;

    switch( msg )
	{	
        case WM_KEYDOWN:
		{
			gKeys[ wParam ] = true;
			//MessageBox(0,0,to_string( wParam).c_str(),0);
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

				case 'W' :
						curr_shad = ++curr_shad  % 3;
					break;
			}
		}
        break;

		case WM_KEYUP:
		{
			gKeys[ wParam ] = false;
		}
		break;
		case WM_LBUTTONDOWN:
		{
			gMouseButton[0] = true;
		}
		break;

		case WM_LBUTTONUP:
			{
				gMouseButton[0] = false;
			}
			break;

		case WM_MOUSEMOVE:
			{
				ptCurrentMousePosit.x = LOWORD (lParam);
				ptCurrentMousePosit.y = HIWORD (lParam);

				static float angle_y =0;
				static int last_x=LOWORD (lParam),last_y=HIWORD (lParam);
	
				if( LOWORD (lParam)==last_x && HIWORD (lParam)==last_y )
					break;

				int diff_x = (ptCurrentMousePosit.x - ptLastMousePosit.x);
				int diff_y = ptCurrentMousePosit.y - ptLastMousePosit.y;

				if(!player ) break;

				player->angle += (diff_x ) *  .005;
				angle_y -= (diff_y)*.005;

				player->dir.x = sin( player->angle );
				player->dir.z = cos( player->angle );
				player->dir.y = tan(angle_y );

				ptLastMousePosit.x = ptCurrentMousePosit.x;
				ptLastMousePosit.y = ptCurrentMousePosit.y;

				if( diff_x  || diff_y  ) // lets limit it
				{
				   RECT rc ; GetClientRect( g_hWnd , &rc );
				   POINT p = {(rc.right - rc.left)/2 , (rc.bottom - rc.top)/2}; ClientToScreen( g_hWnd , &p );
				   SetCursorPos( p.x ,p.y );

				   ptLastMousePosit.x = (rc.right - rc.left)/2 ;
				   ptLastMousePosit.y = (rc.bottom - rc.top)/2 ;
				}
			}
		break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}
		
        case WM_DESTROY:
		{
            PostQuitMessage(0);
		}
        break;

		default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------

Model_X *model , *model_Gun , *model_Map; 
CXFileEntity *Anim_model; 




void InitDx()
{
	// Setup a material for the teapot
    ZeroMemory( &g_teapotMtrl, sizeof(D3DMATERIAL9) );

    g_teapotMtrl.Diffuse.r = 0.1f;
    g_teapotMtrl.Diffuse.g = 0.1f;
    g_teapotMtrl.Diffuse.b = 0.1f;
    g_teapotMtrl.Diffuse.a = 1.0f;

	g_teapotMtrl.Ambient.r = 0.1f;
    g_teapotMtrl.Ambient.g = 0.1f;
    g_teapotMtrl.Ambient.b = 0.1f;
    g_teapotMtrl.Ambient.a = 1.0f;

	g_teapotMtrl.Emissive.r = 0.f;
    g_teapotMtrl.Emissive.g = .1f;
    g_teapotMtrl.Emissive.b = .1f;
    g_teapotMtrl.Emissive.a = 1.0f;


	 // Setup a simple directional light and some ambient...
    g_pLight0.Type = D3DLIGHT_DIRECTIONAL;
	g_pLight0.Direction = D3DXVECTOR3( -player->dir(0),player->dir(1)-.5,-player->dir(2) );
	//g_pLight0.Type = D3DLIGHT_POINT;
	//g_pLight0.Position =  D3DXVECTOR3( player->pos(0),player->pos(1),player->pos(2) );
	g_pLight0.Range = 200.f;
	g_pLight0.Attenuation0 = 1.f;
	/*g_pLight0.Attenuation1 = 1.f;
	g_pLight0.Attenuation2 = 1.f;*/

    g_pLight0.Diffuse.r = 1.0f;
    g_pLight0.Diffuse.g = 1.0f;
    g_pLight0.Diffuse.b = 1.0f;
    g_pLight0.Diffuse.a = 1.0f;
	g_pLight0.Ambient.r = .5f;
    g_pLight0.Ambient.g = .5f;
    g_pLight0.Ambient.b = .5f;
    g_pLight0.Ambient.a = .5f;
	g_pLight0.Specular.r = 0.f;
    g_pLight0.Specular.g = 0.f;
    g_pLight0.Specular.b = .5f;
    g_pLight0.Specular.a = .5f;

    g_pd3dDevice->SetLight( 0, &g_pLight0 );
    g_pd3dDevice->LightEnable( 0, TRUE );

	  // Fill in a light structure defining our light
	D3DLIGHT9 g_pLight1;
	ZeroMemory( &g_pLight1, sizeof(D3DLIGHT9) );
	g_pLight1.Type       = D3DLIGHT_DIRECTIONAL;
	g_pLight1.Diffuse.r  = 0.0f;
	g_pLight1.Diffuse.g  = 0.0f;
	g_pLight1.Diffuse.b  = 1.0f;
	g_pLight1.Diffuse.a  = 1.0f;
	g_pLight0.Ambient.r = 1.f;
	g_pLight1.Ambient.g = 0.f;
	g_pLight1.Ambient.b = 0.f;
	g_pLight1.Ambient.a = .5f;
	g_pLight1.Specular.r = 1.f;
	g_pLight1.Specular.g = 0.f;
	g_pLight1.Specular.b = .5f;
	g_pLight1.Specular.a = .5f;
   
	  // Spot g_pLight1s have direction and a position
	  g_pLight1.Position = D3DXVECTOR3(player->pos(0),player->pos(1),player->pos(2));
	  g_pLight1.Direction = D3DXVECTOR3( -player->dir(0),player->dir(1),-player->dir(2)  );
   
	  // Tell the device about the g_pLight1 and turn it on

	  g_pLight1.Attenuation0= 0.1f;

	  g_pd3dDevice->SetLight( 1, &g_pLight1 );
	  g_pd3dDevice->LightEnable( 1, FALSE ); 


	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE , D3DCULL_NONE  );
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );

	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.2f, 0.2f, 0.2f, 1.0f ) );

}

void init( void )
{
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    D3DDISPLAYMODE d3ddm;

    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
	
    d3dpp.Windowed               = TRUE;
	d3dpp.MultiSampleType		= D3DMULTISAMPLE_NONE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.PresentationInterval   =  /*D3DPRESENT_INTERVAL_IMMEDIATE/*/D3DPRESENT_INTERVAL_ONE;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;    // set the back buffer format to 32-bit
    d3dpp.BackBufferWidth = gWindowScreenX;    // set the width of the buffer
    d3dpp.BackBufferHeight = gWindowScreenY;    // set the height of the buffer

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                          D3DCREATE_HARDWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );

	InitResources();
	//initEffect(  );

    D3DXMatrixPerspectiveFovLH( &g_matProj, D3DXToRadian( 60.0f ), 
                                1.0f , 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &g_matProj );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    if( g_pTexture != NULL ) 
        g_pTexture->Release();

    if( g_pVertexBuffer != NULL ) 
        g_pVertexBuffer->Release(); 

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

Object *obj;
vector<Triangle> MapTriangles;
Octree map_octree;
void InitResources( void )
{
	D3DXCreateTextureFromFile( g_pd3dDevice, "test.bmp", &g_pTexture );

	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);


	D3DXCreateTextureFromFile( g_pd3dDevice, "test.bmp", &g_pTexture );

	//D3DXCreateTextureFromFile( g_pd3dDevice, "crosshair.bmp", &g_pTextureCrosshair );
	D3DXCreateTextureFromFileExA(g_pd3dDevice,			//device
                             "crosshair.bmp",        //file name
                             D3DX_DEFAULT,      //width
                             D3DX_DEFAULT,      //height
                             D3DX_DEFAULT,      //mip levels
                             NULL,              //usage
                             D3DFMT_UNKNOWN,    //texture color format
                             D3DPOOL_MANAGED,   //memory class
                             D3DX_DEFAULT,      //filter
                             D3DX_DEFAULT,      //mip filter
                             0xff000000,        //color key
                             NULL,              //source info
                             NULL,              //pallette
                             &g_pTextureCrosshair);    //texture object

	D3DXCreateTextureFromFile( g_pd3dDevice, "particle.bmp", &g_pTexture_Particle );



	model = new Model_X(g_pd3dDevice );
	//model->LoadXFile( "teapot.x" );
	//model->Scale(.01,.01,.01);

	 //D3DXLoadMeshFromX( "teapot.x", D3DXMESH_SYSTEMMEM,g_pd3dDevice ,  NULL, NULL, NULL, NULL, &g_pTeapotMesh );

	Anim_model = new CXFileEntity(g_pd3dDevice );

	char DirPath[256];GetCurrentDirectoryA(255,DirPath );
	
	//if(!Anim_model->Load( string(string(DirPath)+"\\Models\\Tiny\\tiny.x" ).c_str()) )
	//if(!Anim_model->Load( "C:\\Users\\dheeraj.patni\\Desktop\\DirectX C++\\Models\\soldier\\soldier.x" ))
	{
		//MessageBox(0,string(string(DirPath)+"\\Models\\Tiny\\tiny.x" ).c_str(),"Error",0);
		//exit(0);
	}
	//Anim_model->SetAnimationSet(7);

	model_Gun = new Model_X( g_pd3dDevice );
	model_Gun->LoadXFile( "Models/Gun1/1st Person.x");

	D3DXCreateMatrixStack(0, &stack);

	//D3DXCreateFontA( g_pd3dDevice , 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &m_font );
	createFont(g_pd3dDevice , m_font);

	obj = new Object( g_pd3dDevice );
	obj->Load( g_cubeVertices , g_cubeIndices , g_cubeVertices->FVF_Flags , sizeof( g_cubeVertices ) ,sizeof( g_cubeIndices ) , sizeof( g_cubeVertices[0]) );

	//New   ----------------------- 
	player = new Player();
	player->dir.set( -1,0,0,0);
	player->pos.set( 0,10.0f,1,0);
	player->lhw = Vector3( .5,1.0f,.5f );
	player->gunModel = model_Gun ;

	for(int i=0;i<1;i++)
	{
		Enemy *enem = new Enemy (  (char*)string(string(DirPath)+"\\Models\\soldier\\soldier.x").c_str() , "Data\\models_md2\\igdosh.pcx",  "Models/Gun1/3rd Person.x" , "Data\\models_md2\\Weapon.pcx" );
		enem->Scale( Vector3(.007,.008,.007) );
		//enem->pos =  Vector3( i*2,1.f,i);
		enem->pos =  Vector3( i*2,1.f,i+5);
		enem->lhw =  Vector3(.2,0.7,.2 );
		enems.push_back(enem);
	}

	model_Map = new Model_X( g_pd3dDevice );
	model_Map->LoadXFile( "Models/level1/map.x");
	model_Map->Scale(.01,.01,.01);
	//model_Map->LoadXFile( "Models/room/room.x");
	//model_Map->Scale(1,1,1);

	model_Map->GetVerticesX( model_Map->g_pMesh );
	model_Map->GetIndices( model_Map->g_pMesh );

	map_octree.Create( g_pd3dDevice , model_Map );
}

VOID SetupMatrices()
{
    // For our world matrix, we will just rotate the object about the y-axis.
    D3DXMATRIXA16 matWorld;
	D3DXMatrixTranslation( &matWorld ,0,0,0 );

    D3DXVECTOR3 vEyePt( player->pos.x, player->pos.y, player->pos.z);
    D3DXVECTOR3 vLookatPt( player->pos.x+player->dir.x, player->pos.y+player->dir.y,  player->pos.z+player->dir.z );
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    // For the projection matrix, we set up a perspective transform (which
    // transforms geometry from 3D view space to 2D viewport space, with
    // a perspective divide making objects smaller in the distance). To build
    // a perpsective transform, we need the field of view (1/4 pi is common),
    // the aspect ratio, and the near and far clipping planes (which define at
    // what distances geometry should be no longer be rendered).
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI /4 , 1.0f, .1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	g_matWorld = matWorld; 
	g_matView = matView; 
	g_matProj = matProj; 
}

void render( void )
{
	static int counter = 0; counter++;

    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,.3f,.3f,1.0f), 1.0f, 0 );

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(hProcess,&pmc,sizeof(pmc));
	char str[500];sprintf(str,"fps : %d \nProcess Memory : %d K",CalculateFPS(),pmc.WorkingSetSize/1024);
	PushString( str );

    g_pd3dDevice->BeginScene();
	{
		InitDx();
		SetupMatrices();


			model_Map->Draw();

		 
		 
		//player
			player->PressedKeysHandler(gKeys,gSpKeys);
			player->MouseKeyHandler(gMouseButton);
			player->ApplyGravity();
			//player->checkCollosionWithMap( worldMap , totalMapTris );
			//player->checkCollosionWithMap( &MapTriangles[0] , MapTriangles.size() );
			player->checkCollosionWithMap( map_octree );

			//Draw gun and bullets
				player->gun->SetPosAndDir( Vector3( player->pos.x + (player->dir.x*.3) ,player->pos.y + (player->dir.y*.3)   , player->pos.z + (player->dir.z*.3) ) , player->dir );
				player->gun->Draw( );
				player->gun->StepUpBullets();
			//End
			PushString( string() + "Player-Health:" + to_string( player->health )  );
			PushString( string()+"Bullets Counter :" + to_string(player->gun->bullets.size()));
	//	//End

	//enemy bullets collide to wall
		//for( int i=0; i < totalMapTris ;i++)
			for( int j=0; j < enems.size() ;j++)
				for( int k=0;k<enems[j]->gun->bullets.size() ;k++)
					enems[j]->gun->bullets[k]->checkCollosionWithMap( map_octree );
	//End

	//Draw MD2 Models
		bool bAnimated=true;
		for(int i=0;i<enems.size();i++)
		{
			//If enemy died
			if( enems[i]->died_time >0)
			{
				enems[i]->StepDieTimer();
				if( enems[i]->died_time >=1 )
				{
					enems.erase( enems.begin() + i );
					i--;
					continue;
				}
			}
			//End

			//enemy StepUp
				enems[i]->dir = (player->pos - enems[i]->pos).normalize();
				if( player->pos.GetDistance(enems[i]->pos) >=5.0f)
				{
					enems[i]->Run();
				}
				else if( enems[i]->died_time <=0)
				{
					enems[i]->gun->dir = (player->pos -  enems[i]->gun->pos).normalize();
					enems[i]->gun->fire();
					enems[i]->gun->StepUpBullets();
					enems[i]->gun->Draw();

					enems[i]->SetAnim(0);
				}
				enems[i]->ApplyGravity();
				//enems[i]->checkCollosionWithMap( worldMap , totalMapTris , enems[i]->lhw.x );
				enems[i]->checkCollosionWithMap( map_octree );

				g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, 0);
				enems[i]->Draw( 0  );
				g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, 1);

				PushString( string() + "Enemy-"+to_string(i)+" Health:" + to_string( enems[i]->health )  );
			//End

			//player collides with enemy
				if( player->pos.GetDistance(enems[i]->pos) <= 1.5 )
					player->pos=player->pos-(player->dir* player->speedOffset);
			//End

			//enemy collide with enemy
				for (int j=0;j<enems.size();j++)
				{
					if(i==j)continue;
					if( (enems[i]->pos).GetDistance(enems[j]->pos) <= .5 )
					{
						Vector3 tDir =  enems[i]->pos - enems[j]->pos;
						if ( enems[i]->dir.dot(tDir) < 0.f  )
						{
							enems[i]->pos=enems[i]->pos-(enems[i]->dir*enems[i]->speedOffset);
						}
					}
				}
			//End


			//player collide with enemy bullets
				for( int k=0;k<enems[i]->gun->bullets.size() ;k++)
				{
					float u = 0;bool nearest = 0;
					if( enems[i]->gun->bullets[k]->CollideToPlane(  Vector3(0,0,0)-enems[i]->gun->bullets[k]->dir  , player->pos , .3 , u ,nearest) )
					{
						enems[i]->gun->bullets[k]->min_distance_from_objs = u;
						player->health-=10;

						//change that bullet's intersectPoint and sprite  engine to view the effects..
						enems[i]->gun->bullets[k]->spriteEngine.mMaxAge = 50;
						enems[i]->gun->bullets[k]->intersectPoint_Ideal =  enems[i]->gun->bullets[k]->intersectPoint_Ideal - enems[i]->gun->bullets[k]->dir * .15f /* -(player->dir* .15f )*/;
						enems[i]->gun->bullets[k]->spriteEngine.mpColor = 0xff00ff;
						enems[i]->gun->bullets[k]->spriteEngine.mpSize = 0.01f;
						//enems[i]->gun->bullets[k]->spriteEngine.mMaxParticles = 10;

						//push the player
						player->dir =  player->dir +   Vector3().getRandom() * sin( Vector3().getRandom().x ) *.1;
						player->dir = player->dir.normalize();
						player->pos += enems[i]->gun->dir *.02 ; 

					}
				}
			//End

			//enemy Bullets removing
				for( int k=0;k<enems[i]->gun->bullets.size() ;k++)
				{
					if( enems[i]->gun->bullets[k]->min_distance_from_objs < abs(enems[i]->gun->bullets[k]->speed/2))
					{
						//SAFE_DELETE( enems[i]->gun->bullets[k] );
						//enems[i]->gun->bullets.erase( enems[i]->gun->bullets.begin()+k); 
						//k--;

						enems[i]->gun->bullets[k]->stop();
						enems[i]->gun->bullets[k]->size = 0.0f;
						enems[i]->gun->bullets[k]->DivideToParts();
					}
				}
			//End
		}
	//End MD2 Model 

	//enemy collide with player bullets
		
		//player bullets collide to wall
			for( int k=0;k<player->gun->bullets.size() ;k++)
			{
				player->gun->bullets[k]->checkCollosionWithMap( map_octree );
			}
		//End

		for( int k=0;k<player->gun->bullets.size() ;k++)
		{
			int nearest_enem_no = -1;
			for(int i=0;i<enems.size();i++)
			{
				Vector3 eDir = player->gun->bullets[k]->dir.negative(); eDir.y = 0;
				Polygen *p = enems[i]->getSurroundedPolygen( eDir );
				//((Rect*)(p))->Draw(); 
				
				float u = 0;bool nearest = false;
				//if( player->gun->bullets[k]->CollideToPlane(  player->gun->bullets[k]->dir.negative()  , enems[i]->pos , .5 , u ,nearest  ) )
				if( player->gun->bullets[k]->CollideWithPolygen( p ) )
				{
						nearest_enem_no = i;
				}
				delete p;
			}
			if( player->gun->bullets[k]->stopped==false && nearest_enem_no !=-1)
			{
				if( enems[nearest_enem_no]->health <= 0  )
				{
					enems[nearest_enem_no]->StepDieTimer();
					player->gun->bullets[k]->spriteEngine.mpSize = .02f;
				}
				else
				{
					enems[nearest_enem_no]->health -= 5;
					player->gun->bullets[k]->spriteEngine.mpColor = 0xff0000;
					player->gun->bullets[k]->spriteEngine.mpSize = 0.3f;
					player->gun->bullets[k]->spriteEngine.mMaxParticles = 10;
				}
			}
			if( player->gun->bullets[k]->min_distance_from_objs < abs(player->gun->bullets[k]->speed/2) )
			{
				player->gun->bullets[k]->stop();
				player->gun->bullets[k]->size = 0.0f;
				player->gun->bullets[k]->DivideToParts();
			}
		}
	//End

		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, 0);
		//Draw CrossHair
			stack->Push();
				g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
					D3DXMATRIX matWorld , matView = g_matView ; 
					D3DXMatrixInverse(&matWorld, 0, &matView);
			
					stack->Scale(.1,.1,.1);
					stack->Translate(0,0,5);   
					stack->MultMatrix(&matWorld);

					g_pd3dDevice->SetTransform( D3DTS_WORLD, stack->GetTop() ); 

					DrawRect(  g_pd3dDevice , g_pTextureCrosshair );
				g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

			stack->Pop();g_pd3dDevice->SetTransform(D3DTS_WORLD, stack->GetTop());

			g_pd3dDevice->SetTexture(0,0);
		//End

		//Draw Gun
			stack->Push();
			{
				g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

				D3DXMATRIX mat;
				stack->Scale(.015,.015,.015);

				stack->Translate(.3 ,-.6,1);
				D3DXMatrixMultiply(stack->GetTop(),stack->GetTop(),&matWorld); //multyply inverse view matrix
				stack->Translate(0,-.05 ,0);
				g_pd3dDevice->SetTransform( D3DTS_WORLD, stack->GetTop() );
					
				player->Draw();
				
				g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
			}stack->Pop();g_pd3dDevice->SetTransform(D3DTS_WORLD, stack->GetTop());
			g_pd3dDevice->SetTexture(0,0);
		//End
		//Draw Gun- Smoke
			stack->Push();
			{
				g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

				D3DXMATRIX mat;
				stack->Scale(.015,.015,.015);

				stack->Translate(.6 ,-.6,3.8);
				D3DXMatrixMultiply(stack->GetTop(),stack->GetTop(),&matWorld); //multyply inverse view matrix
					
				g_pd3dDevice->SetTransform( D3DTS_WORLD, stack->GetTop() );
					
				static Sprites s(100,300);
				if( counter==1 )
				{
					for(int i=0;i<100;i++)
					{
						Vector3 dir = Vector3(0,-1.98,0) + D3DVecToVector3(getRandomVector()).normalize()*10  ;
						Particle p( Vector3(0,0,0) , dir , 0 );  
						p.speed = .1;
						p.age = 100 ;
						s.parts.push_back(p);
					}
					s.mpSize = .3f;
				}
				if(  gMouseButton[0] ) 
				{
					for(int i=0;i<s.mMaxParticles;i++)
					{
						if( s.parts[i].pos.y <= -10.f || s.parts[i].pos.y >= 10.f || abs(s.parts[i].pos.x) >= 10.f )
							s.parts[i].age = s.mMaxAge;
					}
					s.Draw();
				}
				else
				{
					for(int i=0;i<s.mMaxParticles;i++)
						s.parts[i].age = s.mMaxAge;
				}
				g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
			}stack->Pop();g_pd3dDevice->SetTransform(D3DTS_WORLD, stack->GetTop());
			g_pd3dDevice->SetTexture(0,0);
		//End
	}
    g_pd3dDevice->EndScene();



	DrawString(m_font,5,5,D3DXCOLOR(1.f,1.f,1.f,1.f), str_hud );
	str_hud.clear();

    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

