#include "RgTimer.hpp"

// clang-format off
// only in clang/icl mode on x64, sorry
/*
static naked void detour() {
	__asm {
		mov qword ptr [RgTimer::variable], rbx
		mov rax, 0xDEADBEEF
		jmp qword ptr [jmp_ret]
	}
}
*/
// clang-format on

static uint32_t g_frame = 0;

static uint32_t g_guard_frame = 0;
static uint32_t g_release_frame = 0;

static uint32_t g_input_release_frame = 0;
static uint32_t g_input_guard_frame = 0;

static float g_pl_block_timer = 0;

static uint8_t g_release_lock = 0;
static float g_dif = 0.0f;

static bool g_show_releases = false;
static bool g_show_rg_window = false;

static void my_rg_release_callback() {
    static CPlDante* pl = Devil3SDK::get_pl_dante();
    g_dif = pl->release_timer / 1.20f;
    g_release_lock = 8;
}

static uintptr_t g_rg_rel_jump_back = NULL;
static __declspec(naked) void rg_release() {
	__asm {


	originalCode:
        push ecx
            push ebx
            push ebp
            push esi
            mov esi, ecx

            push eax

            mov eax, dword ptr[g_frame]

            mov dword ptr[g_guard_frame], eax
            mov dword ptr[g_release_frame], eax
			pop eax
            pushad
            call my_rg_release_callback
            popad
		jmp dword ptr [g_rg_rel_jump_back]
	}
}

#if 0
static uintptr_t g_rg_blk_jump_back = NULL;
static __declspec(naked) void rg_block() {
	__asm {



		jmp dword ptr [g_rg_blk_jump_back]
	}
}
#endif

std::optional<std::string> RgTimer::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  if (!install_hook_absolute(0x005981D0, m_hook_guard_release, &rg_release, &g_rg_rel_jump_back, 6)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize RgTimer";
  }

  return Mod::on_initialize();
}

// during load
//void RgTimer::on_config_load(const utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_load(cfg);
//	}
//}
// during save
//void RgTimer::on_config_save(utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_save(cfg);
//	}
//}
// do something every frame

static std::array<glm::vec4, 60> block_colors;
static std::array<glm::vec4, 60> rel_block_colors;

static glm::vec4 def { 0.0f / 255.0f, 104.0f / 255.0f, 255.0f / 255.0f, 1.0f };
static glm::vec4 grn { 0.0f / 255.0f, 255.0f / 255.0f, 104.0f / 255.0f, 1.0f };
static glm::vec4 red { 255.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f,   1.0f };

float inverse_smoothstep(float x) {
	return 0.5f - sin(asin(1.0f - 2.0f * x) / 3.0f);
}
static float g_release_timer_back = 0.0f;

void RgTimer::on_frame() {
	static CPlDante* pl = Devil3SDK::get_pl_dante();
	if (*(uint32_t*)pl == 0x744D38) {
		g_pl_block_timer = pl->block_timer;
        //printf("[%d] release_timer=%f\n", g_frame, pl->release_timer);
#if 1
		if(pl->block_timer < 1.0f) {
			g_input_guard_frame = g_frame;
		}

		if (pl->release_timer < 1.2f && pl->release_timer >= 0.0f && (g_release_lock == 0)) {
			g_input_release_frame = g_frame;
            g_release_timer_back = pl->release_timer;
		}

        if (g_release_lock) {
            g_release_lock--;
            g_release_timer_back += pl->workrate;
        }
        else {
            g_release_timer_back = pl->release_timer;
        }
#endif
		//block_colors[g_frame % 60] = glm::vec4(glm::clamp(pl->block_timer, 0.0f, 1.0f), 1.0f - glm::clamp(pl->block_timer, 0.0f, 1.0f), 0.0f, 1.0f);//glm::smoothstep(def, grn, glm::vec4(glm::smoothstep(0.0f, 5.0f, pl->block_timer)));
		if (pl->block_timer < 5.0f) {
			block_colors[g_frame % 60] = grn;//glm::smoothstep(def, grn, glm::vec4(glm::smoothstep(0.0f, 5.0f, pl->block_timer)));
		}
		else {
			block_colors[g_frame % 60] = ImVec4(89.0f / 255.0f, 8.0f / 255.0f, 177.0f / 255.0f, 1.0f);
		}

#if 0
		if (g_release_timer_back < 10.0f && g_release_timer_back > 7.0f/*  && !g_release_lock*/) {
			rel_block_colors[g_frame % 60] = ImVec4(14.0f / 255.0f, 104.0f / 255.0f, 175.0f / 255.0f, 1.0f);//ImVec4(255.0f / 255.0f, 215.0f / 255.0f, 0.0f / 255.0f, 1.0f);
		}
		else if (g_release_timer_back < 7.0f && g_release_timer_back > 3.0f/*  && !g_release_lock*/) {
			rel_block_colors[g_frame % 60] = ImVec4(14.0f / 255.0f, 175.0f / 255.0f, 125.0f / 255.0f, 1.0f);//ImVec4(255.0f / 255.0f, 215.0f / 255.0f, 0.0f / 255.0f, 1.0f);
		}
		else if (g_release_timer_back < 3.0f/*  && !g_release_lock*/) {
			rel_block_colors[g_frame % 60] = ImVec4(255.0f / 255.0f, 215.0f / 255.0f, 0.0f / 255.0f, 1.0f);
		}
        else {
            rel_block_colors[g_frame % 60] = ImVec4(89.0f / 255.0f, 8.0f / 255.0f, 177.0f / 255.0f, 1.0f);
        }
#else
        if (!g_show_releases) {
            goto skip_releases;
        }
        if (g_release_timer_back < 10.0f && g_release_timer_back > 7.0f/*  && !g_release_lock*/) {
            block_colors[g_frame % 60] = ImVec4(14.0f / 255.0f, 104.0f / 255.0f, 175.0f / 255.0f, 1.0f);//ImVec4(255.0f / 255.0f, 215.0f / 255.0f, 0.0f / 255.0f, 1.0f);
        }
        else if (g_release_timer_back < 7.0f && g_release_timer_back > 3.0f/*  && !g_release_lock*/) {
            block_colors[g_frame % 60] = ImVec4(14.0f / 255.0f, 175.0f / 255.0f, 125.0f / 255.0f, 1.0f);//ImVec4(255.0f / 255.0f, 215.0f / 255.0f, 0.0f / 255.0f, 1.0f);
        }
        else if (g_release_timer_back < 3.0f/*  && !g_release_lock*/) {
            block_colors[g_frame % 60] = ImVec4(255.0f / 255.0f, 215.0f / 255.0f, 0.0f / 255.0f, 1.0f);
        }
#endif
skip_releases:
		g_frame++;
        
		//printf("[RGTimer] [%d] - %f\n", g_frame, pl->block_timer);
	}
}
// will show up in debug window, dump ImGui widgets you want here
//void RgTimer::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void RgTimer::on_draw_ui() {
    if (!ImGui::CollapsingHeader(get_name().data())) {
        return;
    }
    ImGui::Checkbox("Show rg timing window", &g_show_rg_window);
}

