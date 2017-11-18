#ifndef _CCUE_
#define _CCUE_
#include "d3dUtility.h"
#include "CObject.h"

#define M_HEIGHT 0.01

class CCue : public CObject{
	float					m_x;
	float					m_z;
	float                   m_width;
	float                   m_depth;
	float					m_height;

public:
	CCue(void);
	~CCue(void);

	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE);

	void destroy(void);

	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);

	void rotate(IDirect3DDevice9* pDevice, D3DXMATRIX& mWorld);

	void setPosition(float x, float y, float z);

	D3DXVECTOR3 getCenter(void) const;

	float getHeight(void) const;

	float getWidth(void) const;


private:
	void setLocalTransform(const D3DXMATRIX& mLocal);

	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh*              m_pBoundMesh;
	DWORD m_numMaterials;
	LPD3DXBUFFER pMaterialBuffer;

	/*
	LPDIRECT3DDEVICE9 g_pD3DDevice = NULL;
	D3DMATERIAL9 *m_pMeshMaterials;
	LPDIRECT3DTEXTURE9 *m_ppMeshTextures;
	LPD3DXBUFFER pMaterialBuffer;
	*/

};
#endif // !1
