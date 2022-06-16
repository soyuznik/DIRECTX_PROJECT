#pragma once

struct Vertex
{
	D3DXVECTOR3 position, normal;
	D3DXVECTOR2 tex0;	
	
	static IDirect3DVertexDeclaration9* Decl;
};

struct VertexPos
{
	D3DXVECTOR3 position;		

	static IDirect3DVertexDeclaration9* Decl;
};

struct VertexNormal 
{
	D3DXVECTOR3 position, normal;
	
	static IDirect3DVertexDeclaration9* Decl;
};

struct VertexTexture
{
	D3DXVECTOR3 position;
	D3DXVECTOR2 tex0;
	
	static IDirect3DVertexDeclaration9* Decl;
};


struct VertexTransformed
{
	D3DXVECTOR4 position;		
	D3DCOLOR    color;

	static IDirect3DVertexDeclaration9* Decl;
};

struct VertexNormalMap
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 tangent;
	D3DXVECTOR3 binormal;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 tex0;

	static IDirect3DVertexDeclaration9* Decl;
};

struct VertexTerrain 
{
	VertexTerrain() { }
	VertexTerrain(D3DXVECTOR3 pos, D3DCOLOR col, D3DXVECTOR2 _uv)
	{
		position = pos;
		color = col;
		normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		tex0 = _uv;
	}

	D3DXVECTOR3 position, normal;
	D3DCOLOR    color;
	D3DXVECTOR2 tex0;	
	D3DXVECTOR2 tex1;
	
	static IDirect3DVertexDeclaration9* Decl;
};

struct VertexParticle
{
	D3DXVECTOR3 position;	
	float       pointsize; // Must ALWAYS come after the position
	D3DCOLOR    color;
};

#define VERTEX_FVF					( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )
#define VERTEX_FVF_SIZE	 D3DXGetFVFVertexSize( VERTEX_FVF )

#define D3DFVF_VERTEX				( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )
#define D3DFVF_VERTEXTRANSFORMED	( D3DFVF_XYZRHW | D3DFVF_DIFFUSE )
#define D3DFVF_VERTEXNORMAL			( D3DFVF_XYZ | D3DFVF_NORMAL )
#define D3DFVF_VERTEXPOSITION		( D3DFVF_XYZ )
#define D3DFVF_VERTEXTERRAIN		( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 )
#define D3DFVF_VERTEXPARTICLE		( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_PSIZE )

void CreateAllVertexDeclarations( LPDIRECT3DDEVICE9 pDevice );
void DestroyAllVertexDeclarations();