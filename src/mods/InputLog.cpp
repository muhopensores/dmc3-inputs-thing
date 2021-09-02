#include "InputLog.hpp"
#include <deque>
#include <queue>
#include "../PromptFontGlyphs.hpp"

struct input_entry {
	uint32_t m_frame;
	uint16_t m_value;
	uint32_t m_time = 255;

	input_entry(uint16_t value, uint32_t frame) : m_value(value),
	m_frame(frame) {};

	bool update() {
		m_time -= 1;
		if (!m_time) { return false; }
		else { return true; }
	}
};

struct FixedQueue {

	size_t max_size = 20;
	std::deque<input_entry> m_deq;

	void push(uint16_t val, uint32_t frame) {
		if (m_deq.size() == max_size) {
			m_deq.pop_back();
		}
		m_deq.emplace_front(val, frame);
	}
};

static FixedQueue input_fdeq;

uint16_t g_prev_input;
uint32_t g_frame_count;

void get_button(uint16_t input, char buffer[]) {
	// idk sorta constexpr map or something?
	static constexpr std::array<std::pair<const char*, uint16_t>, 16> icon_values{ 
	{
		{ICON_BUTTON_L2_PF,    1},
		{ICON_BUTTON_R2_PF,    2},
		{ICON_BUTTON_L1_PF,    4},
		{ICON_BUTTON_R1_PF,    8},
		{ICON_BUTTON_Y_PF,     16},
		{ICON_BUTTON_B_PF,     32},
		{ICON_BUTTON_A_PF,     64},
		{ICON_BUTTON_X_PF,     128},
		{ICON_SELECT_SHARE_PF, 256},
		{ICON_LEFT_ANALOG_PF,  512},
		{ICON_RIGHT_ANALOG_PF, 1024},
		{ICON_START_PF,        2048},
		{ICON_DPAD_UP_PF,      4096},
		{ICON_DPAD_LEFT_PF,    8192},
		{ICON_DPAD_DOWN_PF,    16384},
		{ICON_DPAD_RIGHT_PF,   32768}
	}
	};
	for (auto& icon : icon_values) {
		if (icon.second & input) {
			strcat(buffer, icon.first);
			strcat(buffer, " ");
		}
	}
}

void InputLog::custom_imgui_window() {
	if (!m_enabled->value()) { return; }

	ImGui::Begin("Input log", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
	
	ImGui::PushFont(g_framework->get_prompt_font());
	size_t index = 0;
	ImGui::Text("Frame:%d", g_frame_count);

	for (auto& input : input_fdeq.m_deq) {
		char buffer[MAX_PATH]{ 0 };
		get_button(input.m_value, buffer);
		// NOTE(): add some smooth fading by using input.m_time as alpha
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, input.m_time));
		ImGui::Text("%d > %s", input.m_frame, buffer);
		ImGui::PopStyleColor();
		++index;
		if (!input.update()) {
			input_fdeq.m_deq.pop_back();
		}
	}
	ImGui::PopFont();
	ImGui::End();
}

std::optional<std::string> InputLog::on_initialize() {
	return Mod::on_initialize();
}

// during load
void InputLog::on_config_load(const utility::Config &cfg) {
	for (IModValue& option : m_options) {
		option.config_load(cfg);
	}
}
// during save
void InputLog::on_config_save(utility::Config &cfg) {
	for (IModValue& option : m_options) {
		option.config_save(cfg);
	}
}
// do something every frame
void InputLog::on_frame() {
	if (!m_enabled->value()) { return; }

	uint16_t input = Devil3SDK::get_buttons_pressed();

	if (input != g_prev_input && input) {
		
		input_fdeq.push(input,g_frame_count);
		g_frame_count = 0;
	}
	++g_frame_count;
	g_prev_input = input;
}
// will show up in debug window, dump ImGui widgets you want here
//void InputLog::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void InputLog::on_draw_ui() {
	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}
	m_enabled->draw("Enable input log");

}
