#pragma once

#include "BoundingVolumes.h"

#include <vector>

class Mesh
{
public:
	Mesh(void);
	~Mesh(void);

	virtual HRESULT Create( LPDIRECT3DDEVICE9 device, char *filename, float scaleTo = 0.f, bool adjustHeight = true, D3DXVECTOR3 *offset = NULL );
	virtual void Destroy();
	HRESULT ScaleAndCenterMesh( float amount, bool adjustHeight, D3DXVECTOR3 *offset = NULL );
	void ComputeBounds();

	LPD3DXMESH GetMesh()	{ return mMesh; }
	int GetNumMaterials()	{ return mMaterials.size(); }

	D3DXVECTOR3				GetCenter();
	const BoundingBox				* GetBoundingBox() { return &mBoundingBox; }
	const BoundingSphere			* GetBoundingSphere() { return &mBoundingSphere; }	
	LPD3DXMESH GetBoundingSphereMesh() { return mBoundingSphereMesh; }

	std::vector<LPDIRECT3DTEXTURE9> * GetTextures() { return &mTextures; }
	std::vector<D3DMATERIAL9>		* GetMaterials() { return &mMaterials; }

	void Render();

private:
	LPDIRECT3DDEVICE9	mDevice;
	LPD3DXMESH			mMesh;	
	LPD3DXMESH			mBoundingBoxMesh;
	LPD3DXMESH			mBoundingSphereMesh;

	DWORD							mNumMaterials;
	std::vector<LPDIRECT3DTEXTURE9> mTextures;
	std::vector<D3DMATERIAL9>		mMaterials;	

	LPDIRECT3DTEXTURE9				mWhiteTex;
	BoundingBox						mBoundingBox;
	BoundingSphere					mBoundingSphere;
};