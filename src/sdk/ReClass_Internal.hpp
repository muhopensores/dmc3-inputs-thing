// #include "sdk/Math.hpp"
// #include <stdint.h>
// Created with ReClass.NET 1.1 by KN4CK3R

class kbStruct {
public:
    char pad_0000[4];   // 0x0000
    float health;       // 0x0004
    float knockback;    // 0x0008
    float stun;         // 0x000C
    char pad_0010[308]; // 0x0010
};                      // Size: 0x0144
static_assert(sizeof(kbStruct) == 0x144);

class CDamageCalc {
public:
    char pad_0000[4];          // 0x0000
    class N0000029C* ptrTable; // 0x0004
    class kbStruct* ptrKb;     // 0x0008
    class N00000274* enemyPtr; // 0x000C
    char pad_0010[48];         // 0x0010
};                             // Size: 0x0040
static_assert(sizeof(CDamageCalc) == 0x40);

class CPlDante {
public:
    int pad_0000;                              // 0x0000
    uint8_t unkFlag;                           // 0x0004
    char pad_0005[11];                         // 0x0005
    float workrate;                            // 0x0010
    float speed;                               // 0x0014
    char pad_0018[52];                         // 0x0018
    Vector4 Poistion;                          // 0x004C
    Vector4 DeltaP;                            // 0x005C
    char pad_006C[4];                          // 0x006C
    float weightMaybe;                         // 0x0070
    char pad_0074[24];                         // 0x0074
    uint16_t angle;                            // 0x008C
    char pad_008E[110];                        // 0x008E
    Vector4 somePosVector;                     // 0x00FC
    Vector4 somePrevPosVector;                 // 0x010C
    Matrix4x4 TransformMatrix;                 // 0x011C
    float momentumMagnitude;                   // 0x015C cm
    float momentumDelta;                       // 0x0160
    char pad_0164[9736];                       // 0x0164
    class VelLookupTable* velocityLookupTable; // 0x276C 0x276c zeroesUpMomentum_sub_5A4CB20
    char pad_2770[32];                         // 0x2770
    uint32_t animationStatus01;                // 0x2790
    char pad_2794[4];                          // 0x2794
    uint32_t animationStatusPrev;              // 0x2798
    char pad_279C[4];                          // 0x279C
    uint8_t animationFlagUnk;                  // 0x27A0
    char pad_27A1[35];                         // 0x27A1
    float someAnimationVelocityFloat;          // 0x27C4 walking when below 2
    char pad_27C8[144];                        // 0x27C8
    uint16_t stickDirection;                   // 0x2858
    uint16_t someOtherDirection;               // 0x285A
    uint16_t anotherDirection;                 // 0x285C
    uint16_t idkDirection;                     // 0x285E
    uint16_t viewDirection;                    // 0x2860
    char pad_2862[22];                         // 0x2862
    uint8_t N00000AD3;                         // 0x2878
    uint8_t N00000FDA;                         // 0x2879
    char pad_287A[2];                          // 0x287A
    uint32_t N00000AD4;                        // 0x287C 0x287c edx at start zeroesUpMomentum_sub_5A4CB0ú
    char pad_2880[5044];                       // 0x2880
    uint32_t rg_meter_uint;                    // 0x3C34
    float rg_meter_flt;                        // 0x3C38
    float block_timer;                         // 0x3C3C
    float release_timer;                       // 0x3C40
    char pad_3C44[108];                        // 0x3C44
};                                             // Size: 0x3CB0
static_assert(sizeof(CPlDante) == 0x3CB0);

class devil3StaticPxShaderArray {
public:
    class devil3pxShader* shaders[6]; // 0x0000
    char pad_0018[60];                // 0x0018
};                                    // Size: 0x0054
// static_assert(sizeof(devil3StaticPxShaderArray) == 0x54);

class devil3pxShader {
public:
    char pad_0000[4];               // 0x0000
    char name[32];                  // 0x0004
    char pad_0024[256];             // 0x0024
    IDirect3DPixelShader9* pShader; // 0x0124
};                                  // Size: 0x0128
static_assert(sizeof(devil3pxShader) == 0x128);

class CCharTableMgrPart {
public:
    union {
        struct {
            uint8_t bank_id;   // 0x0000
            uint8_t motion_id; // 0x0001
        };
        uint16_t current_anim; // 0x0000
    };
    char pad_0002[6];   // 0x0002
    float time;         // 0x0008
    char pad_000C[104]; // 0x000C
    float anim_frame;   // 0x0074
    float loop_point;   // 0x0078
};                      // Size: 0x007C
static_assert(sizeof(CCharTableMgrPart) == 0x7C);

