#include "UIButton.hpp"

// kinda bad but dont care lol
static WPARAM ui_toggle_button = 0;

WPARAM UIButton::ui_button_get_wparam() {
	return ui_toggle_button;
}

std::optional<std::string> UIButton::on_initialize() {
  return Mod::on_initialize();
}

// during load
void UIButton::on_config_load(const utility::Config &cfg) {
	for (IModValue& option : m_options) {
		option.config_load(cfg);
	}
	ui_toggle_button = (WPARAM)m_ui_key->value();
	//ui_toggle_button = cfg.get<WPARAM>("UIButton").value_or(0);
}

// during save
void UIButton::on_config_save(utility::Config &cfg) {
	for (IModValue& option : m_options)  {
		option.config_save(cfg);
	}
}

// do something every frame
//void UIButton::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void UIButton::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void UIButton::on_draw_ui() {
	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}

	ImGui::Text("You can change ui key button here, save config after");

	if (m_ui_key->draw("UI Button Key")) {
		ui_toggle_button = m_ui_key->value();
	}
}
