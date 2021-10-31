#include <thread>
#include <chrono>
#include <windows.h>

#include "ModFramework.hpp"

//static HMODULE g_dinput;

#if 0
extern "C" {
	// DirectInput8Create wrapper for dinput8.dll
	__declspec(dllexport) HRESULT WINAPI direct_input8_create(HINSTANCE hinst, DWORD dw_version, const IID& riidltf, LPVOID* ppv_out, LPUNKNOWN punk_outer) {
		// This needs to be done because when we include dinput.h in DInputHook,
		// It is a redefinition, so we assign an export by not using the original name
		#pragma comment(linker, "/EXPORT:DirectInput8Create=_direct_input8_create@20")
		return ((decltype(DirectInput8Create)*)GetProcAddress(g_dinput, "DirectInput8Create"))(hinst, dw_version, riidltf, ppv_out, punk_outer);
	}
}
#endif

void failed() {
	MessageBox(0, "DMC3 ModFramework: Unable to load the original version.dll. Please report this to the developer.", "ModFramework", 0);
	ExitProcess(0);
}

void startup_thread() {
#ifndef NDEBUG
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif
#if 0
	wchar_t buffer[MAX_PATH]{ 0 };
	if (GetSystemDirectoryW(buffer, MAX_PATH) != 0) {
		// Load the original dinput8.dll
		if ((g_dinput = LoadLibraryW((std::wstring{ buffer } + L"\\dinput8.dll").c_str())) == NULL) {
			failed();
		}
		g_dinputo = LoadLibraryA("\\dinput8_original.dll");
		g_framework = std::make_unique<ModFramework>();
	}
	else {
		failed();
	}
#else
	g_framework = std::make_unique<ModFramework>();
#endif
}

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH) {
#ifndef NDEBUG
#endif
		MessageBox(NULL, "Debug attach opportunity", "DMC3", MB_ICONINFORMATION);
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)startup_thread, nullptr, 0, nullptr);
	}
	if (reason == DLL_PROCESS_DETACH) {
		//FreeLibrary(version.dll);
	}
	return TRUE;
}