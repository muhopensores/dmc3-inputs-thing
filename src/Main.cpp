#include <thread>
#include <chrono>
#include <windows.h>

#include "ModFramework.hpp"

HMODULE g_dinput = 0;
HMODULE g_orig_dinput = 0;

extern "C" {
    // DirectInput8Create wrapper for dinput8.dll
    __declspec(dllexport) HRESULT WINAPI direct_input8_create(HINSTANCE hinst, DWORD dw_version, const IID& riidltf, LPVOID* ppv_out, LPUNKNOWN punk_outer) {
        // This needs to be done because when we include dinput.h in DInputHook,
        // It is a redefinition, so we assign an export by not using the original name
        #pragma comment(linker, "/EXPORT:DirectInput8Create=_direct_input8_create@20")
        return ((decltype(DirectInput8Create)*)GetProcAddress(g_dinput, "DirectInput8Create"))(hinst, dw_version, riidltf, ppv_out, punk_outer);
    }
}

void failed() {
    MessageBox(0, "ModFramework: Unable to load the original dinput8.dll. Please report this to the developer.", "ModFramework", 0);
    ExitProcess(0);
}

void startup_thread() {
#ifndef NDEBUG
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif

    wchar_t buffer[MAX_PATH]{ 0 };
	wchar_t buffer_cwd[MAX_PATH]{ 0 };
	if (GetCurrentDirectoryW(MAX_PATH, buffer_cwd) != 0) {
		if ((g_orig_dinput = LoadLibraryW((std::wstring{ buffer_cwd } +L"\\dinput8_original.dll").c_str())) == NULL) {
			g_orig_dinput = LoadLibraryW(L"\\dinput8_original.dll");
		}
	}
    if (GetSystemDirectoryW(buffer, MAX_PATH) != 0) {
        // Load the original dinput8.dll
        if ((g_dinput = LoadLibraryW((std::wstring{ buffer } + L"\\dinput8.dll").c_str())) == NULL) {
            failed();
        }
		
        g_framework = std::make_unique<ModFramework>();
    }
    else {
        failed();
    }
}

BOOL APIENTRY DllMain(HANDLE handle, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
#ifndef NDEBUG
		MessageBox(NULL, "Debug attach opportunity", "DMC3", MB_ICONINFORMATION);
#endif
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)startup_thread, nullptr, 0, nullptr);
    }

    return TRUE;
}
