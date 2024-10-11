#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"

class AreaJump : public Mod {
public:
  AreaJump() = default;
  // mod name string for config
  std::string_view get_name() const override { return "AreaJump"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;


  //void on_frame() override;
  void on_draw_ui() override;
  //void on_draw_debug_ui() override;
private:
  // function hook instance for our detour, convinient wrapper
  // around minhook
  std::unique_ptr<FunctionHook> m_function_hook;
  std::unique_ptr<FunctionHook> m_function_hook1;
};