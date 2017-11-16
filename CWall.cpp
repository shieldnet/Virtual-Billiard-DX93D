#include "d3dUtility.h"
#include "CWall.h"
#include "CSphere.h"

#include "CObject.h"

// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

CWall::CWall(void)	// 벽 생성자
{
	D3DXMatrixIdentity(&m_mLocal);
	ZeroMemory(&m_mtrl, sizeof(m_mtrl));
	m_width = 0;
	m_depth = 0;
	m_pBoundMesh = NULL;
}
CWall::~CWall(void) {}

bool CWall::create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color)
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

void CWall::destroy(void)
{
	if (m_pBoundMesh != NULL) {
		m_pBoundMesh->Release();
		m_pBoundMesh = NULL;
	}
}

void CWall::draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
{
	if (NULL == pDevice)
		return;
	pDevice->SetTransform(D3DTS_WORLD, &mWorld);
	pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
	pDevice->SetMaterial(&m_mtrl);
	m_pBoundMesh->DrawSubset(0);
}

bool CWall::hasIntersected(CSphere& ball) noexcept	// 공과의 충돌 판정
{
	D3DXVECTOR3 temp_coor = ball.getCenter();				//공의 위치
	if (temp_coor.x >= 4.5 - M_RADIUS || temp_coor.z >= 3 - M_RADIUS ||
		temp_coor.x <= -(4.5 - M_RADIUS) || temp_coor.z <= -(3 - M_RADIUS)) {
		return true;
	}
	else return false;
}

void CWall::hitBy(CSphere& ball) noexcept	// 공과의 충돌 처리
{
	if (hasIntersected(ball)) {//공이 벽보다 밖에 있을 경우
		float retheta;
		D3DXVECTOR3 temp_coor = ball.getCenter();
		if (temp_coor.x > 4.4 - M_RADIUS) {						//오른쪽 벽
			ball.setPower(-ball.getVelocity_X(), ball.getVelocity_Z());
			ball.setCenter(4.4 - M_RADIUS, temp_coor.y, temp_coor.z);

		}
		if (temp_coor.x < -(4.4 - M_RADIUS)) {						//왼쪽 벽
			ball.setPower(-ball.getVelocity_X(), ball.getVelocity_Z());
			ball.setCenter(-(4.4 - M_RADIUS), temp_coor.y, temp_coor.z);
		}
		if (temp_coor.z > 2.9 - M_RADIUS) {					//위쪽 벽
			ball.setPower(ball.getVelocity_X(), -ball.getVelocity_Z());
			ball.setCenter(temp_coor.x, temp_coor.y, 2.9 - M_RADIUS);
		}
		if (temp_coor.z < -(2.9 - M_RADIUS)) {					//아래쪽 벽
			ball.setPower(ball.getVelocity_X(), -ball.getVelocity_Z());
			ball.setCenter(temp_coor.x, temp_coor.y, -(2.9 - M_RADIUS));
		}
	}
}

float CWall::getHeight(void) const { return M_HEIGHT; }

void CWall::setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

void CWall::setPosition(float x, float y, float z)
{
	D3DXMATRIX m;
	this->m_x = x;
	this->m_z = z;

	D3DXMatrixTranslation(&m, x, y, z);
	setLocalTransform(m);
}