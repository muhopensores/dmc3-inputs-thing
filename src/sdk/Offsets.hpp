#pragma once
#include <cstdint>

// Offsets from outside of the table
namespace offsets {
constexpr uintptr_t c_pl_dante_ptr = 0x1C8A600;

constexpr uintptr_t c_pl_dante_human_anim_table_ptr = 0x01C8E7D8;
constexpr uintptr_t c_pl_dante_dt1_anim_table_ptr   = 0x01C8EB18;
constexpr uintptr_t c_pl_dante_dt2_anim_table_ptr   = 0x01C8EA48;

constexpr uintptr_t c_pl_dante_curr_style_ptr   = 0xB6B220;
constexpr uintptr_t c_pl_dante_state_ptr        = 0x01C8EDDE;

constexpr uintptr_t c_cam_ctrl_ptr = 0x01371978;
} // namespace offsets