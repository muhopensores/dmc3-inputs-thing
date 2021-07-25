// Created with ReClass.NET 1.1 by KN4CK3R
#include "d3d9.h"

class kbStruct
{
public:
	char pad_0000[4]; //0x0000
	float health; //0x0004
	float knockback; //0x0008
	float stun; //0x000C
	char pad_0010[308]; //0x0010
}; //Size: 0x0144
static_assert(sizeof(kbStruct) == 0x144);

class CDamageCalc
{
public:
	char pad_0000[4]; //0x0000
	class N0000029C *ptrTable; //0x0004
	class kbStruct *ptrKb; //0x0008
	class N00000274 *enemyPtr; //0x000C
	char pad_0010[48]; //0x0010
}; //Size: 0x0040
static_assert(sizeof(CDamageCalc) == 0x40);

class CCameraCtrl
{
public:
	char pad_0000[4]; //0x0000
	Vector4 UpVector; //0x0004
	Matrix3x3 N00000F96; //0x0014
	char pad_0038[20]; //0x0038
	Vector4 tgt; //0x004C
	Vector4 pos; //0x005C
}; //Size: 0x006C
static_assert(sizeof(CCameraCtrl) == 0x6C);

class CPlDante
{
public:
	//char pad_0000[4]; //0x0000
	int pad_0000; //0x0000
	uint8_t unkFlag; //0x0004
	char pad_0005[11]; //0x0005
	float workrate; //0x0010
	float speed; //0x0014
	char pad_0018[52]; //0x0018
	Vector4 Poistion; //0x004C
	Vector4 DeltaP; //0x005C
	char pad_006C[4]; //0x006C
	float weightMaybe; //0x0070
	char pad_0074[24]; //0x0074
	uint16_t angle; //0x008C
	char pad_008E[110]; //0x008E
	Vector4 somePosVector; //0x00FC
	Vector4 somePrevPosVector; //0x010C
	Matrix4x4 TransformMatrix; //0x011C
	float momentumMagnitude; //0x015C cm
	float momentumDelta; //0x0160
	char pad_0164[9772]; //0x0164
	uint32_t animationStatus01; //0x2790
	char pad_2794[4]; //0x2794
	uint32_t animationStatusPrev; //0x2798
	char pad_279C[4]; //0x279C
	uint8_t animationFlagUnk; //0x27A0
	char pad_27A1[35]; //0x27A1
	float someAnimationVelocityFloat; //0x27C4 walking when below 2
	char pad_27C8[144]; //0x27C8
	uint16_t stickDirection; //0x2858
	uint16_t someOtherDirection; //0x285A
	uint16_t anotherDirection; //0x285C
	uint16_t idkDirection; //0x285E
	uint16_t viewDirection; //0x2860
	char pad_2862[546]; //0x2862

	void plr_sky_star_velocity_sub_5A5EC0(uint16_t angle, float tweak);
}; //Size: 0x2A84
static_assert(sizeof(CPlDante) == 0x2A84);


class devil3StaticPxShaderArray
{
public:
	class devil3pxShader *shaders[6]; //0x0000
	char pad_0018[60]; //0x0018
}; //Size: 0x0054
//static_assert(sizeof(devil3StaticPxShaderArray) == 0x54);

class devil3pxShader
{
public:
	char pad_0000[4]; //0x0000
	char name[32]; //0x0004
	char pad_0024[256]; //0x0024
	IDirect3DPixelShader9* pShader; //0x0124
}; //Size: 0x0128
static_assert(sizeof(devil3pxShader) == 0x128);

class CCharTableMgrPart
{
public:
	union {
		struct {
			uint8_t bank_id; //0x0000
			uint8_t motion_id; //0x0001
		};
		uint16_t current_anim; //0x0000
	};
	char pad_0002[6]; //0x0002
	float time; //0x0008
	char pad_000C[104]; //0x000C
	float anim_frame; //0x0074
	float loop_point; //0x0078
}; //Size: 0x007C
static_assert(sizeof(CCharTableMgrPart) == 0x7C);
