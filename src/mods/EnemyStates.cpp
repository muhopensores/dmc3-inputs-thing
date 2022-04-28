#include "EnemyStates.hpp"
#if 1

constexpr uintptr_t playerBase        = 0x01C8A600;
constexpr uintptr_t lockedOnEnemyPtr = 0x01C8E1E4;
static float savedEnemyPosXYZ[3]{0.0f, 0.0f, 0.0f};
static float savedPlayerPosXYZ[3]{0.0f, 0.0f, 0.0f};
static int8_t savedPlayerMoveID = 0;

// clang-format off
// only in clang/icl mode on x64, sorry
/*
static naked void detour() {
	__asm {
		mov qword ptr [ModSample::variable], rbx
		mov rax, 0xDEADBEEF
		jmp qword ptr [jmp_ret]
	}
}
*/
// clang-format on

std::optional<std::string> EnemyStates::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  //if (!install_hook_offset(0xBADF00D, m_function_hook, &detour, &jmp_ret, 5)) {
  //  return a error string in case something goes wrong
  //  spdlog::error("[{}] failed to initialize", get_name());
  //  return "Failed to initialize ModSample";
  //}
  return Mod::on_initialize();
}

void EnemyStates::on_draw_ui() {
	if (ImGui::CollapsingHeader("Enemy States")) {
		uintptr_t enemyBase = *(uintptr_t*)lockedOnEnemyPtr;
		if (playerBase && playerBase != -1 && enemyBase && enemyBase != -1) {

			//////////////////////////////////////////////////

			ImGui::Text("Player Info:");
			float* playerPosXYZ[3];
				playerPosXYZ[0] = (float*)(playerBase + 0x4C);
				playerPosXYZ[1] = (float*)(playerBase + 0x50);
				playerPosXYZ[2] = (float*)(playerBase + 0x54);
			int8_t* playerMovePart = (int8_t*)(playerBase + 0x2794);
			int8_t* playerMoveID = (int8_t*)(playerBase + 0x2914);
			ImGui::InputFloat3("XYZ Position ##1", *playerPosXYZ);
			ImGui::InputScalar("Move ID ##1", ImGuiDataType_U8, playerMoveID);

			//////////////////////////////////////////////////

			ImGui::Text("Enemy Info:");
			float* enemyPosXYZ[3];
				enemyPosXYZ[0] = (float*)(enemyBase + 0x10);
				enemyPosXYZ[1] = (float*)(enemyBase + 0x14);
				enemyPosXYZ[2] = (float*)(enemyBase + 0x18);
			int8_t* enemyMovePart = (int8_t*)(enemyBase + 0x2A44);
            // was int8_t* enemyMovePart = (int8_t*)((uintptr_t)enemyBase + 0x2A44);
			ImGui::InputFloat3("XYZ Position ##2", *enemyPosXYZ);

			//////////////////////////////////////////////////

			if (ImGui::Button("Save States ##1")) {
				savedPlayerPosXYZ[0] = *playerPosXYZ[0];
				savedPlayerPosXYZ[1] = *playerPosXYZ[1];
				savedPlayerPosXYZ[2] = *playerPosXYZ[2];
				savedPlayerMoveID	 = *playerMoveID;

				savedEnemyPosXYZ[0] = *enemyPosXYZ[0];
				savedEnemyPosXYZ[1] = *enemyPosXYZ[1];
				savedEnemyPosXYZ[2] = *enemyPosXYZ[2];
			}

			if (ImGui::Button("Load States ##1")) {
				*playerPosXYZ[0] = savedPlayerPosXYZ[0];
				*playerPosXYZ[1] = savedPlayerPosXYZ[1];
				*playerPosXYZ[2] = savedPlayerPosXYZ[2];
				*playerMoveID    = savedPlayerMoveID; // this gets written 0 if not doing a similar move already, need to find some other state value first
				*playerMovePart  = 0;

				*enemyPosXYZ[0] = savedEnemyPosXYZ[0];
				*enemyPosXYZ[1] = savedEnemyPosXYZ[1];
				*enemyPosXYZ[2] = savedEnemyPosXYZ[2];
				*enemyMovePart = 0;
			}
		}
	}
}

// during load
//void ModSample::on_config_load(const utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_load(cfg);
//	}
//}
// during save
//void ModSample::on_config_save(utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_save(cfg);
//	}
//}
// do something every frame
//void ModSample::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void ModSample::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here

#endif
