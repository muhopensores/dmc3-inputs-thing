#include <thread>
#include <chrono>
#include <windows.h>

#include "ModFramework.hpp"
#include "utility/ExceptionHandler.hpp"

static HMODULE g_dinput;
static HMODULE g_styleswitcher;

#if 1
extern "C" {
	// DirectInput8Create wrapper for dinput8.dll
	__declspec(dllexport) HRESULT WINAPI direct_input8_create(HINSTANCE hinst, DWORD dw_version, const IID& riidltf, LPVOID* ppv_out, LPUNKNOWN punk_outer) {
		// This needs to be done because when we include dinput.h in DInputHook,
		// It is a redefinition, so we assign an export by not using the original name
		#pragma comment(linker, "/EXPORT:DirectInput8Create=_direct_input8_create@20")
		return ((decltype(direct_input8_create)*)GetProcAddress(g_dinput, "DirectInput8Create"))(hinst, dw_version, riidltf, ppv_out, punk_outer);
	}
}
#endif

void failed() {
	MessageBox(0, "DMC3 ModFramework: Unable to load the original version.dll. Please report this to the developer.", "ModFramework", 0);
	ExitProcess(0);
}

void startup_thread() {

	reframework::setup_exception_handler();
#ifndef NDEBUG
	AllocConsole();
	HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD consoleMode;
	GetConsoleMode( handleOut , &consoleMode);
	consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	consoleMode |= DISABLE_NEWLINE_AUTO_RETURN;            
	SetConsoleMode( handleOut , consoleMode );

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif
#if 1
	wchar_t buffer[MAX_PATH]{ 0 };
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
#else
	g_framework = std::make_unique<ModFramework>();
#endif
}

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		LoadLibrary("StyleSwitcher.dll");
#ifndef NDEBUG
		MessageBox(NULL, "Debug attach opportunity", "DMC3", MB_ICONINFORMATION);
#endif
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)startup_thread, nullptr, 0, nullptr);
	}
	if (reason == DLL_PROCESS_DETACH) {
		FreeLibrary(g_styleswitcher);
	}
	return TRUE;
}