// TODO(): fix this to yeet xinput plus
#if DINPUT_HOOK
#include "GamepadsFix.hpp"
#include <vector>

#include <SDL.h>
#include <SDL_gamecontroller.h>

static bool     g_di_swap_sticks  = false;
static bool     g_di_fix_triggers = false;

static int32_t g_di_l2_btn_index = 0;
static int32_t g_di_r2_btn_index = 0;

static SDL_Event g_event;

static uintptr_t g_jmp_back;
// clang-format off
static __declspec(naked) void detour() {
	__asm {
		int 3
	    jmp DWORD PTR [g_jmp_back]
	}
}

struct SDLGamepadState{
	// Axis values range from -1.0f to 1.0f
	struct LeftStickAxis {float x= 0.0f; float y = 0.0f;} LeftStick;
	// Axis values range from -1.0f - 1.0f
	struct RightStickAxis {float x = 0.0f; float y = 0.0f;} RightStick;
	int A = 0;
	int B = 0;
	int X = 0;
	int Y = 0;
	int DPadUp = 0;
	int DPadDown = 0;
	int DPadLeft = 0;
	int DPadRight = 0;
	int LeftShoulder = 0;
	int RightShoulder = 0;
	int LeftStickClick = 0;
	int RightStickClick = 0;
	int Start = 0;
	int Back = 0;
	int Touchpad = 0;
	int Guide = 0;
	int Misc = 0;
	int Paddle1 = 0;
	int Paddle2 = 0;
	int Paddle3 = 0;
	int Paddle4 = 0;
	// Axis values range from 0.0f to 1.0f
	float LeftTrigger = 0.0f;
	// Axis values range from 0.0f to 1.0f
	float RightTrigger = 0.0f;
};

struct SDLGamepadSensorState {
	// Explaination taken from SDL_sensor.h
	// For game controllers held in front of you,
	// the axes are defined as follows:
	// -X ... +X : left ... right
	// -Y ... +Y : bottom ... top
	// -Z ... +Z : farther ... closer

	// values[0]: Acceleration on the x axis
	// values[1]: Acceleration on the y axis
	// values[2]: Acceleration on the z axis
	float Accelerometer[3] = {0.0f, 0.0f, 0.0f};

	// values[0]: Angular speed around the x axis (pitch)
	// values[1]: Angular speed around the y axis (yaw)
	// values[2]: Angular speed around the z axis (roll)
	float Gyroscope[3] = {0.0f, 0.0f, 0.0f};
};

struct SDLGamepadTouchpadFinger{
	Uint8 state;
	float x = 0.0f;
	float y = 0.0f;
	float pressure = 0.0f;
};

struct SDLGamepadTouchpad {
	std::vector<SDLGamepadTouchpadFinger> fingers;
};

class SDLGamepad {
private:
	std::string name = "";
	SDL_GameController * controller;
	int touchpadCount = 0;
	bool hapticsSupported = false;
	bool triggerHapticsSupported = false;
	bool sensorSupported = false;
	bool gyroSupported = false;
	bool accelSupported = false;
	bool touchpadSupported = false;

public:
	//What's below was added pureply for the purpose of ImGui.
	struct VibrationValues{
		float motor_left = 0.0;
		float motor_right = 0.0;
		float trigger_left = 0.0;
		float trigger_right = 0.0;
	} vibration;

	SDL_Color led_color{0, 0, 255, 255};
	//Required stuff is below___________________
	SDL_JoystickID id;
	//SDLGamepadState last_state;
	SDLGamepadState state;
	//SDLGamepadSensorState last_sensor_state;
	SDLGamepadSensorState sensor_state;
	std::vector<SDLGamepadTouchpad> touchpads;
	bool sensorEnabled = false;
	bool gyroActive = false;
	bool accelActive = false;
	bool queryTouchpads = false;

