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

#include "CObject.h"
#include "CWall.h"
#include "CSphere.h"
#include "CLight.h"
#include "CHole.h"
#include "CCUe.h"

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


// There are BALL_COUNT balls
// initialize the position (coordinate) of each ball (ball[0] ~ ball[BALL_COUNT])

//0~6번까지는 plyaer1 ball, 7~13~ player 2 ball 14는 blackball 15는 흰공 //배열 위치만 기억 --> 배치는 섞음//2.3f,0,0f,-3.0f,0,0f

const float spherePos[BALL_COUNT][2] = { { 1.5f,0.0f },{ 1.9f,0.25f },{ 1.9f,-0.25f },{ 2.3f,0.45f },{ 3.1f,-0.95f },{ 2.3f,-0.45f },{ 2.7f,0.8f },{ 2.7f,0.3f },{ 2.7f,-0.3f },{ 2.7f,-0.8f },{ 3.1f, 0.95f },{ 3.1f,0.45f },{ 3.1f,0.0f },{ 3.1f,-0.45f },{ 2.3f,0.0f },{ -3.0f,0.0f } };

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

class Referee {
private:
	CSphere* sp;
	CHole* hp;
	char str[100];
	int i, j, k;

public:
	Referee(CSphere* s, CHole* h) {
		sp = s;
		hp = h;
	}

	/*bool isinOrder(int ballN, int player) { //plyaer 1의 공이 0~6까지 //player2가 7~~13까지 // 14는 black ball이고 15은 white ball
	//true면 턴 안바뀜 // false면 턴 바뀜
	if (player == 0) {

	if (player1 == ballN) {
	player1++;
	return true;
	}
	else if (player1 != ballN) {

	return false;

	}

	}
	else if (player == 1) {

	if (player2 == ballN + 7) {
	player2++;
	return true;
	}
	else if (player2 != ballN) {

	return false;

	}

	}

	}*/

	bool isinOrder(int ballN, int player) { //plyaer 1의 공이 0~6까지 //player2가 7~~13까지 // 14는 black ball이고 15은 white ball
											//true면 턴 안바뀜 // false면 턴 바뀜
		if (player == 0) {
			if (ballN<7) {
				player1++;
				return true;
			}
			else {
				return false;
			}
		}

		else if (player == 1) {
			if (ballN >= 7 && ballN <= 13) {
				player2++;
				return true;
			}
			else {
				return false;
			}
		}
	}

	void holeDetect() { //들어가는 ball 확인해서 맞으면 진행, 틀린 순서면 메세지 박스 띄우고 다른 플레이어로 진행
		memset(str, 0, 100);
		bool temp = false;
		for (i = 0; i < HOLE_COUNT; i++) {
			for (j = 0; j < BALL_COUNT; j++) {
				if (sp[j].hasIntersected(hp[i])) {
					if (j != 15 && j != 14) {
						if (isinOrder(j, playermode)) {
							sp[j].setPower(0.0f, 0.0f);
							sp[j].setPosition(99.99f, M_RADIUS, 99.99f);
							cout << j << " th ball goalin" << endl;
						}
						else {
							cout << j << " th ball goalin" << endl;
							/*							for (i = 0; i < BALL_COUNT; i++) {
							sp[i].setPower(0.0f, 0.0f);
							}*/
							//sp[j].setPosition(1.0f, M_RADIUS ,1.0f);
							//sprintf(str, "PLAYER %d GOT WRONG BALL", playermode + 1);
							playermode++;
							playermode = playermode % 2;
							//MessageBox(nullptr, str, "NEXT PLAYER`S TURN", 0);
						}
					}
				}
			}

		}
	}

