#include "EnemyStepCooldown.hpp"

uintptr_t EnemyStepCooldown::jmp_ret{NULL};
bool EnemyStepCooldown::modEnabled{FALSE};

void EnemyStepCooldown::toggle(bool state) {
    patch1->toggle(state);
}

std::optional<std::string> EnemyStepCooldown::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  /*
  if (!install_hook_absolute(0x79F163F7, m_function_hook, &detour, &jmp_ret, 6)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemyStepCooldown";
  }
  */
  patch1 = Patch::create_nop(0x0058D93E, 12, false);
  return Mod::on_initialize();
}

void EnemyStepCooldown::on_draw_ui() {
  if (ImGui::Checkbox("Remove Enemy Step Cooldown", &modEnabled)) {
    toggle(modEnabled);
  }
}

// during load
void EnemyStepCooldown::on_config_load(const utility::Config& cfg) {
  modEnabled = cfg.get<bool>("remove_enemy_step_cooldown").value_or(false);
  toggle(modEnabled);
}

// during save
void EnemyStepCooldown::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("remove_enemy_step_cooldown", modEnabled);
}
