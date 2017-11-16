#ifndef _CLIGHT_
#define _CLIGHT_
#include "d3dUtility.h"

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
	CLight(void);
	~CLight(void);
public:
	bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius);
	void destroy(void);
	bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);

	void draw(IDirect3DDevice9* pDevice);

	D3DXVECTOR3 getPosition(void) const;

private:
	DWORD               m_index;
	D3DXMATRIX          m_mLocal;
	D3DLIGHT9           m_lit;
	ID3DXMesh*          m_pMesh;
	d3d::BoundingSphere m_bound;
};


#endif

