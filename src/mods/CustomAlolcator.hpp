#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include "utility/Patch.hpp"

// Created with ReClass.NET 1.2 by KN4CK3R

class SomeMemoryManagerShit
{
public:
	class SomeStackFramePointerMaybe* ptr1; //0x0000
	void* ptr2; //0x0004
	uint32_t uint1; //0x0008
	char pad_000C[52]; //0x000C
}; //Size: 0x0040

class SomeStackFramePointerMaybe
{
public:
	void* ptr1; //0x0000
	void* ptr2; //0x0004
	void* ptr3; //0x0008
	void* ptr4; //0x000C
	uint32_t uint1; //0x0010
	uint32_t uint2; //0x0014
	uint32_t uint3; //0x0018
	uint32_t uint4; //0x001C
	void* ptrEnd1; //0x0020
	void* ptrEnd2; //0x0024
	uint32_t uint5; //0x0028
}; //Size: 0x002C


class CustomAlolcator : public Mod {
public:
  CustomAlolcator() = default;
  // mod name string for config
  std::string_view get_name() const override { return "CustomAlolcator"; }
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
  bool mem_patch_applied{ false };

  std::unique_ptr<Patch> patch01;
  std::unique_ptr<Patch> patch02;
  std::unique_ptr<Patch> patch03;
  std::unique_ptr<Patch> patch04;
  std::unique_ptr<Patch> patch05;
  std::unique_ptr<Patch> patch06;
  std::unique_ptr<Patch> patch07;
  std::unique_ptr<Patch> patch08;

protected:
	uintptr_t __fastcall sub_6D4580_internal(SomeMemoryManagerShit* p_this, uintptr_t unused, uintptr_t size);
	static uintptr_t __fastcall sub_6D4580(SomeMemoryManagerShit* p_this, uintptr_t unused, uintptr_t size);
private:
	std::unique_ptr<FunctionHook> m_alloc_hook;
  // function hook instance for our detour, convinient wrapper 
  // around minhook
  // std::unique_ptr<FunctionHook> m_function_hook;
};

extern CustomAlolcator* g_custom_alolcator; // NOTE(): WARNING WARNING big global NOT static i repeat NOT STATIC