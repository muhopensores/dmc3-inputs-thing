#include "EnemyStates.hpp"

#if 1
static bool hotkey_enabled = false;
static int hotkey_save     = 0;
static int hotkey_load     = 0;
// static uint16_t g_prev_input;

// player
constexpr uintptr_t playerBase  = 0x01C8A600;
static float savedPlayerPosXYZ[3]{0.0f, 0.0f, 0.0f};
static float savedPlayerRotation = 0.0f;
static int8_t savedPlayerMoveID = 0;
static int8_t savedPlayerIdk1   = 0;
static int8_t savedPlayerIdk2   = 0;
static int savedPlayerIdk3      = 0;
static int savedPlayerGrounded  = 0;

// enemy
static float savedEnemyPosXYZ[3]{0.0f, 0.0f, 0.0f};
static float savedEnemyRotation = 0.0f;
static int savedEnemyMoveID = 0;
static int savedEnemyMoveID2 = 0;
static int savedEnemyGrounded = 0;
constexpr uintptr_t lockedOnEnemyPtr = 0x01C8E1E4;

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
        ImGui::Checkbox("Save/Load Hotkey Enabled", &hotkey_enabled);
        if (ImGui::CollapsingHeader("Split player and enemy states")) {
			if (playerBase && playerBase != -1) {
				ImGui::Text("Player Info:");
				float* playerPosXYZ[3];
				playerPosXYZ[0]	       = (float*)(playerBase + 0x4C);
				playerPosXYZ[1]	       = (float*)(playerBase + 0x50);
				playerPosXYZ[2]	       = (float*)(playerBase + 0x54);
				int8_t* playerMovePart = (int8_t*)(playerBase + 0x2794);
				int8_t* playerMoveID   = (int8_t*)(playerBase + 0x2914);
				int8_t* playerIdk1     = (int8_t*)(playerBase + 0x278C);
				int8_t* playerIdk2     = (int8_t*)(playerBase + 0x2790);
				int*    playerIdk3     = (int*)(playerBase + 0x27DC);
				int*    playerGrounded = (int*)(playerBase + 0x27E0);
                float*  playerRotation = (float*)(playerBase + 0x8C);
				ImGui::InputFloat3("Player XYZ Position ##1", *playerPosXYZ);
				ImGui::InputScalar("Player Move ID ##1", ImGuiDataType_U8, playerMoveID);

				if (ImGui::Button("Save Player State ##1")) {
					savedPlayerPosXYZ[0] = *playerPosXYZ[0];
					savedPlayerPosXYZ[1] = *playerPosXYZ[1];
					savedPlayerPosXYZ[2] = *playerPosXYZ[2];
					savedPlayerRotation  = *playerRotation;
					savedPlayerIdk1		 = *playerIdk1;
					savedPlayerIdk2		 = *playerIdk2;
					savedPlayerIdk3      = *playerIdk3;
					savedPlayerGrounded  = *playerGrounded;
					savedPlayerMoveID	 = *playerMoveID;
				}

				if (ImGui::Button("Load Player State ##1")) {
					*playerPosXYZ[0] = savedPlayerPosXYZ[0];
					*playerPosXYZ[1] = savedPlayerPosXYZ[1];
					*playerPosXYZ[2] = savedPlayerPosXYZ[2];
					*playerRotation  = savedPlayerRotation;
					*playerIdk1      = savedPlayerIdk1;      // player falls incorrectly without
					*playerIdk2      = savedPlayerIdk2;      // player floats in air without
					*playerIdk3		 = savedPlayerIdk3;      // player won't start move unless already doing move without
					*playerGrounded  = savedPlayerGrounded;  // catches both grounded state and some cancel byte 2 bytes later
					*playerMoveID    = savedPlayerMoveID;
					*playerMovePart  = 0;
				}
			}

			if (enemyBase && enemyBase != -1) {
				ImGui::Text("Enemy Info:");
				float* enemyPosXYZ[3];
				enemyPosXYZ[0] = (float*)(enemyBase + 0x10);
				enemyPosXYZ[1] = (float*)(enemyBase + 0x14);
				enemyPosXYZ[2] = (float*)(enemyBase + 0x18);
				int8_t* enemyMovePart = (int8_t*)(enemyBase + 0x2A44);
				int* enemyGrounded    = (int*)(enemyBase + 0x2A70);
				int* enemyMoveID = (int*)(enemyBase + 0x2A40);
				int* enemyMoveID2 = (int*)(enemyBase + 0x2A58);
				ImGui::InputFloat3("Enemy XYZ Position ##2", *enemyPosXYZ);
				if (ImGui::Button("Save Enemy State ##2")) {
					savedEnemyPosXYZ[0] = *enemyPosXYZ[0];
					savedEnemyPosXYZ[1] = *enemyPosXYZ[1];
					savedEnemyPosXYZ[2] = *enemyPosXYZ[2];
					savedEnemyGrounded = *enemyGrounded;
					savedEnemyMoveID = *enemyMoveID;
				}

				if (ImGui::Button("Load Enemy State ##2")) {
					*enemyPosXYZ[0] = savedEnemyPosXYZ[0];
					*enemyPosXYZ[1] = savedEnemyPosXYZ[1];
					*enemyPosXYZ[2] = savedEnemyPosXYZ[2];
					*enemyGrounded = savedEnemyGrounded;
					*enemyMoveID = savedEnemyMoveID;
					*enemyMoveID2 = savedEnemyMoveID2;
					*enemyMovePart = 0;
				}
			}
        }

		if (playerBase && playerBase != -1 && enemyBase && enemyBase != -1) {
			float* playerPosXYZ[3];
			playerPosXYZ[0]	       = (float*)(playerBase + 0x4C);
			playerPosXYZ[1]	       = (float*)(playerBase + 0x50);
			playerPosXYZ[2]	       = (float*)(playerBase + 0x54);
			float*  playerRotation = (float*)(playerBase + 0x8C);
			int8_t* playerMovePart = (int8_t*)(playerBase + 0x2794);
			int8_t* playerMoveID   = (int8_t*)(playerBase + 0x2914);
            int8_t* playerIdk1     = (int8_t*)(playerBase + 0x278C);
			int8_t* playerIdk2     = (int8_t*)(playerBase + 0x2790);
            int*    playerIdk3     = (int*)(playerBase + 0x27DC);
            int*    playerGrounded = (int*)(playerBase + 0x27E0);

			float* enemyPosXYZ[3];
            enemyPosXYZ[0]        = (float*)(enemyBase + 0x10);
            enemyPosXYZ[1]        = (float*)(enemyBase + 0x14);
            enemyPosXYZ[2]        = (float*)(enemyBase + 0x18);
            int8_t* enemyMovePart = (int8_t*)(enemyBase + 0x2A44);
            int* enemyGrounded    = (int*)(enemyBase + 0x2A70);
            int* enemyMoveID      = (int*)(enemyBase + 0x2A40);
            int* enemyMoveID2     = (int*)(enemyBase + 0x2A58);

			if (ImGui::Button("Save States ##3")) {
				savedPlayerPosXYZ[0] = *playerPosXYZ[0];
				savedPlayerPosXYZ[1] = *playerPosXYZ[1];
				savedPlayerPosXYZ[2] = *playerPosXYZ[2];
				savedPlayerRotation  = *playerRotation;
				savedPlayerIdk1      = *playerIdk1;
				savedPlayerIdk2      = *playerIdk2;
				savedPlayerIdk3      = *playerIdk3;
				savedPlayerGrounded  = *playerGrounded;
				savedPlayerMoveID    = *playerMoveID;

				savedEnemyPosXYZ[0] = *enemyPosXYZ[0];
				savedEnemyPosXYZ[1] = *enemyPosXYZ[1];
				savedEnemyPosXYZ[2] = *enemyPosXYZ[2];
				savedEnemyGrounded  = *enemyGrounded;
				savedEnemyMoveID    = *enemyMoveID;
            }

			if (ImGui::Button("Load States ##3")) {
				*playerPosXYZ[0] = savedPlayerPosXYZ[0];
				*playerPosXYZ[1] = savedPlayerPosXYZ[1];
				*playerPosXYZ[2] = savedPlayerPosXYZ[2];
				*playerRotation  = savedPlayerRotation;
                *playerIdk1      = savedPlayerIdk1;      // player falls incorrectly without
                *playerIdk2      = savedPlayerIdk2;      // player floats in air without
				*playerIdk3		 = savedPlayerIdk3;      // player won't start move unless already doing move without
				*playerGrounded  = savedPlayerGrounded;  // catches both grounded state and some cancel byte 2 bytes later
				*playerMoveID    = savedPlayerMoveID;
				*playerMovePart  = 0;

				*enemyPosXYZ[0] = savedEnemyPosXYZ[0];
				*enemyPosXYZ[1] = savedEnemyPosXYZ[1];
				*enemyPosXYZ[2] = savedEnemyPosXYZ[2];
				*enemyGrounded = savedEnemyGrounded;
				*enemyMoveID = savedEnemyMoveID;
				*enemyMoveID2 = savedEnemyMoveID2;
				*enemyMovePart = 0;
            }
		}
	}
}

