#if DINPUT_HOOK
#define INITGUID
#include "GamepadsFix.hpp"
#include <vector>

#include <SDL.h>
#include <SDL_gamecontroller.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "utility/SDLGamepad.hpp"

#define CONTROLLERS_RESERVED 8

static GamepadsFix* g_gamepad_mod_inst_ptr;
static std::vector<SDLGamepad*> g_gamepads;

bool ifEmulatingXboxController;

// TODO(): too tired not doing that anytime soon
class PadVibeStuffStatic
{
public:
	char pad_0000[560]; //0x0000
	float vibration_strength; //0x0230
	uint32_t index_something; //0x0234
}; //Size: 0x0238

const std::vector<int16_t> default_controls_bytes() {
  return {0xC7, 0x44, 0x24, 0x14, 0x02, 0x00, 0x00, 0x00,  // mov [esp+0x14], 00000002 // was 3
		  0xC7, 0x44, 0x24, 0x18, 0x03, 0x00, 0x00, 0x00,  // mov [esp+0x18], 00000003 // was 1
		  0xC7, 0x44, 0x24, 0x1C, 0x00, 0x00, 0x00, 0x00,  // mov [esp+0x1C], 00000000 // was 0
		  0xC7, 0x44, 0x24, 0x20, 0x01, 0x00, 0x00, 0x00}; // mov [esp+0x20], 00000001 // was 2
}

std::optional<std::string> GamepadsFix::on_initialize() {
	// WARNING(): dirty hack to only init once here:
	static bool init = false;
	if (init) {
		return Mod::on_initialize();
	}

	bool dmc_ini_exists = _waccess_s(L"dmc3se.ini", 00) != ENOENT;
	spdlog::info("[GamepadFix] dmc_ini_exists? {}", dmc_ini_exists);
	if (!dmc_ini_exists) {
		m_dinput8_create_hook = std::make_unique<FunctionHook>(dinput8_create_sub_address(), &Dinput8Create_sub_404BB0);
		if (!m_dinput8_create_hook->create()) {
			return "[GamepadFix] Tried to init since no dmc3se.ini. Failed to hook dinput8_create();";
		}
	}
	g_gamepad_mod_inst_ptr = this;
	init = true;
  return Mod::on_initialize();
}

//void GamepadsFix::on_config_load(const utility::Config &cfg) {}
//void GamepadsFix::on_config_save(utility::Config &cfg) {}

