#include "CSphere.h"
#include <string>
#include <d3d10.h>
#include <D3DX10.h>


using namespace std;

bool isHole;
int	ballNum;

CSphere::CSphere(void)
{
	D3DXMatrixIdentity(&m_mLocal);
	ZeroMemory(&m_mtrl, sizeof(m_mtrl));
	m_radius = 0;	// 공 반지름
	m_velocity_x = 0;	// 공 x 속도
	m_velocity_z = 0;	// 공 z 속도
	m_pSphereMesh = NULL;	// ??


	D3DXMatrixIdentity(&mLocal);
	D3DXMatrixIdentity(&this->matBallRoll);
}

CSphere::~CSphere(void) {

}

D3DXVECTOR3 CSphere::getPosition() const	// 공 위치 get
{
	D3DXVECTOR3 org(center_x, center_y, center_z);
	return org;
}


void CSphere::adjustPosition(CSphere& ball) {	// 공 위치 조정
	D3DXVECTOR3 ball_cord = ball.getCenter();
	//보간법으로 근사하여 충돌 시점의 좌표로 이동함.
	this->setPosition((center_x + this->pre_center_x) / 2, center_y, (center_z + this->pre_center_z) / 2);
	ball.setPosition((ball_cord.x + ball.pre_center_x) / 2, ball_cord.y, (ball_cord.z + ball.pre_center_z) / 2);
	if (this->hasIntersected(ball))
	{
		this->setPosition(this->pre_center_x, center_y, this->pre_center_z);
		ball.setPosition(ball.pre_center_x, ball_cord.y, ball.pre_center_z);
	}
}

bool CSphere::create(IDirect3DDevice9* pDevice, int num, D3DXCOLOR color = d3d::WHITE)
{
	if (NULL == pDevice)
		return false;
		m_mtrl.Ambient = d3d::WHITE;
	m_mtrl.Diffuse = d3d::WHITE;
	m_mtrl.Specular = d3d::WHITE;
	m_mtrl.Emissive = d3d::BLACK;
	m_mtrl.Power = 5.0f;

	string filePath = "./image/" + to_string(num + 1) + ".png";

	this->m_pSphereMesh = _createMappedSphere(pDevice);
		if (FAILED(D3DXCreateTextureFromFile(pDevice, filePath.c_str(), &Texture)))
	{
		return false;
	}
	return true;
}

bool CSphere::create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE) //function overloading for blue ball and 6 balck hole
{
	if (NULL == pDevice)
		return false;
		m_mtrl.Ambient = color;
	m_mtrl.Diffuse = color;
	m_mtrl.Specular = color;
	m_mtrl.Emissive = d3d::BLACK;
	m_mtrl.Power = 5.0f;
	if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
		return false;
	return true;
}


void CSphere::destroy(void)
{
	if (m_pSphereMesh != NULL) {
		m_pSphereMesh->Release();
		d3d::Release<IDirect3DTexture9*>(Texture);
		m_pSphereMesh = NULL;
	}
}


LPD3DXMESH CSphere::_createMappedSphere(IDirect3DDevice9* pDev)
{
	// create the sphere
	LPD3DXMESH mesh;
	if (FAILED(D3DXCreateSphere(pDev, this->getRadius(), 50, 50, &mesh, NULL)))
		return nullptr;
		// create a copy of the mesh with texture coordinates,
	// since the D3DX function doesn't include them
	LPD3DXMESH texMesh;

	if (FAILED(mesh->CloneMeshFVF(D3DXMESH_SYSTEMMEM, FVF_VERTEX, pDev, &texMesh)))
	// failed, return un-textured mesh
		return mesh;
	// finished with the original mesh, release it
	mesh->Release();
	// lock the vertex buffer
	//LPVERTEX pVerts;
	struct _VERTEX* pVerts;
	if (SUCCEEDED(texMesh->LockVertexBuffer(0, reinterpret_cast<void **>(&pVerts))))
	{
		// get vertex count
		int numVerts = texMesh->GetNumVertices();
			// loop through the vertices
		for (int i = 0; i < numVerts; i++)
		{
			// calculate texture coordinates
			pVerts->tu = asinf(pVerts->norm.x) / D3DX_PI + 0.5f;
			pVerts->tv = asinf(pVerts->norm.y) / D3DX_PI + 0.5f;
			// go to next vertex
			pVerts++;
		}
			// unlock the vertex buffer
		texMesh->UnlockVertexBuffer();
	}
		// return pointer to caller
	return texMesh;
}


void CSphere::draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
{
	
	if (NULL == pDevice)
		return;
	pDevice->SetTransform(D3DTS_WORLD, &mWorld);
	pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
	pDevice->MultiplyTransform(D3DTS_WORLD, &matBallRoll);
	pDevice->SetTexture(0, Texture);
	pDevice->SetMaterial(&m_mtrl);
	m_pSphereMesh->DrawSubset(0);
}

