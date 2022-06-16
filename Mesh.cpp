#include "Mesh.h"

#include "Vertex.h"
#include "Utilities.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#define LOG_DEBUG(m)  true;
#define LOG_INFO(m)   ;
#define LOG_WARN(m)   ;
#define LOG_ERROR(m)  ;
#define LOG_FATAL(m)  ;

Mesh::Mesh(void)
{
	mDevice				= NULL;
	mMesh				= NULL;
	mBoundingBoxMesh	= NULL;
	mBoundingSphereMesh = NULL;
	mWhiteTex			= NULL;
}

Mesh::~Mesh(void)
{
	Destroy();	
}

HRESULT Mesh::Create( LPDIRECT3DDEVICE9 device, char *filename, float scaleTo, bool adjustHeight, D3DXVECTOR3 *offset )
{
	mDevice = device;

	WCHAR wstring[128];
	ConvertString( wstring, filename );
	LOG_DEBUG( "Loading x file... " << filename );	

	D3DXCreateTextureFromFile(mDevice, "whitetex.dds", &mWhiteTex);

	HRESULT  hr = S_OK;
	ID3DXBuffer * adjacencyBfr = NULL;
	ID3DXBuffer * materialBfr = NULL;	

	if(FAILED(D3DXLoadMeshFromXA(filename, D3DXMESH_MANAGED, mDevice,	
		&adjacencyBfr, &materialBfr, NULL, &mNumMaterials, &mMesh)))
	{
		LOG_ERROR( "Failed to load file."  );
		return E_FAIL;
	}

	D3DXComputeNormals(mMesh, 0);

	LOG_DEBUG( "Found materials: " << mNumMaterials );

	D3DXMATERIAL *mtrls = (D3DXMATERIAL*)materialBfr->GetBufferPointer();	

	for(DWORD i = 0; i < mNumMaterials; i++)
	{		
		mMaterials.push_back(mtrls[i].MatD3D);
		{
			mTextures.push_back(NULL);			
		}
	}

	SAFE_RELEASE( materialBfr );
	SAFE_RELEASE( adjacencyBfr );

	ComputeBounds();

	if( scaleTo != 0.0f )
	{	
		ScaleAndCenterMesh( scaleTo, adjustHeight, offset );
	}

	LOG_DEBUG( "Total faces: " << mMesh->GetNumFaces() << " Total vertices: " << mMesh->GetNumVertices() ) ;

	return S_OK;
}

D3DXVECTOR3	Mesh::GetCenter()
{
	return mBoundingBox.center;
}

void Mesh::Destroy()
{
	SAFE_RELEASE( mMesh );		
	for(size_t i = 0; i < mTextures.size(); i++)
	{
		SAFE_RELEASE(mTextures[i]);
	}

	mTextures.clear();
	mMaterials.clear();	

	SAFE_RELEASE(mBoundingBoxMesh);
	SAFE_RELEASE(mBoundingSphereMesh)
	SAFE_RELEASE(mWhiteTex);
}

void Mesh::Render()
{	    
	for( DWORD i = 0; i < mMaterials.size(); i++ )
	{			
		// Set the material and texture for this subset
		mDevice->SetMaterial( &mMaterials[i] );
		if( mTextures[i] )
		{
			mDevice->SetTexture( 0, mTextures[i] );
		}
		else
		{
				mDevice->SetTexture( 0, mWhiteTex );
		}
		// Draw the mesh subset
		mMesh->DrawSubset( i );
	}
}

void Mesh::ComputeBounds()
{
	HRESULT hr = S_OK;

	BYTE* pData;
	mMesh->LockVertexBuffer( D3DLOCK_READONLY, (void**)&pData) ;

	D3DXComputeBoundingBox( (const D3DXVECTOR3*)(pData),
		mMesh->GetNumVertices(),
		mMesh->GetNumBytesPerVertex(), 
		&mBoundingBox.min, &mBoundingBox.max); 

	mBoundingBox.halfExtent = ( mBoundingBox.max - mBoundingBox.min ) * .5f; 
	mBoundingBox.center = ( mBoundingBox.max + mBoundingBox.min ) * .5f; 

	D3DXComputeBoundingSphere( (const D3DXVECTOR3*)(pData),
		mMesh->GetNumVertices(), mMesh->GetNumBytesPerVertex(), 
		&mBoundingSphere.center, &mBoundingSphere.radius );

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
	D3DXCreateSphere( mDevice, mBoundingSphere.radius, 15, 10, &mBoundingSphereMesh, NULL );
	float width = mBoundingBox.max.x - mBoundingBox.min.x;
	float height = mBoundingBox.max.y - mBoundingBox.min.y;
	float depth = mBoundingBox.max.z - mBoundingBox.min.z;
	D3DXCreateBox( mDevice, width, height, depth, &mBoundingBoxMesh, NULL );

	LOG_DEBUG( "Bounding box min: " << mBoundingBox.min.x << "," << mBoundingBox.min.y << "," << mBoundingBox.min.z << " max: " << mBoundingBox.max.x << "," << mBoundingBox.max.y << "," << mBoundingBox.max.z );
	LOG_DEBUG( "Bounding box half extent: " << mBoundingBox.halfExtent.x << "," << mBoundingBox.halfExtent.y << "," << mBoundingBox.halfExtent.z );	
	LOG_DEBUG( "Bounding box center: " << mBoundingBox.center.x << "," << mBoundingBox.center.y << "," << mBoundingBox.center.z );	
	LOG_DEBUG( "Bounding sphere radius: " << mBoundingSphere.radius << " " << ", center: " << mBoundingSphere.center.x << "," << mBoundingSphere.center.y << "," << mBoundingSphere.center.z );

}

HRESULT Mesh::ScaleAndCenterMesh( float amount, bool adjustHeight, D3DXVECTOR3 *offset)
{
	//LOG( "Scaling and centering / offsetting mesh..." );

	BYTE *ptr = NULL;
	HRESULT hr;
	D3DXVECTOR3 vOff;

	float radius = D3DXVec3Length( &(mBoundingBox.max - mBoundingBox.center) );	

	if( adjustHeight )
	{
		// Make sure mesh is aligned in Y plane
		mBoundingBox.center.y -= mBoundingBox.halfExtent.y;
	}

	// Select default or specified offset vector
	if (offset)
		vOff =* offset;
	else
		vOff = -mBoundingBox.center;

	float scale = amount / mBoundingSphere.radius;

	// Get the face count
	DWORD numVerts = mMesh->GetNumVertices();

	// Get the FVF flags
	DWORD fvf = mMesh->GetFVF();

	// Calculate vertex size
	DWORD vertSize = D3DXGetFVFVertexSize(fvf);

	// Lock the vertex buffer
	if (FAILED(hr = mMesh->LockVertexBuffer(0, (LPVOID*)&ptr)))
	{
		// return on failure
		return hr;
	}

	// Loop through the vertices
	for( DWORD i = 0; i < numVerts; i++ ) 
	{
		// Get pointer to location
		D3DXVECTOR3 *vPtr = (D3DXVECTOR3 *) ptr;

		// Scale the vertex
		*vPtr += vOff;
		vPtr->x *= scale;
		vPtr->y *= scale;
		vPtr->z *= scale;

		// Increment pointer to next vertex
		ptr += vertSize;
	}

	// Unlock the vertex buffer
	if (FAILED( hr = mMesh->UnlockVertexBuffer()))
	{
		// Return on failure
		return hr;
	}

	D3DXComputeNormals( mMesh, NULL );

	// Recompute new bounding volumes
	ComputeBounds();

	// Return success to caller
	return S_OK;
}
