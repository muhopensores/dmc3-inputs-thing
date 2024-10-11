#include "Devil3.hpp"
#include "mods/CustomAlolcator.hpp"

CSceneGameMain* g_devil3_main_scene_pointer {nullptr};

#if 1
struct dante_anim_table {
    CCharTableMgrPart* human = (CCharTableMgrPart*)0x41D8; // 0x01C8E7D8;
    CCharTableMgrPart* dt1   = (CCharTableMgrPart*)0x4518; // 0x01C8EB18;
    CCharTableMgrPart* dt2   = (CCharTableMgrPart*)0x4448; // 0x01C8EA48;
};

static dante_anim_table g_dante_table;

uint16_t devil3_sdk::get_buttons_pressed() {
    uint16_t* ptr = (uint16_t*)0x01C8EFF4;
    return *ptr;
}

CPlDante* devil3_sdk::get_pl_dante() {
    if (g_mem_patch_applied) {
        return (CPlDante*)0x04557540;
    }
    return (CPlDante*)0x1C8A600;
}

cCameraCtrl* devil3_sdk::get_cam_ctrl() {
    static cCameraCtrl** camera_ptr_ptr = (cCameraCtrl**)0x00B6BAAC;
    return *camera_ptr_ptr;
}

Vector2f devil3_sdk::get_window_dimensions() {
    return Vector2f{*(float*)0x00832914, *(float*)0x00832918};
}

const int devil3_sdk::get_pl_dante_style() {
    static int* current_style = (int*)0xB6B220;
    return *current_style;
}

const char devil3_sdk::get_pl_state() {
    static char* pl_state = (char*)((uintptr_t)get_pl_dante() + (ptrdiff_t)0x47DE);
    return *pl_state;
}

const bool devil3_sdk::pl_dante_is_air() {
    static char* air = (char*)((uintptr_t)get_pl_dante() + (ptrdiff_t)0x73);
    //static char* air = (char*)0x01C8A673;
    return *air != 0;
}
const bool devil3_sdk::pl_dante_is_grounded() {
#if 0
	char state = get_pl_state();
	return (
		(state == 11) ||
		(state == 13));
#else
    static bool* grounded = (bool*)((uintptr_t)get_pl_dante() + (ptrdiff_t)0x2358);
    return !*grounded;
#endif
}

const bool devil3_sdk::pl_is_dt_check() {
    static const bool* pl_dt = (bool*)((uintptr_t)get_pl_dante() + (ptrdiff_t)0x2817);
    return *pl_dt;
}

CCharTableMgrPart* devil3_sdk::pl_dante_get_human_anim_table() {
    auto base = get_pl_dante();
    return (CCharTableMgrPart*)((uintptr_t)base + (uintptr_t)g_dante_table.human);
}

CCharTableMgrPart* devil3_sdk::pl_dante_get_dt1_anim_table() {
    auto base = get_pl_dante();
    return (CCharTableMgrPart*)((uintptr_t)base + (uintptr_t)g_dante_table.dt1);
}

CCharTableMgrPart* devil3_sdk::pl_dante_get_dt2_anim_table() {
    auto base = get_pl_dante();
    return (CCharTableMgrPart*)((uintptr_t)base + (uintptr_t)g_dante_table.dt2);
}

const bool devil3_sdk::pl_dante_check_animation_id(uint16_t id) {
    auto table_human = pl_dante_get_human_anim_table();
    auto table_dt1   = pl_dante_get_dt1_anim_table();
    auto table_dt2   = pl_dante_get_dt2_anim_table();

    if (IsBadReadPtr(&table_human->current_anim, sizeof(short)) ||
        IsBadReadPtr(&table_dt1->current_anim, sizeof(short)) ||
        IsBadReadPtr(&table_dt2->current_anim, sizeof(short))) {

        return false;
    }
    return (
        ((id == table_human->current_anim) && !pl_is_dt_check()) ||
        ((id == table_dt1->current_anim) && pl_is_dt_check()) ||
        ((id == table_dt2->current_anim) && pl_is_dt_check()));
}
#else
struct dante_anim_table {
    CCharTableMgrPart* human = (CCharTableMgrPart*)0x01C8E7D8;
    CCharTableMgrPart* dt1   = (CCharTableMgrPart*)0x01C8EB18;
    CCharTableMgrPart* dt2   = (CCharTableMgrPart*)0x01C8EA48;
};

