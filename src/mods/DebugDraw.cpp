#include "DebugDraw.hpp"
#define DEBUG_DRAW_IMPLEMENTATION
#define DEBUG_DRAW_EXPLICIT_CONTEXT
#include "debug_draw.hpp"
#include <DirectXMath.h>
#include <d3d9.h>
#include <wrl.h>
#include <D3dx9math.h>

#include <glm/glm.hpp> //basic vector/matrix math and defs
#include <glm/gtc/matrix_transform.hpp> //we will use this to make tranformation matrices

bool g_enabled = false;

using Microsoft::WRL::ComPtr;

std::optional<Vector2> world_to_screen(const Vector3f& world_pos) {
#if 0
	cCameraCtrl* camera = Devil3SDK::get_cam_ctrl();
	auto& transform = camera->transform;
	auto right = Vector3f{ transform[0][0], transform[0][1], transform[0][2] };
	auto up = Vector3f{ transform[1][0], transform[1][1], transform[1][2] };
	auto forward = Vector3f{ transform[2][0], transform[2][1], transform[2][2] };
	auto origin = Vector3f{ transform[3][0], transform[3][1], transform[3][2] };
	auto delta = world_pos - origin;

	Vector2f window = Devil3SDK::get_window_dimensions();

	float z = glm::dot(forward, delta);
	if (z <= 0.0f) {
		return std::nullopt;
	}

	// get the width height here
	float width, height;
	width = window.x;
	height = window.y;

	auto aspect_ratio = width / height;
	auto fov = 2 * atanf(tanf((camera->FOV * (-1.0f)) / 2.0f) * (height / width));
	auto y_scale = 1.0f / tanf(fov / 2.0f);
	auto x_scale = y_scale / aspect_ratio;
	auto x = glm::dot(delta, right) * (1.0f / z) * x_scale;
	auto y = glm::dot(delta, up) * (1.0f / z) * y_scale;

	return Vector2{ (int)((1.0f + x) * width * 0.5f), (int)((1.0f - y) * height * 0.5f) };
#else
	cCameraCtrl* camera = Devil3SDK::get_cam_ctrl();
	if (!camera || camera == (cCameraCtrl*)-1) { return Vector2{ 0.0f, 0.0f }; };

	Vector2f window = Devil3SDK::get_window_dimensions();
	
	float near_plane = 0.1f; //nearest distance from which you can see
	float far_plane  = 100.f; //you cant see more
	float aspect = window.x / window.y;

	//the perspective matrix
	glm::mat4 projection = glm::perspective(camera->FOV*-1.0f, aspect, near_plane, far_plane);

	glm::mat4 model = camera->transform;
	auto res = glm::project(world_pos, model, projection, Vector4{ 0.0f, 0.0f, window.x, window.y });
	return Vector2{ res.x, res.y };
#endif
}

