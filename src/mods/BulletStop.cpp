#include "BulletStop.hpp"

void BulletStop::apply_patches(bool enable) {
	if (enable) {
		m_nop_patch01 = Patch::create_nop(0x427B73, 6, true);
		m_nop_patch02 = Patch::create_nop(0x43D3C3, 6, true);
		m_patched = true;
	}
	else {
		if (!m_patched) { return; }
		m_nop_patch01.reset();
		m_nop_patch02.reset();
		m_patched = false;
	}
}

std::optional<std::string> BulletStop::on_initialize() {

  return Mod::on_initialize();
}

// during load
void BulletStop::on_config_load(const utility::Config &cfg) {
	for (IModValue& option : m_options) {
		option.config_load(cfg);
	}
	apply_patches(m_bs_tgl->value());
}
// during save
void BulletStop::on_config_save(utility::Config &cfg) {
	for (IModValue& option : m_options) {
		option.config_save(cfg);
	}
}
// do something every frame
//void BulletStop::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void BulletStop::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void BulletStop::on_draw_ui() {
	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}
	if (m_bs_tgl->draw("Enable bullet stop")) {
		apply_patches(m_bs_tgl->value());
	}
}