void RgTimer::custom_imgui_window() {

    if (!g_show_rg_window) {
        return;
    }
    ImGui::SetNextWindowBgAlpha(0.3f);
	ImGui::Begin("rg", 0, ImGuiWindowFlags_NoTitleBar);
	int dif = g_frame - g_guard_frame;
	dif = glm::clamp(dif, 0, 5);

	for (int i = 0; i < 60; i++)
	{
		if (i > 0)
			ImGui::SameLine();

		int curr_frame = (g_frame % 60);
		int guard_frame = (g_guard_frame % 60);
		int input_guard_frame = (g_input_guard_frame % 60);
		int guard_delta = guard_frame - input_guard_frame;

        int release_frame = (g_release_frame % 60);
        int input_release_frame = (g_input_release_frame % 60);
        int release_delta = release_frame - input_release_frame;

		ImGui::PushID(i);
		{
			/*ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 60.0f, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 60.0f, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 60.0f, 0.8f, 0.8f));*/

			if (i == guard_frame) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 238.0f / 255.0f, 255.0f / 255.0f, 1.0f));
			}
			else if (i == input_guard_frame) {
				//255, 0, 182
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(255.0f / 255.0f, 0.0f / 255.0f, 182.0f / 255.0f, 1.0f));
			}
            else if (i == input_release_frame) {
                //214, 84, 84
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(214.0f / 255.0f, 84.0f / 255.0f, 84.0f / 255.0f, 1.0f));
            }
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(block_colors[i].r,block_colors[i].g,block_colors[i].b,block_colors[i].w));
			}
		}
		if (i == curr_frame) {
			ImGui::Button("@", ImVec2(30, 30));
		}
		else {
			ImGui::Button(" ", ImVec2(20, 20));
		}
		ImGui::PopStyleColor(1);
		ImGui::PopID();
	}
#if 0
	for (int i = 0; i < 60; i++)
	{
		if (i > 0)
			ImGui::SameLine();

		int curr_frame = (g_frame % 60);
		int release_frame = (g_release_frame % 60);
		int input_release_frame = (g_input_release_frame % 60);
		int release_delta = release_frame - input_release_frame;

		ImGui::PushID(i);
		{
			/*ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 60.0f, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 60.0f, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 60.0f, 0.8f, 0.8f));*/

			if (i == release_frame) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 238.0f / 255.0f, 255.0f / 255.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			}
			else if (i == input_release_frame) {
				//255, 0, 182
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(255.0f / 255.0f, 0.0f / 255.0f, 182.0f / 255.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(rel_block_colors[i].r,rel_block_colors[i].g,rel_block_colors[i].b,rel_block_colors[i].w));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			}
		}
		if (i == curr_frame) {
			ImGui::Button("@", ImVec2(30, 30));
		}
		else {
			ImGui::Button(" ", ImVec2(20, 20));
		}
		ImGui::PopStyleColor(3);
		ImGui::PopID();
	}
#endif
    static CPlDante* pl = Devil3SDK::get_pl_dante();

	ImGui::Text("difference guard: %d frames", g_guard_frame - g_input_guard_frame);
	ImGui::Text("difference release: %f frames", g_dif);

    ImGui::Text("rg_meter_float: %f", pl->rg_meter_flt);
    ImGui::Text("rg_meter_uint:  %d", pl->rg_meter_uint);

    ImGui::Checkbox("Show releases on timeline", &g_show_releases);
#if 0
    static CPlDante* pl = Devil3SDK::get_pl_dante();
    ImGui::Text("pl_release_timer: %f", pl->release_timer);
    ImGui::Text("g_dif %f", g_dif);
    ImGui::Text("g_release_lock %d", g_release_lock);
#endif
	ImGui::End();
}