class RenderInterfaceD3D9 final
	: public dd::RenderInterface
{
public:
	/*RenderInterfaceD3D9() {
		//init_shaders();
		//init_buffers();
	}*/

	void setCameraFrame(const Vector3 & up, const Vector3 & right, const Vector3 & origin)
	{
		camUp = up; camRight = right; camOrigin = origin;
	}

	void beginDraw() override
	{
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
		ImGui::SetNextWindowSize(ImVec2{1920,1080});
		auto imgui_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;
		ImGui::Begin("WEW", 0, imgui_flags);
		draw_list = ImGui::GetWindowDrawList();
	}

	void endDraw() override
	{
		ImGui::End();
		// No work done here at the moment.
	}

	void drawPointList(const dd::DrawVertex * points, int count, bool depthEnabled) override
	{
		(void)depthEnabled; // TODO: not implemented yet - not required by this sample

							// Emulating points as billboarded quads, so each point will use 6 vertexes.
							// D3D11 doesn't support "point sprites" like OpenGL (gl_PointSize).
		const int maxVerts = DEBUG_DRAW_VERTEX_BUFFER_SIZE / 6;

		const float D3DPointSpriteScalingFactor = 1.0f;

		assert(points != nullptr);
		assert(count > 0 && count <= maxVerts);

		// Map the vertex buffer:

		const int numVerts = count * 6;
		//const int indexes[6] = {0, 1, 2, 2, 3, 0};

		// Expand each point into a quad:
		for (int p = 0; p < count; ++p)
		{
			const float ptSize      = points[p].point.size * D3DPointSpriteScalingFactor;
			const Vector2 halfWidth = (ptSize * 0.5f) * Vector2f(1.0f, 0.0f); // X
			const Vector2 halfHeigh = (ptSize * 0.5f) * Vector2f(0.0f, -1.0f);//camUp;    // Y
			const Vector3 origin    = Vector3{ points[p].point.x, points[p].point.y, points[p].point.z };
			const Vector2 sorigin   = world_to_screen(origin).value();

			Vector2 corners[4];
			corners[0] = sorigin + halfWidth + halfHeigh;
			corners[1] = sorigin - halfWidth + halfHeigh;
			corners[2] = sorigin - halfWidth - halfHeigh;
			corners[3] = sorigin + halfWidth - halfHeigh;

			
			/*Vector2 corners_screen[4];
			for (int i = 0; i < 4; i++) {
				corners_screen[i] = world_to_screen(corners[i]).value();
			}*/
			Vector2 center = world_to_screen(origin).value();
			/*draw_list->AddNgonFilled(center, ptSize*100.0f,
				ImColor(
				(int)(points[p].point.r * 255.0f),
					(int)(points[p].point.g * 255.0f),
					(int)(points[p].point.b * 255.0f)), 4);*/
			draw_list->AddQuadFilled(corners[0], corners[1], corners[2], corners[3],
				ImColor((int)(points[p].point.r * 255.0f), (int)(points[p].point.g * 255.0f), (int)(points[p].point.b * 255.0f)));
		}
		//assert(v == numVerts);
	}

	void drawLineList(const dd::DrawVertex * lines, int count, bool depthEnabled) override
	{
		(void)depthEnabled; // TODO: not implemented yet - not required by this sample

		assert(lines != nullptr);
		assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

		for (int v = 1; v < count; ++v)
		{
			Vector3 pp1; 
			Vector3 pp2;
			Vector2 p1, p2;
			pp1 = *(Vector3*)&lines[v - 1];
			pp2 = *(Vector3*)&lines[v];
			p1 = world_to_screen(pp1).value();
			p2 = world_to_screen(pp2).value();

			/*auto in_screen = [](Vector2 a) {
				return ((a.x >= 0.0f) && (a.x <= 1920.0f) && (a.y >= 0.0f) && (a.y <= 1080.0f));
			};

			if (in_screen(p1) && in_screen(p2)) {
				continue;
			}*/
			//draw_list->AddLine(p1, p2, 0x34B1B0);
			draw_list->AddLine(p1, p2, ImColor((int)(lines[v].line.r * 255.0f), (int)(lines[v].line.g * 255.0f), (int)(lines[v].line.b * 255.0f)), 0.06f);
			//draw_list->PathLineTo(p2);
			/*verts[v].pos.x = lines[v].line.x;
			verts[v].pos.y = lines[v].line.y;
			verts[v].pos.z = lines[v].line.z;
			verts[v].pos.w = 1.0f;

			verts[v].color.x = lines[v].line.r;
			verts[v].color.y = lines[v].line.g;
			verts[v].color.z = lines[v].line.b;
			verts[v].color.w = 1.0f;*/
		}
		
		//draw_list->PathStroke(ImColor((int)(lines[0].line.r * 255.0f), (int)(lines[0].line.g * 255.0f), (int)(lines[0].line.b * 255.0f)));
		
	}

	struct ConstantBufferData
	{
		DirectX::XMMATRIX mvpMatrix        = DirectX::XMMatrixIdentity();
		//DirectX::XMFLOAT4 screenDimensions = {float(WindowWidth), float(WindowHeight), 0.0f, 0.0f};
	};

	struct Vertex
	{
		DirectX::XMFLOAT4A pos;   // 3D position
		DirectX::XMFLOAT4A uv;    // Texture coordinates
		DirectX::XMFLOAT4A color; // RGBA float
	};

	//
	// Members:
	//

	ImDrawList* draw_list = nullptr;

	ComPtr<IDirect3DDevice9>          p_device;

	// Camera vectors for the emulated point sprites
	Vector3                       camUp     = Vector3{0.0f};
	Vector3                       camRight  = Vector3{0.0f};
	Vector3                       camOrigin = Vector3{0.0f};

};

static RenderInterfaceD3D9* dd_render_iface = nullptr;
static dd::ContextHandle dd_context = nullptr;

#if 0
class collisioni
{
public:
	char pad_0000[16]; //0x0000
	float size; //0x0010
	Vector4 idk_vector0; //0x0014
	Vector4 idk_vector1; //0x0024
	Vector4 idk_vector2; //0x0034
	char pad_0044[8]; //0x0044
	Vector4 pos_maybe; //0x004C
}; //Size: 0x005C
static_assert(sizeof(collisioni) == 0x5C);
#endif

class colisioni
{
public:
	uint32_t flags_maybe; //0x0000
	uint32_t idk01; //0x0004
	uint32_t idk02; //0x0008
	uint32_t idk03; //0x000C
	float radius_maybe; //0x0010
	float idk_float; //0x0014
	char pad_0018[4]; //0x0018
	Matrix4x4 tranform01; //0x001C
	Matrix4x4 tranform02; //0x005C
	char pad_009C[32]; //0x009C
	Vector4 pos01; //0x00BC
	Vector4 pos02; //0x00CC
	Vector4 pos03; //0x00DC
	char pad_00EC[24]; //0x00EC
	Vector4 pos04; //0x0104
	float rad01; //0x0114
	char pad_0118[412]; //0x0118
}; //Size: 0x02B4
static_assert(sizeof(colisioni) == 0x2B4);


