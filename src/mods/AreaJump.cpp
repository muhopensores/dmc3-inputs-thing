#include "AreaJump.hpp"


// clang-format off
static uintptr_t detour_return {NULL};
static void __declspec(naked) detour() {
    __asm {
        mov dword ptr [esi],007462C4h
        mov dword ptr [g_devil3_main_scene_pointer], esi
        jmp dword ptr [detour_return]
    }
}
static uintptr_t detour1_retun {NULL};
static void __declspec(naked) detour1() {
    __asm {
        test eax, eax
        jne original_code
        ret
    original_code:
        push ebx
        mov bl, 20
        mov cl, [eax]
        jmp dword ptr [detour1_retun]
    }

}
// clang-format on

struct Room {
	uint16_t id;
	const char* name;
};
static constexpr std::array<Room, 189> room_items = {
    Room { 0,    "Dante's Office" },
    Room { 1,    "Dante's Office: Front" },
    Room { 2,    "66 Slum Avenue" },
    Room { 3,    "Bullseye" },
    Room { 4,    "Love Planet" },
    Room { 5,    "13th Avenue" },
    Room { 6,    "Ice Guardian's Chamber" },
    Room { 7,    "Love Planet" },
    Room { 8,    "13th Avenue: Aftermath" },
    Room { 9,    "Dante's Office: Front" },
    Room { 10,   "Dante's Office: Front" },
    Room { 11,   "66 Slum Avenue" },
    Room { 12,   "Credits" },
    Room { 100,  "Chamber of Echoes" },
    Room { 101,  "Entranceway" },
    Room { 102,  "Living Statue Room" },
    Room { 103,  "Silence Statuary" },
    Room { 104,  "Chamber of Sins" },
    Room { 105,  "Cursed Skull Chamber" },
    Room { 106,  "Sun & Moon Chamber" },
    Room { 107,  "Ice Guardian's Chamber" },
    Room { 108,  "Entranceway" },
    Room { 109,  "Chamber of Sins" },
    Room { 110,  "Incandescent Space" },
    Room { 111,  "Giantwalker Chamber" },
    Room { 112,  "Endless Infernum" },
    Room { 113,  "Surge of Fortunas" },
    Room { 114,  "Heavenrise Chamber" },
    Room { 115,  "The Divine Library" },
    Room { 116,  "Incandescent Space" },
    Room { 117,  "Endless Infernum" },
    Room { 118,  "Surge of Fortunas" },
    Room { 119,  "High-fly Zone" },
    Room { 120,  "Azure Garden" },
    Room { 121,  "Firestorm Chamber" },
    Room { 122,  "Mute Goddess' Chamber" },
    Room { 123,  "Chamber of 3 Trials" },
    Room { 124,  "Trial of the Warrior" },
    Room { 125,  "Trial of Skill" },
    Room { 126,  "Trial of Wisdom" },
    Room { 127,  "The Dark Corridor" },
    Room { 128,  "God-cube Chamber" },
    Room { 129,  "Tri-sealed Antechamber" },
    Room { 130,  "Trial of the Warrior" },
    Room { 131,  "Trial of Skill" },
    Room { 132,  "Trial of Wisdom" },
    Room { 133,  "The Dark Corridor" },
    Room { 134,  "Pitch-black Void" },
    Room { 135,  "Skull Spire" },
    Room { 136,  "Tranquil Souls Room" },
    Room { 137,  "Lift Room" },
    Room { 138,  "Moonlight Mile" },
    Room { 139,  "Apparition Incarnate" },
    Room { 140,  "Pitch-black Void" },
    Room { 141,  "Skull Spire" },
    Room { 142,  "Peak of Darkness" },
    Room { 143,  "Dark-pact Chamber" },
    Room { 144,  "Peak of Darkness" },
    Room { 145,  "Astral Chamber" },
    Room { 146,  "Waking Sun Chamber" },
    Room { 200,  "Forbidden Land: Front" },
    Room { 201,  "The Rotating Bridge" },
    Room { 202,  "Provisions Storeroom" },
    Room { 203,  "Subterranean Garden" },
    Room { 204,  "Subground Water Vein" },
    Room { 205,  "Rounded Pathway" },
    Room { 206,  "Subterranean Lake" },
    Room { 207,  "Rounded Pathway" },
    Room { 208,  "Provisions Storeroom" },
    Room { 209,  "Limestone Cavern" },
    Room { 210,  "Sunken Opera House" },
    Room { 211,  "Marble Throughway" },
    Room { 212,  "Gears of Madness" },
    Room { 213,  "Altar of Evil Pathway" },
    Room { 214,  "Altar of Evil" },
    Room { 215,  "Debug Room" },
    Room { 216,  "Temperance Wagon" },
    Room { 217,  "Torture Chamber" },
    Room { 218,  "Spiral Corridor" },
    Room { 219,  "Devilsprout Lift" },
    Room { 220,  "Debug Room" },
    Room { 221,  "Subterran Garden" },
    Room { 222,  "Upper Subterran Garden" },
    Room { 223,  "Debug Room" },
    Room { 224,  "Rounded Pathway" },
    Room { 225,  "Subterran Lake" },
    Room { 226,  "Top Subterria Lack" },
    Room { 227,  "Rounded Pathway" },
    Room { 228,  "Underground Arena" },
    Room { 229,  "Effervescence Corridor" },
    Room { 230,  "Spiral Staircase" },
    Room { 231,  "Top Obsidian Path" },
    Room { 232,  "Obsidian Path" },
    Room { 233,  "Vestibule" },
    Room { 234,  "Lair of Judgement" },
    Room { 235,  "Underwater Elevator" },
    Room { 236,  "Hell's Highway" },
    Room { 237,  "Lair of Judgement Ruins" },
    Room { 238,  "Underground Arena" },
    Room { 239,  "Temperance Wagon" },
    Room { 240,  "Temperance Wagon" },
    Room { 241,  "Lux-luminous Corridor" },
    Room { 300,  "Leviathan's Stomach" },
    Room { 301,  "Leviathan's Intestines" },
    Room { 302,  "Leviathan's Heartcore" },
    Room { 303,  "Leviathan's Intestines" },
    Room { 304,  "Leviathan's Intestines" },
    Room { 305,  "Leviathan's Retina" },
    Room { 306,  "Leviathan's Intestines" },
    Room { 307,  "Leviathan's Intestines" },
    Room { 308,  "Leviathan's Stomach" },
    Room { 309,  "Leviathan's Intestines" },
    Room { 310,  "Leviathan's Heartcore (Debug Room)" },
    Room { 311,  "Leviathan's Intestines" },
    Room { 312,  "Leviathan's Intestines" },
    Room { 313,  "Leviathan's Retina" },
    Room { 400,  "Unsacred Hellgate" },
    Room { 401,  "Damned Chess Board" },
    Room { 402,  "Road to Despair" },
    Room { 403,  "Lost Souls Nirvana" },
    Room { 404,  "Infinity Nirvana" },
    Room { 405,  "Nirvana of Illusions" },
    Room { 406,  "Room of Fallen Ones" },
    Room { 407,  "Debug Room" },
    Room { 408,  "End of the Line" },
    Room { 409,  "Forbidden Nirvana" },
    Room { 410,  "No use" },
    Room { 411,  "Unsacred Hellgate" },
    Room { 412,  "Ice Guardian Reborn" },
    Room { 413,  "Giantwalker Reborn" },
    Room { 414,  "Firestorm Reborn" },
    Room { 415,  "Lightning Witch Reborn" },
    Room { 416,  "Lightbeast Reborn" },
    Room { 417,  "Timesteed Reborn" },
    Room { 418,  "Deathvoid Reborn" },
    Room { 419,  "Evil God-beast Reborn" },
    Room { 420,  "Demon Army Reborn" },
    Room { 421,  "Forbidden Nirvana" },
    Room { 422,  "Demon Clown Chamber" },
    Room { 423,  "Bloody Palace" },
    Room { 424,  "Bloody Palace" },
    Room { 425,  "Bloody Palace" },
    Room { 426,  "Bloody Palace" },
    Room { 427,  "Bloody Palace" },
    Room { 428,  "Bloody Palace" },
    Room { 429,  "Bloody Palace" },
    Room { 430,  "Bloody Palace" },
    Room { 431,  "Bloody Palace" },
    Room { 432,  "Bloody Palace" },
    Room { 433,  "Bloody Palace Boss" },
    Room { 434,  "Bloody Palace Boss" },
    Room { 435,  "Bloody Palace Boss" },
    Room { 436,  "Bloody Palace Boss" },
    Room { 437,  "Bloody Palace Boss" },
    Room { 438,  "Bloody Palace Boss" },
    Room { 439,  "Bloody Palace Boss" },
    Room { 440,  "Bloody Palace Boss" },
    Room { 441,  "Bloody Palace Boss" },
    Room { 442,  "Bloody Palace Boss" },
    Room { 443,  "Bloody Palace Boss" },
    Room { 444,  "Bloody Palace Boss" },
    Room { 445,  "Bloody Palace Boss" },
    Room { 446,  "Bloody Palace Boss" },
    Room { 447,  "Bloody Palace Boss" },
    Room { 448,  "Demon Clown Chamber" },
    Room { 449,  "Demon Clown Chamber" },
    Room { 600,  "Secret Mission" },
    Room { 601,  "Secret Mission" },
    Room { 602,  "Secret Mission" },
    Room { 603,  "Secret Mission" },
    Room { 604,  "Secret Mission" },
    Room { 605,  "Secret Mission" },
    Room { 606,  "Secret Mission" },
    Room { 607,  "Secret Mission" },
    Room { 608,  "Secret Mission" },
    Room { 609,  "Secret Mission" },
    Room { 610,  "Secret Mission" },
    Room { 611,  "Secret Mission" },
    Room { 900,  "Debug Room" },
    Room { 901,  "Debug Room" },
    Room { 902,  "Debug Room" },
    Room { 903,  "Debug Room" },
    Room { 904,  "Debug Room" },
    Room { 905,  "Debug Room" },
    Room { 906,  "Debug Room" },
    Room { 907,  "Debug Room" },
    Room { 908,  "Debug Room" },
    Room { 909,  "Debug Room" },
    Room { 910,  "Debug Room" }
};

