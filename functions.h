using namespace std;
#include <sstream>
#pragma comment (lib, "DXErr.lib")

#define SHOWERROR(s,f,l)	char buf[1024]; sprintf( buf, "File: %s\nLine: %d\n%s",f,l,s); MessageBox( 0, buf, "Error", 0 );
#define SAFE_DELETE(x)		if( x ) { delete(x); (x) = NULL; }
#define SAFE_RELEASE(x) if( x ) { (x)->Release(); (x) = NULL; }
#define SAFE_DELETE_ARRAY(x) if( x ) { delete [] (x); (x) = NULL; }

template <class T>
inline std::string to_string(const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();  
}

//struct Vector3
//{
//	float x,y,z;
//};

void createFont( LPDIRECT3DDEVICE9       &g_pd3dDevice , LPD3DXFONT &m_font )
{
    //
    // To create a Windows friendly font using only a point size, an 
    // application must calculate the logical height of the font.
    // 
    // This is because functions like CreateFont() and CreateFontIndirect() 
    // only use logical units to specify height.
    //
    // Here's the formula to find the height in logical pixels:
    //
    //             -( point_size * LOGPIXELSY )
    //    height = ----------------------------
    //                          72
    //

    HRESULT hr;
    HDC hDC;
    //HFONT hFont;
    int nHeight;
    int nPointSize = 9;
    //char strFontName[] = "Arial";

    hDC = GetDC( NULL );

    nHeight = -( MulDiv( nPointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72 ) );

    ReleaseDC( NULL, hDC );


    // Create a font for statistics and help output
    hr = D3DXCreateFont( g_pd3dDevice, nHeight, 0, FW_BOLD, 0, FALSE, 
                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                         DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), 
                         &m_font );

    if( FAILED( hr ) )
        MessageBox(NULL,"Call to D3DXCreateFont failed!", "ERROR",MB_OK|MB_ICONEXCLAMATION);
}

void DrawString(LPD3DXFONT m_font,int left,int top,D3DXCOLOR color,string txt)
{
	//D3DCOLOR fontColor = D3DCOLOR_ARGB(255,0,0,255);   
	RECT destRect;
    SetRect( &destRect, left, top, 0, 0 );

	m_font->DrawText( NULL, txt.c_str() , -1, &destRect, DT_NOCLIP, color );
}

inline void PushString(string str)
{
	str_hud.append( str ).append("\n");
}


inline DWORD FtoDW( FLOAT f ) 
{ 
	return *((DWORD*)&f); 
}

inline float getRandomMinMax( float fMin, float fMax )
{
    float fRandNum = (float)rand () / RAND_MAX;
    return fMin + (fMax - fMin) * fRandNum;
}

//-----------------------------------------------------------------------------
// Name: getRandomVector()
// Desc: Generates a random vector where X,Y, and Z components are between
//       -1.0 and 1.0
//-----------------------------------------------------------------------------
D3DXVECTOR3 getRandomVector( void )
{
	D3DXVECTOR3 vVector;

    // Pick a random Z between -1.0f and 1.0f.
    vVector.z = getRandomMinMax( -1.0f, 1.0f );
    
    // Get radius of this circle
    float radius = (float)sqrt(1 - vVector.z * vVector.z);
    
    // Pick a random point on a circle.
    float t = getRandomMinMax( -D3DX_PI, D3DX_PI );

    // Compute matching X and Y for our Z.
    vVector.x = (float)cosf(t) * radius;
    vVector.y = (float)sinf(t) * radius;

	return vVector;
}

Vector3 D3DVecToVector3(D3DXVECTOR3 vec)
{
	return Vector3( vec.x , vec.y , vec.z ,1.0 ); 
}


struct VertexX
{
	float x, y, z;
	DWORD color;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_DIFFUSE
	};
};

