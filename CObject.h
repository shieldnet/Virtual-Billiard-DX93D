#ifndef _COBJECT_
#define _COBJECT_
#include "d3dUtility.h"

class CObject
{
protected:
	float center_x, center_y, center_z;
	D3DXMATRIX mLocal;

public:
	virtual bool create(IDirect3DDevice9* pDevice) = 0;
	virtual void destroy() = 0;
	virtual void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld) = 0;

	void setPosition(float x, float y, float z); // 객체의 중심 위치를 설정
	void setLocalTransform(const D3DXMATRIX& mLocal); // 객체의 Local Transform Matrix 설정

	D3DXVECTOR3 getPosition() const; // 객체의 중심 위치를 반환
	D3DXMATRIX getLocalTransform() const; // 객체의 Local Transform Matrix 반환

	virtual ~CObject();
};

#endif

