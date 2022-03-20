#include "TurnSpeed.hpp"

uintptr_t TurnSpeed::jmp_ret{NULL};
bool TurnSpeed::modEnabled{FALSE};

void TurnSpeed::toggle(bool state) {
    patch1->toggle(state);
}

std::optional<std::string> TurnSpeed::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  /*
  if (!install_hook_absolute(0x79F163F7, m_function_hook, &detour, &jmp_ret, 6)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize TurnSpeed";
  }
  */
  patch1 = Patch::create(0x005A5891, {0xEB, 0x0C}, false); // attacking
  return Mod::on_initialize();
}

void TurnSpeed::on_draw_ui() {
  if (ImGui::Checkbox("Increased Turn Speed", &modEnabled)) {
    toggle(modEnabled);
  }
}

// during load
void TurnSpeed::on_config_load(const utility::Config& cfg) {
  modEnabled = cfg.get<bool>("increased_turn_speed").value_or(false);
  toggle(modEnabled);
}

// during save
void TurnSpeed::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("increased_turn_speed", modEnabled);
}