VertexX g_cubeVertices[] =
{
	{-1.0f, 1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, // 0
	{ 1.0f, 1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) }, // 1
	{-1.0f,-1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) }, // 2
	{ 1.0f,-1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) }, // 3
	{-1.0f, 1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 1.0, 1.0 ) }, // 4
	{-1.0f,-1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) }, // 5
	{ 1.0f, 1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 1.0, 1.0 ) }, // 6
	{ 1.0f,-1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }  // 7 
};

WORD g_cubeIndices[] =
{
	0, 1, 2,	2,3,0, // Quad 0
	4, 5, 6,	6,7,4, // Quad 1
	4, 6, 0,	0,1,4, // Quad 2
	5, 2, 7,	7,3,5, // Quad 3
	1, 6, 3,	3,7,1, // Quad 4
	0, 2, 4,	4,5,0,  // Quad 5
};



class Object
{
public:
	LPDIRECT3DDEVICE9  g_pd3dDevice;
	
	LPDIRECT3DVERTEXBUFFER9 m_v_buffer;  
	LPDIRECT3DINDEXBUFFER9 m_i_buffer;  

	int m_vertices_size;
	int m_indices_size;
	int m_per_vertice_size;

	DWORD m_fvf;

	Object(LPDIRECT3DDEVICE9  g_pd3dDevice)
	{
		this->g_pd3dDevice=g_pd3dDevice;
	}
	
	void Load( void *verticesX , void *indicesX , DWORD FVF , UINT vertices_size , UINT indices_size , UINT per_vertices_size )
	{
		this->m_vertices_size = vertices_size;
		this->m_indices_size =  indices_size;
		this->m_fvf = FVF;
		this->m_per_vertice_size =  per_vertices_size ;

		if ( FAILED( g_pd3dDevice->CreateVertexBuffer(vertices_size,
								D3DUSAGE_WRITEONLY,
								FVF,
								D3DPOOL_DEFAULT,
								&m_v_buffer,
								NULL) ) )
		{
			SHOWERROR( "CreateVertexBuffer failed.", __FILE__, __LINE__ );
		}
		
		VOID* pVoid; 

		m_v_buffer->Lock(0, 0, (void**)&pVoid, 0);
			memcpy(pVoid, verticesX, vertices_size);
		m_v_buffer->Unlock();
		
		if( indicesX != NULL )
		{
			g_pd3dDevice->CreateIndexBuffer(indices_size*sizeof(short),
									  0,
									  D3DFMT_INDEX16,
									  D3DPOOL_MANAGED,
									  &m_i_buffer,
									  NULL);

			m_i_buffer->Lock(0, 0, (void**)&pVoid, 0);
				memcpy(pVoid, indicesX, indices_size);
			m_i_buffer->Unlock();
		}
	}
	void Draw( D3DPRIMITIVETYPE primitiveType )  
	{
		g_pd3dDevice->SetFVF( this->m_fvf );
		g_pd3dDevice->SetStreamSource(0, m_v_buffer, 0, this->m_per_vertice_size );
		if( this->m_indices_size > 0 )
		{
			g_pd3dDevice->SetIndices(m_i_buffer);
			g_pd3dDevice->DrawIndexedPrimitive(primitiveType, 0, 0, m_vertices_size, 0, m_indices_size/3);
		}
		else
		{
			//DrawString(m_font,0,30,D3DXCOLOR(1,1,1,1),to_string(this->m_vertices_size).c_str() );
			g_pd3dDevice->DrawPrimitive( primitiveType, 0, (this->m_vertices_size/ this->m_per_vertice_size ) / 3 );
			//g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 6 );
		}
	}
};