bool CSphere::hasIntersected(CSphere& ball) noexcept	// 공 충돌 판정
{
	D3DXVECTOR3 cord = this->getPosition();
	D3DXVECTOR3 ball_cord = ball.getPosition();
	double xDistance = abs((cord.x - ball_cord.x) * (cord.x - ball_cord.x));
	double zDistance = abs((cord.z - ball_cord.z) * (cord.z - ball_cord.z));
	double totalDistance = sqrt(xDistance + zDistance);
	
	if (totalDistance < (this->getRadius() + ball.getRadius()))
	{
		return true;
	}
	
	return false;
}

void CSphere::hitBy(CSphere& ball) noexcept	// 공 충돌 시 처리
{
	if (this->hasIntersected(ball))
	{
		if (!ball.isHole) {
			adjustPosition(ball);
			D3DXVECTOR3 ball_cord = ball.getPosition();
			//두 공 사이의 방향 벡터
			double d_x = center_x - ball_cord.x;
			double d_z = center_z - ball_cord.z;
			double size_d = sqrt((d_x * d_x) + (d_z * d_z));
			double vax = this->m_velocity_x;
			double vaz = this->m_velocity_z;
			double vbx = ball.m_velocity_x;
			double vbz = ball.m_velocity_z;
			double size_this_v = sqrt((vax * vax) + (vaz * vaz));
			double cos_t = d_x / size_d;
			double sin_t = d_z / size_d;
			double vaxp = vbx * cos_t + vbz * sin_t;
			double vbxp = vax * cos_t + vaz * sin_t;
			double vazp = vaz * cos_t - vax * sin_t;
			double vbzp = vbz * cos_t - vbx * sin_t;
			this->setPower(vaxp * cos_t - vazp * sin_t, vaxp * sin_t + vazp * cos_t);
			ball.setPower(vbxp * cos_t - vbzp * sin_t, vbxp * sin_t + vbzp * cos_t);
			//tempState = isinOrder(ball.ballNum, playermode);
		}

		else {
			if (this->ballNum != 15 && this->ballNum != 14);
			else {
				
				/*this->setPosition(0.0f, this->center_y, 0.0f);
				this->m_velocity_x = 0;
				this->m_velocity_z = 0;*/
			}
		}
	}
}


void CSphere::ballUpdate(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld, float timeDiff)
	{
	
		const float TIME_SCALE = 3.3F;
		D3DXVECTOR3 cord = this->getPosition();
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		this->pre_center_x = cord.x;
		this->pre_center_z = cord.z;

		if (vx > 0.01 || vz > 0.01)
		{
			const float SPIN_RATIO = 0.01;

			float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
			float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;

			this->setPosition(tX, cord.y, tZ);
			float force = sqrt(pow(this->m_velocity_x, 2) + pow(this->m_velocity_z, 2));
			D3DXMATRIX tmp;
			D3DXVECTOR3 c(this->m_velocity_z, 0, -this->m_velocity_x);
			D3DXMatrixRotationAxis(&tmp, &c, force * SPIN_RATIO);
			matBallRoll *= tmp;

		}
		else
		{
			this->setPower(0, 0);
		}

		//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
		double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400;
		if (rate < 0) rate = 0;

		
		this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate);// 공이 움직일 때마다, 속도를 낮춤
		
	
	}

double CSphere::getVelocity_X() { return this->m_velocity_x; }
double CSphere::getVelocity_Z() { return this->m_velocity_z; }

void CSphere::setPower(double vx, double vz)	// 공의 속도 설정
{
	this->m_velocity_x = vx;
	this->m_velocity_z = vz;
}

void CSphere::setCenter(float x, float y, float z)	// 공의 중심 설정
{
	D3DXMATRIX m;
	center_x = x;	center_y = y;	center_z = z;
	D3DXMatrixTranslation(&m, x, y, z);	// 공 옮김
	setLocalTransform(m);	// 공 실제로 옮김
}

float CSphere::getRadius(void)  const { return (float)(M_RADIUS); }

D3DXVECTOR3 CSphere::getCenter(void) const
{
	D3DXVECTOR3 org(center_x, center_y, center_z);
	return org;
}

void CSphere::setPosition(float x, float y, float z)
{
	D3DXMATRIX m;

	this->center_x = x;
	this->center_y = y;
	this->center_z = z;

	D3DXMatrixTranslation(&m, x, y, z);
	this->setLocalTransform(m);
}

const D3DXMATRIX& CSphere::getLocalTransform(void) const { return m_mLocal; }
void CSphere::setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

void CSphere::setDX(float dx)				//x, y의 변화량에 대한 getter & setter 함수
{
	m_dx = dx;
}

void CSphere::setDZ(float dz)
{
	m_dz = dz;
}

float CSphere::getDX()
{
	return m_dx;
}

float CSphere::getDZ()
{
	return m_dz;
}