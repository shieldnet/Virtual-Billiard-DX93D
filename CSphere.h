#ifndef _CSPHERE_
#define _CSPHERE_
#include "d3dUtility.h"

#include "CObject.h"


#define FVF_VERTEX    D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1
#define DECREASE_RATE 0.997

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.997

struct _VERTEX
{
	D3DXVECTOR3 pos; // vertex position
	D3DXVECTOR3 norm; // vertex normal
	float tu; // texture coordinates
	float tv;
};

class CSphere : public CObject {
private:
	float					center_x, center_y, center_z;
	float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;
	float					pre_center_x, pre_center_z;
	D3DXMATRIX				matBallRoll;
	IDirect3DTexture9* Texture = nullptr;
	// 이미지 texture
	// 이전 위치 보관, 충돌 시에 사용해야 함

public:

	bool isHole;
	int	ballNum;

	CSphere(void);
	~CSphere(void);

public:
	void setPosition(float x, float y, float z);	// 공 위치 설정

	D3DXVECTOR3 getPosition() const;	// 공 위치 get

	void adjustPosition(CSphere& ball);// 공 위치 조정


	bool create(IDirect3DDevice9* pDevice, int num, D3DXCOLOR color);
	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color); //function overloading for blue ball and 6 balck hole

	void destroy(void);
	LPD3DXMESH _createMappedSphere(IDirect3DDevice9* pDev);
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);

	bool hasIntersected(CSphere& ball) noexcept;

	void hitBy(CSphere& ball) noexcept;

	void ballUpdate(float timeDiff);

	double getVelocity_X();
	double getVelocity_Z();

	void setPower(double vx, double vz);	// 공의 속도 설정

	void setCenter(float x, float y, float z);	// 공의 중심 설정

	float getRadius(void) const;
	const D3DXMATRIX& getLocalTransform(void) const;
	void setLocalTransform(const D3DXMATRIX& mLocal);
	D3DXVECTOR3 getCenter(void) const;

	void setDX(float dx);				//x, y의 변화량에 대한 getter & setter 함수

	void setDZ(float dz);
	float getDX();
	float getDZ();


private:
	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh*              m_pSphereMesh;
	float                   m_unit;
	float					m_dx;
	float					m_dz;
};

#endif
