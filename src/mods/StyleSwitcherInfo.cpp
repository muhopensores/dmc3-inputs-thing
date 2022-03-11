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

  if (!install_hook_absolute(0x79F163F7, m_function_hook, &detour, &jmp_ret, 6)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize StyleSwitcherInfo";
  }
  return Mod::on_initialize();
}

void StyleSwitcherInfo::on_draw_ui() {
	if (ImGui::CollapsingHeader("Style Switcher Info")) {
		StyleSwitcher* currentStyleSwitcher = (StyleSwitcher*)styleSwitcherBase;
		if (currentStyleSwitcher) {
			// ImGui::Checkbox("One Hit Kill", &currentStyleSwitcher->oneHitKill);
			bool& oneHitKill = *(bool*)((uintptr_t)currentStyleSwitcher - 0x1C0);
			ImGui::Text("One Hit Kill (F2) = %i", oneHitKill);
			bool& noDeath = *(bool*)((uintptr_t)currentStyleSwitcher - 0x1C4);
			ImGui::Text("No Death (F3) = %i", noDeath);
			bool& hideHud = *(bool*)((uintptr_t)currentStyleSwitcher - 0x114);
			ImGui::Text("Hide HUD (F4) = %i", hideHud);
		}
	}
}
