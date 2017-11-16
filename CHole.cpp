#include "CHole.h"
#include "CSphere.h"

CHole::CHole(void)
{
	D3DXMatrixIdentity(&m_mLocal);
	ZeroMemory(&m_mtrl, sizeof(m_mtrl));
	m_radius = 0;
	m_velocity_x = 0;
	m_velocity_z = 0;
	m_pSphereMesh = NULL;
	isHole = true;
}

CHole::~CHole(void) {}