void GamepadsFix::on_frame() {
	m_focused = g_framework->get_window_focus();
	if (g_gamepads.empty() || !m_led_animation) {
		return;
	}
	auto& controller = g_gamepads.at(m_gamepad_index);

	if (controller->hasLED()) {
		if (m_time % 2 == 0) {
			// let's call this load balancing
			glm::vec3 color;
			ImGui::ColorConvertHSVtoRGB(std::sin((float)m_time * 0.02f) * 0.5f + 0.5f, 1.0f, 0.7f, color.r, color.g, color.b);

			controller->SetLED(color.r * 255, color.g * 255, color.b * 255);
		}
	}
	m_time += 1;
}
// will show up in debug window, dump ImGui widgets you want here
//void GamepadTriggersFix::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void GamepadsFix::on_draw_ui() {

	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}
	ImGui::TextWrapped("Check the following checkbox if you are using or emulating an xbox controller. This will set the default controls to match PS4 so the following fixes are correctly applied. Press save config after, then reset the game.");
	ImGui::Checkbox("Emulating or using an xbox controller", &ifEmulatingXboxController);

	ImGui::TextWrapped("To get this working rename dmc3se.ini in the game root to something else and restart the game, this would replace dinput8 stuff with SDL2 for better gamepad support. Hooks need to be installed on launch so restart is required.");
	ImGui::Text("Window focus: %d", g_framework->get_window_focus());

	ImGui::TextWrapped("You can select what controller to use below, keep in mind this is zero indexed. First controller - 0, second controller - 1 etc.");
	if (ImGui::InputInt("Gamepad index: ", &m_gamepad_index, 1, 1)) {
		m_gamepad_index = glm::clamp<int>(m_gamepad_index, 0, g_gamepads.size() - 1);
	}

	ImGui::Checkbox("LED RGB slide for PS4/PS5 gamepads", &m_led_animation);
	
	static ImVec4 color = {.500, .500, .500, 1.0};

	ImGui::Text("Detected gamepads:");
	for (auto& controller : g_gamepads) {
		if (ImGui::TreeNode(controller->getName().c_str())) {
			ImGui::Text(controller->getName().c_str());
			ImVec4 pressed = ImVec4(0.0, 1.0, 0.0, 1.0);
			if (controller->hasLED()) {
				std::vector<float> LED_Color = { float(controller->led_color.r) / float(255),
					float(controller->led_color.g) / float(255),
					float(controller->led_color.b) / float(255),
					1.0f };

				ImGui::ColorEdit3("LED Color", LED_Color.data());
				controller->led_color.r = LED_Color[0] * 255;
				controller->led_color.g = LED_Color[1] * 255;
				controller->led_color.b = LED_Color[2] * 255;

				controller->SetLED(controller->led_color.r, controller->led_color.g, controller->led_color.b);
			}
			ImGui::NewLine();
			// Show number of touchpads if supported.
			if (controller->getTouchpadCount()) {
				ImGui::Text("Number of touchpads: %i", controller->getTouchpadCount());
			}

			// Provide options to enable gyro and accelerometer.
			if (controller->hasSensors()) {
				controller->sensorEnabled = true;
				if (controller->hasGyroscope()) {
					ImGui::Checkbox("Gyroscope", &controller->gyroActive);
					controller->setSensor(SDL_SENSOR_GYRO, (SDL_bool)controller->gyroActive);
				}
				if (controller->hasAccelerometer()) {
					ImGui::Checkbox("Accelerometer", &controller->accelActive);
					controller->setSensor(SDL_SENSOR_ACCEL, (SDL_bool)controller->accelActive);
				}
			}

			if (controller->getTouchpadCount()) {
				ImGui::Checkbox("Touchpad Polling", &controller->queryTouchpads);
			}

			// Allow controller rumble to be activated.
			if (controller->hasHaptics()) {
				ImGui::SliderFloat("Left Motor", &controller->vibration.motor_left, 0, 1, "%.3f", 1.0f);
				ImGui::SliderFloat("Right Motor", &controller->vibration.motor_right, 0, 1, "%.3f", 1.0f);
				controller->Rumble(controller->vibration.motor_left, controller->vibration.motor_right, 100);
			}

			// Allow controller trigger rumble to be activated.
			if (controller->hasTriggerHaptics()) {
				ImGui::SliderFloat("Left Trigger Motor", &controller->vibration.trigger_left, 0, 1, "%.3f", 1.0f);
				ImGui::SliderFloat("Right Trigger Motor", &controller->vibration.trigger_right, 0, 1, "%.3f", 1.0f);
				controller->RumbleTriggers(controller->vibration.trigger_left, controller->vibration.trigger_right, 100);
			}


			ImGui::NewLine();
			// Print the face buttons, and color them if pressed.
			// Using the class, to query buttons you check the state struct.
			ImGui::TextColored(color, "Face Buttons");
			if (controller->state.A) {
				ImGui::TextColored(pressed, "Button A");
			}
			else { ImGui::Text("Button A"); }

			if (controller->state.B) {
				ImGui::TextColored(pressed, "Button B");
			}
			else { ImGui::Text("Button B"); }

			if (controller->state.X) {
				ImGui::TextColored(pressed, "Button X");
			}
			else { ImGui::Text("Button X"); }

			if (controller->state.Y) {
				ImGui::TextColored(pressed, "Button Y");
			}
			else { ImGui::Text("Button Y"); }


			ImGui::NewLine();
			// Print the DPad Buttons, and color them if they are pressed.   
			// Using the class, to query buttons you check the state struct.
			ImGui::TextColored(color, "DPAD Buttons");
			if (controller->state.DPadUp) {
				ImGui::TextColored(pressed, "Up");
			}
			else { ImGui::Text("Up"); }

			if (controller->state.DPadDown) {
				ImGui::TextColored(pressed, "Down");
			}
			else { ImGui::Text("Down"); }

			if (controller->state.DPadLeft) {
				ImGui::TextColored(pressed, "Left");
			}
			else { ImGui::Text("Left"); }

			if (controller->state.DPadRight) {
				ImGui::TextColored(pressed, "Right");
			}
			else { ImGui::Text("Right"); }


			ImGui::NewLine();
			// Print the DPad Buttons, and color them if they are pressed.   
			// Using the class, to query buttons you check the state struct.
			ImGui::TextColored(color, "Shoulder Buttons and Stick Clicks");
			if (controller->state.LeftShoulder) {
				ImGui::TextColored(pressed, "Left Shoulder");
			}
			else { ImGui::Text("Left Shoulder"); }

			if (controller->state.RightShoulder) {
				ImGui::TextColored(pressed, "Right Shoulder");
			}
			else { ImGui::Text("Right Shoulder"); }

			if (controller->state.LeftStickClick) {
				ImGui::TextColored(pressed, "Left Stick");
			}
			else { ImGui::Text("Left Stick"); }

			if (controller->state.RightStickClick) {
				ImGui::TextColored(pressed, "Right Stick");
			}
			else { ImGui::Text("Right Stick"); }


			ImGui::NewLine();
			// Print the DPad Buttons, and color them if they are pressed.   
			// Using the class, to query buttons you check the state struct.
			ImGui::TextColored(color, "Start, Back, Guide");
			if (controller->state.Start) {
				ImGui::TextColored(pressed, "Start");
			}
			else { ImGui::Text("Start"); }

			if (controller->state.Back) {
				ImGui::TextColored(pressed, "Back");
			}
			else { ImGui::Text("Back"); }

			if (controller->state.Guide) {
				ImGui::TextColored(pressed, "Guide");
			}
			else { ImGui::Text("Guide"); }


			ImGui::NewLine();
			// Print the Axis values for the Triggers
			ImGui::TextColored(color, "Left Trigger and Right Trigger");
			ImGui::Text("Left Trigger: %.3f ,  Right Trigger: %.3f", controller->state.LeftTrigger, controller->state.RightTrigger);


			ImGui::NewLine();
			// Print the Axis values for the Sticks.
			ImGui::TextColored(color, "Left Stick and Right Stick");
			ImGui::Text("Left Stick (x: %.3f ,  y: %.3f)", controller->state.LeftStick.x, controller->state.LeftStick.y);
			ImGui::Text("Right Stick (x: %.3f ,  y: %.3f)", controller->state.RightStick.x, controller->state.RightStick.y);


			if (controller->sensorEnabled) {
				ImGui::NewLine();
				// Print the Axis values for the Sticks.
				ImGui::TextColored(color, "Gyro and/or Accelerometer");
				if (controller->hasGyroscope()) {
					ImGui::Text("Gyroscope (x: %.3f ,  y: %.3f, z: %.3f)", controller->sensor_state.Gyroscope[0],
						controller->sensor_state.Gyroscope[1],
						controller->sensor_state.Gyroscope[2]);
				}
				if (controller->hasAccelerometer()) {
					ImGui::Text("Accelerometer (x: %.3f ,  y: %.3f, z: %.3f)", controller->sensor_state.Accelerometer[0],
						controller->sensor_state.Accelerometer[1],
						controller->sensor_state.Accelerometer[2]);
				}

			}

			ImGui::NewLine();
			// Print the Paddle Buttons
			ImGui::TextColored(color, "Paddle Buttons");
			if (controller->state.Paddle1) {
				ImGui::TextColored(pressed, "Paddle1");
			}
			else { ImGui::Text("Paddle1"); }

			if (controller->state.Paddle2) {
				ImGui::TextColored(pressed, "Paddle2");
			}
			else { ImGui::Text("Paddle2"); }

			if (controller->state.Paddle3) {
				ImGui::TextColored(pressed, "Paddle3");
			}
			else { ImGui::Text("Paddle3"); }

			if (controller->state.Paddle4) {
				ImGui::TextColored(pressed, "Paddle4");
			}
			else { ImGui::Text("Paddle4"); }


			ImGui::NewLine();
			// Print the Touchpad, and Misc button (Capture, Mic, and Share button respectively)
			ImGui::TextColored(color, "Touchpad and Misc");
			if (controller->state.Touchpad) {
				ImGui::TextColored(pressed, "Touchpad");
			}
			else { ImGui::Text("Touchpad"); }

			if (controller->state.Misc) {
				ImGui::TextColored(pressed, "Misc");
			}
			else { ImGui::Text("Misc"); }


			ImGui::NewLine();
			// Show Touchpad coordinates.
			if (controller->getTouchpadCount() && controller->queryTouchpads) {
				if (ImGui::CollapsingHeader("Touchpads")) {
					for (int i = 0; i < controller->touchpads.size(); i++) {
						if (ImGui::CollapsingHeader(("Touchpad: " + std::to_string(i)).c_str())) {
							if (ImGui::BeginTable("Fingers", 1)) {
								for (int j = 0; j < controller->touchpads[i].fingers.size(); j++) {
									ImGui::TableNextColumn();
									ImGui::Text("Finger %i: (x: %f, y: %f, pressure: %f, state: %d)", j,
										controller->touchpads[i].fingers[j].x,
										controller->touchpads[i].fingers[j].y,
										controller->touchpads[i].fingers[j].pressure,
										controller->touchpads[i].fingers[j].state);
								}

								ImGui::EndTable();
							}
						}
					}
				}
			}

			ImGui::TreePop();
			ImGui::Separator();
		}
	}
	ImGui::Separator();
}

