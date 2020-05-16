#define APP_NAME    "Tiny DLL Proxy"
#define APP_VERSION "1.0 [26.03.2019]"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


HMODULE guidx_dll;
HMODULE dinput8_dll;
bool started = false;

extern "C" {
	
  LPBYTE DirectInput8Create;
  LPBYTE DllCanUnloadNow;
  LPBYTE DllGetClassObject;
  LPBYTE DllRegisterServer;
  LPBYTE DllUnregisterServer;

	void _DirectInput8Create();
	void _DllCanUnloadNow();
	void _DllGetClassObject();
	void _DllRegisterServer();
	void _DllUnregisterServer();
}

DWORD WINAPI MyThread(LPVOID lParam)
{
	Sleep(1000);
	guidx_dll = LoadLibrary("guidx.dll");
	MessageBeep(MB_OK);

	return 0;
};
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  char path[MAX_PATH];
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
      CopyMemory(path + GetSystemDirectory(path, MAX_PATH-12), "\\dinput8.dll", 13);
      dinput8_dll= LoadLibrary(path);
	  if (dinput8_dll == false)
      {
        MessageBox(0, "Failed to load dinput8.dll!", APP_NAME, MB_ICONERROR);
        ExitProcess(0);
      }
      DirectInput8Create = (LPBYTE)GetProcAddress(dinput8_dll, "DirectInput8Create");
	  DllCanUnloadNow = (LPBYTE)GetProcAddress(dinput8_dll, "DllCanUnloadNow");
	  DllGetClassObject = (LPBYTE)GetProcAddress(dinput8_dll, "DllGetClassObject");
	  DllRegisterServer = (LPBYTE)GetProcAddress(dinput8_dll, "DllRegisterServer");
	  DllUnregisterServer = (LPBYTE)GetProcAddress(dinput8_dll, "DllUnregisterServer");
	  //DisableThreadLibraryCalls(hModule);
	  CloseHandle(CreateThread(NULL, NULL, MyThread, NULL, NULL, NULL));
    break;

    case DLL_PROCESS_DETACH:
		FreeLibrary(dinput8_dll);
		FreeLibrary(guidx_dll);
    break;
  }
  return TRUE;
}