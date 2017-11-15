//
// File: virtualLego.cpp
//
// Original Author: 박창현 Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#include "d3dUtility.h"
#include <d3d10.h>
#include <D3DX10.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cassert>

enum { PLAYER1, PLAYER2 };

using namespace std;

//Playere
#define NORMAL_MODE 0
#define BACK_MODE 1
#define AHEAD_MODE 2
#define BALL_COUNT 16
#define HOLE_COUNT 6
#define FVF_VERTEX    D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1

IDirect3DDevice9* Device = NULL;

// window size
const int Width = 1024;
const int Height = 768;
int play_mode;
int playermode = PLAYER1;
int player1 = 0;
int player2 = 7;
int threeOut = 0;
//bool player[16] = { false, };
bool eigthball = false;
bool tempState = true;

struct _VERTEX
{
	D3DXVECTOR3 pos; // vertex position
	D3DXVECTOR3 norm; // vertex normal
	float tu; // texture coordinates
	float tv;
};

// There are BALL_COUNT balls
// initialize the position (coordinate) of each ball (ball[0] ~ ball[BALL_COUNT])

//0~6번까지는 plyaer1 ball, 7~13~ player 2 ball 14는 blackball 15는 흰공 //배열 위치만 기억 --> 배치는 섞음

const float spherePos[BALL_COUNT][2] = { { 1.5f,0.0f },{ 1.9f,0.25f },{ 1.9f,-0.25f },{ 2.3f,0.45f },{ 2.3f,0.0f },{ 2.3f,-0.45f },{ 2.7f,0.8f },{ 2.7f,0.3f },{ 2.7f,-0.3f },{ 2.7f,-0.8f },{ 3.1f, 0.95f },{ 3.1f,0.45f },{ 3.1f,0.0f },{ 3.1f,-0.45f },{ 3.1f,-0.95f },{ -3.0f,0.0f } };

const float holePos[HOLE_COUNT][2] = { { 4.2f,2.7f } ,{ 0.2f,2.7f } ,{ -4.2f,2.7f } ,{ 4.2f,-2.7f } ,{ 0.2f,-2.7f } ,{ -4.2f,-2.7f } };
// initialize the color of each ball (ball[0] ~ ball[BALL_COUNT])
const D3DXCOLOR sphereColor[BALL_COUNT] = { d3d::MAGENTA, d3d::MAGENTA, d3d::MAGENTA, d3d::MAGENTA, d3d::MAGENTA, d3d::MAGENTA, d3d::MAGENTA, d3d::MAGENTA,d3d::MAGENTA,d3d::MAGENTA,d3d::MAGENTA,d3d::MAGENTA,d3d::MAGENTA,d3d::MAGENTA,d3d::BLACK,d3d::WHITE };
const D3DXCOLOR holeColor[HOLE_COUNT] = { d3d::BLACK,d3d::BLACK, d3d::BLACK, d3d::BLACK, d3d::BLACK, d3d::BLACK };

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.997
// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------


class CSphere {
private:
	float					center_x, center_y, center_z;
	float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;
	float					pre_center_x, pre_center_z;
	D3DXMATRIX				matBallRoll;
	IDirect3DTexture9* Texture = nullptr;
	// 이미지 texture
	// 이전 위치 보관, 충돌 시에 사용해야 함
public:

	bool isHole;
	int	ballNum;

	CSphere(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_radius = 0;	// 공 반지름
		m_velocity_x = 0;	// 공 x 속도
		m_velocity_z = 0;	// 공 z 속도
		m_pSphereMesh = NULL;	// ??
	}
	~CSphere(void) {}

public:
	void setPosition(float x, float y, float z)	// 공 위치 설정
	{
		D3DXMATRIX m;

		this->center_x = x;
		this->center_y = y;
		this->center_z = z;

		D3DXMatrixTranslation(&m, x, y, z);	// 공 옮김
		this->setLocalTransform(m);	// 공 실제로 옮김
	}