// NOTE(): dont mind me just doing some crimes against winapi programming
struct dummy_dinput8_device_object_inst {
	DWORD dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
	GUID  guidType;
	DWORD dwOfs;
	DWORD dwType;
	DWORD dwFlags;
	TCHAR tszName[MAX_PATH];
	DWORD dwFFMaxForce;
	DWORD dwFFForceResolution;
	WORD wCollectionNumber;
	WORD wDesignatorIndex;
	WORD wUsagePage;
	WORD wUsage;
	DWORD dwDimension;
	WORD wExponent;
	WORD wReportId;
};
static dummy_dinput8_device_object_inst g_dummy_di8_device_object_inst;
struct dummy_dinput8_device {
	SDL_Event sdl_evt { 0 };
	char pad[sizeof(IDirectInputDevice)]{ 0 };
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj)  { return S_OK; };
	STDMETHOD_(ULONG,AddRef)(THIS) { return S_OK; };
	STDMETHOD_(ULONG,Release)(THIS) { return S_OK; };

	/*** IDirectInputDeviceA methods ***/
	STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS) { return S_OK; };
	STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA a1,LPVOID a2,DWORD a3) { 
		spdlog::info("[dummy_dinput8_device::EnumObjects()] called");
		dummy_dinput8_device_object_inst objects[8] { 0 };
		objects[0].dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
		objects[0].guidType = GUID_XAxis;
		objects[1].dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
		objects[1].guidType = GUID_YAxis;
		objects[2].dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
		objects[2].guidType = GUID_ZAxis;
		objects[3].dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
		objects[3].guidType = GUID_RxAxis;
		objects[4].dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
		/*objects[4].guidType = GUID_RyAxis;
		objects[5].dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
		objects[5].guidType = GUID_RzAxis;
		objects[6].dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
		objects[6].guidType = GUID_Slider;
		objects[7].dwSize = sizeof(DIDEVICEOBJECTINSTANCE);*/
		objects[7].guidType = GUID_POV;

		for (size_t i = 0; i < 9; i++) {
			a1((LPCDIDEVICEOBJECTINSTANCE)&objects[i], a2);
		}

		return S_OK; 
	};
	STDMETHOD(GetProperty)(THIS_ REFGUID,LPDIPROPHEADER) { return S_OK; };
	STDMETHOD(SetProperty)(THIS_ REFGUID,LPCDIPROPHEADER) { return S_OK; };
	STDMETHOD(Acquire)(THIS) { return DI_OK; };
	STDMETHOD(Unacquire)(THIS) { return S_OK; };
	// NOTE(): lpvData should be LPVOID but the game here only passes DIJOYSTATE
	// also this port crashes if you sneeze on it anyway
	STDMETHOD(GetDeviceState)(THIS_ DWORD cbData,DIJOYSTATE* lpvData) { 
#ifdef _DEBUG
		spdlog::debug("[dummy_dinput8_device::GetDeviceState()] called");
#endif
		if (g_gamepads.empty()) {
			return S_OK;
		}

		for (auto& controller: g_gamepads){
			controller->pollState();
		}

		auto& controller = g_gamepads.at(g_gamepad_mod_inst_ptr->m_gamepad_index);

		lpvData->rgbButtons[0] = controller->state.X << 7;
		lpvData->rgbButtons[1] = controller->state.A << 7;
		lpvData->rgbButtons[2] = controller->state.B << 7;
		lpvData->rgbButtons[3] = controller->state.Y << 7;

		lpvData->rgbButtons[4] = controller->state.LeftShoulder << 7;
		lpvData->rgbButtons[5] = controller->state.RightShoulder << 7;

		lpvData->rgbButtons[6] = controller->state.LeftTrigger  > 20 ? 0x80 : 0;
		lpvData->rgbButtons[7] = controller->state.RightTrigger > 20 ? 0x80 : 0;
		
		// not sure
		lpvData->rgbButtons[8] = controller->state.LeftStickClick  << 7;
		lpvData->rgbButtons[9] = controller->state.RightStickClick << 7;

		lpvData->rgbButtons[10] = 
			controller->state.Back  << 7 | 
			controller->state.Touchpad << 7; // idk thats how it supposed to work on ps4?

		lpvData->rgbButtons[11] = controller->state.Start << 7;
		
		//dpad
		//r 9000 u 0 l 27000 d 18000
		lpvData->rgdwPOV[0] = -1;
		
		if (controller->state.DPadUp) {
			lpvData->rgdwPOV[0] &= 0;
		}
		if (controller->state.DPadDown) {
			lpvData->rgdwPOV[0] &= 18000;
		}
		if (controller->state.DPadLeft) {
			lpvData->rgdwPOV[0] &= 27000;
		}
		if (controller->state.DPadRight) {
			lpvData->rgdwPOV[0] &= 9000;
		}

		lpvData->lX = controller->state.RightStick.y >> 8;
		lpvData->lY = controller->state.RightStick.x >> 8; // What the fuck capcom
		lpvData->lZ  = controller->state.LeftStick.y >> 8;
		lpvData->lRx = controller->state.LeftStick.x >> 8;
		lpvData->lRy = 0;
		lpvData->lRz = 0;
		lpvData->rglSlider[0] = 0;
		lpvData->rglSlider[1] = 0;

		return S_OK; 
	};
	STDMETHOD(GetDeviceData)(THIS_ DWORD,LPDIDEVICEOBJECTDATA,LPDWORD,DWORD) { return S_OK; };
	STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT) { return S_OK; };
	STDMETHOD(SetEventNotification)(THIS_ HANDLE) { return S_OK; };
	STDMETHOD(SetCooperativeLevel)(THIS_ HWND,DWORD) { return S_OK; };
	STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA,DWORD,DWORD) { return S_OK; };
	STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA a1) { 
		return S_OK; 
	};
	STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) { return S_OK; };
	STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD,REFGUID) { return S_OK; };
	// idk what are those
	STDMETHOD(Function0x48)(THIS_ HINSTANCE) { return S_OK; };
	STDMETHOD(Function0x4C)(THIS_ HINSTANCE) { return S_OK; };
	STDMETHOD(Function0x50)(THIS_ HINSTANCE) { return S_OK; };
	STDMETHOD(Function0x54)(THIS_ HINSTANCE) { return S_OK; };
	STDMETHOD(Function0x58)(THIS_ HINSTANCE) { return S_OK; };
	STDMETHOD(Function0x5C)(THIS_ HINSTANCE) { return S_OK; };
	STDMETHOD(Function0x60)(THIS_ HINSTANCE) { return S_OK; };
	// poll should return DI_OK if it's safe to call GetDeviceState it seems
	// at least thats how they programmed it (not very good)
	STDMETHOD(Poll)() { 

#ifdef _DEBUG
		//spdlog::debug("Dinput8::Poll called\n");
#endif // _DEBUG

		while (SDL_PollEvent(&sdl_evt)) {
			if (sdl_evt.type == SDL_CONTROLLERDEVICEADDED) {
				spdlog::info("[dummy_dinput8_device::Poll()] Controller connected: {}", sdl_evt.cdevice.which);
#ifdef _DEBUG
				printf("sdl_evt: %d = SDL_CONTROLLERDEVICEADDED\n", sdl_evt.type);
#endif
				bool add_device = true;
				for (auto& gamepad : g_gamepads) {
					if (gamepad->id == sdl_evt.cdevice.which) {
						add_device = false;
						break;
					}
				}
				if (add_device) {
					g_gamepads.push_back(new SDLGamepad(sdl_evt.cdevice.which));
				}
			}

			if (sdl_evt.type == SDL_CONTROLLERDEVICEREMOVED) {
#ifdef _DEBUG
				printf("sdl_evt: %d = SDL_CONTROLLERDEVICEREMOVED\n", sdl_evt.type);
#endif
				spdlog::info("[dummy_dinput8_device::Poll()] Controller removed: {}", sdl_evt.cdevice.which);
				// Remove the controller from the vector and then delete it. This can probably be handled 
				// much more efficiently if you simply maintain an array of controllers and delete at the index, or if
				// handled differently in general, but this is simply one way of doing it with this structure.
				int popped = 0;
				SDLGamepad * instance = nullptr;
				for (int i = 0; i < g_gamepads.size(); i++) {
					if (g_gamepads[i]->id == sdl_evt.cdevice.which) {
						instance = g_gamepads[i];
						popped = i;
						break;
					}
				}
				g_gamepads.erase(g_gamepads.begin() + popped);
				delete instance;
			}
		}

		if (!g_gamepad_mod_inst_ptr->m_focused) {
			return 0x8007000C; // returning this to not call get device state when not focused
			                   // should be some hresult macro but i cant be arsed
		}
		return S_OK;
	};
	STDMETHOD(Function0x68)(THIS_ HINSTANCE) { return S_OK; };
};
static dummy_dinput8_device g_di8_dummy_device;

