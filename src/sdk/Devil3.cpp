#include "Devil3.hpp"

struct dante_anim_table {
	CCharTableMgrPart* human = (CCharTableMgrPart*)0x01C8E7D8;
	CCharTableMgrPart* dt1   = (CCharTableMgrPart*)0x01C8EB18;
	CCharTableMgrPart* dt2   = (CCharTableMgrPart*)0x01C8EA48;
};

static dante_anim_table g_dante_table;

uint16_t Devil3SDK::get_buttons_pressed() {
	uint16_t* ptr = (uint16_t*)0x01C8EFF4;
	return *ptr;
}

CPlDante* Devil3SDK::get_pl_dante() {
	return (CPlDante*)0x1C8A600;
}

cCameraCtrl* Devil3SDK::get_cam_ctrl() {
	static cCameraCtrl** camera_ptr_ptr = (cCameraCtrl**)0x00B6BAAC;
	return *camera_ptr_ptr;
}

Vector2f Devil3SDK::get_window_dimensions() {
	return Vector2f{ *(float*)0x00832914, *(float*)0x00832918};
}
const int Devil3SDK::get_pl_dante_style()
{
	static int* current_style = (int*)0xB6B220;
	return *current_style;
}

const char Devil3SDK::get_pl_state()
{
	static char* pl_state = (char*)0x01C8EDDE;
	return *pl_state;
}

const bool Devil3SDK::pl_dante_is_air() {
	static char* air = (char*)0x01C8A673;
	return *air != 0;
}
const bool Devil3SDK::pl_dante_is_grounded() {
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

const bool Devil3SDK::pl_is_dt_check()
{
	static const bool* pl_dt = (bool*)0x1C8CE17;
	return *pl_dt;
}

CCharTableMgrPart * Devil3SDK::pl_dante_get_human_anim_table()
{
	return g_dante_table.human;
}

CCharTableMgrPart * Devil3SDK::pl_dante_get_dt1_anim_table()
{
	return g_dante_table.dt1;
}

CCharTableMgrPart * Devil3SDK::pl_dante_get_dt2_anim_table()
{
	return g_dante_table.dt2;
}

const bool Devil3SDK::pl_dante_check_animation_id(uint16_t id)
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