	D3DXVECTOR3 getPosition() const	// 공 위치 get
	{
		D3DXVECTOR3 org(center_x, center_y, center_z);
		return org;
	}

	void adjustPosition(CSphere& ball) {	// 공 위치 조정
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
	bool create(IDirect3DDevice9* pDevice, int num, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;

		m_mtrl.Ambient = d3d::WHITE;
		m_mtrl.Diffuse = d3d::WHITE;
		m_mtrl.Specular = d3d::WHITE;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		string filePath = "./image/" + to_string(num + 1) + ".jpg";
		this->m_pSphereMesh = _createMappedSphere(pDevice);

		if (FAILED(D3DXCreateTextureFromFile(pDevice, filePath.c_str(), &Texture)))
		{
			return false;
		}
		return true;

	}

	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE) //function overloading for blue ball and 6 balck hole
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

	void destroy(void)
	{
		if (m_pSphereMesh != NULL) {
			m_pSphereMesh->Release();
			d3d::Release<IDirect3DTexture9*>(Texture);
			m_pSphereMesh = NULL;
		}
	}
	LPD3DXMESH _createMappedSphere(IDirect3DDevice9* pDev)
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
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		pDevice->SetTexture(0, Texture);
		m_pSphereMesh->DrawSubset(0);
	}

	bool hasIntersected(CSphere& ball) const noexcept	// 공 충돌 판정
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
	bool isinOrder(int ballN, int player) { //plyaer 1의 공이 0~6까지 //player2가 7~~13까지 // 14는 black ball이고 15은 white ball
											//true면 턴 안바뀜 // false면 턴 바뀜
		if (player == 0) {
			if (player1 != 6 && ballN == 14) {
				return false;
			}
			if (player1 == 6 && ballN == 14) {
				return true;
			}
			if (player1 == ballN) {
				player1++;
				return true;
			}

		}
		else if (player == 1) {
			if (player2 != 13 && ballN == 14) {
				return false;
			}
			if (player2 == 13 && ballN == 14) {
				return true;
			}

			if (player2 == ballN + 7) {
				player2++;
				return true;
			}

		}
		return false;
	}

	void hitBy(CSphere& ball) noexcept	// 공 충돌 시 처리
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
				if (this->ballNum != 15 && this->ballNum != 14) this->setPosition(99.99f, this->center_y, 99.99f);


				else {
					/*this->setPosition(0.0f, this->center_y, 0.0f);
					this->m_velocity_x = 0;
					this->m_velocity_z = 0;*/
				}
			}

		}
	}


	void ballUpdate(float timeDiff)
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

			D3DXMATRIX tmp;
			D3DXVECTOR3 c(this->m_velocity_z, 0, -this->m_velocity_x);

			float force = sqrt(pow(this->m_velocity_x, 2) + pow(this->m_velocity_z, 2));
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

	double getVelocity_X() { return this->m_velocity_x; }
	double getVelocity_Z() { return this->m_velocity_z; }

	void setPower(double vx, double vz)	// 공의 속도 설정
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setCenter(float x, float y, float z)	// 공의 중심 설정
	{
		D3DXMATRIX m;
		center_x = x;	center_y = y;	center_z = z;
		D3DXMatrixTranslation(&m, x, y, z);	// 공 옮김
		setLocalTransform(m);	// 공 실제로 옮김
	}

	float getRadius(void)  const { return (float)(M_RADIUS); }
	const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	D3DXVECTOR3 getCenter(void) const
	{
		D3DXVECTOR3 org(center_x, center_y, center_z);
		return org;
	}

	void setDX(float dx)				//x, y의 변화량에 대한 getter & setter 함수
	{
		m_dx = dx;
	}

	void setDZ(float dz)
	{
		m_dz = dz;
	}

	float getDX()
	{
		return m_dx;
	}

	float getDZ()
	{
		return m_dz;
	}