	SDLGamepad(int index){
		controller = SDL_GameControllerOpen(index);
		id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
		name = SDL_GameControllerName(controller);
		if (SDL_GameControllerRumble(controller, 0, 0, 0) == 0){
			hapticsSupported = true;
		}
		if (SDL_GameControllerRumbleTriggers(controller, 0, 0, 0) == 0){
			triggerHapticsSupported = true;
		}
		if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_ACCEL) || SDL_GameControllerHasSensor(controller, SDL_SENSOR_GYRO)){
			sensorSupported = true;
			if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_ACCEL)){
				accelSupported = true;
			}
			if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_GYRO)){
				gyroSupported = true;
			}
		}
		touchpadCount = SDL_GameControllerGetNumTouchpads(controller);
		if (touchpadCount){
			touchpadSupported = true;
			touchpads.resize(touchpadCount);
			for (int i = 0; i < touchpadCount; i++){
				touchpads[i].fingers.resize(SDL_GameControllerGetNumTouchpadFingers(controller, i));
			}
		}
	}

	~SDLGamepad(){
		SDL_GameControllerClose(controller);
	}

	std::string getName(){
		return name;
	}

	SDL_GameController * getController(){
		return controller;
	}

	int getTouchpadCount(){
		return touchpadCount;
	}

	bool hasHaptics(){
		return hapticsSupported;
	}

	bool hasTriggerHaptics(){
		return triggerHapticsSupported;
	}

	bool hasSensors(){
		return sensorSupported;
	}

	bool hasAccelerometer(){
		return sensorSupported && accelSupported;
	}

	bool hasGyroscope(){
		return sensorSupported && gyroSupported;
	}

	bool hasAllSensors(){
		return hasAccelerometer() && hasGyroscope();
	}

	bool hasLED(){
		return SDL_GameControllerHasLED(controller);
	}

	void setSensor(SDL_SensorType type, SDL_bool active){
		if (type == SDL_SENSOR_GYRO){
			gyroActive = active;
		}
		if (type == SDL_SENSOR_ACCEL){
			accelActive = active;
		}
		sensorEnabled = (gyroActive || accelActive);
		SDL_GameControllerSetSensorEnabled(controller, type, active);
	}

	void setTouchpadSensing(bool active){
		if (touchpadSupported){
			queryTouchpads = active;
		}
	}

	void pollTouchpad(){
		if (queryTouchpads){
			for (int index = 0; index < touchpadCount; index++){
				for (int finger = 0; finger < touchpads[index].fingers.size(); finger++){
					SDL_GameControllerGetTouchpadFinger(controller, index, finger,
						&touchpads[index].fingers[finger].state,
						&touchpads[index].fingers[finger].x,
						&touchpads[index].fingers[finger].y,
						&touchpads[index].fingers[finger].pressure);
				}
			}
		}
	}

	void pollState(){
		//last_state = state;
		state = SDLGamepadState();
		//last_sensor_state = sensor_state;
		sensor_state = SDLGamepadSensorState();
		//DPad buttons
		state.DPadUp = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
		state.DPadDown = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
		state.DPadLeft = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
		state.DPadRight = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
		//Face Buttons (based on Xbox controller layout)
		state.A = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
		state.B = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);
		state.X = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
		state.Y = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);
		// Start, Back, and Guide
		state.Start = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START);
		state.Back = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK);
		state.Guide = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_GUIDE);
		//Left Click and Right Click
		state.LeftStickClick = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK);
		state.RightStickClick = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
		//Paddles 1-4
		state.Paddle1 = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_PADDLE1);
		state.Paddle2 = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_PADDLE2);
		state.Paddle3 = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_PADDLE3);
		state.Paddle4 = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_PADDLE4);
		//Touchpad Button and Misc (Xbox Share button, Switch Pro Capture button, and Mic button for PS4/PS5 controllers)
		state.Touchpad = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_TOUCHPAD);
		state.Misc = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_MISC1);
		//Left and Right Shoulder
		state.LeftShoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		state.RightShoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
		// Axis values for the left and right stick
		state.LeftStick.x = float(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX)) / float(SDL_JOYSTICK_AXIS_MAX);
		state.LeftStick.y = float(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY)) / float(SDL_JOYSTICK_AXIS_MAX);
		state.RightStick.x = float(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX)) / float(SDL_JOYSTICK_AXIS_MAX);
		state.RightStick.y = float(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY)) / float(SDL_JOYSTICK_AXIS_MAX);
		//Left and Right Trigger
		state.LeftTrigger = float(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT)) / float(SDL_JOYSTICK_AXIS_MAX);
		state.RightTrigger = float(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) / float(SDL_JOYSTICK_AXIS_MAX);

		if (sensorEnabled){
			if (accelActive){
				SDL_GameControllerGetSensorData(controller, SDL_SENSOR_ACCEL, sensor_state.Accelerometer, 3);
			}
			if (gyroActive){
				SDL_GameControllerGetSensorData(controller, SDL_SENSOR_GYRO, sensor_state.Gyroscope, 3);
			}
		}  
		pollTouchpad();
	}

	// left and right values go from 0.0 to 1.0, while duration is in ms.
	void Rumble(float left, float right, Uint32 duration){
		if (hapticsSupported){
			SDL_GameControllerRumble(controller, 0xFFFF*left, 0xFFFF*right, duration);
		}
	}

	// left and right trigger values go from 0.0 to 1.0, while duration is in ms.
	void RumbleTriggers(float left_trigger, float right_trigger, Uint32 duration){
		if (triggerHapticsSupported){
			SDL_GameControllerRumbleTriggers(controller, 0xFFFF*left_trigger, 0xFFFF*right_trigger, duration);
		}    
	}

	void SetLED(Uint8 r, Uint8 g, Uint8 b){
		SDL_GameControllerSetLED(controller, r, g, b);
	}
};