	void isWBinHole() { // 하얀공이 구멍에 들어갔을 때
		for (int i = 0; i < BALL_COUNT - 1; i++) {
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

	void isWorthNothing() { // 아무것도 못넣고 하얀공이 멈춘경우. 지금 구현중
		bool spbool = true, hpbool = true, start = false;
		if ((abs(sp[15].getVelocity_X()) > 0.1) && (abs(sp[15].getVelocity_Z()) > 0.1)) start = true;
		//cout << "worth!" << " " << spbool << endl;

		if (start && (abs(sp[15].getVelocity_X()) < 0.1) && (abs(sp[15].getVelocity_Z()) < 0.1)) {

			cout << "you worth nothing" << endl;
		}
	}
	void isEightBallLegit() { // 플레이어가 자신의 모든 공을 다 넣지 않고 8번공을 넣을경우 플레이어가 바뀜
		for (k = 0; k < HOLE_COUNT; k++) {
			if (sp[14].hasIntersected(hp[k])) {
				sp[14].setPower(0.0f, 0.0f);
				sp[14].setPosition(99.99f, M_RADIUS, 99.99f);
				if (playermode == PLAYER1) {
					if (player1 == 6) {
						cout << "player1 legit" << endl;
						MessageBox(nullptr, "PLAYER 1 WIN", "GAME WIN", 0);
						exit(0);
					}
					else {
						cout << "player1 not legit" << endl;
						MessageBox(nullptr, "PLAYER 1 LOSE", "GAME OVER", 0);
						exit(0);
						if (playermode == PLAYER1) playermode = PLAYER2;
						break;
					}
				}
				else if (playermode == PLAYER2) {
					if (player2 == 13) {
						cout << "player2 legit" << endl;
						MessageBox(nullptr, "PLAYER 1 WIN", "GAME WIN", 0);
						exit(0);

					}
					else {
						cout << "player2 not legit" << endl;
						MessageBox(nullptr, "PLAYER 2 LOSE", "GAME OVER", 0);
						exit(0);
						if (playermode == PLAYER2) playermode = PLAYER1;
						break;
					}
				}
			}
		}
	}
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

		if (false == g_sphere[i].create(Device,i, sphereColor[i])) return false;
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

	if (false == g_light.create(Device, lit, 0.1))
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
	RECT rect2;
	char str[100];
	char str2[100];

	int k = 0;
	Referee referee(g_sphere, g_hole);
	

	if (Device)
	{
		
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();

		SetRect(&rect, 0, 0, 800, 600);
		SetRect(&rect2, 0, 0, 1024,768);
		ZeroMemory(str, 100);
		ZeroMemory(str2, 100);
		// display the font
		sprintf(str, "Player 1 Points : %d \nPlayer 2 Points :%d\nPLAYER %d TURN", player1,player2-7,playermode+1);
		m_pFont->DrawTextA(NULL, str, -1, &rect, DT_LEFT, D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff));
	
	/*
		if (!playermode)

		{
			sprintf(str, "Player 1 Turn");
			m_pFont->DrawTextA(NULL, str2, -1, &rect2, DT_RIGHT, D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff));
		}
		else
		{
			sprintf(str, "Player 2 Turn");
			m_pFont->DrawTextA(NULL, str2, -1, &rect2, DT_RIGHT, D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff));
		}
		*/
		// update the font
	/*	if (tempState == false)
		{
			playermode++;
			playermode = playermode % 2;
			threeOut = 0;
			tempState = true;
		}*/

		// update the position of each ball. during update, check whether each ball hit by walls.
		for (i = 0; i < BALL_COUNT; i++) {
			g_sphere[i].ballUpdate(timeDelta);

			for (j = 0; j < BALL_COUNT; j++) { g_legowall[i].hitBy(g_sphere[j]); }
		}
		for (i = 0; i < HOLE_COUNT; i++) {
			g_hole[i].ballUpdate(timeDelta);

		}

		// check whether any two balls hit together and update the direction of balls
		for (i = 0; i < BALL_COUNT; i++) {
			for (j = 0; j < BALL_COUNT; j++) {
				if (i >= j) { continue; }
				g_sphere[i].hitBy(g_sphere[j]);
			}
			
		}
		for (i = 0; i < BALL_COUNT; i++) {
			for (j = 0; j < HOLE_COUNT; j++) {
				//if (i >= j) { continue; }
				g_sphere[i].hitBy(g_hole[j]);
			}
			
		}

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
	referee.holeDetect();
	referee.isWBinHole();
	//referee.isWorthNothing();
	referee.isEightBallLegit();

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