private:
	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh*              m_pSphereMesh;
	float                   m_unit;
	float					m_dx;
	float					m_dz;
};


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
	CHole(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_radius = 0;
		m_velocity_x = 0;
		m_velocity_z = 0;
		m_pSphereMesh = NULL;
		isHole = true;
	}
	~CHole(void) {}





};
// -----------------------------------------------------------------------------
// CEightball class definition
// -----------------------------------------------------------------------------
class CEightball : public CSphere {

private:
	float					center_x, center_y, center_z;
	float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;
	float					pre_center_x, pre_center_z;
	D3DXMATRIX				matBallRoll;
};

class Referee {
private:
	CSphere* sp;
	CHole* hp;
	int i, j, k;



public:
	Referee(CSphere* s, CHole* h) {

		sp = s;
		hp = h;



	}

	void isWBinHole() {
		for (int i = 0; i < 15; i++) {
			if (!sp[15].hasIntersected(sp[i])) {
				for (int j = 0; j < 6; j++) {
					if (sp[15].hasIntersected(hp[j])) {

						sp[15].setPosition(0.0f, M_RADIUS, 0.0f);
						sp[15].setPower(0.0f, 0.0f);
						threeOut++;
						cout << "goalin" << threeOut << endl;

						if (threeOut == 3) {
							playermode++;
							playermode = playermode % 2;
							threeOut = 0;
						}

					}
				}

			}

		}
	}

	void isWorthNothing() {
		bool spbool = true, hpbool = true;
		if ((abs(sp[15].getVelocity_X()) > 0.01) && (abs(sp[15].getVelocity_Z()) > 0.01)) {
			for (i = 0; i < BALL_COUNT; i++) {
				for (j = 0; j < HOLE_COUNT; j++) {
					spbool = sp[i].hasIntersected(hp[j]);
				}

			}

			cout << "worth!" << " " << spbool << endl;
		}
		if (!spbool && (abs(sp[15].getVelocity_X()) < 0.1) && (abs(sp[15].getVelocity_Z()) < 0.1)) {

			cout << "you worth nothing" << endl;


		}



	}
	void isEightBallLegit() {


		for (k = 0; k < HOLE_COUNT; k++) {

			if (sp[14].hasIntersected(hp[k])) {
				if (playermode == PLAYER1) {
					if (player1 == 7) cout << "player1 legit" << endl;
					else {
						cout << "player1 not legit" << endl;
						sp[14].setPosition(99.99f, M_RADIUS, 99.99f);
						if (playermode == PLAYER1) playermode = PLAYER2;
					}
				}
				else if (playermode == PLAYER2) {
					if (player2 == 14) cout << "player2 legit" << endl;
					else {
						cout << "player2 not legit" << endl;
						sp[14].setPosition(99.99f, M_RADIUS, 99.99f);
						if (playermode == PLAYER2) playermode = PLAYER1;
					}


				}
			}


		}


	}


};

class CCue {
	float					m_x;
	float					m_z;
	float                   m_width;
	float                   m_depth;
	float					m_height;

public:
	CCue(void)	// 큐 생성자
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_width = 0;
		m_depth = 0;
		m_pBoundMesh = NULL;

	}
	~CCue(void) {}

	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
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

	void destroy(void)
	{
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release();
			m_pBoundMesh = NULL;
		}
	}

	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{

		D3DXLoadMeshFromX("./image/cuew.x", D3DXMESH_MANAGED, Device, NULL, &pMaterialBuffer, NULL, &m_numMaterials, &m_pBoundMesh);


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

	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getHeight(void) const { return M_HEIGHT; }


private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

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



// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:

	float					m_x;
	float					m_z;
	float                   m_width;
	float                   m_depth;
	float					m_height;

public:
	CWall(void)	// 벽 생성자
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_width = 0;
		m_depth = 0;
		m_pBoundMesh = NULL;
	}
	~CWall(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
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

	void destroy(void)
	{
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release();
			m_pBoundMesh = NULL;
		}
	}

	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);



	}

	bool hasIntersected(CSphere& ball)	// 공과의 충돌 판정
	{
		D3DXVECTOR3 temp_coor = ball.getCenter();				//공의 위치
		if (temp_coor.x >= 4.5 - M_RADIUS || temp_coor.z >= 3 - M_RADIUS ||
			temp_coor.x <= -(4.5 - M_RADIUS) || temp_coor.z <= -(3 - M_RADIUS)) {
			return true;
		}
		else return false;
	}

	void hitBy(CSphere& ball)	// 공과의 충돌 처리
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

	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getHeight(void) const { return M_HEIGHT; }



