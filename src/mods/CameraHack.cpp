#include "CameraHack.hpp"
#include "rpc/server.h"

static uintptr_t    cam_jmp_ret = NULL;
static CameraHack*  g_ch_ptr = nullptr;
static cCameraCtrl* c_camera_ctrl_ptr = nullptr;

static void* g_ret = NULL;

void* __fastcall CameraHack::cCameraCtrl__something_idk_sub_416880_internal(cCameraCtrl* p_this)
{
	if (!m_cam_hack_enabled) {
		auto res = m_cam_ctrl_update_hook->get_original<decltype(cCameraCtrl__something_idk_sub_416880)>()(p_this);
		g_ret = res;
		return res;
	}
	
	return nullptr;
}

void* CameraHack::cCameraCtrl__something_idk_sub_416880(cCameraCtrl* p_this) {
	return g_ch_ptr->cCameraCtrl__something_idk_sub_416880_internal(p_this);
}

// clang-format off
static __declspec(naked) void camera_ctrl_thiscall_thing() {
	__asm {
		mov dword ptr [c_camera_ctrl_ptr], ecx
		sub esp, 70h
		push ebx
		push ebp
		jmp qword ptr [cam_jmp_ret]
	}
}

struct pixel8 {
	uint8_t a, r, g, b;
};



// clang-format on
static void rpc_thread(CameraHack* cam) {

	rpc::server srv(42069);

	srv.bind("get_pixels", [&](int width, int height) {
		std::vector<pixel8> data;
		data.resize(width*height);

		auto dev = g_framework->get_d3d9_device();

		IDirect3DSurface9* backbuffer = nullptr;

		HRESULT hr = dev->GetRenderTarget(0, &backbuffer);
		if (FAILED(hr)) {
			throw std::runtime_error("failed to get rendertarget d3d9 rip;");
		}

		std::memcpy(&data, backbuffer, data.size());

		return data;
	});
}
std::optional<std::string> CameraHack::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

	/*if (!install_hook_absolute(0x00416880, m_cam_ctrl_hook, &camera_ctrl_thiscall_thing, &cam_jmp_ret, 5)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize CameraHack";
	}*/
	
	m_rpc_thread = std::thread(rpc_thread, this);
	m_rpc_thread.detach();

	g_ch_ptr = this;
	m_cam_ctrl_update_hook = std::make_unique<FunctionHook>(cCameraCtrl_update_or_something(), &cCameraCtrl__something_idk_sub_416880);
	
	if (!m_cam_ctrl_update_hook->create()) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize CameraHack";
	}

  return Mod::on_initialize();
}

// during load
//void CameraHack::on_config_load(const utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_load(cfg);
//	}
//}
// during save
//void CameraHack::on_config_save(utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_save(cfg);
//	}
//}
// do something every frame
//void CameraHack::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void CameraHack::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void CameraHack::on_draw_ui() {
	ImGui::Checkbox("Enable camera hack", &m_cam_hack_enabled);
}
