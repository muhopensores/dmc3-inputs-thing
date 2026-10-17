#pragma once
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3/SDL_gamepad.h>

struct SDLGamepadState{
	// Axis values range from -32768 to 32767
	struct LeftStickAxis {int16_t x = 0; int16_t y = 0;} LeftStick;
	// Axis values range from -32768 to 32767
	struct RightStickAxis {int16_t x = 0; int16_t y = 0;} RightStick;
	uint8_t A = 0;
	uint8_t B = 0;
	uint8_t X = 0;
	uint8_t Y = 0;
	uint8_t DPadUp = 0;
	uint8_t DPadDown = 0;
	uint8_t DPadLeft = 0;
	uint8_t DPadRight = 0;
	uint8_t LeftShoulder = 0;
	uint8_t RightShoulder = 0;
	uint8_t LeftStickClick = 0;
	uint8_t RightStickClick = 0;
	uint8_t Start = 0;
	uint8_t Back = 0;
	uint8_t Touchpad = 0;
	uint8_t Guide = 0;
	uint8_t Misc = 0;
	uint8_t Paddle1 = 0;
	uint8_t Paddle2 = 0;
	uint8_t Paddle3 = 0;
	uint8_t Paddle4 = 0;
	// Axis values range from 0 to 32767
	int16_t LeftTrigger = 0;
	// Axis values range from 0 to 32767
	int16_t RightTrigger = 0;
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
	// NOTE(): SDL3's SDL_GetGamepadTouchpadFinger() fills a bool* "down" instead
	// of SDL2's Uint8* state, so this field is now a bool.
	bool state = false;
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
	// NOTE(): SDL_GameController => SDL_Gamepad in SDL3.
	SDL_Gamepad * gamepad;
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
	SDLGamepadState state;
	SDLGamepadSensorState sensor_state;
	std::vector<SDLGamepadTouchpad> touchpads;
	bool sensorEnabled = false;
	bool gyroActive = false;
	bool accelActive = false;
	bool queryTouchpads = false;

	// NOTE(): in SDL3, SDL_OpenGamepad() takes the joystick instance ID
	// (SDL_JoystickID), not a device index like SDL_GameControllerOpen() did.
	// The caller (SDL_EVENT_GAMEPAD_ADDED's event.gdevice.which) already
	// hands us that instance ID, so this still works unchanged at the call site.
	SDLGamepad(SDL_JoystickID index){
		gamepad = SDL_OpenGamepad(index);
		// SDL_GetGamepadID() replaces SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(...)).
		id = SDL_GetGamepadID(gamepad);
		name = SDL_GetGamepadName(gamepad);
		// SDL_GameControllerRumble()/RumbleTriggers() returned 0 on success (SDL2);
		// SDL_RumbleGamepad()/RumbleGamepadTriggers() return bool, true on success (SDL3).
		if (SDL_RumbleGamepad(gamepad, 0, 0, 0)){
			hapticsSupported = true;
		}
		if (SDL_RumbleGamepadTriggers(gamepad, 0, 0, 0)){
			triggerHapticsSupported = true;
		}
		if (SDL_GamepadHasSensor(gamepad, SDL_SENSOR_ACCEL) || SDL_GamepadHasSensor(gamepad, SDL_SENSOR_GYRO)){
			sensorSupported = true;
			if (SDL_GamepadHasSensor(gamepad, SDL_SENSOR_ACCEL)){
				accelSupported = true;
			}
			if (SDL_GamepadHasSensor(gamepad, SDL_SENSOR_GYRO)){
				gyroSupported = true;
			}
		}
		touchpadCount = SDL_GetNumGamepadTouchpads(gamepad);
		if (touchpadCount){
			touchpadSupported = true;
			touchpads.resize(touchpadCount);
			for (int i = 0; i < touchpadCount; i++){
				touchpads[i].fingers.resize(SDL_GetNumGamepadTouchpadFingers(gamepad, i));
			}
		}
	}

	~SDLGamepad(){
		SDL_CloseGamepad(gamepad);
	}

	std::string getName(){
		return name;
	}