private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh*              m_pBoundMesh;
};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
	CLight(void)
	{
		static DWORD i = 0;
		m_index = i++;
		D3DXMatrixIdentity(&m_mLocal);
		::ZeroMemory(&m_lit, sizeof(m_lit));
		m_pMesh = NULL;
		m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_bound._radius = 0.0f;
	}
	~CLight(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
	{
		if (NULL == pDevice)
			return false;
		if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
			return false;

		m_bound._center = lit.Position;
		m_bound._radius = radius;

		m_lit.Type = lit.Type;
		m_lit.Diffuse = lit.Diffuse;
		m_lit.Specular = lit.Specular;
		m_lit.Ambient = lit.Ambient;
		m_lit.Position = lit.Position;
		m_lit.Direction = lit.Direction;
		m_lit.Range = lit.Range;
		m_lit.Falloff = lit.Falloff;
		m_lit.Attenuation0 = lit.Attenuation0;
		m_lit.Attenuation1 = lit.Attenuation1;
		m_lit.Attenuation2 = lit.Attenuation2;
		m_lit.Theta = lit.Theta;
		m_lit.Phi = lit.Phi;
		return true;
	}
	void destroy(void)
	{
		if (m_pMesh != NULL) {
			m_pMesh->Release();
			m_pMesh = NULL;
		}
	}
	bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return false;

		D3DXVECTOR3 pos(m_bound._center);
		D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
		D3DXVec3TransformCoord(&pos, &pos, &mWorld);
		m_lit.Position = pos;

		pDevice->SetLight(m_index, &m_lit);
		pDevice->LightEnable(m_index, TRUE);
		return true;
	}

	void draw(IDirect3DDevice9* pDevice)
	{
		if (NULL == pDevice)
			return;
		D3DXMATRIX m;
		D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
		pDevice->SetTransform(D3DTS_WORLD, &m);
		pDevice->SetMaterial(&d3d::WHITE_MTRL);
		m_pMesh->DrawSubset(0);
	}

	D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
	DWORD               m_index;
	D3DXMATRIX          m_mLocal;
	D3DLIGHT9           m_lit;
	ID3DXMesh*          m_pMesh;
	d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[4];
CSphere	g_sphere[BALL_COUNT];
CHole g_hole[HOLE_COUNT];
CSphere	g_target_blueball;
CLight	g_light;

CCue	g_cue;

double g_camera_pos[3] = { 0.0, 7.0, -8.0 };

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// Font
LPD3DXFONT m_pFont;
D3DXFONT_DESC desc;

