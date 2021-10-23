#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include <thread>

class CameraHack : public Mod {
public:
  CameraHack() = default;
  // mod name string for config
  std::string_view get_name() const override { return "CameraHack"; }
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
	void* __fastcall cCameraCtrl__something_idk_sub_416880_internal(cCameraCtrl* p_this);
	static void* __fastcall cCameraCtrl__something_idk_sub_416880(cCameraCtrl* p_this);
private:
	std::thread m_rpc_thread;
	bool m_cam_hack_enabled = false;
	constexpr uintptr_t cCameraCtrl_update_or_something() { return (uintptr_t)0x00416880; }
  // function hook instance for our detour, convinient wrapper 
  // around minhook
  // std::unique_ptr<FunctionHook> m_function_hook;
  std::unique_ptr<FunctionHook> m_cam_ctrl_hook;
  std::unique_ptr<FunctionHook> m_cam_ctrl_update_hook;
};