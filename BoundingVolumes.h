#include <windows.h>
#include <windef.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>  
#include <string>
#include <Psapi.h>
#include "mmsystem.h"

#pragma once

class BoundingBox
{
public:
	BoundingBox(void);
	BoundingBox(D3DXVECTOR3 _max, D3DXVECTOR3 _min);
	~BoundingBox(void);

	D3DXVECTOR3 min;
	D3DXVECTOR3 max;
	D3DXVECTOR3	halfExtent;
	D3DXVECTOR3	center;
	D3DXVECTOR3	boxPoints[8];
};

class BoundingSphere
{
public:
	D3DXVECTOR3 center;
	float radius;
};