void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup()
{
	int i;

	D3DXMatrixIdentity(&g_mWorld);
	D3DXMatrixIdentity(&g_mView);
	D3DXMatrixIdentity(&g_mProj);

	//Create Cue
	if (false == g_cue.create(Device, 1, -1, 9, 0.5f, 0.12, d3d::WHITE)) return false;
	g_cue.setPosition(0.0f, 1.0f, 0.0f);

	// create plane and set the position
	if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0.12f, 3.06f);
	if (false == g_legowall[1].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(0.0f, 0.12f, -3.06f);
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(4.56f, 0.12f, 0.0f);
	if (false == g_legowall[3].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[3].setPosition(-4.56f, 0.12f, 0.0f);

	// create four balls and set the position
	for (i = 0; i<BALL_COUNT; i++) {

		if (false == g_sphere[i].create(Device, i, sphereColor[i])) return false;
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
		g_sphere[i].setPower(0, 0);
		g_sphere[i].ballNum = i;
	}
	//create 6 holess and set the position
	for (i = 0; i<HOLE_COUNT; i++) {
		if (false == g_hole[i].create(Device, holeColor[i])) return false;
		g_hole[i].setCenter(holePos[i][0], (float)M_RADIUS, holePos[i][1]);
		g_hole[i].setPower(0, 0);
	}

	// create blue ball for set direction
	if (false == g_target_blueball.create(Device, d3d::BLUE)) return false;
	g_target_blueball.setCenter(.0f, (float)M_RADIUS, .0f);

	// light setting 
	D3DLIGHT9 lit;
	::ZeroMemory(&lit, sizeof(lit));
	lit.Type = D3DLIGHT_POINT;
	lit.Diffuse = d3d::WHITE;
	lit.Specular = d3d::WHITE * 0.9f;
	lit.Ambient = d3d::WHITE * 0.9f;
	lit.Position = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
	lit.Range = 100.0f;
	lit.Attenuation0 = 0.0f;
	lit.Attenuation1 = 0.9f;
	lit.Attenuation2 = 0.0f;
	if (false == g_light.create(Device, lit))
		return false;

	// Position and aim the camera.
	D3DXVECTOR3 pos(0.0f, 5.0f, -8.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);

	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
		(float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

	// Set render states.
	Device->SetRenderState(D3DRS_LIGHTING, TRUE);
	Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	g_light.setLight(Device, g_mWorld);

	// Set the font
	memset(&desc, 0, sizeof(D3DXFONT_DESC));
	desc.CharSet = HANGUL_CHARSET;
	strcpy(desc.FaceName, "Consolas");
	desc.Height = 20;
	desc.Width = 10;
	desc.Weight = FW_NORMAL;
	desc.Quality = DEFAULT_QUALITY;
	desc.MipLevels = 1;
	desc.Italic = 0;
	desc.OutputPrecision = OUT_DEFAULT_PRECIS;
	desc.PitchAndFamily = FF_DONTCARE;

	D3DXCreateFontIndirect(Device, &desc, &m_pFont);

	return true;
}

void Cleanup(void)
{
	g_legoPlane.destroy();
	for (int i = 0; i < 4; i++) {
		g_legowall[i].destroy();
	}
	destroyAllLegoBlock();
	g_light.destroy();
}

// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
	int i = 0;
	int j = 0;
	RECT rect;
	char str[100];

	int k = 0;


	D3DXMATRIX m;

	float x = 0;
	float y = 2;
	float z = 0;

	D3DXMatrixTranslation(&m, x, y, z);


	Referee referee(g_sphere, g_hole);

	if (Device)
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();

		SetRect(&rect, 0, 0, 800, 600);
		ZeroMemory(str, 100);
		// display the font
		if (!playermode)

		{
			sprintf(str, "Player 1");
			m_pFont->DrawTextA(NULL, str, -1, &rect, DT_LEFT, D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff));
		}
		else
		{
			sprintf(str, "Player 2");
			m_pFont->DrawTextA(NULL, str, -1, &rect, DT_LEFT, D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff));
		}

		// update the font
		if (tempState == false)
		{
			playermode++;
			playermode = playermode % 2;
			threeOut = 0;
			tempState = true;
		}

		// update the position of each ball. during update, check whether each ball hit by walls.
		for (i = 0; i < BALL_COUNT; i++) {
			g_sphere[i].ballUpdate(timeDelta);

			for (j = 0; j < BALL_COUNT; j++) { g_legowall[i].hitBy(g_sphere[j]); }
		}
		for (i = 0; i < HOLE_COUNT; i++) {
			g_hole[i].ballUpdate(timeDelta);

			for (j = 0; j < HOLE_COUNT; j++) { g_legowall[i].hitBy(g_hole[j]); }
		}

		// check whether any two balls hit together and update the direction of balls
		for (i = 0; i < BALL_COUNT; i++) {
			for (j = 0; j < BALL_COUNT; j++) {
				if (i >= j) { continue; }
				g_sphere[i].hitBy(g_sphere[j]);
			}
			for (k = 0; k < HOLE_COUNT; k++) {
				if (i >= j) { continue; }
				g_sphere[i].hitBy(g_hole[k]);

			}
		}
		referee.isWBinHole();
		//referee.isWorthNothing();
		referee.isEightBallLegit();

		//Draw cue
		g_cue.draw(Device, g_mWorld);

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);
		for (i = 0; i<4; i++) {
			g_legowall[i].draw(Device, g_mWorld);

		}
		for (i = 0; i < BALL_COUNT; i++) {
			g_sphere[i].draw(Device, g_mWorld);
		}
		for (i = 0; i < HOLE_COUNT; i++) {
			g_hole[i].draw(Device, g_mWorld);
		}
		g_target_blueball.draw(Device, g_mWorld);
		g_light.draw(Device);

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture(0, NULL);
	}

	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static bool isReset = true;
	static int old_x = 0;
	static int old_y = 0;
	static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

	switch (msg) {
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_ESCAPE:
			::DestroyWindow(hwnd);
			break;
		case VK_RETURN:
			if (NULL != Device) {
				wire = !wire;
				Device->SetRenderState(D3DRS_FILLMODE,
					(wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
			}
			break;
		case VK_SPACE:

			D3DXVECTOR3 targetpos = g_target_blueball.getCenter();
			D3DXVECTOR3	whitepos = g_sphere[15].getCenter();
			double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
				pow(targetpos.z - whitepos.z, 2)));		// 기본 1 사분면
			if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }	//4 사분면
			if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = PI - theta; } //2 사분면
			if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0) { theta = PI + theta; } // 3 사분면
			double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
			g_sphere[15].setPower(distance * cos(theta), distance * sin(theta));

			break;

		}
		break;
	}

	case WM_MOUSEMOVE:
	{
		int new_x = LOWORD(lParam);
		int new_y = HIWORD(lParam);
		float dx;
		float dy;

		if (LOWORD(wParam) & MK_RBUTTON) {

			if (isReset) {
				isReset = false;
			}
			else {
				D3DXVECTOR3 vDist;
				D3DXVECTOR3 vTrans;
				D3DXMATRIX mTrans;
				D3DXMATRIX mX;
				D3DXMATRIX mY;

				switch (move) {
				case WORLD_MOVE:
					dx = (old_x - new_x) * 0.01f;
					dy = (old_y - new_y) * 0.01f;
					D3DXMatrixRotationY(&mX, dx);
					D3DXMatrixRotationX(&mY, dy);
					g_mWorld = g_mWorld * mX * mY;

					break;
				}
			}

			old_x = new_x;
			old_y = new_y;

		}
		else {
			isReset = true;

			if (LOWORD(wParam) & MK_LBUTTON) {
				dx = (old_x - new_x);// * 0.01f;
				dy = (old_y - new_y);// * 0.01f;

				D3DXVECTOR3 coord3d = g_target_blueball.getCenter();
				g_target_blueball.setCenter(coord3d.x + dx*(-0.007f), coord3d.y, coord3d.z + dy*0.007f);
			}
			old_x = new_x;
			old_y = new_y;

			move = WORLD_MOVE;
		}
		break;
	}
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	srand(static_cast<unsigned int>(time(NULL)));

	if (!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	Device->Release();

	return 0;
}