#include "CCUe.h"

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
	return true;
}

void CCue::destroy(void)
{
	if (m_pBoundMesh != NULL) {
		m_pBoundMesh->Release();
		m_pBoundMesh = NULL;
	}
}

void CCue::draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
{
	D3DXLoadMeshFromX("./image/cuew.x", D3DXMESH_MANAGED, pDevice, NULL, &pMaterialBuffer, NULL, &m_numMaterials, &m_pBoundMesh);

	D3DXMATERIAL* tempMaterials = (D3DXMATERIAL*)pMaterialBuffer->GetBufferPointer();
	D3DMATERIAL9* material = new D3DMATERIAL9[m_numMaterials];
	LPDIRECT3DTEXTURE9* texture = new LPDIRECT3DTEXTURE9[m_numMaterials];
	for (DWORD i = 0; i < m_numMaterials; i++)    // for each material...
	{
		material[i] = tempMaterials[i].MatD3D;    // get the material info
		material[i].Ambient = material[i].Diffuse;   // make ambient the same as diffuse
		D3DXCreateTextureFromFile(pDevice,
			"./image/cuew.x",
			&texture[i]);
		//texture[i] = NULL;
	}

	if (NULL == pDevice)
		return;
	for (int i = 0; i < m_numMaterials; i++) {
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(i);
	}
}

void CCue::setPosition(float x, float y, float z)
{
	D3DXMATRIX m;
	this->m_x = x;
	this->m_z = z;
	D3DXMatrixTranslation(&m, x, y, z);
	setLocalTransform(m);
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