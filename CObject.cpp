#include "CObject.h"

D3DXVECTOR3 CObject::getPosition() const
{
	D3DXVECTOR3 org(center_x, center_y, center_z);
	return org;
}

void CObject::setPosition(float x, float y, float z)
{
	D3DXMATRIX m;

	this->center_x = x;
	this->center_y = y;
	this->center_z = z;

	D3DXMatrixTranslation(&m, x, y, z);
	this->setLocalTransform(m);
}

void CObject::setLocalTransform(const D3DXMATRIX& mLocal)
{
	this->mLocal = mLocal;
}

D3DXMATRIX CObject::getLocalTransform() const
{
	return mLocal;
}

CObject::~CObject()
{
}