static std::vector<SDLGamepad*> g_gamepads;

std::optional<std::string> GamepadsFix::on_initialize() {
	//SDL_Init(SDL_INIT_GAMECONTROLLER);
	SDL_InitSubSystem(SDL_INIT_SENSOR|SDL_INIT_GAMECONTROLLER|SDL_INIT_HAPTIC);
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");

	SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");
	//SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_JOY_CONS, "1");
	if (!install_hook_absolute(0x0405D24, m_function_hook, &detour, &g_jmp_back, 8)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize GamepadsFix";
	}

  return Mod::on_initialize();
}

void GamepadsFix::dinput_hook_callback(LPDIJOYSTATE joy1) {
#if 0
	if (g_di_fix_triggers) {

		int sign = (joy1->lZ != 0) | (joy1->lZ >> (sizeof(int) * CHAR_BIT - 1));
		if (sign > 0) {
			joy1->rgbButtons[g_di_l2_btn_index] = 0x80;
		}
		if (sign < 0) {
			joy1->rgbButtons[g_di_r2_btn_index] = 0x80;
		}
	}
	if (g_di_swap_sticks) {

		LONG lx = joy1->lX;
		LONG ly = joy1->lY;

		LONG rx = joy1->lRx;
		LONG ry = joy1->lRy;

		joy1->lX = rx;
		joy1->lY = ry;

		joy1->lRx = lx;
		joy1->lRy = ly;
	}
#endif
}

void GamepadsFix::on_config_load(const utility::Config &cfg) {

	g_di_fix_triggers = cfg.get<bool>("di_fix_triggers").value_or(false);
	g_di_swap_sticks  = cfg.get<bool>("di_swap_sticks").value_or(false);

	g_di_l2_btn_index = cfg.get<int32_t>("di_l2_button_index").value_or(0);
	g_di_r2_btn_index = cfg.get<int32_t>("di_r2_button_index").value_or(0);

}