class cCameraCtrl {
public:
    Vector3 upVector;    // 0x0004
    float fltUnused0;    // 0x0010
    float FOV;           // 0x0014
    float roll_radians;  // 0x0018
    Matrix4x4 transform; // 0x001C
    Vector4 pos;         // 0x005C
    Vector4 lookat;      // 0x006C
    friend int cCameraCtrl__something_idk_sub_416880(cCameraCtrl& this_ptr);
    virtual void Function0();
    virtual void Function1();
    virtual void Function2();
    virtual void Function3();
    virtual void Function4();
    virtual void Function5();
    virtual void Function6();
    virtual void Function7();
    virtual void Function8();
    virtual void Function9();
}; // Size: 0x007C
static_assert(sizeof(cCameraCtrl) == 0x7C);

class cCameraPlayer {
public:
    char pad_0000[4];    // 0x0000
    Vector3 up;          // 0x0004
    float fltUnused;     // 0x0010
    float FOV;           // 0x0014
    float roll_radians;  // 0x0018
    Vector4 q1;          // 0x001C
    Vector4 q2;          // 0x002C
    Matrix4x4 N00000A9F; // 0x003C
    char pad_007C[1056]; // 0x007C
};                       // Size: 0x049C
static_assert(sizeof(cCameraPlayer) == 0x49C);

// Created with ReClass.NET 1.2 by KN4CK3R

enum class GAME_STATE : int32_t
{
    GAME_MAIN_CUSTOMIZE = 9,
    GAME_MAIN_DEAD = 6,
    GAME_MAIN_DELETE = 11,
    GAME_MAIN_DOOR = 2,
    GAME_MAIN_END = 12,
    GAME_MAIN_ITEMGET = 7,
    GAME_MAIN_MAIN = 1,
    GAME_MAIN_MESSAGE = 8,
    GAME_MAIN_OPTIONS = 5,
    GAME_MAIN_PAUSE = 3,
    GAME_MAIN_SAVE = 10,
    GAME_MAIN_STATUS = 4,
    UNK = 0
};

enum class SCREEN_EFFECT : int32_t
{
    DOOR = 1,
    IDK = 3,
    MISSION_END = 2,
    NONE = 0
};

class CSceneGameMain
{
public:
    uint32_t vtable; //0x0000 for now- just to check vtable pointer
    // this probably has some useful virtual functions that i cba to reverse
    char pad_0004[8]; //0x0004
    class CGameW *cGameW_ptr; //0x000C
    uint16_t currentLevel; //0x0010
    char pad_0012[6]; //0x0012
    GAME_STATE flag_state; //0x0018
    char pad_001C[12]; //0x001C
    SCREEN_EFFECT screen_effect; //0x0028
    char pad_002C[1040]; //0x002C
}; //Size: 0x043C

class CGameW
{
public:
    char pad_0000[4]; //0x0000
    class CEventMission *cEvtMiss_ptr; //0x0004
}; //Size: 0x0008

class CEventMission
{
public:
    char pad_0000[244]; //0x0000
    uint16_t current_map; //0x00F4
    uint16_t N00000A3F; //0x00F6
    uint16_t next_map; //0x00F8
    uint16_t N00000A42; //0x00FA
    char pad_00FC[4]; //0x00FC
}; //Size: 0x0100

struct Devil3Texture
{
    char pad_0000[4];
    uint32_t width;
    uint32_t height;
    void *decodeDataPointer;
    void *decodeFunctionPointer;
    D3DFORMAT format;
    IDirect3DTexture9* texturePointer;
    bool dirty;
    char pad_0020[2080];
};

#pragma region WIP
// Created with ReClass.NET 1.2 by KN4CK3R

class TextureTableEntry
{
public:
    class TextureData *ptrTextureData; //0x0000
    class Devil3Texture *ptrD3Texture; //0x0004
}; //Size: 0x0008

class TextureTable
{
public:
    class TextureTableEntry* textures; //0x0000
}; //Size: 0x0208

class unkTextureData
{
public:
    char pad_0000[4]; //0x0000
}; //Size: 0x0004

class N000000F9
{
public:
    char pad_0000[4]; //0x0000
}; //Size: 0x0004

class N0000011E
{
public:
    char pad_0000[4]; //0x0000
}; //Size: 0x0004

class TextureData
{
public:
    char pad_0000[260]; //0x0000
}; //Size: 0x0104

#pragma endregion