void DrawRect( LPDIRECT3DDEVICE9  g_pd3dDevice  ,LPDIRECT3DTEXTURE9 texture=NULL )
{

	struct D3DVERTEX_TEX
				{
					float	fX,fY,fZ;
					float	fU,fV;
				};
				#define D3DFVF_CUSTOMVERTEX	D3DFVF_XYZ|D3DFVF_TEX1 

				D3DVERTEX_TEX g_quadVertices[] =
				{
					{-1.0f, 1.0f, 0.0f,  0.0f,0.0f },
					{ 1.0f, 1.0f, 0.0f,  1.0f,0.0f },
					{-1.0f,-1.0f, 0.0f,  0.0f,1.0f },
					{ 1.0f,-1.0f, 0.0f,  1.0f,1.0f }
				};

				g_pd3dDevice->SetTexture(0,texture);

				//z-buffer disabled
				g_pd3dDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE);

				//alpha blending enabled
				g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true);
				//source blend factor
				g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
				//dest blend factor
				g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

				g_pd3dDevice->SetFVF( D3DFVF_XYZ|D3DFVF_TEX1  );
				g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,        //PrimitiveType
											2,						//PrimitiveCount
											g_quadVertices,              //pVertexStreamZeroData
											sizeof(D3DVERTEX_TEX));   //VertexStreamZeroStride
}

int CalculateFPS()
{
	static int iCurrentTick = 0,  iFps = 0, iFrames = 0;
	static int iStartTick = GetTickCount();

	iFrames++;
    iCurrentTick = GetTickCount();
	if ((iCurrentTick - iStartTick) >= 1000) 
	{
		iFps = (int)((float)iFrames/(iCurrentTick-iStartTick)*1000.0f);
		iFrames = 0;
		iStartTick = iCurrentTick;
	}
	return iFps;
}

BOOL SearchFolders( char* filename, char* exeFolder, char* exeName, char fullPath[] )
{
    char* searchFolders[] = 
    { 
        ".\\", "..\\", "..\\..\\", "%s", "%s..\\", "%s..\\..\\", "%s..\\%s", "%s..\\..\\%s"
    };

    // Look through each folder to find the file
    char currentPath[MAX_PATH] = {0};
    for ( int i = 0; i < 8; i++ )
    {
        sprintf( currentPath, searchFolders[i], exeFolder, exeName );
        strcat( currentPath, filename );
        if ( GetFileAttributes( currentPath ) != INVALID_FILE_ATTRIBUTES )
        {
            strcpy( fullPath, currentPath );
            return TRUE;
        }
    }

    // Crap...didn't find it
    return FALSE;
}

bool GetMediaFile( char* file, OUT char path[] )
{
    char exeName[MAX_PATH] = {0};
    char exeFolder[MAX_PATH] = {0};

    // Get full executable path
    GetModuleFileName( NULL, exeFolder, MAX_PATH );
    exeFolder[MAX_PATH - 1] = 0;

    // Get pointer to beginning of executable file name
    // which is after the last slash
    char* pCutPoint = NULL;
    for ( int i = 0; i < MAX_PATH; i++ )
    {
        if ( exeFolder[i] == '\\' )
        {
            pCutPoint = &exeFolder[i + 1];
        }
    }

    if ( pCutPoint )
    {
        // Copy over the exe file name
        strcpy( exeName, pCutPoint );

        // Chop off the exe file name from the path so we
        // just have the exe directory
        *pCutPoint = 0;

        // Get pointer to start of the .exe extension 
        pCutPoint = NULL;
        for ( int i = 0; i < MAX_PATH; i++ )
        {
            if ( exeName[i] == '.' )
            {
                pCutPoint = &exeName[i];
            }
        }
        // Chop the .exe extension from the exe name
        if ( pCutPoint )
        {
            *pCutPoint = 0;
        }

        // Add a slash
        strcat( exeName, "\\" );
    }

    // Search all the folders in searchFolders
    if ( SearchFolders( file, exeFolder, exeName, path ) )
    {
        return TRUE;
    }

    // Search all the folders in searchFolders with media\ appended to the end
    char mediaFile[MAX_PATH] = "media\\";
    strcat( mediaFile, file );
    if ( SearchFolders( mediaFile, exeFolder, exeName, path ) )
    {
        return TRUE;
    }

    return FALSE;
}