struct dummy_dinput8_iface {
	char pad[sizeof(IDirectInput)];
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) { return S_OK; };
	STDMETHOD_(ULONG, AddRef)(THIS) { return S_OK; };
	STDMETHOD_(ULONG,Release)(THIS) { return S_OK; };

	/*** IDirectInputA methods ***/
	STDMETHOD(CreateDevice)(THIS_ REFGUID a1, LPDIRECTINPUTDEVICEA *a2, LPUNKNOWN a3) { 
		// NOTE(): we return our fake device to the game here
		spdlog::info("[IDirectInput::CreateDevice] called");
		*a2 = (LPDIRECTINPUTDEVICEA)&g_di8_dummy_device; 
		return S_OK; 
	};
	STDMETHOD(EnumDevices)(THIS_ DWORD a1, LPDIENUMDEVICESCALLBACKA a2, LPVOID a3, DWORD a4) { 
		// NOTE(): the game does not seem to give a fuck about device instance members
		// as evident by it not crashing when i pass obvious bait here
		spdlog::info("[IDirectInput::EnumDevices] called");
		a2((LPCDIDEVICEINSTANCEA)0xDEADBEEF, a3); 
		return S_OK; 
	};
	STDMETHOD(GetDeviceStatus)(THIS_ REFGUID) { return S_OK; };
	STDMETHOD(RunControlPanel)(THIS_ HWND,DWORD) { return S_OK; };
	STDMETHOD(Initialize)(THIS_ HINSTANCE,DWORD) { return S_OK; };
};
static dummy_dinput8_iface di8_dummy;
static IDirectInput** g_game_dinput8_ptr = (IDirectInput**)0x00833238;

int _cdecl GamepadsFix::Dinput8Create_sub_404BB0(HWND hWnd)
{
	spdlog::info("[Dinput8Create_sub_404BB0] hooked Dinput8Create called");
	SDL_InitSubSystem(SDL_INIT_SENSOR|SDL_INIT_GAMECONTROLLER|SDL_INIT_HAPTIC);
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");
	//SDL_SetHint(SDL_HINT_AUTO_UPDATE_JOYSTICKS, "0");

	SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");
	g_gamepads.reserve(CONTROLLERS_RESERVED);
	*g_game_dinput8_ptr = (IDirectInput*)&di8_dummy;
	spdlog::info("[Dinput8Create_sub_404BB0] swapping Dinput8Interface ptr inside game memory {}->", (uintptr_t)g_game_dinput8_ptr, (uintptr_t)&di8_dummy);
	return DI_OK;
}

void GamepadsFix::on_config_load(const utility::Config& cfg) {
  ifEmulatingXboxController = cfg.get<bool>("emulating_xbox_controller").value_or(false);
  patchcontrols = Patch::create(0x00405D34, default_controls_bytes(), (ifEmulatingXboxController));
};

void GamepadsFix::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("emulating_xbox_controller", ifEmulatingXboxController);
};

#endif