	SDL_Gamepad * getController(){
		return gamepad;
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
		return SDL_GetGamepadProperties(gamepad);
	}

	// NOTE(): SDL_bool is gone in SDL3, everything just uses plain bool now.
	void setSensor(SDL_SensorType type, bool active){
		if (type == SDL_SENSOR_GYRO){
			gyroActive = active;
		}
		if (type == SDL_SENSOR_ACCEL){
			accelActive = active;
		}
		sensorEnabled = (gyroActive || accelActive);
		SDL_SetGamepadSensorEnabled(gamepad, type, active);
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
					// NOTE(): the "state" out-param became a bool* "down" in SDL3.
					bool down = false;
					SDL_GetGamepadTouchpadFinger(gamepad, index, finger,
						&down,
						&touchpads[index].fingers[finger].x,
						&touchpads[index].fingers[finger].y,
						&touchpads[index].fingers[finger].pressure);
					touchpads[index].fingers[finger].state = down;
				}
			}
		}
	}

	void pollState(){
		state = SDLGamepadState();
		sensor_state = SDLGamepadSensorState();
		//DPad buttons
		state.DPadUp = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
		state.DPadDown = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
		state.DPadLeft = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
		state.DPadRight = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
		//Face Buttons (based on Xbox controller layout)
		// NOTE(): SDL3 renamed the face buttons to positional names:
		// A -> SOUTH, B -> EAST, X -> WEST, Y -> NORTH.
		state.A = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
		state.B = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_EAST);
		state.X = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_WEST);
		state.Y = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_NORTH);
		// Start, Back, and Guide
		state.Start = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_START);
		state.Back = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_BACK);
		state.Guide = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_GUIDE);
		//Left Click and Right Click
		state.LeftStickClick = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK);
		state.RightStickClick = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
		//Paddles 1-4
		// NOTE(): PADDLE1 (upper-left) -> LEFT_PADDLE1, PADDLE2 (upper-right) -> RIGHT_PADDLE1,
		// PADDLE3 (lower-left) -> LEFT_PADDLE2, PADDLE4 (lower-right) -> RIGHT_PADDLE2.
		state.Paddle1 = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_PADDLE1);
		state.Paddle2 = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1);
		state.Paddle3 = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_PADDLE2);
		state.Paddle4 = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2);
		//Touchpad Button and Misc (Xbox Share button, Switch Pro Capture button, and Mic button for PS4/PS5 controllers)
		state.Touchpad = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_TOUCHPAD);
		state.Misc = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_MISC1);
		//Left and Right Shoulder
		state.LeftShoulder = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
		state.RightShoulder = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
		// Axis values for the left and right stick
		state.LeftStick.x  = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
		state.LeftStick.y  = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
		state.RightStick.x = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
		state.RightStick.y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
		//Left and Right Trigger
		state.LeftTrigger = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
		state.RightTrigger = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

		if (sensorEnabled){
			if (accelActive){
				SDL_GetGamepadSensorData(gamepad, SDL_SENSOR_ACCEL, sensor_state.Accelerometer, 3);
			}
			if (gyroActive){
				SDL_GetGamepadSensorData(gamepad, SDL_SENSOR_GYRO, sensor_state.Gyroscope, 3);
			}
		}  
		pollTouchpad();
	}

	// left and right values go from 0.0 to 1.0, while duration is in ms.
	void Rumble(float left, float right, Uint32 duration){
		if (hapticsSupported){
			SDL_RumbleGamepad(gamepad, 0xFFFF*left, 0xFFFF*right, duration);
		}
	}

	// left and right trigger values go from 0.0 to 1.0, while duration is in ms.
	void RumbleTriggers(float left_trigger, float right_trigger, Uint32 duration){
		if (triggerHapticsSupported){
			SDL_RumbleGamepadTriggers(gamepad, 0xFFFF*left_trigger, 0xFFFF*right_trigger, duration);
		}    
	}

	void SetLED(Uint8 r, Uint8 g, Uint8 b){
		SDL_SetGamepadLED(gamepad, r, g, b);
	}
};