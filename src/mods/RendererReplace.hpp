#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"

struct d3d_vbuffer
{
    DWORD vfv;
    DWORD unk;
    DWORD size;
    UINT length;
    IDirect3DVertexBuffer9* pidirect3dvertexbuffer910;
    IDirect3DIndexBuffer9*  pindexbuffer;// WARNING(): not sure if this offset is a part of the structure
};

class RendererReplace : public Mod {
public:
  RendererReplace() = default;
  ~RendererReplace();
  // mod name string for config
  std::string_view get_name() const override { return "RendererReplace"; }
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
private:
    std::unique_ptr<FunctionHook> m_d3d_init_hook_sub_408DE0;
    std::unique_ptr<FunctionHook> m_d3d_vb_create_lock_sub_6E1260;
    std::unique_ptr<FunctionHook> m_d3d_create_vb_and_lock_sub_006E1ADF;
    std::unique_ptr<FunctionHook> m_d3d_begin_scene_sub_006DC3CD;
    std::unique_ptr<FunctionHook> m_d3d_end_scene_sub_006DC483;
    std::unique_ptr<FunctionHook> m_d3d_unlock_vb_006E1B59;
    std::unique_ptr<FunctionHook> m_d3d_set_transform_world_006DBCB0;
    std::unique_ptr<FunctionHook> m_d3d_draw_primitive_006E1C5B;

    // temp
    std::unique_ptr<FunctionHook> m_cdraw_set_render_data_hook;
    std::unique_ptr<FunctionHook> m_r_render_prep_world_hook;
    std::unique_ptr<FunctionHook> m_r_render_prep_articulated_models_hook;
    std::unique_ptr<FunctionHook> m_r_render_reset_globals_hook;
    std::unique_ptr<FunctionHook> m_r_render_vertex_data_hook;

    std::unique_ptr<FunctionHook> m_d3d_dispatch_drawcall_006DF65D;
    std::unique_ptr<FunctionHook> m_d3d_before_unlock_006E1B54;

    std::unique_ptr<FunctionHook> m_d3d_release_vb_and_unlock_sub_6E1220;
  // function hook instance for our detour, convinient wrapper 
  // around minhook
  // std::unique_ptr<FunctionHook> m_function_hook;
public:

    //TODO
    void d3d_create_and_lock_vb_sub_6E1260_internal() noexcept;
    static void d3d_create_and_lock_vb_sub_6E1260() noexcept;


    void init_d3d9_sub_408DE0_internal() noexcept;
    static void init_d3d9_sub_408DE0() noexcept;
};