#include "dxut.h"
#include "Vertex.h"

// Initialize static variables.
IDirect3DVertexDeclaration9* Vertex::Decl = 0;
IDirect3DVertexDeclaration9* VertexPos::Decl = 0;
IDirect3DVertexDeclaration9* VertexNormalMap::Decl = 0;
IDirect3DVertexDeclaration9* VertexTerrain::Decl = 0;
IDirect3DVertexDeclaration9* VertexTexture::Decl = 0;

void CreateAllVertexDeclarations( LPDIRECT3DDEVICE9 pDevice )
{
	LOG_DEBUG("Initializing vertex declarations...");
	
	D3DVERTEXELEMENT9 VertexElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},		
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},						
		D3DDECL_END()
	};	
	pDevice->CreateVertexDeclaration(VertexElements, &Vertex::Decl);

	D3DVERTEXELEMENT9 VertexPosElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},		
		D3DDECL_END()
	};	
	pDevice->CreateVertexDeclaration(VertexPosElements, &VertexPos::Decl);
	
	D3DVERTEXELEMENT9 VertexTextureElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};	
	pDevice->CreateVertexDeclaration(VertexTextureElements, &VertexTexture::Decl);

	D3DVERTEXELEMENT9 VertexNormalMapElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
		{0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
		{0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 48, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},	
		D3DDECL_END()
	};	
	pDevice->CreateVertexDeclaration(VertexNormalMapElements, &VertexNormalMap::Decl);
}

void DestroyAllVertexDeclarations()
{
	LOG_DEBUG("Destroying vertex declarations...");
	
	SAFE_RELEASE(VertexPos::Decl);	
	SAFE_RELEASE(Vertex::Decl);	
	SAFE_RELEASE(VertexTexture::Decl);
	SAFE_RELEASE(VertexNormalMap::Decl);
	SAFE_RELEASE(VertexTerrain::Decl)
}