std::optional<std::string> AreaJump::on_initialize() {

    if (!install_hook_absolute(0x005E10D9, m_function_hook, &detour, &detour_return, 6)) {
        return "Failed to install CSceneGameMain::CSceneGameMain() hook";
    }
#if 0
    if (!install_hook_absolute(0x00695A64, m_function_hook1, &detour1, &detour1_retun, 5)) {
        return "Failed to install CStage::something() hook";
    }
#endif

    return Mod::on_initialize();
}

void AreaJump::on_draw_ui() {
    if (!ImGui::CollapsingHeader(get_name().data())) { 
        return;
    }
    static char filter_inputbox[MAX_PATH] = {0};

    ImGui::InputText("Filter: ", filter_inputbox, MAX_PATH);

    int item_current_idx = 0;
    if (ImGui::BeginListBox("##Room Codes Listbox")) {
        for (size_t n = 0; n < room_items.size(); n++) {
            const bool is_selected = (item_current_idx == n);

            // sigh
            char buffer[MAX_PATH];
            int result = snprintf(buffer, sizeof(buffer), "%d - %s", room_items[n].id, room_items[n].name);
            IM_ASSERT(result > 0); // encoding error
            IM_ASSERT(result < MAX_PATH); // output was truncated or null terminator didnt fit in

            bool filtered = std::strstr(buffer, filter_inputbox) == 0;

            if(filtered) { continue; }

            if (ImGui::Selectable(buffer, is_selected)) {
                item_current_idx = n;
                devil3_sdk::area_jump(room_items[n].id);
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }
}