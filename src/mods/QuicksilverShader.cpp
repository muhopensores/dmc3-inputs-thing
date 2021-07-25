#include "QuicksilverShader.hpp"
#include "d3dx9shader.h"

devil3StaticPxShaderArray* px_shader_array_static = nullptr;

std::optional<std::string> QuicksilverShader::on_initialize() {

	auto default_shader_program = m_shader_programs[shader_type::DEFAULT];
	
	memset(m_own_shader, 0, 1024);
	memcpy(m_own_shader, default_shader_program, strlen(default_shader_program));

	px_shader_array_static = (devil3StaticPxShaderArray*)0x0253007C;

    return Mod::on_initialize();
}

// TODO() save/load
// during load
void QuicksilverShader::on_config_load(const utility::Config &cfg) {
	m_current_type = cfg.get<int>("SlowmoShaderType").value_or(false);
	create_shader_program((shader_type)m_current_type);
}
// during save
void QuicksilverShader::on_config_save(utility::Config &cfg) {
	cfg.set<int>("SlowmoShaderType", m_current_type);
}
// do something every frame
static int counter = 0;
static int color_counter = 0;
static int constant = 0;
std::array<const char *, 7> colors = {
	"def c0,  1.000,  0.000,  0.000,  0.400\n",
	"def c0,  0.000,  1.000,  0.000,  0.400\n",
	"def c0,  0.000,  0.000,  1.000,  0.400\n",
	"def c0,  1.000,  0.500,  0.000,  0.400\n",
	"def c0,  1.000,  1.000,  0.000,  0.400\n",
	"def c0,  1.000,  0.000,  1.000,  0.400\n",
	"def c0,  0.000,  1.000,  1.000,  0.400\n",
};
void QuicksilverShader::on_frame() {
	if (m_current_type != shader_type::GREEN_SCREEN) { return; }
	if ((counter % 18) == 0) {
		static D3DDevicePtr* device_ptr = (D3DDevicePtr*)0x0252F374;
		static auto device = device_ptr->device;
		char shader[512];
		sprintf(shader, "ps_1_2\n\%s\n\mov r0, c0", colors[color_counter]);
		create_shader_program(shader);
		color_counter = (color_counter + 1) % 7;
	}
	counter++;
}
// will show up in debug window, dump ImGui widgets you want here
//void QuicksilverShader::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void QuicksilverShader::on_draw_ui() {
	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}
	if (ImGui::Combo("Quicksilver effect", &m_current_type, "Default\0Sepia\0Negative Mono\0Deep fried negative\0carameldansen\0Own\0\0")) {
		create_shader_program((shader_type)m_current_type);
	}
	ImGui::InputInt("Constatn:", &constant);
	if (m_current_type == shader_type::OWN) {
		ImGui::Text(m_error_string);
		ImGui::InputTextMultiline("Code your own shader! (God help you)", m_own_shader, 1024);
		if (ImGui::Button("REcompile shader")) {
			create_shader_program(shader_type::OWN);
		}
	}
}
void QuicksilverShader::create_shader_program(const char* shader_program) {
	IDirect3DPixelShader9* shader;
	HRESULT result;
	LPD3DXBUFFER codeBuffer;
	LPD3DXBUFFER errorBuffer;

	D3DDevicePtr* device_ptr = (D3DDevicePtr*)0x0252F374;
	auto device = device_ptr->device;

	result = D3DXAssembleShader( shader_program, strlen(shader_program), NULL, NULL, 0, &codeBuffer, &errorBuffer );
	if( errorBuffer != NULL )
	{
		const char* errorMessage = (const char*)errorBuffer->GetBufferPointer();
		sprintf(m_error_string, "Pixel shader failed to compile: %s", errorMessage);
		printf("%s\n", m_error_string);
		return;
	}
	else if( FAILED(result) )
	{
		sprintf(m_error_string, "Pixel shader failed to compile\n");
		printf(m_error_string);
		return;
	}

	result = device->CreatePixelShader( (DWORD*)codeBuffer->GetBufferPointer(), &shader );
	codeBuffer->Release();

	if( result != D3D_OK )
	{
		sprintf(m_error_string, "Failed to allocate pixel shader.\n");
		printf(m_error_string);
		return;
	}
	const char* name = px_shader_array_static->shaders[0]->name;
	px_shader_array_static->shaders[0]->pShader->Release();
	printf("overwriting shader named: %s\n", name);
	IDirect3DPixelShader9** pShaderLoc= &(px_shader_array_static->shaders[0]->pShader);
	InterlockedCompareExchange((LONG*)(pShaderLoc), (LONG)shader, (LONG)(px_shader_array_static->shaders[0]->pShader));
	//memcpy(px_shader_array_static->shaders[0]->pShader, shader, sizeof(void*));
	return;
}

void QuicksilverShader::create_shader_program(shader_type type) {

	const char* shader_program;
	if (type == shader_type::OWN) {
		shader_program = m_own_shader;
	}
	else {
		shader_program = m_shader_programs[type];
	}
	IDirect3DPixelShader9* shader;
	HRESULT result;
	LPD3DXBUFFER codeBuffer;
	LPD3DXBUFFER errorBuffer;

	D3DDevicePtr* device_ptr = (D3DDevicePtr*)0x0252F374;
	auto device = device_ptr->device;

	result = D3DXAssembleShader( shader_program, strlen(shader_program), NULL, NULL, 0, &codeBuffer, &errorBuffer );
	if( errorBuffer != NULL )
	{
		const char* errorMessage = (const char*)errorBuffer->GetBufferPointer();
		sprintf(m_error_string, "Pixel shader failed to compile: %s", errorMessage);
		printf("%s\n", m_error_string);
		return;
	}
	else if( FAILED(result) )
	{
		sprintf(m_error_string, "Pixel shader failed to compile\n");
		printf(m_error_string);
		return;
	}

	result = device->CreatePixelShader( (DWORD*)codeBuffer->GetBufferPointer(), &shader );
	codeBuffer->Release();

	if( result != D3D_OK )
	{
		sprintf(m_error_string, "Failed to allocate pixel shader.\n");
		printf(m_error_string);
		return;
	}
	const char* name = px_shader_array_static->shaders[0]->name;
	px_shader_array_static->shaders[0]->pShader->Release();
	printf("overwriting shader named: %s\n", name);
	IDirect3DPixelShader9** pShaderLoc= &(px_shader_array_static->shaders[0]->pShader);
	InterlockedCompareExchange((LONG*)(pShaderLoc), (LONG)shader, (LONG)(px_shader_array_static->shaders[0]->pShader));
	//memcpy(px_shader_array_static->shaders[0]->pShader, shader, sizeof(void*));
	return;
}
