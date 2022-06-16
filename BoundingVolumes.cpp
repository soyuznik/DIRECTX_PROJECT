#include "BoundingVolumes.h"

BoundingBox::BoundingBox(void)
{
	max = D3DXVECTOR3(-10000.0f, -10000.0f, -10000.0f);
	min = D3DXVECTOR3(10000.0f, 10000.0f, 10000.0f);
}


BoundingBox::BoundingBox(D3DXVECTOR3 _max, D3DXVECTOR3 _min)
{
	max = _max;
	min = _min;
}

BoundingBox::~BoundingBox(void)
{
}

