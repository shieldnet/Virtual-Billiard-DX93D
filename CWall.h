#ifndef _CWALL_
#define _CWALL_
#include "d3dUtility.h"
#include "CSphere.h"

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.997

// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall : public CObject {

private:

	float					m_x;
	float					m_z;
	float                   m_width;
	float                   m_depth;
	float					m_height;

public:
	CWall(void);
	~CWall(void);
public:
	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE);

	void destroy(void);

	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);

	bool hasIntersected(CSphere& ball) noexcept;

	void hitBy(CSphere& ball) noexcept;

	float getHeight(void) const;
	
	void setPosition(float x, float y, float z);

private:
	void setLocalTransform(const D3DXMATRIX&);
	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh*              m_pBoundMesh;
};

#endif // !_CWALL_
