#pragma once
using namespace std;
#include "vector3.h"
#include "boundingVolumes.h"

#define SHOWERROR(s,f,l)	char buf[1024]; sprintf( buf, "File: %s\nLine: %d\n%s",f,l,s); MessageBox( 0, buf, "Error", 0 );
#define SAFE_DELETE(x)		if( x ) { delete(x); (x) = NULL; }
#define SAFE_RELEASE(x) if( x ) { (x)->Release(); (x) = NULL; }
#define SAFE_DELETE_ARRAY(x) if( x ) { delete [] (x); (x) = NULL; }

class Model_X
{
public:
	LPDIRECT3DDEVICE9   g_pd3dDevice;

	struct Material{
		D3DMATERIAL9*       g_pMeshMaterials;
		LPDIRECT3DTEXTURE9* g_pMeshTextures;
	}*material;
	
	DWORD               g_dwNumMaterials;
	LPD3DXMESH          g_pMesh;

	char fileName[512];

	vector<Vector3> verticesX;
	vector<float> vertices;
	vector<short> indices;

	Model_X(LPDIRECT3DDEVICE9  g_pd3dDevice)
	{
		this->g_pd3dDevice=g_pd3dDevice;
		//g_pMeshMaterials=NULL;
		//g_pMeshTextures=NULL;
		g_dwNumMaterials=NULL;
		g_pMesh=NULL;

		mBoundingBoxMesh	= NULL;
		mBoundingSphereMesh = NULL;
	}