static void draw_sphere_maybe(colisioni* col) {
	if (!g_enabled) { return; }

	/*auto origin = Vector4f{ col->tranform01[3][0], col->tranform01[3][1], col->tranform01[3][2], 1.0f };
	auto pos = rot * origin;*/
	auto right = Vector3f{ col->tranform01[0][0], col->tranform01[0][1], col->tranform01[0][2] };
	auto up = Vector3f{ col->tranform01[1][0], col->tranform01[1][1], col->tranform01[1][2] };
	auto forward = Vector3f{ col->tranform01[2][0], col->tranform01[2][1], col->tranform01[2][2] };
#if 1
		dd::circle(dd_context, *(ddVec3*)&col->pos01, *(ddVec3*)&up, dd::colors::Coral, col->radius_maybe, 8);
		dd::circle(dd_context, *(ddVec3*)&col->pos01, *(ddVec3*)&right, dd::colors::Chartreuse, col->radius_maybe, 8);
		dd::circle(dd_context, *(ddVec3*)&col->pos01, *(ddVec3*)&forward, dd::colors::Crimson, col->radius_maybe, 8);

		dd::circle(dd_context, *(ddVec3*)&col->pos02, *(ddVec3*)&up, dd::colors::Coral, col->radius_maybe, 8);
		dd::circle(dd_context, *(ddVec3*)&col->pos02, *(ddVec3*)&right, dd::colors::Chartreuse, col->radius_maybe, 8);
		dd::circle(dd_context, *(ddVec3*)&col->pos02, *(ddVec3*)&forward, dd::colors::Crimson, col->radius_maybe, 8);

		dd::circle(dd_context, *(ddVec3*)&col->pos03, *(ddVec3*)&up, dd::colors::Coral, col->radius_maybe, 8);
		dd::circle(dd_context, *(ddVec3*)&col->pos03, *(ddVec3*)&right, dd::colors::Chartreuse, col->radius_maybe, 8);
		dd::circle(dd_context, *(ddVec3*)&col->pos03, *(ddVec3*)&forward, dd::colors::Crimson, col->radius_maybe, 8);
#else
	dd::sphere(dd_context, *(ddVec3*)&col->pos01, dd::colors::Coral, col->radius_maybe);
	dd::sphere(dd_context, *(ddVec3*)&col->pos02, dd::colors::Chartreuse, col->radius_maybe);
	dd::sphere(dd_context, *(ddVec3*)&col->pos03, dd::colors::Crimson, col->radius_maybe);
#endif
}

static uintptr_t g_detour_jmp = NULL;
// clang-format off
static __declspec(naked) void detour() {
	__asm {
		fstp dword ptr [esi+10h]
		pushad
		push esi
		call draw_sphere_maybe
		pop esi
		popad
		pop esi
		pop ebp

		jmp DWORD PTR [g_detour_jmp]
	}
}
// clang-format on

std::optional<std::string> DebugDraw::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  //if (!install_hook_offset(0xBADF00D, m_function_hook, &detour, &jmp_ret, 5)) {
  //  return a error string in case something goes wrong
  //  spdlog::error("[{}] failed to initialize", get_name());
  //  return "Failed to initialize DebugDraw";
  //}
	dd_render_iface = new RenderInterfaceD3D9;
	dd::initialize(&dd_context, dd_render_iface);
	if (!install_hook_absolute(0x006617B4, m_function_hook, &detour, &g_detour_jmp, 5)) {
		return "error";
	}
  return Mod::on_initialize();
}

void DebugDraw::custom_imgui_window()
{
	if (!g_enabled) { return; }
	CPlDante* pl = Devil3SDK::get_pl_dante();
	// keeping capcom traditions of typoing member fields like a motehfckure
	auto screen = world_to_screen(pl->Poistion);
	if (!screen.has_value()) { return; }
	//dd::xzSquareGrid(dd_context, -5000, 5000, pl->Poistion.y, 1.5f, dd::colors::Green);
	ddVec3 origin = { pl->Poistion.x, pl->Poistion.y, pl->Poistion.z };
	dd::sphere(dd_context, origin, dd::colors::Blue, 10.0f);
	//dd::point(dd_context, origin, dd::colors::Crimson, 15.0f);
	dd::flush(dd_context, ImGui::GetIO().DeltaTime);
}

// during load
//void DebugDraw::on_config_load(const utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_load(cfg);
//	}
//}
// during save
//void DebugDraw::on_config_save(utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_save(cfg);
//	}
//}
// do something every frame
//void DebugDraw::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void DebugDraw::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void DebugDraw::on_draw_ui() {
	ImGui::Checkbox("debug draw hitspheres", &g_enabled);
	/*ImGui::Checkbox("g_invert_forward", &g_invert_forward);
	ImGui::InputFloat("fov", &g_fov, 0.01f);*/
}