void GamepadsFix::on_config_save(utility::Config &cfg) {
	
	cfg.set<bool>("di_fix_triggers", g_di_fix_triggers);
	cfg.set<bool>("di_swap_sticks",  g_di_swap_sticks);

	cfg.set<int32_t>("di_l2_button_index", g_di_l2_btn_index);
	cfg.set<int32_t>("di_r2_button_index", g_di_r2_btn_index);

}
// do something every frame
void GamepadsFix::on_frame() {

  SDL_PollEvent(&g_event);

  if (g_event.type == SDL_CONTROLLERDEVICEADDED) {
    bool add_device = true;
    for (auto& gamepad : g_gamepads) {
      if (gamepad->id == g_event.cdevice.which) {
        add_device = false;
        break;
      }
    }
    if (add_device) {
      g_gamepads.push_back(new SDLGamepad(g_event.cdevice.which));
    }
  }

  if (g_event.type == SDL_CONTROLLERDEVICEREMOVED) {
    // Remove the controller from the vector and then delete it. This can
    // probably be handled much more efficiently if you simply maintain an array
    // of controllers and delete at the index, or if handled differently in
    // general, but this is simply one way of doing it with this structure.
    SDLGamepad* instance = nullptr;
    for (auto& gamepad : g_gamepads) {
      if (gamepad->id == g_event.cdevice.which) {
        instance = gamepad;
        break;
      }
    }
    g_gamepads.erase(
        std::remove_if(g_gamepads.begin(), g_gamepads.end(), [](const auto& g) {
          return g->id == g_event.cdevice.which;
        }));
    delete instance;
  }
  for (auto& controller: g_gamepads){
	  controller->pollState();
  }
}
// will show up in debug window, dump ImGui widgets you want here
//void GamepadsFix::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void GamepadsFix::on_draw_ui() {

	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}
	static ImVec4 color = {.500, .500, .500, 1.0};

	for (auto& controller : g_gamepads) {
		ImGui::Text(controller->getName().c_str());
		ImVec4 pressed = ImVec4(0.0, 1.0, 0.0, 1.0);
		if (controller->hasLED()){
			std::vector<float> LED_Color = {float(controller->led_color.r) / float(255),
				float(controller->led_color.g) / float(255), 
				float(controller->led_color.b) / float(255), 
				1.0f};

			ImGui::ColorEdit3("LED Color", LED_Color.data());
			controller->led_color.r = LED_Color[0] * 255;
			controller->led_color.g = LED_Color[1] * 255;
			controller->led_color.b = LED_Color[2] * 255;

			controller->SetLED(controller->led_color.r, controller->led_color.g, controller->led_color.b);
		}
		ImGui::NewLine();
		// Show number of touchpads if supported.
		if (controller->getTouchpadCount()){
			ImGui::Text("Number of touchpads: %i", controller->getTouchpadCount());
		}

		// Provide options to enable gyro and accelerometer.
		if (controller->hasSensors()){
			controller->sensorEnabled = true;
			if (controller->hasGyroscope()){
				ImGui::Checkbox("Gyroscope", &controller->gyroActive);
				controller->setSensor(SDL_SENSOR_GYRO, (SDL_bool)controller->gyroActive);
			}
			if (controller->hasAccelerometer()){
				ImGui::Checkbox("Accelerometer", &controller->accelActive);
				controller->setSensor(SDL_SENSOR_ACCEL, (SDL_bool)controller->accelActive);
			}
		}

		if (controller->getTouchpadCount()){
			ImGui::Checkbox("Touchpad Polling", &controller->queryTouchpads);
		}

		// Allow controller rumble to be activated.
		if (controller->hasHaptics()){
			ImGui::SliderFloat("Left Motor", &controller->vibration.motor_left, 0, 1, "%.3f", 1.0f);
			ImGui::SliderFloat("Right Motor", &controller->vibration.motor_right, 0, 1,"%.3f", 1.0f);
			controller->Rumble(controller->vibration.motor_left, controller->vibration.motor_right, 100);   
		}

		// Allow controller trigger rumble to be activated.
		if (controller->hasTriggerHaptics()){
			ImGui::SliderFloat("Left Trigger Motor", &controller->vibration.trigger_left, 0, 1, "%.3f", 1.0f);
			ImGui::SliderFloat("Right Trigger Motor", &controller->vibration.trigger_right, 0, 1,"%.3f", 1.0f);
			controller->RumbleTriggers(controller->vibration.trigger_left, controller->vibration.trigger_right, 100);   
		}


		ImGui::NewLine();
		// Print the face buttons, and color them if pressed.
		// Using the class, to query buttons you check the state struct.
		ImGui::TextColored(color, "Face Buttons");
		if (controller->state.A){
			ImGui::TextColored(pressed, "Button A");
		}else {ImGui::Text("Button A");}

		if (controller->state.B){
			ImGui::TextColored(pressed, "Button B");
		}else {ImGui::Text("Button B");}

		if (controller->state.X){
			ImGui::TextColored(pressed, "Button X");
		}else {ImGui::Text("Button X");}

		if (controller->state.Y){
			ImGui::TextColored(pressed, "Button Y");
		}else {ImGui::Text("Button Y");}


		ImGui::NewLine();
		// Print the DPad Buttons, and color them if they are pressed.   
		// Using the class, to query buttons you check the state struct.
		ImGui::TextColored(color, "DPAD Buttons");
		if (controller->state.DPadUp){
			ImGui::TextColored(pressed, "Up");
		}else {ImGui::Text("Up");}

		if (controller->state.DPadDown){
			ImGui::TextColored(pressed, "Down");
		}else {ImGui::Text("Down");}

		if (controller->state.DPadLeft){
			ImGui::TextColored(pressed, "Left");
		}else {ImGui::Text("Left");}

		if (controller->state.DPadRight){
			ImGui::TextColored(pressed, "Right");
		}else {ImGui::Text("Right");}


		ImGui::NewLine();
		// Print the DPad Buttons, and color them if they are pressed.   
		// Using the class, to query buttons you check the state struct.
		ImGui::TextColored(color, "Shoulder Buttons and Stick Clicks");
		if (controller->state.LeftShoulder){
			ImGui::TextColored(pressed, "Left Shoulder");
		}else {ImGui::Text("Left Shoulder");}

		if (controller->state.RightShoulder){
			ImGui::TextColored(pressed, "Right Shoulder");
		}else {ImGui::Text("Right Shoulder");}

		if (controller->state.LeftStickClick){
			ImGui::TextColored(pressed, "Left Stick");
		}else {ImGui::Text("Left Stick");}

		if (controller->state.RightStickClick){
			ImGui::TextColored(pressed, "Right Stick");
		}else {ImGui::Text("Right Stick");}


		ImGui::NewLine();
		// Print the DPad Buttons, and color them if they are pressed.   
		// Using the class, to query buttons you check the state struct.
		ImGui::TextColored(color, "Start, Back, Guide");
		if (controller->state.Start){
			ImGui::TextColored(pressed, "Start");
		}else {ImGui::Text("Start");}

		if (controller->state.Back){
			ImGui::TextColored(pressed, "Back");
		}else {ImGui::Text("Back");}

		if (controller->state.Guide){
			ImGui::TextColored(pressed, "Guide");
		}else {ImGui::Text("Guide");}


		ImGui::NewLine();
		// Print the Axis values for the Triggers
		ImGui::TextColored(color, "Left Trigger and Right Trigger");
		ImGui::Text("Left Trigger: %.3f ,  Right Trigger: %.3f", controller->state.LeftTrigger, controller->state.RightTrigger);


		ImGui::NewLine();
		// Print the Axis values for the Sticks.
		ImGui::TextColored(color, "Left Stick and Right Stick");
		ImGui::Text("Left Stick (x: %.3f ,  y: %.3f)", controller->state.LeftStick.x, controller->state.LeftStick.y);
		ImGui::Text("Right Stick (x: %.3f ,  y: %.3f)", controller->state.RightStick.x, controller->state.RightStick.y);


		if (controller->sensorEnabled){
			ImGui::NewLine();
			// Print the Axis values for the Sticks.
			ImGui::TextColored(color, "Gyro and/or Accelerometer");
			if (controller->hasGyroscope()){
				ImGui::Text("Gyroscope (x: %.3f ,  y: %.3f, z: %.3f)", controller->sensor_state.Gyroscope[0],
					controller->sensor_state.Gyroscope[1],
					controller->sensor_state.Gyroscope[2]);
			}
			if (controller->hasAccelerometer()){
				ImGui::Text("Accelerometer (x: %.3f ,  y: %.3f, z: %.3f)", controller->sensor_state.Accelerometer[0],
					controller->sensor_state.Accelerometer[1],
					controller->sensor_state.Accelerometer[2]);
			}

		}

		ImGui::NewLine();
		// Print the Paddle Buttons
		ImGui::TextColored(color, "Paddle Buttons");
		if (controller->state.Paddle1){
			ImGui::TextColored(pressed, "Paddle1");
		}else {ImGui::Text("Paddle1");}

		if (controller->state.Paddle2){
			ImGui::TextColored(pressed, "Paddle2");
		}else {ImGui::Text("Paddle2");}

		if (controller->state.Paddle3){
			ImGui::TextColored(pressed, "Paddle3");
		}else {ImGui::Text("Paddle3");}

		if (controller->state.Paddle4){
			ImGui::TextColored(pressed, "Paddle4");
		}else {ImGui::Text("Paddle4");}


		ImGui::NewLine();
		// Print the Touchpad, and Misc button (Capture, Mic, and Share button respectively)
		ImGui::TextColored(color, "Touchpad and Misc");
		if (controller->state.Touchpad){
			ImGui::TextColored(pressed, "Touchpad");
		}else {ImGui::Text("Touchpad");}

		if (controller->state.Misc){
			ImGui::TextColored(pressed, "Misc");
		}else {ImGui::Text("Misc");}


		ImGui::NewLine();
		// Show Touchpad coordinates.
		if (controller->getTouchpadCount() && controller->queryTouchpads){
			if (ImGui::CollapsingHeader("Touchpads")){
				for (int i = 0; i < controller->touchpads.size(); i++){
					if (ImGui::CollapsingHeader(("Touchpad: "+ std::to_string(i)).c_str())){
						if (ImGui::BeginTable("Fingers", 1)){
							for (int j = 0; j < controller->touchpads[i].fingers.size(); j++){
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
	}

	/*
	ImGui::Checkbox("Swap Left and Right Sticks", &g_di_swap_sticks);
	ImGui::Text("<3 to endneo");

	ImGui::Checkbox("Fix L2/R2 buttons", &g_di_fix_triggers);

	ImGui::Text("Set inputs below to values you have in dmc3se.ini");
	ImGui::Text("look for lines L2=value R2=value");

	ImGui::InputInt("L2", &g_di_l2_btn_index);
	ImGui::InputInt("R2", &g_di_r2_btn_index);
	ImGui::TextWrapped(
		"there is no bounds checking done so, dont set those things"
		" to values below zero or larger than 32 or you'll crash the game or something");

	*/
}
#endif