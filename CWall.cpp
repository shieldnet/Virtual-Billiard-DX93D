#ifndef _CWALL_
#define _CWALL_
#include "d3dUtility.h"
#include "CSphere.h"
#include "CObject.h"
#include "HittableObj.h"

// -----------------------------------------------------------------------------
// CWall class definition
// 벽 클래스에 대한 정의
// -----------------------------------------------------------------------------
class CWall : public CObject, public Hittable
{
private:
	D3DMATERIAL9 m_mtrl;
	ID3DXMesh* m_pBoundMesh;

protected:
	float m_width;																		// 벽의 너비
	float m_depth;																		// 벽의 두께
	float m_height;																		// 벽의 높이


public:
	static const float LOSS_RATIO;

	CWall(float iwidth, float iheight, float idepth, D3DXCOLOR color);					// 벽의 생성자
	virtual ~CWall();																	// 벽의 소멸자
	bool create(IDirect3DDevice9* pDevice);												// 벽을 화면에 생성함
	void destroy();																		// 벽을 화면에서 소멸시킴
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld);						// 벽을 화면에 그려냄

	virtual bool hasIntersected(CSphere& ball) const noexcept = 0;						// 벽에 공이 충돌 했는지 확인
	virtual void hitBy(CSphere& ball) noexcept = 0;
	void adjustPosition(CSphere& ball);													// 벽과 충돌한 공의 위치를 조정함
};

#endif

