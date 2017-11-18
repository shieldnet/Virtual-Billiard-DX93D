#include "CCUe.h"
#include <iostream>

using namespace std;

CCue::CCue(void)	// 큐 생성자
{
	D3DXMatrixIdentity(&m_mLocal);
	ZeroMemory(&m_mtrl, sizeof(m_mtrl));
	m_width = 0;
	m_depth = 0;
	m_pBoundMesh = NULL;

}

CCue::~CCue(void) {}


bool CCue::create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color)
{
	if (NULL == pDevice)
		return false;
	m_mtrl.Ambient = color;
	m_mtrl.Diffuse = color;
	m_mtrl.Specular = color;
	m_mtrl.Emissive = d3d::BLACK;
	m_mtrl.Power = 5.0f;

	m_width = iwidth;
	m_depth = idepth;

	if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
		return false;
	cout << "Cue created" << endl;
	return true;
}

void CCue::destroy(void)
{
	if (m_pBoundMesh != NULL) {
		m_pBoundMesh->Release();
		m_pBoundMesh = NULL;
	}
}

float CCue::getWidth() const {

	return this->m_width;
}


void CCue::draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
{

	
	if (NULL == pDevice)
		return;
	pDevice->SetTransform(D3DTS_WORLD, &mWorld);
	pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
	pDevice->SetMaterial(&m_mtrl);
	m_pBoundMesh->DrawSubset(0);

}

void CCue::rotate(IDirect3DDevice9* pDevice, D3DXMATRIX& mWorld) {
	

	
	D3DXMATRIX mX;
	D3DXMATRIX mY;

	float dx = 0.1f;
	D3DXMatrixRotationY(&mX, dx);

	mWorld = mX;

	pDevice->SetTransform(D3DTS_WORLD, &mWorld);
	/*D3DXMatrixRotationY(&mX, dx);
	D3DXMatrixRotationX(&mY, dy);
*/



}

void CCue::setPosition(float x, float y, float z)
{
	D3DXMATRIX m;
	this->m_x = x;
	this->m_z = z;
	D3DXMatrixTranslation(&m, x, y, z);
	setLocalTransform(m);
}
D3DXVECTOR3 CCue::getCenter(void) const
{
	D3DXVECTOR3 org(this->center_x, this->center_y, this->center_z);
	return org;
}

float CCue::getHeight(void) const { 
	return M_HEIGHT; 
}

void CCue::setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

/*
LPDIRECT3DDEVICE9 g_pD3DDevice = NULL;
D3DMATERIAL9 *m_pMeshMaterials;
LPDIRECT3DTEXTURE9 *m_ppMeshTextures;
LPD3DXBUFFER pMaterialBuffer;
*/