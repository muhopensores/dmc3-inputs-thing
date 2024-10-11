#pragma once

#include <functional>

#include <d3d9.h>
#include <d3d9types.h>

#include "utility/FunctionHook.hpp"

class D3D9Hook {
public:
    typedef std::function<void(D3D9Hook&)> OnEndSceneFn;
    typedef std::function<void(D3D9Hook&)> OnResetFn;

	D3D9Hook() = default;
    virtual ~D3D9Hook();

    bool hook();
    bool unhook();

    void on_end_scene(OnEndSceneFn fn) { m_on_end_scene = fn; }
    void on_reset(OnResetFn fn)        { m_on_reset = fn; }

	IDirect3DDevice9* get_device() { return m_device; }
	void set_device(IDirect3DDevice9* pDevice) { m_device = pDevice; }

	D3DPRESENT_PARAMETERS* get_presentation_params() { return m_presentation_params; }

	OnEndSceneFn m_on_end_scene{ nullptr };
	OnResetFn m_on_reset{ nullptr };

protected:
	IDirect3DDevice9* m_device{ nullptr };
	D3DPRESENT_PARAMETERS* m_presentation_params{ nullptr };

    bool m_hooked{ false };

    std::unique_ptr<FunctionHook> m_end_scene_hook{};
    std::unique_ptr<FunctionHook> m_reset_hook{};

    static HRESULT WINAPI end_scene(LPDIRECT3DDEVICE9 pDevice);
    static HRESULT WINAPI reset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters);
};