// do something every frame
void EnemyStates::on_frame() {
	if (hotkey_enabled) {
		// uint16_t input = devil3_sdk::get_buttons_pressed();
		// uint8_t inputByte1 = *(uint8_t*)0x01C8EFF4;
		// uint8_t inputByte2 = *(uint8_t*)0x01C8EFF5;
		// if (input != g_prev_input && input) {
		// if (inputByte1 & 0x08 && inputByte1 && 0x00 && inputByte2 & 0x01) {
		if (GetAsyncKeyState(hotkey_load)) { // load hotkey
			uintptr_t enemyBase = *(uintptr_t*)lockedOnEnemyPtr;
			if (playerBase && playerBase != -1 && enemyBase && enemyBase != -1) {
				float* playerPosXYZ[3];
				playerPosXYZ[0]	       = (float*)(playerBase + 0x4C);
				playerPosXYZ[1]	       = (float*)(playerBase + 0x50);
				playerPosXYZ[2]	       = (float*)(playerBase + 0x54);
				float*  playerRotation = (float*)(playerBase + 0x8C);
				int8_t* playerMovePart = (int8_t*)(playerBase + 0x2794);
				int8_t* playerMoveID   = (int8_t*)(playerBase + 0x2914);
				int8_t* playerIdk1     = (int8_t*)(playerBase + 0x278C);
				int8_t* playerIdk2     = (int8_t*)(playerBase + 0x2790);
				int*    playerIdk3     = (int*)(playerBase + 0x27DC);
				int*    playerGrounded = (int*)(playerBase + 0x27E0);

				float* enemyPosXYZ[3];
				enemyPosXYZ[0]        = (float*)(enemyBase + 0x10);
				enemyPosXYZ[1]        = (float*)(enemyBase + 0x14);
				enemyPosXYZ[2]        = (float*)(enemyBase + 0x18);
				int8_t* enemyMovePart = (int8_t*)(enemyBase + 0x2A44);
				int* enemyGrounded    = (int*)(enemyBase + 0x2A70);
				int* enemyMoveID      = (int*)(enemyBase + 0x2A40);
				int* enemyMoveID2     = (int*)(enemyBase + 0x2A58);

				*playerPosXYZ[0] = savedPlayerPosXYZ[0];
				*playerPosXYZ[1] = savedPlayerPosXYZ[1];
				*playerPosXYZ[2] = savedPlayerPosXYZ[2];
				*playerRotation  = savedPlayerRotation;
				*playerIdk1      = savedPlayerIdk1;
				*playerIdk2      = savedPlayerIdk2;
				*playerIdk3		 = savedPlayerIdk3;
				*playerGrounded  = savedPlayerGrounded;
				*playerMoveID    = savedPlayerMoveID;
				*playerMovePart  = 0;

				*enemyPosXYZ[0] = savedEnemyPosXYZ[0];
				*enemyPosXYZ[1] = savedEnemyPosXYZ[1];
				*enemyPosXYZ[2] = savedEnemyPosXYZ[2];
				*enemyGrounded = savedEnemyGrounded;
				*enemyMoveID = savedEnemyMoveID;
				*enemyMoveID2 = savedEnemyMoveID2;
				*enemyMovePart = 0;
			}
		}
		// if (inputByte1 & 0x08 && inputByte2 & 0x01) {
        if (GetAsyncKeyState(hotkey_save)) { // save hotkey
			uintptr_t enemyBase = *(uintptr_t*)lockedOnEnemyPtr;
			if (playerBase && playerBase != -1 && enemyBase && enemyBase != -1) {
				float* playerPosXYZ[3];
				playerPosXYZ[0]	       = (float*)(playerBase + 0x4C);
				playerPosXYZ[1]	       = (float*)(playerBase + 0x50);
				playerPosXYZ[2]	       = (float*)(playerBase + 0x54);
				float*  playerRotation = (float*)(playerBase + 0x8C);
				int8_t* playerMovePart = (int8_t*)(playerBase + 0x2794);
				int8_t* playerMoveID   = (int8_t*)(playerBase + 0x2914);
				int8_t* playerIdk1     = (int8_t*)(playerBase + 0x278C);
				int8_t* playerIdk2     = (int8_t*)(playerBase + 0x2790);
				int*    playerIdk3     = (int*)(playerBase + 0x27DC);
				int*    playerGrounded = (int*)(playerBase + 0x27E0);

				float* enemyPosXYZ[3];
				enemyPosXYZ[0]        = (float*)(enemyBase + 0x10);
				enemyPosXYZ[1]        = (float*)(enemyBase + 0x14);
				enemyPosXYZ[2]        = (float*)(enemyBase + 0x18);
				int8_t* enemyMovePart = (int8_t*)(enemyBase + 0x2A44);
				int* enemyGrounded    = (int*)(enemyBase + 0x2A70);
				int* enemyMoveID      = (int*)(enemyBase + 0x2A40);
				int* enemyMoveID2     = (int*)(enemyBase + 0x2A58);

				savedPlayerPosXYZ[0] = *playerPosXYZ[0];
				savedPlayerPosXYZ[1] = *playerPosXYZ[1];
				savedPlayerPosXYZ[2] = *playerPosXYZ[2];
				savedPlayerRotation  = *playerRotation;
				savedPlayerIdk1      = *playerIdk1;
				savedPlayerIdk2      = *playerIdk2;
				savedPlayerIdk3      = *playerIdk3;
				savedPlayerGrounded  = *playerGrounded;
				savedPlayerMoveID    = *playerMoveID;

				savedEnemyPosXYZ[0] = *enemyPosXYZ[0];
				savedEnemyPosXYZ[1] = *enemyPosXYZ[1];
				savedEnemyPosXYZ[2] = *enemyPosXYZ[2];
				savedEnemyGrounded  = *enemyGrounded;
				savedEnemyMoveID    = *enemyMoveID;
			}
		}
		// g_prev_input = input;
	}
}
// will show up in debug window, dump ImGui widgets you want here
//void ModSample::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here

// during load
void EnemyStates::on_config_load(const utility::Config &cfg) {
  hotkey_enabled = cfg.get<bool>("enemy_states_hotkey_enable").value_or(false);
  hotkey_save    = cfg.get<int>("enemy_states_save_hotkey").value_or(0x24); // HOME
  hotkey_load    = cfg.get<int>("enemy_states_load_hotkey").value_or(0x23); // END
}

// during save
void EnemyStates::on_config_save(utility::Config & cfg) {
  cfg.set<bool>("enemy_states_hotkey_enable", hotkey_enabled);
  cfg.set<int>("enemy_states_save_hotkey", hotkey_save);
  cfg.set<int>("enemy_states_load_hotkey", hotkey_load);
}

#endif
