#include "NoHeightRestriction.hpp"

uintptr_t NoHeightRestriction::jmp_ret{NULL};
bool NoHeightRestriction::modEnabled{FALSE};
/*
static __declspec(naked) void detour() {
	__asm {
		jmp qword ptr [NoHeightRestriction::jmp_ret]
	}
}
*/
void NoHeightRestriction::toggle(bool state) {
    noHeightRestrictionPatch1->toggle(state);
    noHeightRestrictionPatch2->toggle(state);
    noHeightRestrictionPatch3->toggle(state);
}

std::optional<std::string> NoHeightRestriction::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  /*
  if (!install_hook_absolute(0x79F163F7, m_function_hook, &detour, &jmp_ret, 6)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NoHeightRestriction";
  }
  */
  noHeightRestrictionPatch1 = Patch::create(0x00593FAF, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}, false); // rainstorm
  noHeightRestrictionPatch2 = Patch::create(0x00593FD8, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}, false); // helmbreaker
  noHeightRestrictionPatch3 = Patch::create(0x00594001, {0x90, 0x90},                         false); // rave
  return Mod::on_initialize();
}

void NoHeightRestriction::on_draw_ui() {
  if (ImGui::Checkbox("Disable Height Restriction", &modEnabled)) {
    toggle(modEnabled);
  }
}

// during load
void NoHeightRestriction::on_config_load(const utility::Config& cfg) {
  modEnabled = cfg.get<bool>("no_height_restriction").value_or(false);
  toggle(modEnabled);
}

// during save
void NoHeightRestriction::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("no_height_restriction", modEnabled);
}
