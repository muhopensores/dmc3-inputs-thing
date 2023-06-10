#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"

class cHeapFrame
{
public:
	void* scratch; //0x0000
	void* ptr; //0x0004
	uint32_t size_maybe; //0x0008
	char pad_000C[12]; //0x000C
	uint32_t num_refs; //0x0018
	char pad_001C[40]; //0x001C
}; //Size: 0x0044


class EnemySpawnRate : public Mod {
public:
  EnemySpawnRate() = default;
  // mod name string for config
  std::string_view get_name() const override { return "EnemySpawnRate"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;

  // Override this things if you want to store values in the config file
  //void on_config_load(const utility::Config& cfg) override;
  //void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  //void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  //void on_draw_debug_ui() override;
protected:
	uintptr_t __fastcall spawn_a_guy_sub_54ED10_internal(uintptr_t p_this, float* a2, uintptr_t a3);
	static uintptr_t __fastcall spawn_a_guy_sub_54ED10(uintptr_t p_this, float* a2, uintptr_t a3);

private:
	std::unique_ptr<FunctionHook> m_spawn_guy_hook;
  // function hook instance for our detour, convinient wrapper 
  // around minhook
  // std::unique_ptr<FunctionHook> m_function_hook;
};