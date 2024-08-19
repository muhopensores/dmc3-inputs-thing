#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include "utility/Patch.hpp"

// Created with ReClass.NET 1.2 by KN4CK3R

class SomeMemoryManagerShit {
public:
    class SomeStackFramePointerMaybe* ptr1; // 0x0000
    void* ptr2;                             // 0x0004
    uint32_t uint1;                         // 0x0008
    char pad_000c[52];                      // 0x000C
};                                          // Size: 0x0040

class SomeStackFramePointerMaybe {
public:
    void* ptr1;     // 0x0000
    void* ptr2;     // 0x0004
    void* ptr3;     // 0x0008
    void* ptr4;     // 0x000C
    uint32_t uint1; // 0x0010
    uint32_t uint2; // 0x0014
    uint32_t uint3; // 0x0018
    uint32_t uint4; // 0x001C
    void* ptr_end1; // 0x0020
    void* ptr_end2; // 0x0024
    uint32_t uint5; // 0x0028
};                  // Size: 0x002C

class CustomAlolcator : public Mod {
public:
    CustomAlolcator() = default;

    std::string_view get_name() const override { return "CustomAlolcator"; }
    std::optional<std::string> on_initialize() override;

    void on_config_load(const utility::Config& cfg) override;
    void on_config_save(utility::Config& cfg) override;

    // void on_frame() override;
    void on_draw_ui() override;
    void on_draw_debug_ui() override;

    std::unique_ptr<Patch> patch01;
    std::unique_ptr<Patch> patch02;
    std::unique_ptr<Patch> patch03;
    std::unique_ptr<Patch> patch04;
    std::unique_ptr<Patch> patch05;
    std::unique_ptr<Patch> patch06;
    std::unique_ptr<Patch> patch07;
    std::unique_ptr<Patch> patch08;
    std::unique_ptr<Patch> patch09;
    std::unique_ptr<Patch> patch10;
    std::unique_ptr<Patch> patch11;

public:
    uintptr_t __fastcall sub_6d4580_internal(SomeMemoryManagerShit* p_this, uintptr_t unused, size_t size) noexcept;
    static uintptr_t __fastcall sub_6d4580(SomeMemoryManagerShit* p_this, uintptr_t unused, size_t size) noexcept;

    uintptr_t __cdecl heap_control_something_sub_6D0E30_internal(uint32_t a1) noexcept;
    static uintptr_t __cdecl heap_control_something_sub_6D0E30(uint32_t a1) noexcept;

    void* __cdecl sub_65B880(int a1, size_t sz, int a3);
    static void* sub_65B880_internal(int a1, size_t sz, int a3);
private:
    std::unique_ptr<FunctionHook> m_heap_control_sub_6D0E30_hook;
    std::unique_ptr<FunctionHook> m_sub_65B880_hook;

    std::unique_ptr<FunctionHook> m_main_sub_00403580_hook;
    std::unique_ptr<FunctionHook> m_d3d9_load_texture_hook_sub_006E0B10;
    std::unique_ptr<FunctionHook> m_hook_d3d_sets_texture_maybe_sub_6E0DF0;

    std::unique_ptr<FunctionHook> m_scene_loaded_sub_005E02C0;

    std::unique_ptr<FunctionHook> m_alloc_hook;
    const ModToggle::Ptr m_custom_alolcator_enabled { ModToggle::create(generate_name("CustomAlolcatorEnabled")) };

    ValueList m_options{
        *m_custom_alolcator_enabled
    };
protected:
    HRESULT d3d_sets_texture_maybe_sub_6E0DF0_internal(Devil3Texture* texture);
    static HRESULT __fastcall d3d_sets_texture_maybe_sub_6E0DF0(Devil3Texture* texture);
};

extern bool g_mem_patch_applied;
