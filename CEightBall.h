#ifndef _CEIGHTBALL_
#define _CEIGHTBALL_
#include "d3dUtility.h"

#include "CSphere.h"

class CEightBall : public CSphere {
public:

	CEightBall();

private:
	float					center_x, center_y, center_z;
	float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;
	float					pre_center_x, pre_center_z;
	D3DXMATRIX				matBallRoll;
};



#endif // ! _CEIGHTBALL_
