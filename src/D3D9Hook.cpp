#include <algorithm>
#include <spdlog/spdlog.h>

#include "D3D9Hook.hpp"

using namespace std;

static D3D9Hook* g_d3d9_hook = nullptr;

D3D9Hook::~D3D9Hook() {
    unhook();
}

uintptr_t end_scene_jmp_ret = 0;
__declspec(naked) void end_scene_hook(void) {
	__asm {
		pushad
		push eax
		call D3D9Hook::end_scene
		popad
		pop eax
		push eax
		call dword ptr [ecx+0xA8]
		jmp dword ptr [end_scene_jmp_ret]
	}
}

uintptr_t reset_jmp_ret = 0;
__declspec(naked) void reset_hook(void) {
	__asm {
		pushad
		push eax
		call D3D9Hook::reset
		popad
		pop eax
		push eax

		call dword ptr [ecx+0x40]
		mov esi, eax

		jmp dword ptr [reset_jmp_ret]
	}
}

bool D3D9Hook::hook() {
    spdlog::info("Hooking D3D9");

    g_d3d9_hook = this;
    IDirect3DDevice9** game = (IDirect3DDevice9**)0x0252F374;
    IDirect3DDevice9* d3d9_device = NULL;

    do {
        d3d9_device = *game; // TODO: base + offset?
        Sleep(1);
    } while (d3d9_device == NULL);

	m_device = d3d9_device;

	uintptr_t reset_fn     = (*(uintptr_t**)d3d9_device)[16];
	uintptr_t end_scene_fn = (*(uintptr_t**)d3d9_device)[42];

    m_end_scene_hook = std::make_unique<FunctionHook>(end_scene_fn, (uintptr_t)&end_scene);//std::make_unique<FunctionHook>(end_scene_fn, (uintptr_t)&end_scene_hook);
    m_reset_hook = std::make_unique<FunctionHook>(reset_fn, (uintptr_t)&reset);
	//end_scene_jmp_ret = 0x006DC48E + 0x6;
	//reset_jmp_ret = reset_fn + 5;
    m_hooked = m_end_scene_hook->create() && m_reset_hook->create();

    return m_hooked;
}

bool D3D9Hook::unhook() {
    return true;
}

#if 1
HRESULT WINAPI D3D9Hook::end_scene(LPDIRECT3DDEVICE9 pDevice) {
	auto d3d9 = g_d3d9_hook;

	//d3d9->m_device = pDevice;
	if (d3d9->m_on_end_scene) {
		d3d9->m_on_end_scene(*d3d9);
	}
	//return S_OK;
	auto end_scene_fn = 
		d3d9->m_end_scene_hook->get_original<decltype(D3D9Hook::end_scene)>();
	
	return end_scene_fn(pDevice);
}
#else
static void end_scene_our(LPDIRECT3DDEVICE9 pDevice) {
	auto d3d9 = g_d3d9_hook;

	d3d9->set_device(pDevice);
	if (d3d9->m_on_end_scene) {
		d3d9->m_on_end_scene(*d3d9);
	}
}
#endif


HRESULT WINAPI D3D9Hook::reset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters) {
	
	auto d3d9 = g_d3d9_hook;
	
	d3d9->m_presentation_params = pPresentationParameters;

	if (d3d9->m_on_reset) {
		d3d9->m_on_reset(*d3d9);
	}
	//return S_OK;
	auto reset_fn = 
		d3d9->m_reset_hook->get_original<decltype(D3D9Hook::reset)>();

	return reset_fn(pDevice, pPresentationParameters);
}

