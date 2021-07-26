#include "UIButton.hpp"

static WPARAM ui_toggle_button = 0;

WPARAM UIButton::ui_button_get_wparam() {
	return ui_toggle_button;
}
std::optional<std::string> UIButton::on_initialize() {
  return Mod::on_initialize();
}

// during load
void UIButton::on_config_load(const utility::Config &cfg) {
	ui_toggle_button = cfg.get<WPARAM>("UIButton").value_or(0);
}

// during save
//void UIButton::on_config_save(utility::Config &cfg) {
//
//}
// do something every frame
//void UIButton::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void UIButton::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
//void UIButton::on_draw_ui() {}
