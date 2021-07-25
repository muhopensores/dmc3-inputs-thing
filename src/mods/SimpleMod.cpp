#include "SimpleMod.hpp"

typedef int(__cdecl* particle_thing)(char a1, int a2, int *a3, int a4);
static particle_thing wew = (particle_thing)0x67FE80;
typedef int (__thiscall* other_particle_thing)(CPlDante*);
static other_particle_thing wew2 = (other_particle_thing)0x5A49E0;
// clang-format off
// clang-format on

std::optional<std::string> SimpleMod::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();
  return Mod::on_initialize();
}

// during load
//void SimpleMod::on_config_load(const utility::Config &cfg) {}
// during save
//void SimpleMod::on_config_save(utility::Config &cfg) {}
// do something every frame
//void SimpleMod::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void SimpleMod::on_draw_debug_ui() {
	ImGui::Text("SimpleMod debug data");
	// Animate a simple progress bar
	static float progress = 0.0f, progress_dir = 1.0f;

	progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
	if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
	if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }

	// Typically we would use ImVec2(-1.0f,0.0f) or ImVec2(-FLT_MIN,0.0f) to use all available width,
	// or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
}
// will show up in main window, dump ImGui widgets you want here
bool checkbox;
int vfx_index = 0xDC;
float col[4];
void SimpleMod::on_draw_ui() {
	ImGui::Text("Hello from SimpleMod");
	ImGui::ColorEdit4("Color", col);
	ImGui::InputInt("Vfx index:", &vfx_index);
	if (ImGui::Button("SimpleMod Button")) {
		int a1 = wew(03, vfx_index, 0, 8);
		if (a1) {
			CPlDante* pl = (CPlDante*)0x01C8A600;
			int a2 = wew2(pl);
			int colorama = ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1],col[2],col[3]));
			__asm {
				pushad
				mov     esi, [pl]
				mov     edi, [a1]
				mov     eax, [a2]
				mov     edx, [esi+eax*4+12B8h]
				mov     eax, [edx+0F8h]
				mov     dword ptr [edi+0A0h], 0
				mov     ecx, [colorama]
				mov     [edi+0xA8], ecx
				mov     dword ptr [edi+0xA4], 2

				mov     [edi+90h], eax
				popad
			}
		}
	}
	ImGui::Checkbox("SimpleMod Checkbox", &checkbox);
}