	bool LoadXFile(char *filename)
	{
		LPD3DXBUFFER pD3DXMtrlBuffer;

		if( FAILED( D3DXLoadMeshFromXA( filename, D3DXMESH_SYSTEMMEM,
									   g_pd3dDevice, NULL,
									   &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
									   &g_pMesh ) ) )
		{
				MessageBox( NULL, "Could not find Model File", filename, MB_OK );
				return E_FAIL;
		}

		if(1)  //create optimization of mesh
		{
			DWORD* pdwAdjaceny  = NULL,*optAdj;
			pdwAdjaceny = new DWORD[3*g_pMesh->GetNumFaces() ];
			
			if( FAILED(g_pMesh->GenerateAdjacency( 1.0f, (DWORD*)pdwAdjaceny )) )
				return NULL;

			if( FAILED( g_pMesh->OptimizeInplace( D3DXMESHOPT_COMPACT | D3DXMESHOPT_VERTEXCACHE, pdwAdjaceny,/*optAdj*/ NULL, NULL, NULL ) ) )
			{
				return NULL;
			}
			delete pdwAdjaceny;
		}

		// We need to extract the material properties and texture names from the pD3DXMtrlBuffer
		D3DXMATERIAL* d3dxMaterials = ( D3DXMATERIAL* )pD3DXMtrlBuffer->GetBufferPointer();

		material = new Material[g_dwNumMaterials];

		for( DWORD i = 0; i < g_dwNumMaterials; i++ )
		{
			material[i].g_pMeshMaterials=new D3DMATERIAL9(d3dxMaterials[i].MatD3D);
			
			// Set the ambient color for the material (D3DX does not do this)
			material[i].g_pMeshMaterials->Ambient = material[i].g_pMeshMaterials->Diffuse;

			if( d3dxMaterials[i].pTextureFilename != NULL &&
				lstrlenA( d3dxMaterials[i].pTextureFilename ) > 0 )
			{
					CHAR strTexture[MAX_PATH]="";
					string dir;dir.append(filename);
					
					dir=dir.substr(0,dir.rfind("/")+1);
					strcat_s( strTexture, MAX_PATH,dir.c_str());
					strcat_s( strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename );

					int flag=0;
					for(int j=0;j<i;j++)
					{
						if( !memcmp(d3dxMaterials[i].pTextureFilename,d3dxMaterials[j].pTextureFilename,strlen(d3dxMaterials[i].pTextureFilename) ))
						{
							flag=j;break;
						}
					}
					if(flag==0)
					{
						printf("%s\n",d3dxMaterials[i].pTextureFilename);
						material[i].g_pMeshTextures=new LPDIRECT3DTEXTURE9();
						if( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
															strTexture,
															material[i].g_pMeshTextures ) ) )
						{
						
							printf("Error:File cant load");
						}
					}
					else
					{
						material[i].g_pMeshTextures = material[flag].g_pMeshTextures;
					}
			}
			else material[i].g_pMeshTextures = NULL;
		}
		// Done with the material buffer
		pD3DXMtrlBuffer->Release();

		ComputeBounds() ;

		return 1;
	}
	
	VOID Draw(int xx =-1)
	{
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		for( DWORD i = 0; i < g_dwNumMaterials; i++ )
		{
			material[i].g_pMeshMaterials->Emissive=D3DXCOLOR(.1,0,.1,.1);
			material[i].g_pMeshMaterials->Specular=D3DXCOLOR(1,1,0,1);
		
			g_pd3dDevice->SetMaterial( material[i].g_pMeshMaterials);
			
			if(  material[i].g_pMeshTextures != NULL)
				g_pd3dDevice->SetTexture( 0, material[i].g_pMeshTextures[0] );

			// Draw the mesh subset
			g_pMesh->DrawSubset( xx<0 ? i : xx );
		}
	}
	
	void Scale(float xScale,float yScale,float zScale)
	{
		DWORD stride =  D3DXGetFVFVertexSize(g_pMesh->GetFVF());
		BYTE* vbptr = NULL;
		g_pMesh->LockVertexBuffer(0, (LPVOID*)&vbptr);
		
		for(unsigned int i = 0; i < g_pMesh->GetNumVertices(); i++)
		{
			D3DXVECTOR3* pos = (D3DXVECTOR3*)vbptr;
			pos->x *=  xScale;
			pos->y *=  yScale;
			pos->z *=  zScale;

			vbptr += stride;
		}
		g_pMesh->UnlockVertexBuffer();
	}

	void GetVerticesX(LPD3DXMESH g_pMesh)
	{

	
		DWORD stride =  D3DXGetFVFVertexSize(g_pMesh->GetFVF());
		BYTE* vbptr = NULL;
		g_pMesh->LockVertexBuffer(0, (LPVOID*)&vbptr);
		int ii = -1;
		for(unsigned int i = 0; i < g_pMesh->GetNumVertices(); i++)
		{
			ii++;
			D3DXVECTOR3* pos = (D3DXVECTOR3*)vbptr;
			Vector3 v;
			v.x=pos->x;v.y=pos->y;v.z=pos->z;
			verticesX.push_back(v);
			vbptr += stride;
		}
		g_pMesh->UnlockVertexBuffer();
	}
	
	void GetVertices(LPD3DXMESH g_pMesh)
	{
		DWORD stride =  D3DXGetFVFVertexSize(g_pMesh->GetFVF());
		BYTE* vbptr = NULL;
		g_pMesh->LockVertexBuffer(0, (LPVOID*)&vbptr);
		int ii = -1;
		for(unsigned int i = 0; i < g_pMesh->GetNumVertices(); i++)
		{
			ii++;
			D3DXVECTOR3* pos = (D3DXVECTOR3*)vbptr;
			
			vertices.push_back(pos->x);
			vertices.push_back(pos->y);
			vertices.push_back(pos->z);

			vbptr += stride;
		}
		g_pMesh->UnlockVertexBuffer();
	}

	void GetIndices(LPD3DXMESH     g_pMesh)
	{
		DWORD stride = sizeof(short);
		BYTE* ibptr = NULL;
		short* indices = new short[(g_pMesh)->GetNumFaces() * 3];
		std::vector<short> copy;
		(g_pMesh)->LockIndexBuffer(0, (LPVOID*)&indices);
		for(unsigned int i = 0; i < (g_pMesh)->GetNumFaces() * 3; i++)
		{
			this->indices.push_back(indices[i]);
		}
		(g_pMesh)->UnlockIndexBuffer();
	}

	int SaveToXMLFile(char *filename)
	{
		GetVertices(g_pMesh);
		GetIndices(g_pMesh);

		/*FILE *f=fopen(filename,"w");

		fprintf(f,"<scene>\n");
		fprintf(f," <object name=\"3D_Object_1\" shader_name=\"Material_1\">\n<mesh>\n");
		fprintf(f,"<points>\n");
		for(int i=0;i<vertices.size();i++)
		{
			fprintf(f,"<p x=\"%f\" y=\"%f\" z=\"%f\" />\n",vertices[i].x,vertices[i].y,vertices[i].z);
		}
		fprintf(f,"</points>\n");

		fprintf(f,"<faces>\n");
		for(int i=0;i<indices.size();i+=3)
		{
			fprintf(f,"<f a=\"%i\" b=\"%i\" c=\"%i\" />\n",indices[i+0],indices[i+1],indices[i+2]);
		}
		fprintf(f,"</faces>\n");
		fprintf(f,"</mesh>\n</object>" );
		fprintf(f,"\n</scene>");
		fclose(f);*/
		return 0;
	}

	LPD3DXMESH GetMesh()	{ return  g_pMesh; }
	int GetNumMaterials()	{ return  g_dwNumMaterials; }
	//D3DXVECTOR3				GetCenter();
	const BoundingBox				* GetBoundingBox() { return &mBoundingBox; }
	const BoundingSphere			* GetBoundingSphere() { return &mBoundingSphere; }	
	LPD3DXMESH GetBoundingSphereMesh() { return mBoundingSphereMesh; }
	D3DXVECTOR3	GetCenter(){ return mBoundingBox.center;}


	BoundingBox			mBoundingBox;
	BoundingSphere		mBoundingSphere;
	LPD3DXMESH			mBoundingBoxMesh;
	LPD3DXMESH			mBoundingSphereMesh;


	void ComputeBounds()
	{
		HRESULT hr = S_OK;

		LPD3DXMESH     mMesh =  g_pMesh  ;

		BYTE* pData;
		mMesh->LockVertexBuffer( D3DLOCK_READONLY, (void**)&pData) ;

		D3DXComputeBoundingBox( (const D3DXVECTOR3*)(pData),mMesh->GetNumVertices(),mMesh->GetNumBytesPerVertex(), &mBoundingBox.min, &mBoundingBox.max); 

		mBoundingBox.halfExtent = ( mBoundingBox.max - mBoundingBox.min ) * .5f; 
		mBoundingBox.center = ( mBoundingBox.max + mBoundingBox.min ) * .5f; 

		D3DXComputeBoundingSphere( (const D3DXVECTOR3*)(pData), mMesh->GetNumVertices(), mMesh->GetNumBytesPerVertex(), &mBoundingSphere.center, &mBoundingSphere.radius );

		mMesh->UnlockVertexBuffer();

		// We have min and max values, use these to get the 8 corners of the bounding box
		mBoundingBox.boxPoints[0] = D3DXVECTOR3( mBoundingBox.min.x, mBoundingBox.min.y, mBoundingBox.min.z ); // xyz
		mBoundingBox.boxPoints[1] = D3DXVECTOR3( mBoundingBox.max.x, mBoundingBox.min.y, mBoundingBox.min.z ); // Xyz
		mBoundingBox.boxPoints[2] = D3DXVECTOR3( mBoundingBox.min.x, mBoundingBox.max.y, mBoundingBox.min.z ); // xYz
		mBoundingBox.boxPoints[3] = D3DXVECTOR3( mBoundingBox.max.x, mBoundingBox.max.y, mBoundingBox.min.z ); // XYz
		mBoundingBox.boxPoints[4] = D3DXVECTOR3( mBoundingBox.min.x, mBoundingBox.min.y, mBoundingBox.max.z ); // xyZ
		mBoundingBox.boxPoints[5] = D3DXVECTOR3( mBoundingBox.max.x, mBoundingBox.min.y, mBoundingBox.max.z ); // XyZ
		mBoundingBox.boxPoints[6] = D3DXVECTOR3( mBoundingBox.min.x, mBoundingBox.max.y, mBoundingBox.max.z ); //   
		mBoundingBox.boxPoints[7] = D3DXVECTOR3( mBoundingBox.max.x, mBoundingBox.max.y, mBoundingBox.max.z ); // XYZ

		SAFE_RELEASE(mBoundingBoxMesh);
		SAFE_RELEASE(mBoundingSphereMesh)
		// Create the bounding meshes for representation
		D3DXCreateSphere( g_pd3dDevice, mBoundingSphere.radius, 15, 10, &mBoundingSphereMesh, NULL );
		float width = mBoundingBox.max.x - mBoundingBox.min.x;
		float height = mBoundingBox.max.y - mBoundingBox.min.y;
		float depth = mBoundingBox.max.z - mBoundingBox.min.z;
		D3DXCreateBox( g_pd3dDevice, width, height, depth, &mBoundingBoxMesh, NULL );

		/*LOG_DEBUG( "Bounding box min: " << mBoundingBox.min.x << "," << mBoundingBox.min.y << "," << mBoundingBox.min.z << " max: " << mBoundingBox.max.x << "," << mBoundingBox.max.y << "," << mBoundingBox.max.z );
		LOG_DEBUG( "Bounding box half extent: " << mBoundingBox.halfExtent.x << "," << mBoundingBox.halfExtent.y << "," << mBoundingBox.halfExtent.z );	
		LOG_DEBUG( "Bounding box center: " << mBoundingBox.center.x << "," << mBoundingBox.center.y << "," << mBoundingBox.center.z );	
		LOG_DEBUG( "Bounding sphere radius: " << mBoundingSphere.radius << " " << ", center: " << mBoundingSphere.center.x << "," << mBoundingSphere.center.y << "," << mBoundingSphere.center.z );*/

	}


};