static dante_anim_table g_dante_table;

uint16_t devil3_sdk::get_buttons_pressed() {
    uint16_t* ptr = (uint16_t*)0x01C8EFF4;
    return *ptr;
}

CPlDante* devil3_sdk::get_pl_dante() {
    return (CPlDante*)0x1C8A600;
}

cCameraCtrl* devil3_sdk::get_cam_ctrl() {
    static cCameraCtrl** camera_ptr_ptr = (cCameraCtrl**)0x00B6BAAC;
    return *camera_ptr_ptr;
}

Vector2f devil3_sdk::get_window_dimensions() {
    return Vector2f{ *(float*)0x00832914, *(float*)0x00832918};
}
const int devil3_sdk::get_pl_dante_style()
{
    static int* current_style = (int*)0xB6B220;
    return *current_style;
}

const char devil3_sdk::get_pl_state()
{
    static char* pl_state = (char*)0x01C8EDDE;
    return *pl_state;
}

const bool devil3_sdk::pl_dante_is_air() {
    static char* air = (char*)0x01C8A673;
    return *air != 0;
}
const bool devil3_sdk::pl_dante_is_grounded() {
#if 0
    char state = get_pl_state();
    return (
        (state == 11) ||
        (state == 13));
#else
    static bool* grounded = (bool*)0x01C8C958;
    return !*grounded;
#endif
}

const bool devil3_sdk::pl_is_dt_check()
{
    static const bool* pl_dt = (bool*)0x1C8CE17;
    return *pl_dt;
}

CCharTableMgrPart * devil3_sdk::pl_dante_get_human_anim_table()
{
    return g_dante_table.human;
}

CCharTableMgrPart * devil3_sdk::pl_dante_get_dt1_anim_table()
{
    return g_dante_table.dt1;
}

CCharTableMgrPart * devil3_sdk::pl_dante_get_dt2_anim_table()
{
    return g_dante_table.dt2;
}

const bool devil3_sdk::pl_dante_check_animation_id(uint16_t id)
{
    if ( IsBadReadPtr(&g_dante_table.human->current_anim, sizeof(short)) ||
        IsBadReadPtr(&g_dante_table.dt1->current_anim,   sizeof(short)) ||
        IsBadReadPtr(&g_dante_table.dt2->current_anim,   sizeof(short)) ) {

        return false;
    }
    return (
        ((id == g_dante_table.human->current_anim) && !pl_is_dt_check()) ||
        ((id == g_dante_table.dt1->current_anim) && pl_is_dt_check())   ||
        ((id == g_dante_table.dt2->current_anim) && pl_is_dt_check()));
}
#endif


CSceneGameMain* devil3_sdk::get_main_scene() {
    CSceneGameMain* scn = g_devil3_main_scene_pointer;
    if (scn->vtable != 0x7462C4) {
        return nullptr;
    }
    return scn; // TODO(): memory patch offset, vanilla game would be different

}

void devil3_sdk::area_jump(uint16_t id) {
    CSceneGameMain* scn = get_main_scene();

    if (!scn) {
        return;
    }

    scn->cGameW_ptr->cEvtMiss_ptr->next_map    = id;
    scn->cGameW_ptr->cEvtMiss_ptr->current_map = id;

    scn->flag_state    = GAME_STATE::GAME_MAIN_DOOR;
    scn->screen_effect = SCREEN_EFFECT::DOOR;
}

void devil3_sdk::play_sound(unsigned __int16 a1, unsigned __int16 a2, int a3) {
    typedef unsigned int(__cdecl * PlaySoundMaybeSub409A50Ptr)(unsigned __int16 a1, unsigned __int16 a2, int a3);
    static PlaySoundMaybeSub409A50Ptr fptr_d3_play_sound = (PlaySoundMaybeSub409A50Ptr)0x00409A50;

    fptr_d3_play_sound(a1, a2, a3);
}
