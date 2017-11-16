#ifndef _CHOLE_
#define _CHOLE_
#include "d3dUtility.h"
#include "CSphere.h"
// -----------------------------------------------------------------------------
// CHole class definition
// -----------------------------------------------------------------------------
class CHole : public CSphere {

private:
	float					center_x, center_y, center_z;
	float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;
	float					pre_center_x, pre_center_z;
	D3DXMATRIX				matBallRoll;


private:
	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh*              m_pSphereMesh;
	float                   m_unit;
	float					m_dx;
	float					m_dz;
	
public:

	//bool					isHole;
	CHole(void);
	~CHole(void);


};

#endif // !_CHOLE_
