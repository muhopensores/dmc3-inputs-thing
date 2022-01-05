#pragma once
#include "ReClass.hpp"

namespace Devil3SDK {
	uint16_t get_buttons_pressed();
	CPlDante* get_pl_dante();
	cCameraCtrl* get_cam_ctrl();
	const int  get_pl_dante_style();
	const char get_pl_state();

	Vector2f get_window_dimensions();

	const bool pl_dante_check_animation_id(uint16_t id);
	const bool pl_dante_is_grounded();
	const bool pl_dante_is_air();
	
	const bool pl_is_dt_check();

	CCharTableMgrPart* pl_dante_get_human_anim_table();
	CCharTableMgrPart* pl_dante_get_dt1_anim_table();
	CCharTableMgrPart* pl_dante_get_dt2_anim_table();
}