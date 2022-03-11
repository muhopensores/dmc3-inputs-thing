#include "StyleSwitcherInfo.hpp"

uintptr_t StyleSwitcherInfo::jmp_ret{NULL};
uintptr_t styleSwitcherBase{NULL};

class StyleSwitcher {
public:
	bool oneHitKill = -0x1C0; // i gave up on this because idk how to do negatives this is probably dumb
};

static __declspec(naked) void detour() {
	__asm {
		mov edi, [ebp-0x000001A8]
		mov [styleSwitcherBase], ebp
		jmp qword ptr [StyleSwitcherInfo::jmp_ret]
	}
}

std::optional<std::string> StyleSwitcherInfo::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();
  HANDLE hHandle = GetModuleHandle("StyleSwitcher.dll");

  // check if using Style Switcher 3.1.6
  auto addr = (uintptr_t)hHandle + 0x63F7;
  const unsigned char bytes[7] = {0x8B, 0xBD, 0x30, 0x58, 0xFE, 0xFF, 0xFF};
  if (!memcmp((void*)addr, bytes, sizeof(bytes))) {
    spdlog::error("[{}] incorrect version of StyleSwitcher, initialized without ", get_name());
    return Mod::on_initialize();
  }

  if (!install_hook_absolute(addr, m_function_hook, &detour, &jmp_ret, 6)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize StyleSwitcherInfo";
  }
  return Mod::on_initialize();
}

void StyleSwitcherInfo::on_draw_ui() {
	StyleSwitcher* currentStyleSwitcher = (StyleSwitcher*)styleSwitcherBase;
	if (currentStyleSwitcher) {
		// ImGui::Checkbox("One Hit Kill", &currentStyleSwitcher->oneHitKill);
		bool& noDeath = *(bool*)((uintptr_t)currentStyleSwitcher - 0x1C0);
		ImGui::Text("No Death (F2) = %i", noDeath);
		bool& oneHitKill = *(bool*)((uintptr_t)currentStyleSwitcher - 0x1C4);
		ImGui::Text("One Hit Kill (F3) = %i", oneHitKill);
	}
}
