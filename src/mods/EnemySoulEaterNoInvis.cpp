#include "EnemySoulEaterNoInvis.hpp"

uintptr_t EnemySoulEaterNoInvis::jmp_ret{NULL};
bool EnemySoulEaterNoInvis::modEnabled{FALSE};
/*
static __declspec(naked) void detour() {
	__asm {
		jmp qword ptr [EnemySoulEaterNoInvis::jmp_ret]
	}
}
*/
void EnemySoulEaterNoInvis::toggle(bool state) {
    patch->toggle(state);
}

std::optional<std::string> EnemySoulEaterNoInvis::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  /*
  if (!install_hook_absolute(0x79F163F7, m_function_hook, &detour, &jmp_ret, 6)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemySoulEaterNoInvis";
  }
  */
  patch = Patch::create(0x004A1C9F, {0xEB}, false);
  return Mod::on_initialize();
}

void EnemySoulEaterNoInvis::on_draw_ui() {
  if (ImGui::Checkbox("Look at Soul Eaters", &modEnabled)) {
    toggle(modEnabled);
  }
}

// during load
void EnemySoulEaterNoInvis::on_config_load(const utility::Config& cfg) {
  modEnabled = cfg.get<bool>("soul_eater_no_invis").value_or(false);
  toggle(modEnabled);
}

// during save
void EnemySoulEaterNoInvis::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("soul_eater_no_invis", modEnabled);
}
