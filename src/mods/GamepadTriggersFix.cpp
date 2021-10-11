// TODO(): fix this to yeet xinput plus
#if 0
#include "GamepadTriggersFix.hpp"

static bool     g_di_swap_sticks  = false;
static bool     g_di_fix_triggers = false;

static int32_t g_di_l2_btn_index = 0;
static int32_t g_di_r2_btn_index = 0;

std::optional<std::string> GamepadTriggersFix::on_initialize() {
  return Mod::on_initialize();
}

void GamepadTriggersFix::dinput_hook_callback(LPDIJOYSTATE joy1) {

	if (g_di_fix_triggers) {

		int sign = (joy1->lZ != 0) | (joy1->lZ >> (sizeof(int) * CHAR_BIT - 1));
		if (sign > 0) {
			joy1->rgbButtons[g_di_l2_btn_index] = 0x80;
		}
		if (sign < 0) {
			joy1->rgbButtons[g_di_r2_btn_index] = 0x80;
		}
	}
	if (g_di_swap_sticks) {

		LONG lx = joy1->lX;
		LONG ly = joy1->lY;

		LONG rx = joy1->lRx;
		LONG ry = joy1->lRy;

		joy1->lX = rx;
		joy1->lY = ry;

		joy1->lRx = lx;
		joy1->lRy = ly;
	}
}

void GamepadTriggersFix::on_config_load(const utility::Config &cfg) {

	g_di_fix_triggers = cfg.get<bool>("di_fix_triggers").value_or(false);
	g_di_swap_sticks  = cfg.get<bool>("di_swap_sticks").value_or(false);

	g_di_l2_btn_index = cfg.get<int32_t>("di_l2_button_index").value_or(0);
	g_di_r2_btn_index = cfg.get<int32_t>("di_r2_button_index").value_or(0);

}

void GamepadTriggersFix::on_config_save(utility::Config &cfg) {
	
	cfg.set<bool>("di_fix_triggers", g_di_fix_triggers);
	cfg.set<bool>("di_swap_sticks",  g_di_swap_sticks);

	cfg.set<int32_t>("di_l2_button_index", g_di_l2_btn_index);
	cfg.set<int32_t>("di_r2_button_index", g_di_r2_btn_index);

}
// do something every frame
//void GamepadTriggersFix::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void GamepadTriggersFix::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void GamepadTriggersFix::on_draw_ui() {

	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}

	ImGui::Checkbox("Swap Left and Right Sticks", &g_di_swap_sticks);
	ImGui::Text("<3 to endneo");

	ImGui::Checkbox("Fix L2/R2 buttons", &g_di_fix_triggers);

	ImGui::Text("Set inputs below to values you have in dmc3se.ini");
	ImGui::Text("look for lines L2=value R2=value");

	ImGui::InputInt("L2", &g_di_l2_btn_index);
	ImGui::InputInt("R2", &g_di_r2_btn_index);
	ImGui::TextWrapped(
		"there is no bounds checking done so, dont set those things"
		" to values below zero or larger than 32 or you'll crash the game or something");
}
#endif