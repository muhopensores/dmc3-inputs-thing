#include "RendererReplace.hpp"
//#include "WickedEngine.h"
#include "D3dx9math.h"
#include "meshoptimizer.h"
#include "numeric"
#include "utility/MemArena.hpp"
#include <set>

#include <DxErr.h>
#pragma comment(lib, "dxerr.lib")

// size literals
constexpr std::size_t operator""_KiB(unsigned long long int x) {
    return 1024ULL * x;
}

constexpr std::size_t operator""_MiB(unsigned long long int x) {
    return 1024_KiB * x;
}

constexpr std::size_t operator""_GiB(unsigned long long int x) {
    return 1024_MiB * x;
}

static RendererReplace* g_rreplace {nullptr};
#if 0
static wi::Application application;
static wi::graphics::CommandList g_cmdlist;
static wi::graphics::ColorSpace g_colorspace;
static wi::graphics::GraphicsDevice::GPUAllocation alloc;
static wi::graphics::Shader debug_vs;
static wi::graphics::Shader debug_ps;
static wi::graphics::InputLayout debug_input_layout;
static wi::graphics::PipelineState debug_pso;
#endif
static bool g_3d_draw_cmd {false};
// clang-format off
// only in clang/icl mode on x64, sorry
/*
static naked void detour() {
	__asm {
		mov qword ptr [RendererReplace::variable], rbx
		mov rax, 0xDEADBEEF
		jmp qword ptr [jmp_ret]
	}
}
*/
// clang-format on

#if 0
// create device objects
bool create_device_objects() {
    //constexpr auto vertex_format_dmc3_3d = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 | D3DFVF_TEX2;
    struct DMC3_VERTEX_VFV_LAYOUT {
        float pos[3];
        float norm[3];
        DWORD diffuse;
        DWORD specular;
        float uv0[2];
        float uv1[2];
        float uv2[2];
    };
    static constexpr auto bruh = sizeof(DMC3_VERTEX_VFV_LAYOUT);
    debug_input_layout.elements = 
    {
        { "POSITION",   0, wi::graphics::Format::R32G32B32_FLOAT, 0, offsetof(DMC3_VERTEX_VFV_LAYOUT, pos),      wi::graphics::InputClassification::PER_VERTEX_DATA},
        { "NORMAL",     0, wi::graphics::Format::R32G32B32_FLOAT, 0, offsetof(DMC3_VERTEX_VFV_LAYOUT, norm),     wi::graphics::InputClassification::PER_VERTEX_DATA},
        { "DIFF_COLOR", 0, wi::graphics::Format::R8G8B8A8_UINT,   0, offsetof(DMC3_VERTEX_VFV_LAYOUT, diffuse),  wi::graphics::InputClassification::PER_VERTEX_DATA},
        { "SPEC_COLOR", 0, wi::graphics::Format::R8G8B8A8_UINT,   0, offsetof(DMC3_VERTEX_VFV_LAYOUT, specular), wi::graphics::InputClassification::PER_VERTEX_DATA},
        { "TEXCOORD",   0, wi::graphics::Format::R32G32_FLOAT,    0, offsetof(DMC3_VERTEX_VFV_LAYOUT, uv0),      wi::graphics::InputClassification::PER_VERTEX_DATA},
        { "TEXCOORD",   1, wi::graphics::Format::R32G32_FLOAT,    0, offsetof(DMC3_VERTEX_VFV_LAYOUT, uv1),      wi::graphics::InputClassification::PER_VERTEX_DATA},
        { "TEXCOORD",   2, wi::graphics::Format::R32G32_FLOAT,    0, offsetof(DMC3_VERTEX_VFV_LAYOUT, uv2),      wi::graphics::InputClassification::PER_VERTEX_DATA},
    };

    wi::graphics::PipelineStateDesc desc;
    desc.vs = &debug_vs;
    desc.ps = &debug_ps;
    desc.il = &debug_input_layout;
    desc.dss = wi::renderer::GetDepthStencilState(wi::enums::DSSTYPE_DEPTHDISABLED);
    desc.rs = wi::renderer::GetRasterizerState(wi::enums::RSTYPE_FRONT);
    desc.bs = wi::renderer::GetBlendState(wi::enums::BSTYPE_OPAQUE);
    desc.pt = wi::graphics::PrimitiveTopology::TRIANGLESTRIP;
    

    return wi::graphics::GetDevice()->CreatePipelineState(&desc, &debug_pso);
}


// d3d draw primitive

static HRESULT __cdecl wi_draw_primitive(IDirect3DDevice9* This, UINT StartVertex, UINT PrimitiveCount) {
    if(!g_3d_draw_cmd) {
        return D3D_OK;
    }
    auto device = wi::graphics::GetDevice();

    /*wi::graphics::Viewport viewport;
    viewport.width = application.canvas.width;
    viewport.height = application.canvas.height;
    application.graphicsDevice->BindViewports(1, &viewport, g_cmdlist);*/

    application.graphicsDevice->BindPipelineState(&debug_pso, g_cmdlist);
    UINT primcount_adj = PrimitiveCount;
    device->Draw(primcount_adj, StartVertex, g_cmdlist);
    return D3D_OK;
}

static uintptr_t d3d_draw_primitive_jmp_back = 0x006E1C62;
static __declspec(naked) void d3d_draw_primitive() {
    __asm {
        call wi_draw_primitive
        jmp dword ptr[d3d_draw_primitive_jmp_back]
    }
}

#define BYTEn(x, n)   (*((BYTE*)&(x)+n))
// set world transform
static HRESULT WINAPI wi_set_transform_world(IDirect3DDevice9* d3d9d, D3DTRANSFORMSTATETYPE type, const D3DXMATRIX* p_matrix) {
    auto byte0 = !LOBYTE(p_matrix[3]._11);
    auto byte1 = BYTEn(p_matrix[3]._11, 1);
    auto byte2 = BYTEn(p_matrix[3]._11, 2);

    XMMATRIX modelMatrix;
    if(byte0) { modelMatrix = XMMATRIX((const float*)p_matrix); }
    else { modelMatrix = XMMatrixIdentity(); }

    XMMATRIX viewMatrix;
    if(byte1) { viewMatrix = XMMATRIX((const float*)(p_matrix + 1)); }
    else { viewMatrix = XMMatrixIdentity(); }

    XMMATRIX projectionMatrix;
    if(byte2){
        projectionMatrix = XMMATRIX((const float*)(p_matrix+2));}
    else {
        projectionMatrix = XMMatrixIdentity();
    }


    XMMATRIX mvp_matrix =  modelMatrix * viewMatrix * projectionMatrix;
    XMMATRIX tmvp_matrix = XMMatrixTranspose(mvp_matrix);
    /*D3DXMatrixMultiply(&mat_total, (p_matrix + 2), (p_matrix + 1));
    D3DXMatrixMultiply(&mat_total, &mat_total, p_matrix);*/

    auto device = wi::graphics::GetDevice();
    device->BindDynamicConstantBuffer(tmvp_matrix, 0, g_cmdlist);

    return D3D_OK;
}

static uintptr_t d3d_set_transform_world_jmp_back = 0x006DBCB6;
static __declspec(naked) void d3d_set_transform_world() {
    __asm {
        call wi_set_transform_world
        jmp dword ptr[d3d_set_transform_world_jmp_back]
    }
}

// unlock buffs
static HRESULT wi_unlock_buffer(d3d_vbuffer* vb) {
    auto result = vb->size;
    if (result) {
    const wi::graphics::GPUBuffer* vbs[] = {
        &alloc.buffer,
    };
    const uint32_t strides[] = {
        0x38,
    };
    const uint64_t offsets[] = {
        alloc.offset,
    };
    auto dev = wi::graphics::GetDevice();
    dev->BindVertexBuffers(vbs, 0 ,1, strides, offsets, g_cmdlist);
    auto size = vb->size;
    vb->size = 0;
    vb->unk = size;
    }
    g_3d_draw_cmd = true;
    return result;
}

static uintptr_t d3d_unlock_buffs_jmp_back = 0x006E1B5E;
static __declspec(naked) void d3d_unlock_buffs() {
    __asm {
        push ecx
        call wi_unlock_buffer
        pop ecx
        jmp dword ptr[d3d_unlock_buffs_jmp_back]
    }
}

// end scene
static HRESULT wi_end_scene() {
    application.Compose(g_cmdlist);
    application.graphicsDevice->RenderPassEnd(g_cmdlist);
    bool colorspace_conversion_required = g_colorspace == wi::graphics::ColorSpace::HDR10_ST2084;
    if (colorspace_conversion_required)
    {
        // In HDR10, we perform a final mapping from linear to HDR10, into the swapchain
        application.graphicsDevice->RenderPassBegin(&application.swapChain, g_cmdlist);
        wi::image::Params fx;
        fx.enableFullScreen();
        fx.enableHDR10OutputMapping();
        wi::image::Draw(&application.rendertarget, fx, g_cmdlist);
        application.graphicsDevice->RenderPassEnd(g_cmdlist);
    }
    wi::profiler::EndFrame(g_cmdlist);
    application.graphicsDevice->SubmitCommandLists();
    g_3d_draw_cmd = false;
    return D3D_OK;
}
static uintptr_t d3d_end_scene_jmp_back = 0x006DC489;
static __declspec(naked) void d3d_end_scene() {
    __asm {
        call wi_end_scene
        add esp, 4
        jmp dword ptr[d3d_end_scene_jmp_back]
    }
}
// begin scene
static HRESULT wi_begin_scene() {
    wi::font::UpdateAtlas(application.canvas.GetDPIScaling());

    g_colorspace = application.graphicsDevice->GetSwapChainColorSpace(&application.swapChain);
    if (!wi::initializer::IsInitializeFinished())
    {
        // Until engine is not loaded, present initialization screen...
        wi::graphics::CommandList cmd = application.graphicsDevice->BeginCommandList();
        application.graphicsDevice->RenderPassBegin(&application.swapChain, cmd);
        wi::graphics::Viewport viewport;
        viewport.width = (float)application.swapChain.desc.width;
        viewport.height = (float)application.swapChain.desc.height;
        application.graphicsDevice->BindViewports(1, &viewport, cmd);
        if (wi::initializer::IsInitializeFinished(wi::initializer::INITIALIZED_SYSTEM_FONT))
        {
            wi::backlog::DrawOutputText(application.canvas, cmd, g_colorspace);
        }
        application.graphicsDevice->RenderPassEnd(cmd);
        application.graphicsDevice->SubmitCommandLists();
        return D3D_OK;
    }
    wi::profiler::BeginFrame();
    application.deltaTime = float(std::max(0.0, application.timer.record_elapsed_seconds()));
    // Wake up the events that need to be executed on the main thread, in thread safe manner:
    wi::eventhandler::FireEvent(wi::eventhandler::EVENT_THREAD_SAFE_POINT, 0);
    application.Render(); 
    g_cmdlist = application.graphicsDevice->BeginCommandList();

    // Begin final compositing:
    wi::image::SetCanvas(application.canvas);
    wi::font::SetCanvas(application.canvas);
    wi::graphics::Viewport viewport;
    viewport.width = (float)application.swapChain.desc.width;
    viewport.height = (float)application.swapChain.desc.height;
    application.graphicsDevice->BindViewports(1, &viewport, g_cmdlist);

    bool colorspace_conversion_required = g_colorspace == wi::graphics::ColorSpace::HDR10_ST2084;
    if (colorspace_conversion_required)
    {
        // In HDR10, we perform the compositing in a custom linear color space render target
        if (!application.rendertarget.IsValid())
        {
            wi::graphics::TextureDesc desc;
            desc.width = application.swapChain.desc.width;
            desc.height = application.swapChain.desc.height;
            desc.format = wi::graphics::Format::R11G11B10_FLOAT;
            desc.bind_flags = wi::graphics::BindFlag::RENDER_TARGET | wi::graphics::BindFlag::SHADER_RESOURCE;
            bool success = application.graphicsDevice->CreateTexture(&desc, nullptr, &application.rendertarget);
            assert(success);
            application.graphicsDevice->SetName(&application.rendertarget, "Application::rendertarget");
        }
        wi::graphics::RenderPassImage rp[] = {
            wi::graphics::RenderPassImage::RenderTarget(&application.rendertarget,  wi::graphics::RenderPassImage::LoadOp::CLEAR),
        };
        application.graphicsDevice->RenderPassBegin(rp, arraysize(rp), g_cmdlist);
    }
    else
    {
        // If swapchain is SRGB or Linear HDR, it can be used for blending
        //	- If it is SRGB, the render path will ensure tonemapping to SDR
        //	- If it is Linear HDR, we can blend trivially in linear space
        application.rendertarget = {};
        application.graphicsDevice->RenderPassBegin(&application.swapChain, g_cmdlist);
    }

    return D3D_OK;
}

static uintptr_t d3d_begin_scene_jmp_back = 0x006DC3D3;
static DWORD rets = 0;
static __declspec(naked) void d3d_begin_scene() {
    
    __asm {
        call wi_begin_scene
        add esp, 4
        jmp dword ptr [d3d_begin_scene_jmp_back]
    }
}

// vb update
static void* WINAPI wi_alloc_gpu(d3d_vbuffer* vb, DWORD size) {
    wi::graphics::GraphicsDevice* dev = wi::graphics::GetDevice();

    alloc = dev->AllocateGPU(size, g_cmdlist);
    
    vb->size = size;
    vb->length = size;
    vb->unk = 0;
    return alloc.data;
}

static uintptr_t d3d_create_vb_and_lock_jmp_back = 0x006E1AE4;
// clang-format off
static __declspec(naked) void d3d_create_vb_and_lock() {
    __asm {
        push ecx
        call wi_alloc_gpu
        jmp dword ptr [d3d_create_vb_and_lock_jmp_back]
    }
}
// clang-format on
#endif
// Created with ReClass.NET 1.2 by KN4CK3R

class DrawCommandMaybe
{
public:
    class DrawCommandMaybe* m_ptr_next_draw_cmd; //0x0000
    class MeshDrawDataMaybe* m_ptr_mesh_draw_data; //0x0004
    uint32_t unk; //0x0008
    class SomeOtherTextureData* m_ptr_other_texture_stuff; //0x000C
    class ZAndScrBlend* m_ptr_z_and_blendstate; //0x0010
    class MvpMatrix* m_ptr_mvp_matrix; //0x0014
    void* m_unk_ptr; //0x0018
    class TextureData* m_ptr_texture_data; //0x001C
    uint32_t unk_uint32; //0x0020
    uint32_t unk_uint32_0; //0x0024
    char pad_0028[32]; //0x0028
}; //Size: 0x0048

class MeshDrawDataMaybe
{
public:
    class MeshDrawDataMaybe* m_next_mesh_draw_data; //0x0000
    uint32_t start_vertex; //0x0004
    uint32_t prim_count; //0x0008
}; //Size: 0x000C

class MvpMatrix
{
public:
    Matrix4x4 model; //0x0000
    Matrix4x4 view; //0x0040
    Matrix4x4 proj; //0x0080
}; //Size: 0x00C0

class ZAndScrBlend
{
public:
    bool unk_bool_0; //0x0000
    bool unk_bool_1; //0x0001
    bool unk_bool_2; //0x0002
    bool unk_bool_3; //0x0003
    char pad_0004[64]; //0x0004
}; //Size: 0x0044

#if 0
class TextureData
{
public:
    char pad_0000[4]; //0x0000
}; //Size: 0x0004
#endif

class SomeOtherTextureData
{
public:
    char pad_0000[4]; //0x0000
}; //Size: 0x0004
static DWORD g_vertices{0};
struct D3DDevicePtr {
    IDirect3DDevice9* device;
};

#if 0 
DWORD d3d_create_vb_lock_and_unlock() {

  DWORD ptr_numverts = *(DWORD*)0x0252FFA4;
  DWORD size = 0x38 * ptr_numverts;                   // world
  DWORD someptr = *(DWORD*)0x0252FFAC;
  static constexpr uintptr_t d3d_vbuffer_struct_static = 0x0252FFA8;
  static constexpr uintptr_t d3d_vbuffer_create_and_lock_fptr = 0x006E1260;
  
  if ( someptr < size )
  {
    // create vb and lock
    void* pdata {nullptr};
    {
        __asm {
            mov esi, dword ptr [size]
            push esi
            mov ecx, dword ptr [d3d_vbuffer_struct_static]
            call d3d_vbuffer_create_and_lock_fptr
            mov dword ptr [pdata], eax
        }
        assert(pdata != NULL);

    }
    //pDataVb = d3d_create_and_lock_vb_sub_6E1260((d3d_vbuffer *)&dword_252FFA8, 0x38 * dword_252FFA4);
    v16 = (Vector4 *)pDataVb;
    LOBYTE(pDataVb) = pDataVb != 0;
    assert_sub_6DC540((const CHAR *)pDataVb);
    if ( !v16 )
    {
      v17 = 0x80004005;
      goto LABEL_40;
    }
    v18 = dword_252FF90;
    if ( v14 > 0xE000 )
    {
      v19 = (v14 - 1) / 0xE000;
      do
      {
        vector4_copy_sub_6E1BC0(v16, (Vector4 *)v18, 0xE000u);
        v18 = *(_DWORD *)(v18 + 0xE000);
        v16 += 0xE00;
        v14 -= 0xE000;
        --v19;
      }
      while ( v19 );
    }
    vector4_copy_sub_6E1BC0(v16, (Vector4 *)v18, v14);
    d3d_unlock_sub_6E12E0((d3d_vbuffer *)&dword_252FFA8);
  }
  v17 = 0;
LABEL_40:
  D3D_reportErrorMaybe_sub_6DC510();
  if ( v17 >= 0 )
  {
    if ( (DWORD *)dword_252FFEC == &dword_252FFA8 )
    {
      result = 0;
      dword_252FFF0 = 1;
      return result;
    }
    result = d3d_set_stream_source_and_FVF_sub_6E1310((d3d_vbuffer *)&dword_252FFA8, 0x38u);
  }
  else
  {
    result = v17;
  }
LABEL_45:
  if ( result < 0 )
    goto LABEL_5;
  dword_252FFF0 = a1;
  return result;
}
#endif

struct TempDIPThings {
    INT base_vertex_index { 0 };
    UINT min_vertex_index { 0 };
    UINT num_vertices { 30438 };
    UINT start_index { 3321 };
    UINT prim_count { 24 };
};

static TempDIPThings g_temp_dip_things{};
std::unordered_map<int, int> g_index_lookup_table;

static void d3d_draw_prim_wrapper(std::vector<std::pair<DWORD, DWORD>>& batches) {
    static D3DDevicePtr* device_ptr  = (D3DDevicePtr*)0x0252F374;
    static IDirect3DDevice9* pdevice = device_ptr->device;
    d3d_vbuffer* static_vbuffer      = (d3d_vbuffer*)0x0252FFA8;

    //std::reverse(batches.begin(), batches.end());
    DWORD primitve_count = std::accumulate(batches.begin(), batches.end(), 0, [](DWORD acc, const std::pair<DWORD, DWORD>& pair) {
        return acc + pair.second;
    });
    UINT numvertices = batches.back().first - batches.front().first;
    if (static_vbuffer->pindexbuffer) {
        pdevice->SetIndices(static_vbuffer->pindexbuffer);
    }
    UINT index_count = 0;
    for (const auto& batch: batches) {
    //HRESULT result = pdevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, batch.first, batch.second);
    HRESULT result = pdevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, batch.first, 0, batch.second, 0, batch.second);
    assert(result == S_OK);
    //printf("hehe\n");
    }
    //HRESULT result = pdevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, batches.front().first, primitve_count);
}
void d3d_emplace_back_triangles(std::vector<std::pair<DWORD, DWORD>>& batches, UINT vertexCount, UINT primitve_count, char a3) {
    typedef int (*d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t)(int a1);
    d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t  d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0 = (d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t)0x006E1C00;
    DWORD benis = *(DWORD*)0x0252FFF0;

    if((vertexCount & 3) == benis || 
        d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0(vertexCount & 3) >= 0
    ) {
        DWORD start_vertex = vertexCount >> 2;
        if(a3) {
            auto v5 = 2;
            if( start_vertex < v5) {
                return;
            }
            start_vertex -= v5;
            primitve_count += v5;
        }
        batches.emplace_back(start_vertex, primitve_count - 2);
        //pdevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, start_vertex, primitve_count - 2);
    }
#if 0
    typedef int (*d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t)(int a1);
    d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t  d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0 = (d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t)0x006E1C00;
    DWORD benis = *(DWORD*)0x0252FFF0;
    static D3DDevicePtr* device_ptr = (D3DDevicePtr*)0x0252F374;
    static IDirect3DDevice9* pdevice = device_ptr->device;
    d3d_vbuffer* static_vbuffer = (d3d_vbuffer*)0x0252FFA8;

    std::vector<std::pair<DWORD,DWORD>> batcherinos;
    for (auto& batch: batches) {
        if(d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0(batch.first & 3) >= 0) {
            if (a3) {
                batch.first -= 2;
                batch.second += 2;
            }
            batch.first = batch.first >> 2;
            batcherinos.push_back(batch);
        }
        
    }
    if(batcherinos.empty()) { return;}
        DWORD primitve_count = std::accumulate(batcherinos.begin(), batcherinos.end(),  0, [](DWORD acc, const std::pair<DWORD, DWORD>& pair) {
        return acc + pair.second;
    });
        DWORD a1 = batcherinos.begin()->first;
        DWORD a2 = batcherinos.back().first;
        size_t index_count = a1 - a2;
        size_t unindexed_vertex_count = index_count;
        std::reverse(batcherinos.begin(), batcherinos.end());
        #if 0
        std::vector<unsigned int> remap(index_count);
        std::vector<unsigned int> indices(index_count);
        std::iota(indices.begin(), indices.end(), batches.back().first);
        std::reverse(indices.begin(), indices.end());

        size_t vertex_count = meshopt_generateVertexRemap(&remap[0],NULL, index_count, &indices[0], unindexed_vertex_count, sizeof(DWORD));
        meshopt_remapIndexBuffer(&indices[0], NULL, index_count, &remap[0]);
        pdevice->CreateIndexBuffer(index_count * sizeof(DWORD), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &indexBuffer, NULL);
        DWORD* d3dindices{};
        indexBuffer->Lock(0, 0, (void**)&d3dindices, 0);

        memcpy(d3dindices, indices.data(), sizeof(DWORD) * index_count);

        indexBuffer->Unlock();
        #endif
        if(static_vbuffer->pindexbuffer) {
            pdevice->SetIndices(static_vbuffer->pindexbuffer);
        }

        printf("\tpdevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, %d, %d);\n", batcherinos[0].first, primitve_count);
        //pdevice->DrawPrimitive(D3DPT_TRIANGLELIST, batches[0].first, primitve_count);
        auto result = pdevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, batcherinos.front().first, 0, index_count, 1, primitve_count);
        assert(result == S_OK);
        //indexBuffer->Release();
#endif
}

void d3d_draw_primitive_maybe_sub_6E1C00(DWORD vertexCount, DWORD primitve_count, char a3) {
    typedef int (*d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t)(int a1);
    d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t  d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0 = (d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t)0x006E1C00;
    DWORD benis = *(DWORD*)0x0252FFF0;
    static D3DDevicePtr* device_ptr = (D3DDevicePtr*)0x0252F374;
    static IDirect3DDevice9* pdevice = device_ptr->device;

    if((vertexCount & 3) == benis || 
        d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0(vertexCount & 3) >= 0
    ) {
        DWORD start_vertex = vertexCount >> 2;
        if(a3) {
            auto v5 = 2;
            if( start_vertex < v5) {
                return;
            }
            start_vertex -= v5;
            primitve_count += v5;
        }
        printf("\tpdevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, %d, %d);\n", start_vertex, primitve_count - 2);
        pdevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, start_vertex, primitve_count - 2);
    }
}

static HRESULT __stdcall d3d_dispatch_drawcalls(MeshDrawDataMaybe* mdd, char a3) {

    DWORD start_vertex { mdd->start_vertex};
    DWORD prim_count {mdd->prim_count};
    MeshDrawDataMaybe* m_next_draw_data = mdd->m_next_mesh_draw_data;
    static constexpr void* d3d_draw_prim_func_address = &d3d_draw_primitive_maybe_sub_6E1C00;


    std::vector<std::pair<DWORD, DWORD>> batches;
#if 1
    do {
        if((start_vertex & 3) != 1) {
            d3d_draw_primitive_maybe_sub_6E1C00(start_vertex, prim_count, a3);
            batches.clear();
        }
        else {
            d3d_emplace_back_triangles(batches, start_vertex, prim_count, a3);
        }
        if(m_next_draw_data) {
        start_vertex = m_next_draw_data->start_vertex;
        prim_count = m_next_draw_data->prim_count;
        m_next_draw_data = m_next_draw_data->m_next_mesh_draw_data;
        }
    } while(m_next_draw_data != NULL);
    if(batches.size() > 0) {
        d3d_draw_prim_wrapper(batches);
    }
#else
        do {
        d3d_draw_primitive_maybe_sub_6E1C00(start_vertex, prim_count, a3);
        if(m_next_draw_data) {
        start_vertex = m_next_draw_data->start_vertex;
        prim_count = m_next_draw_data->prim_count;
        m_next_draw_data = m_next_draw_data->m_next_mesh_draw_data;
        }
    } while(m_next_draw_data != NULL);
#endif
    #if 0
    while (m_next_draw_data != NULL) {
        start_vertex = m_next_draw_data->start_vertex;
        prim_count = m_next_draw_data->prim_count;
        m_next_draw_data = m_next_draw_data->m_next_mesh_draw_data;
    }
    #endif
    return D3D_OK;
#if 0
    DWORD start_vertex { mdd->start_vertex };
    DWORD prim_count { mdd->prim_count };
    MeshDrawDataMaybe* m_next_draw_data = mdd->m_next_mesh_draw_data;
    static constexpr uintptr_t d3d_draw_prim_func_address = 0x06E1C00;
    size_t counter = 0;
    HRESULT retval = D3D_OK;
    //static constexpr size_t batchsize = 0x38;
    DWORD* benis = (DWORD*)0x0252FFF0;
    DWORD type = start_vertex & 3;
    if (type == 1 && *benis == 1) {
        __debugbreak;
    }
    do {
        //if (counter % batchsize == 0) {
        printf("ass; benis=%d, type=%d, start_vertex=%d\n", *benis, type, start_vertex >> 2);
        typedef int (*d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t)(int a1);
        d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t  d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0 = (d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t)0x006E1C00;
        if (type == *benis || d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0(type) >= 0) {
            DWORD vertex_count = *(DWORD*)0x0252FFA4;
            retval = pdevice->DrawPrimitive(D3DPT_TRIANGLELIST, start_vertex>>2, prim_count-1);
        }
        //return retval;
        #if 0
        __asm {
            mov edx, dword ptr[a3]
            mov eax, dword ptr[prim_count]
            mov ecx, dword ptr[start_vertex]
            push edx
            push eax
            push ecx
            mov eax, dword ptr[d3d_draw_prim_func_address]
            call eax
            add esp, 0Ch
        }
        #endif
        //}
        if (m_next_draw_data) {
            m_next_draw_data = m_next_draw_data->m_next_mesh_draw_data;
            if (m_next_draw_data) {
                //if(counter % batchsize == 0) {
                //start_vertex = m_next_draw_data->start_vertex;
                prim_count += m_next_draw_data->prim_count;
                    //prim_count = 0;
                //}
                //prim_count += m_next_draw_data->prim_count;
            }
        }
        prev_type = type;
    }
    while(m_next_draw_data);
    if (type == 1) {
        device_ptr->device->SetIndices(indexBuffer);
        return pdevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, g_vertices, 0, prim_count/3);
    }
    return D3D_OK;
    //DWORD* benis = (DWORD*)0x0252FFF0;
    //DWORD type = start_vertex & 3;

    //return device_ptr->device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, g_vertices, 0, prim_count);
    #if 0
    
    #endif

    DWORD iterator = 0;
    while (m_next_draw_data)
    {
        iterator++;
        prim_count += m_next_draw_data->prim_count;
    }
    printf("drawcal iterator: %d \n", iterator);
    return D3D_OK;
    #endif
}

static uintptr_t d3d_dispatch_drawcall_jmp_back = 0x006DF677;
// clang-format off
static __declspec(naked) void d3d_dispatch_drawcall() {
    __asm {
        pushad
        mov dl, [edi+20h]
        push edx
        push esi
        call d3d_dispatch_drawcalls
        popad
        jmp dword ptr[d3d_dispatch_drawcall_jmp_back]
    }
}
// clang-format on

class VertexDef
{
public:
    Vector3 pos; //0x0000
    Vector3 norm; //0x000C
    uint8_t diff_r; //0x0018
    uint8_t diff_g; //0x0019
    uint8_t diff_b; //0x001A
    uint8_t diff_a; //0x001B
    uint8_t spec_r; //0x001C
    uint8_t spec_g; //0x001D
    uint8_t spec_b; //0x001E
    uint8_t spec_a; //0x001F
    Vector2 uv0; //0x0020
    Vector2 uv1; //0x0028
    Vector2 uv2; //0x0030
}; //Size: 0x0038

class VertexBuf
{
public:
    class VertexDef vdef[]; //0x0000
}; //Size: 0x2530

typedef std::pair<VertexDef, int> VPair;

struct CmpClass {
    static constexpr float eps_ = 0.001f;
    bool operator() (const VPair& p1, const VPair& p2) const {
        if(glm::abs(p1.first.pos.x - p2.first.pos.x) > eps_) { return p1.first.pos.x < p2.first.pos.x; }
        if(glm::abs(p1.first.pos.y - p2.first.pos.y) > eps_) { return p1.first.pos.y < p2.first.pos.y; }
        if(glm::abs(p1.first.pos.z - p2.first.pos.z) > eps_) { return p1.first.pos.z < p2.first.pos.z; }

        return false;
    }
};

static void __stdcall d3d_release_ib_hack(d3d_vbuffer* buf) {
    if(buf->pindexbuffer) {
        buf->pindexbuffer->Release();
    }
    buf->pindexbuffer = 0;
    g_index_lookup_table.clear();
}

static constexpr uintptr_t d3d_release_ib_jmp_back = 0x006E1255;
static void __declspec(naked) d3d_release_ib() {
    __asm {
        pushad
        push esi
        call d3d_release_ib_hack
        popad
        mov [esi+8], edi
        mov [esi+0Ch], edi
        jmp DWORD PTR [d3d_release_ib_jmp_back]
    }
}

struct Edge {
    int vertex1;
    int vertex2;
};

static void __stdcall d3d_before_unlock(VertexBuf* vertex_attrib_buffer) {
    DWORD total_idk= *(DWORD*)0x0252FFA4;
    if((total_idk % 0x38) == 0) {
        __debugbreak();
    }
    DWORD total_indices = (total_idk / 0x38);
    static D3DDevicePtr* device_ptr = (D3DDevicePtr*)0x0252F374;
    d3d_vbuffer* static_vbuffer = (d3d_vbuffer*)0x0252FFA8;
    
    std::set<VPair, CmpClass> vertices;
    std::vector<int> indices;
    std::vector<VertexDef> vbuffer;
    int index = 0;

    for (int i = 0; i < total_indices; i = i+3) {
        // compute the triangle's facing direction
        VertexDef& v1 = vertex_attrib_buffer->vdef[i];
        VertexDef& v2 = vertex_attrib_buffer->vdef[i+1];
        VertexDef& v3 = vertex_attrib_buffer->vdef[i+2];

        // get the edges for the basis
        Vector3 fe1 = v3.pos - v1.pos;
        Vector3 fe2 = v2.pos - v1.pos;
        fe1 = glm::normalize(fe1);
        fe2 = glm::normalize(fe2);

        // calculate the face normal
        Vector3 z = glm::cross(fe1, fe2);
        z = glm::normalize(z);

        // add the imported vertex normals together to get the face normal
        Vector3 n1 = v1.norm;
        Vector3 n2 = v2.norm;
        Vector3 n3 = v3.norm;

        Vector3 normal = n1 + n2 + n3;
        normal = glm::normalize(normal);

        // check whether the triangle is facing in the imported normals direction and flip it otherwise
        bool wnd = glm::dot(normal, z) > 0.0f;

        if (wnd) {
            vbuffer.push_back(v1);
            vbuffer.push_back(v3);
            vbuffer.push_back(v2);
        }
        else {
            vbuffer.push_back(v1);
            vbuffer.push_back(v2);
            vbuffer.push_back(v3);
        }
        indices.push_back(index++);
    }
    
#if 0
    for (int i = 0; i < total_indices; i++) {
        std::set<VPair>::iterator it = vertices.find(std::make_pair(vertex_attrib_buffer->vdef[i], 0/*this value doesn't matter*/));
        g_index_lookup_table[i] = index;
        if(it != vertices.end()) {
            indices.push_back(it->second);
        }
        else {
            vertices.insert(std::make_pair(vertex_attrib_buffer->vdef[i], index));
            indices.push_back(index++);
        }
        
    }
#endif
    // NOTE(): vertices in the set are not sorted by the index
    // so we'll have to rearange them like this:
    //vbuffer.resize(vertices.size());
    //for (const auto& vert: vertices) {
    //    vbuffer[vert.second] = vert.first;
    //}
    
    memcpy(vertex_attrib_buffer, vbuffer.data(), vbuffer.size() * sizeof(VertexDef));
    device_ptr->device->CreateIndexBuffer(indices.size() * sizeof(DWORD), D3DUSAGE_DONOTCLIP, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &static_vbuffer->pindexbuffer, NULL);
    DWORD* d3dindices{};
    static_vbuffer->pindexbuffer->Lock(0, 0, (void**)&d3dindices, 0);

    memcpy(d3dindices, indices.data(), sizeof(DWORD) * indices.size());
    static_vbuffer->pindexbuffer->Unlock();

    #if 0
    meshopt_Stream streams[] = {
        {&vertex_attrib_buffer->vdef->pos,  sizeof(float) * 3, 0x38}, // pos
        {&vertex_attrib_buffer->vdef->norm, sizeof(float) * 3, 0x38}, // normals
        {&vertex_attrib_buffer->vdef->uv0,  sizeof(float) * 2, 0x38}, // uvs
    };
    std::vector<unsigned int> remap(total_indices);
    size_t total_vertices = meshopt_generateVertexRemapMulti(&remap[0], NULL, total_indices, total_indices, streams, sizeof(streams) / sizeof(streams[0]));
    g_vertices = total_vertices;
    std::vector<unsigned int> indices(total_indices);
    meshopt_remapIndexBuffer(&indices[0], NULL, total_indices, &remap[0]);

    #if 1
    device_ptr->device->CreateIndexBuffer(total_indices * sizeof(DWORD), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &indexBuffer, NULL);
    DWORD* d3dindices{};
    indexBuffer->Lock(0, 0, (void**)&d3dindices, 0);

    memcpy(d3dindices, indices.data(), sizeof(DWORD) * total_vertices);

    indexBuffer->Unlock();
    //device_ptr->device->SetIndices(indexBuffer);
    #endif
    #endif
}

static uintptr_t d3d_before_unlock_jmp_back = 0x006E1B5E;
static uintptr_t d3d_unlock_sub_6E12E0 = 0x006E12E0;
// clang-format off
static __declspec(naked) void d3d_before_unlock_naked() {
    __asm {
        push ebx
        call d3d_before_unlock
        mov ecx, 0252FFA8h
        call d3d_unlock_sub_6E12E0
        jmp dword ptr[d3d_before_unlock_jmp_back]
    }
}
// clang-format on

#if 0
#pragma region reclass
class MeshDataSorting {
public:
    uint32_t idk0;                            // 0x0000
    uint32_t num_elements;                    // 0x0004
    uint32_t something;                       // 0x0008
    class SomeMeshDataSorting* ptr_mesh_data; // 0x000C
    char pad_0010[48];                        // 0x0010
};                                            // Size: 0x0040

class SomeMeshDataSorting {
public:
    uint16_t number_of_elements;                             // 0x0000
    uint16_t idk;                                            // 0x0002
    char pad_0004[8];                                        // 0x0004
    Vector3 (**ptr_array_of_verts);                      // 0x000C
    Vector3 (**ptr_array_of_normals);                    // 0x0010
    uint32_t (**ptr_array_of_uvs);                        // 0x0014
    char pad_0018[8];                                        // 0x0018
    class MeshPackedColors (**ptr_array_of_spec_colors); // 0x0020
    char pad_0024[12];                                       // 0x0024
};                                                           // Size: 0x0030

class MeshPackedColors {
public:
    uint8_t N000000AB; // 0x0000
    uint8_t N000000AD; // 0x0001
    uint8_t N000000B0; // 0x0002
    uint8_t N000000AE; // 0x0003
};                     // Size: 0x0004

class RDrawSomethingStruct {
public:
    uint8_t col_r0;                         // 0x0000
    uint8_t col_g0;                         // 0x0001
    uint8_t col_b0;                         // 0x0002
    uint8_t col_a0;                         // 0x0003
    uint8_t col_r1;                         // 0x0004
    uint8_t col_g1;                         // 0x0005
    uint8_t col_b1;                         // 0x0006
    uint8_t col_a1;                         // 0x0007
    Matrix4x4* p_some_matrix;               // 0x0008
    class MeshDataSorting* MeshDataSorting; // 0x000C
    uint32_t some_flags;                    // 0x0010
};                                          // Size: 0x0014

#pragma endregion

static void __stdcall optimize_mesh_data(RDrawSomethingStruct* rd) {

    uint32_t index_count     = rd->MeshDataSorting->ptr_mesh_data->number_of_elements;
    if (index_count % 3 != 0) {
        return;
    }
    meshopt_Stream streams[] = {
        {&rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_verts[0], sizeof(float) * 3, sizeof(float) * 3},
        {&rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_normals[0], sizeof(float) * 3, sizeof(float) * 3},
        {&rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_uvs[0], sizeof(uint32_t), sizeof(uint32_t)},
        {&rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_spec_colors[0], sizeof(uint32_t), sizeof(uint32_t)},
    };

    std::vector<unsigned int> remap(index_count);
    size_t vertex_count = meshopt_generateVertexRemapMulti(&remap[0], NULL, index_count, 
        index_count, streams, sizeof(streams) / sizeof(streams[0]));

    std::vector<Vector3> vertices(index_count);
    meshopt_remapVertexBuffer(vertices.data(), &rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_verts[0],
        index_count, sizeof(Vector3f), &remap[0]);
    
    std::vector<unsigned int> index_buffer(index_count);
    meshopt_remapIndexBuffer(index_buffer.data(), NULL, index_count, &remap[0]);

    std::vector<Vector3> normals(index_count);
    meshopt_remapVertexBuffer(normals.data(), &rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_normals[0],
        index_count, sizeof(Vector3f), &remap[0]);

    std::vector<uint32_t> uvs(index_count);
    meshopt_remapVertexBuffer(uvs.data(), &rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_uvs[0],
        index_count, sizeof(uint32_t), &remap[0]);

    std::vector<uint32_t> specs(index_count);
    meshopt_remapVertexBuffer(specs.data(), &rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_spec_colors[0],
        index_count, sizeof(uint32_t), &remap[0]);
    return;
    memcpy(&rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_verts[0],   vertices.data(),  sizeof(Vector3f) * index_count);
    memcpy(&rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_normals[0], normals.data(),   sizeof(Vector3f) * index_count);
    memcpy(&rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_uvs[0],     uvs.data(),       sizeof(uint32_t) * index_count);
    memcpy(&rd->MeshDataSorting->ptr_mesh_data->ptr_array_of_spec_colors[0], specs.data(), sizeof(uint32_t) * index_count);

    //__debugbreak();

}



static uintptr_t r_prep_mesh_datas_jmp_back = 0x006DE49F;
// clang-format off
static __declspec(naked) void r_prep_mesh_datas() {
    __asm {
            mov     ecx, [edi+0Ch]
            pushad
            push edi
            call optimize_mesh_data
            popad
            mov     [esp+14h], ecx
        jmp dword ptr[r_prep_mesh_datas_jmp_back]
    }
}
// clang-format on
#endif

#pragma region cdraw_reclass
// Created with ReClass.NET 1.2 by KN4CK3R

class cDrawSub
{
public:
    class ModTansformsMatrices *obj_transforms_matrices_ptr; //0x0000
    uint8_t *skel_hierarchy; //0x0004
    uint8_t *skel_indicies_ptr; //0x0008
    uint8_t *skel_object_roots; //0x000C
    class ModTransformVec *skel_bones_ptr; //0x0010
    class IDraw *IDrawVtable; //0x0014
    class IDrawOperate *IDrawOperateVtable; //0x0018
    char pad_001C[132]; //0x001C
    uint32_t draw_flag_or_something; //0x00A0
    uint32_t ass; //0x00A4
    uint16_t object_count; //0x00A8
    uint16_t bone_count; //0x00AA
    uint16_t tex_count; //0x00AC
    uint16_t idk3; //0x00AE
    uint32_t unk1; //0x00B0
    uint32_t unk2; //0x00B4
    uint16_t unk3; //0x00B8
    uint16_t unk4; //0x00BA
    class SomeModelData *modnrender_data_ptr; //0x00BC
    class rModel *model_ptr; //0x00C0
    class rTim2 *texture_ptr; //0x00C4
    class TextureStuff0 (*texture_stuff0_ptr)[5]; //0x00C8
    class TextureStuff1 *texture_stuff1_ptr; //0x00CC
    class cDrawSubUnk2 *cdrawsub2_unk0_ptr; //0x00D0
    class cDrawSubUnk2 *cdrawsub2_unk1_ptr; //0x00D4
    class cDrawSubUnk2 *cdrawsub2_unk2_ptr; //0x00D8
    class cDrawSubUnk2 *cdrawsub2_unk3_ptr; //0x00DC
    class cDrawSubUnk2 *cdrawsub2_unk4_ptr; //0x00E0
    class cDrawSubUnk2 *cdrawsub2_unk5_ptr; //0x00E4
    class SomeTextureUintsIdk (*some_texture_uints_ptr)[2]; //0x00E8
    class cDrawSubUnk6 *cdrawsub_unk6_ptr; //0x00EC
    class SomeTextureAlphaThing (*some_texture_alpha_thing_ptr)[1]; //0x00F0
    class SomeTextureAlphaThing *some_texture_alpha_thing_ptr1; //0x00F4
    char pad_00F8[4]; //0x00F8
    class cDrawSubUnk7 *cdrawsub_unk7_ptr; //0x00FC
    Matrix4x4 *some_matrix_ptr; //0x0100
    class UnkVecMaybe *some_ptr_vec_maybe; //0x0104
    char pad_0108[8]; //0x0108
    Matrix4x4 some_matrix0; //0x0110
    Matrix4x4 some_matrix1; //0x0150
}; //Size: 0x0190

class cDrawSCM
{
public:
    char pad_0000[44]; //0x0000
    class cDrawSub N0000005A; //0x002C
    char pad_01BC[4116]; //0x01BC
}; //Size: 0x11D0

class rTim2
{
public:
    char pad_0000[2048]; //0x0000
    char tim2header[4]; //0x0800
    char pad_0804[2048]; //0x0804
}; //Size: 0x1004

class rModelObjectHeaders
{
public:
    uint8_t meshCount; //0x0000
    uint8_t unk; //0x0001
    uint16_t vertCount; //0x0002
    class rModelMesh *objectOffset; //0x0004
    uint32_t objFlags; //0x0008
    uint8_t reserved[20]; //0x000C
    Vector3 spherePos; //0x0020
    float radius; //0x002C
}; //Size: 0x0030

class rModel
{
public:
    char modfourcc[4]; //0x0000
    float version; //0x0004
    uint64_t reserved; //0x0008
    uint8_t objectCount; //0x0010
    uint8_t boneCount; //0x0011
    uint8_t texCount; //0x0012
    uint8_t unk[9]; //0x0013
    uint32_t skelOffset; //0x001C
    char pad_0020[16]; //0x0020
    class rModelObjectHeaders N00000E39; //0x0030
    char pad_0060[220]; //0x0060
}; //Size: 0x013C

class cDraw
{
public:
    char pad_0000[8]; //0x0000
    uint32_t some_field; //0x0008
    uint32_t pad; //0x000C
    uint32_t draw_flag; //0x0010
    class rTim2 *tim2; //0x0014
    uint32_t pad1; //0x0018
    void *some_allocator_type_thing1; //0x001C
    void *some_allocator_type_thing; //0x0020
    uint32_t notSure; //0x0024
    uint32_t notSure1; //0x0028
    class cDrawSub cDrawSub; //0x002C
}; //Size: 0x01BC

class ModTransformVec
{
public:
    Vector4 pos; //0x0000
    Vector4 rot; //0x0010
}; //Size: 0x0020

class ModTansformsMatrices
{
public:
    Matrix4x4 unk; //0x0000
    Matrix4x4 world_maybe; //0x0040
    class ModTansformsMatrices *next; //0x0080
    uint32_t idk; //0x0084
}; //Size: 0x0088

class TextureStuff1
{
public:
    char pad_0000[4100]; //0x0000
}; //Size: 0x1004

class TextureStuff0
{
public:
    void *some_texture_offset; //0x0000
    uint32_t field_04; //0x0004
    uint32_t field_08; //0x0008
    uint32_t field_0C; //0x000C
    uint32_t field_10; //0x0010
    uint32_t field_14; //0x0014
    uint32_t field_18; //0x0018
    uint32_t field_1C; //0x001C
    uint32_t field_20; //0x0020
    uint32_t field_24; //0x0024
    uint32_t field_28; //0x0028
    uint32_t field_2c; //0x002C
}; //Size: 0x0030

class UnkVecMaybe
{
public:
    Vector4 maybe_vec; //0x0000
}; //Size: 0x0010

class SomeTextureUintsIdk
{
public:
    uint32_t field0; //0x0000
    uint32_t field4; //0x0004
    uint32_t field8; //0x0008
    uint32_t fieldC; //0x000C
    char pad_0010[8]; //0x0010
    uint32_t field18; //0x0018
    uint32_t field1C; //0x001C
    char pad_0020[8]; //0x0020
    uint32_t field28; //0x0028
    char pad_002C[4]; //0x002C
}; //Size: 0x0030

class SomeTextureAlphaThing
{
public:
    float flt0; //0x0000
    float flt4; //0x0004
    float flt8; //0x0008
    float fltC; //0x000C
    float flt10; //0x0010
    float flt14; //0x0014
    float flt18; //0x0018
    float flt1C; //0x001C
}; //Size: 0x0020

class rModelHeader
{
public:
    char fourcc[4]; //0x0000
    float version; //0x0004
    uint64_t reserved; //0x0008
    uint8_t objectCount; //0x0010
    uint8_t boneCount; //0x0011
    uint8_t texCount; //0x0012
    uint8_t unk[9]; //0x0013
    void *skelOffs; //0x001C
    void *someOffset; //0x0020
    void *someOtherOffset; //0x0024
    void *someOtherOtherOffset; //0x0028
    void *someOtherOtherOtherOffset; //0x002C
}; //Size: 0x0030

class rModelSCM : public rModelHeader
{
public:
    class rModelObjectHeaders objects[114]; //0x0030 size = header.objectCount
    char pad_1590[5524]; //0x1590
}; //Size: 0x2B24

class rModelMeshHdr
{
public:
    uint16_t vertCount; //0x0000
    uint16_t texInd; //0x0002
    uint64_t reserved; //0x0004
    Vector3 *vertsOffset; //0x000C
    Vector3 *normalsOffs; //0x0010
    class rModelUVData *uvsOffset; //0x0014
    void *someOffset; //0x0018
    void *someOtherOffset; //0x001C
    class rModelSCMData *scmDataOffset; //0x0020
    uint32_t idk00; //0x0024
    uint32_t idk01; //0x0028
    uint32_t idk02; //0x002C
}; //Size: 0x0030

class rModelMesh : public rModelMeshHdr
{
public:
}; //Size: 0x0030

class Object : public rModelMesh
{
public:
}; //Size: 0x0030

class rModelUVData
{
public:
    int16_t u; //0x0000
    int16_t t; //0x0002
}; //Size: 0x0004

class rModelVertColor
{
public:
    uint8_t red; //0x0000
    uint8_t green; //0x0001
    uint8_t blue; //0x0002
}; //Size: 0x0003

class rModelSCMData : public rModelVertColor
{
public:
    uint8_t triSkipFlag; //0x0003
}; //Size: 0x0004

class rModelBone
{
public:
    Vector3 pos; //0x0000
    float length; //0x000C
    Vector3 unkVec; //0x0010
    float unk; //0x001C
}; //Size: 0x0020

class rModelSkeleton
{
public:
    void *hierarchyOffset; //0x0000
    void *indexOffset; //0x0004
    void *objectRootsOffset; //0x0008
    void *boneTransformOffset; //0x000C
    uint32_t notBoneCount; //0x0010
    uint8_t unk[12]; //0x0014
    uint8_t hierarchy[116]; //0x0020 length == notBoneCount?
    uint8_t indicies[116]; //0x0094
    uint8_t objectRoots[116]; //0x0108
    uint32_t reserved; //0x017C
    class rModelBone N000010A8[115]; //0x0180
}; //Size: 0x0FE0

class SomeRenderDataIdkAnonStruct
{
public:
    uint32_t some_int; //0x0000
    uint32_t N00001DEE; //0x0004
    uint32_t eeee_int; //0x0008
    char pad_000C[4]; //0x000C
    uint32_t some_table_lookup_int0; //0x0010
    uint32_t some_table_lookup_int1; //0x0014
    uint32_t n_int; //0x0018
    char pad_001C[4]; //0x001C
    uint32_t table_lookup_int2; //0x0020
    uint32_t table_lookup_int3; //0x0024
    uint32_t g_int; //0x0028
    uint32_t N00001DF8; //0x002C
    uint32_t some_int1; //0x0030
    uint32_t N00001392_; //0x0034
    uint32_t b_int; //0x0038
    uint32_t N00001DFC; //0x003C
    uint32_t some_obj_flags_bitwise_thing; //0x0040
    uint32_t some_int_unk; //0x0044
    char pad_0048[4]; //0x0048
    uint32_t N00001E07; //0x004C
}; //Size: 0x0050

class SmdScratchBufferMaybe
{
public:
    char pad_0000[144]; //0x0000
}; //Size: 0x0090

class SomeModelData
{
public:
    uint32_t fld0; //0x0000
    uint8_t flag0; //0x0004
    uint8_t flag1; //0x0005
    uint8_t flag2; //0x0006
    uint8_t flag3; //0x0007
    uint32_t vert_accumulator; //0x0008
    uint8_t mesh_count; //0x000C
    uint8_t N000019A8; //0x000D
    uint8_t some_skel_stuff; //0x000E
    uint8_t N000019A9; //0x000F
    uint32_t N00001380; //0x0010
    uint32_t some_rendering_flags_again; //0x0014
    class rModelObjectHeaders *r_model_obj_headers_ptr; //0x0018
    class SomeRenderingData *ptr_has_some_rendering_data; //0x001C
    Vector4 sphere; //0x0020
    class cDrawSubUnk2 *cdrawsubs_array[6]; //0x0030
    uint32_t N0000138B; //0x0048
    uint32_t N0000138C; //0x004C
    uint32_t some_table_lookup_int0; //0x0050
    uint32_t some_table_lookup_int1; //0x0054
    uint32_t some_table_lookup_int2; //0x0058
    uint32_t some_table_lookup_int3; //0x005C
    uint32_t some_int1; //0x0060
    uint32_t N00001392; //0x0064
    uint32_t some_obj_flags_bitwise_thing; //0x0068
    uint32_t some_int_unk; //0x006C
    uint32_t some_result_int; //0x0070
    char pad_0074[12]; //0x0074
    class SomeRenderDataIdkAnonStruct some_anon_struct0; //0x0080
    class SomeRenderDataIdkAnonStruct some_anon_struct1; //0x00D0
    uint32_t N000013C1; //0x0120
    uint32_t N000013C2; //0x0124
    uint32_t N000013C3; //0x0128
    uint32_t N000013C4; //0x012C
    Vector4 emissive; //0x0130
    uint32_t N000013C9; //0x0140
    uint32_t N000013CA; //0x0144
    uint32_t N000013CB; //0x0148
    uint32_t some_flag0; //0x014C
    char pad_0150[16]; //0x0150
    Vector4 emissive_copy; //0x0160
    char pad_0170[12]; //0x0170
    uint32_t some_flag1; //0x017C
    char pad_0180[304]; //0x0180
    class SmdScratchBufferMaybe some_scratch_buffer_maybe; //0x02B0
}; //Size: 0x0340

class cDrawSubUnk2
{
public:
    uint32_t f0; //0x0000
    class cDrawSubUnk2 *cdsu_next_ptr; //0x0004
    uint32_t f8; //0x0008
    uint32_t fc; //0x000C
    uint32_t f10; //0x0010
    class SomeTextureAlphaThing *some_tex_alpha_thing_ptr; //0x0014
    uint32_t some_static_memory_stuff; //0x0018
    uint32_t f1c; //0x001C
    uint32_t field_20; //0x0020
    Matrix4x4 *projection_maybe; //0x0024
    uint32_t field_28; //0x0028
    uint32_t field_2C; //0x002C
}; //Size: 0x0030

class SomeRenderingData
{
public:
    uint32_t batches_maybe; //0x0000
    uint32_t vert_count; //0x0004
    uint32_t tex_ind; //0x0008
    class rModelMeshHdr *model_mesh_hdr_ptr; //0x000C
    uint32_t offset_0x10; //0x0010
    uint32_t offset_0x14; //0x0014
    uint32_t offset_0x18; //0x0018
    uint32_t offset_0x1C; //0x001C
    uint32_t offset_0x20; //0x0020
    uint32_t offset_0x24; //0x0024
    uint32_t offset_0x28; //0x0028
    uint32_t offset_0x2C; //0x002C
    uint8_t offset_0x30; //0x0030
    char pad_0031[3]; //0x0031
    struct DIPData* index_buffer; //0x0034
    uint32_t startin_vertex; //0x0038
    char pad_003C[4]; //0x003C
    class SomeRenderDataIdkAnonStruct N00001A0B; //0x0040
    class SomeRenderDataIdkAnonStruct N00001A0C; //0x0090
    uint32_t offset_0xE0; //0x00E0
    uint32_t offset_0xE4; //0x00E4
    uint32_t offset_0xE8; //0x00E8
    uint32_t offset_0xEC; //0x00EC
    void *anon_struct0; //0x00F0
    void *anon_struct1; //0x00F4
    Vector3 *verts_ptr0; //0x00F8
    Vector3 *verts_ptr1; //0x00FC
    Vector3 *normals_ptr; //0x0100
    Vector3 *normals_ptr1; //0x0104
    class rModelUVData *uvdata_ptr; //0x0108
    class rModelUVData *uvdata_ptr1; //0x010C
    char pad_0110[8]; //0x0110
    class rModelWeightsData *weightdata_ptr0; //0x0118
    class rModelWeightsData *weightdata_ptr1; //0x011C
    class rModelSCMData *scmdata_ptr; //0x0120
    class rModelSCMData *scmdata_ptr1; //0x0124
    class RenderUVFudge *render_uv_fudge0; //0x0128
    class RenderUVFudge *render_uv_fudge1; //0x012C
    Vector3 *verts_ptr2; //0x0130
}; //Size: 0x0134

class RenderUVFudge
{
public:
    int32_t some_uvx_fudge; //0x0000
    int32_t some_uvy_fude; //0x0004
    uint32_t N00001CC7; //0x0008
    uint32_t N00001CC8; //0x000C
    uint32_t N00001CC9; //0x0010
    char pad_0014[4]; //0x0014
}; //Size: 0x0018

class cDrawSubUnk6
{
public:
    char pad_0000[260]; //0x0000
}; //Size: 0x0104

class cDrawSubUnk7
{
public:
    char pad_0000[4]; //0x0000
}; //Size: 0x0004

class IDraw
{
public:
    void* N00001BDA[57]; //0x0000
}; //Size: 0x00E4

class IDrawOperate
{
public:
    void* N00001F51[45]; //0x0000
}; //Size: 0x00B4

class N00002118
{
public:
    Matrix4x4 N00002119; //0x0000
    char pad_0040[288]; //0x0040
}; //Size: 0x0160

class rModelWeightsData
{
public:
    uint16_t bone_weights; //0x0000
}; //Size: 0x0004
#pragma endregion


void* g_index_buffer_backing_area {};
utility::Arena g_index_buffer {};

struct IndexData {
    std::uint32_t prim_count;
    std::uint32_t index_count;
    std::vector<uint16_t> ib;
};

IndexData make_index_buffer(SomeRenderingData* srd) {
    assert(srd->vert_count);
#ifndef NDEBUG
    //printf("make_index_buffer() vert_count:%d\n", srd->vert_count);
#endif // !NDEBUG

    IndexData result;
    result.prim_count = 0;

    std::uint16_t p1 = 0, p2 = 1;
    bool  wnd = 1; // winding order
    std::vector<uint16_t> indices; 
    indices.reserve(srd->vert_count * 3);

    for (uint32_t i = 2; i < srd->vert_count; ++i) {
        std::uint16_t p3 = i;
        bool tri_skip;
        if (srd->scmdata_ptr) {
            tri_skip = srd->scmdata_ptr[i].triSkipFlag;
        }
        else { // stored in bone weight for characters
            tri_skip = ((srd->weightdata_ptr0[i].bone_weights >> 0xF) & 1);
        }
        if (!tri_skip) {
            // compute triangle normal and winding order
            Vector3 v1 = srd->verts_ptr0[p1];
            Vector3 v2 = srd->verts_ptr0[p2];
            Vector3 v3 = srd->verts_ptr0[p3];

            Vector3 face_edge1 = v3 - v1;
            Vector3 face_edge2 = v2 - v1;
            face_edge1 = glm::normalize(face_edge1);
            face_edge2 = glm::normalize(face_edge2);

            Vector3 z  = glm::cross(face_edge1, face_edge2);
            z = glm::normalize(z);

            Vector3 normal1 = srd->normals_ptr[p1];
            Vector3 normal2 = srd->normals_ptr[p2];
            Vector3 normal3 = srd->normals_ptr[p3];
            Vector3 normal = glm::normalize(normal1 + normal2 + normal3);

            // Add indices in the correct winding order
            //wnd = glm::dot(normal, z) > 0.0f;
            // ratmix normals look inverted with code above for some reason
            wnd = glm::dot(normal, z) < 0.0f;
            if (wnd) {
                indices.push_back(p1);
                indices.push_back(p3);
                indices.push_back(p2);
            }
            else {
                indices.push_back(p1);
                indices.push_back(p2);
                indices.push_back(p3);
            }
            result.prim_count += 1;
        }

        // Update previous indices for next iteration
        p1 = p2;
        p2 = p3;
    }

    auto num_indices = std::distance(indices.begin(), indices.end());
    //srd->starting_index = num_indices;
    result.index_count = num_indices;
    result.ib = std::move(indices);
    return result;
}

void cdraw_set_render_data_hook(SomeRenderingData* srd) {
    if (!srd) { return; }
    assert(srd->vert_count);
    //srd->starting_index = 0xDEADBEEF; // test

#if 0
    const size_t size = indices.size() * sizeof(WORD);
    void* mem = utility::arena_alloc(&g_index_buffer, size );
    memcpy(mem, indices.data(), size);
#endif
}


static uintptr_t cdraw_set_render_data_struct_jmp = 0x0068BE10;
// clang-format off
static __declspec(naked) void cdraw_set_render_data_struct() {
    __asm {
        pushad
        push edi
        call cdraw_set_render_data_hook
        add esp, 4
        popad
    originalCode:
        mov [esp+18h], eax
        cmp eax,edx
        jmp dword ptr[cdraw_set_render_data_struct_jmp]
    }
}
// clang-format on


struct RDrawSomethingStruct
{
    DWORD verts_or_whatever;
/*
    uint8_t col_r0;
    uint8_t col_g0;
    uint8_t col_b0;
    uint8_t col_a0;
*/
    uint8_t col_r1;
    uint8_t col_g1;
    uint8_t col_b1;
    uint8_t col_a1;
    Matrix4x4 *p_some_matrix;
    struct SomeRenderingData *srd;
    uint32_t some_flags;
};

size_t g_vertices_offset {0};
size_t g_indices_offset {0};

enum EVertexBufferType{
    ARTICULATED_MODELS = 1, // unused i think
    GUI_QUADS = 2,
    WORLD = 3,
};

struct DIPData {
    std::int32_t ref_count {0};
    std::uint32_t vertex_count_ours;
    std::uint32_t primitive_count_ours;
    std::uint32_t index_count_ours;
    std::vector<std::uint16_t> index_buffer_ours;
    std::vector<VertexDef> vertex_buffer_ours;
};


// TODO(): find where to stick this
static constexpr DWORD vertex_format_dmc3_3d = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 | D3DFVF_TEX2;

struct DynamicRenderBuffer {
    static const size_t round_to_multiple(size_t number, size_t multiple) {
        return (((number + multiple / 2) / multiple) * multiple);
    }

    void re_create_buffers(IDirect3DDevice9* pdevice) {
        assert(m_vb_cur_size < m_vb_capacity);
        if (m_create_flags & CREATE_FLAGS::VERTEX_BUFFER) {
            // release existing
            if (p_vb) {
                p_vb->Release();
                p_vb = nullptr;
            }
            // create vb
            if (FAILED(pdevice->CreateVertexBuffer(m_vb_capacity,
                       0, vertex_format_dmc3_3d, D3DPOOL_DEFAULT, &p_vb, NULL))) {
                assert("create_vb failed!");
                return;
            }
            m_create_flags &= ~CREATE_FLAGS::VERTEX_BUFFER;
        }

        assert(m_ib_cur_size < m_ib_capacity);
        if (m_create_flags & CREATE_FLAGS::INDEX_BUFFER) {
            // release existing 
            if (p_ib) {
                p_ib->Release();
                p_ib = nullptr;
            }
            // create ib
            if (FAILED(pdevice->CreateIndexBuffer(m_ib_capacity,
                       0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &p_ib, NULL))) {
                assert("create_ib failed!");
                return;
            }
            m_create_flags &= ~CREATE_FLAGS::INDEX_BUFFER;
        }
    }

    void reset() {
        m_vb_cur_size = 0;
        m_ib_cur_size = 0;
        m_create_flags = CREATE_FLAGS::NONE;
    }

    void upload(std::vector<VertexDef>& verts, std::vector<WORD>& inds) {
        assert(p_vb);
        // upload vert data
        void* pvertices;
        const size_t vbsize = verts.size() * sizeof(VertexDef);
        if (FAILED(p_vb->Lock(0, vbsize, (void**)&pvertices, 0))) {
            return;
        }
        memcpy(pvertices, verts.data(), vbsize);
        p_vb->Unlock();

        assert(p_ib);
        // upload index data
        void* pindices;
        const size_t ibsize = inds.size() * sizeof(WORD);
        if (FAILED(p_ib->Lock(0, ibsize, (void**)&pindices, 0))) {
            return ;
        }
        memcpy(pindices, inds.data(), ibsize);
        p_ib->Unlock();
    }

    void check_and_flag_for_resize(size_t verts, size_t indexs) {
        m_vb_cur_size += (verts * sizeof(VertexDef));
        m_ib_cur_size += (indexs * sizeof(WORD));
        // vertex buffer stuff
        size_t new_vert_capacity = m_vb_capacity;
        if (m_vb_cur_size > m_vb_capacity) {
            // check if its the first time
            if (!new_vert_capacity) {
                new_vert_capacity = 1;
            }
            // make cap next pow of 2
            while (new_vert_capacity < m_vb_cur_size) {
                new_vert_capacity <<= 1;
            }
            new_vert_capacity = round_to_multiple(new_vert_capacity, sizeof(VertexDef));
        }
        if (new_vert_capacity != m_vb_capacity) {
            m_vb_capacity = new_vert_capacity;
            // mark vb for (re)creation later
            m_create_flags |= CREATE_FLAGS::VERTEX_BUFFER;
        }

        size_t new_index_capacity = m_ib_capacity;
        if (m_ib_cur_size > new_index_capacity) {

            // check if its the first time
            if (!new_index_capacity) {
                new_index_capacity = 1;
            }

            // make cap next pow of 2
            while (new_index_capacity < m_ib_cur_size) {
                new_index_capacity <<= 1;
            }
            new_index_capacity = round_to_multiple(new_index_capacity, sizeof(WORD));
        }
        if (new_index_capacity != m_ib_capacity) {
            m_ib_capacity = new_index_capacity;
            // mark ib for (re)creation later
            m_create_flags |= CREATE_FLAGS::INDEX_BUFFER;
        }
    }

    enum CREATE_FLAGS {
        NONE = 0,
        VERTEX_BUFFER = 1 << 0,
        INDEX_BUFFER = 1 << 1,
    };
    uint32_t m_create_flags = NONE;


    LPDIRECT3DVERTEXBUFFER9 p_vb = NULL;
    LPDIRECT3DINDEXBUFFER9  p_ib = NULL;

    size_t m_vb_cur_size {0};
    size_t m_ib_cur_size {0};

    size_t m_vb_capacity {0};
    size_t m_ib_capacity {0};
};

static DynamicRenderBuffer* g_dyn_ren_buf {nullptr};

static std::map<size_t, DIPData*> g_dip_map;

static VertexDef* g_current_vertex_data_pointer = 0x0;
static uintptr_t r_malloc_vertex_datas_jmp = 0x006E15AC;
// clang-format off
static __declspec(naked) void r_malloc_vertex_datas_detour() {
    __asm {
        mov eax,ecx
        mov dword ptr [g_current_vertex_data_pointer], eax
    originalCode:
        add ecx,38h
        jmp dword ptr[r_malloc_vertex_datas_jmp]
    }
}
// clang-format on
static void r_render_world_intercept(RDrawSomethingStruct* rdss) {
    if(!rdss) { return; }
    //printf("|rdss->srd->vert_count=%d\n", rdss->srd->vert_count);

    typedef void(__fastcall * rPrepDrawDataSub6DF5A0)(unsigned int a1, int start_offset, int tri_strip_count);
    static rPrepDrawDataSub6DF5A0 r_prep_draw_data_sub_6DF5A0 = (rPrepDrawDataSub6DF5A0)0x6DF5A0;
    static int* some_graphic_struct_dword_252F490_ = (int*)0x0252F490;
    ptrdiff_t start_offset = 0;
    DWORD tri_strip_count = 0;

#if 0
    for (size_t resb = 0; resb < rdss->srd->vert_count; resb = (resb + 1)) {
        typedef int(__fastcall * cDrawVertexDataSetSub6DE480)(size_t vert_index, RDrawSomethingStruct* a2);
        static cDrawVertexDataSetSub6DE480 cDraw_vertex_data_set_sub_6DE480 = (cDrawVertexDataSetSub6DE480)0x6DE480;
        if (cDraw_vertex_data_set_sub_6DE480(resb, rdss)) {
            r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, start_offset, tri_strip_count);
            start_offset    = 0;
            tri_strip_count = 0;
        } else {
            if (!tri_strip_count) {
                uint32_t* dword_252FFA4 = (uint32_t*)0x0252FFA4;
                uint32_t object_count = *dword_252FFA4;
                start_offset  = 4 * object_count - 3;
            }
            ++tri_strip_count;
        }
    }
#endif
    uint8_t value = EVertexBufferType::WORLD;

    size_t hash = std::hash<uintptr_t>()((uintptr_t)rdss->srd->vert_count + (uintptr_t)rdss->srd->tex_ind + (uintptr_t)rdss->srd->model_mesh_hdr_ptr);
    if (auto& it = g_dip_map.find(hash); it != g_dip_map.end()) {
        DIPData* render_info = it->second;
        g_dyn_ren_buf->check_and_flag_for_resize(
            render_info->vertex_buffer_ours.size(),
            render_info->index_buffer_ours.size());
        render_info->ref_count -= 1;
        r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, g_vertices_offset | (value << 24), (int)it->second);
        return;
    }

    DIPData* render_info = new DIPData;
    render_info->vertex_buffer_ours.reserve(rdss->srd->vert_count);

    for (size_t resb = 0; resb < rdss->srd->vert_count; resb = (resb + 1)) {
        typedef int(__fastcall * cDrawVertexDataSetSub6DE480)(size_t vert_index, RDrawSomethingStruct* a2);
        static cDrawVertexDataSetSub6DE480 cDraw_vertex_data_set_sub_6DE480 = (cDrawVertexDataSetSub6DE480)0x6DE480;
        cDraw_vertex_data_set_sub_6DE480(resb, rdss);
        render_info->vertex_buffer_ours.emplace_back();
        memcpy(&render_info->vertex_buffer_ours[resb], g_current_vertex_data_pointer, 0x38);
    }

    IndexData result = make_index_buffer(rdss->srd);
    size_t ib_size = result.index_count * sizeof(uint16_t);

    render_info->index_count_ours     = result.index_count;
    render_info->primitive_count_ours = result.prim_count;
    render_info->index_buffer_ours    = std::move(result.ib);
    render_info->vertex_count_ours    = rdss->srd->vert_count;
    render_info->ref_count -= 1;

    g_dyn_ren_buf->check_and_flag_for_resize(
        render_info->vertex_buffer_ours.size(),
        render_info->index_buffer_ours.size());

    r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, g_vertices_offset | (value << 24), (int)render_info);
    g_dip_map.emplace(hash, render_info);

#if 0
    typedef int(__fastcall * cDrawVertexDataSetSub6DE480)(size_t vert_index, RDrawSomethingStruct* a2);
    static cDrawVertexDataSetSub6DE480 cDraw_vertex_data_set_sub_6DE480 = (cDrawVertexDataSetSub6DE480)0x6DE480;
    size_t index_end = 0;
    for(size_t i = index; i < rdss->srd->vert_count; i++) {
        if (cDraw_vertex_data_set_sub_6DE480(index, rdss)) {
            typedef void(__fastcall * rPrepDrawDataSub6DF5A0)(unsigned int a1, int start_offset, int tri_strip_count);
            uint32_t some_graphic_struct_dword_252F490_               = 0x0252F490;
            static rPrepDrawDataSub6DF5A0 r_prep_draw_data_sub_6DF5A0 = (rPrepDrawDataSub6DF5A0)0x6DF5A0;
            r_prep_draw_data_sub_6DF5A0(some_graphic_struct_dword_252F490_, g_vertices_offset, g_indices_offset);

        }
        index_end = i;
    }
        auto& indices = g_mapping.at((uintptr_t)rdss->srd->model_mesh_hdr_ptr);
        size_t isize = indices.size() * sizeof(DWORD);
        auto mem = utility::arena_alloc(&g_index_buffer,isize);
        memcpy(mem, indices.data(), isize);

        g_vertices_offset += (index_end - index);
        g_indices_offset += indices.size();
#endif

}

static void r_render_chars_intercept(SomeRenderingData* srd0, RDrawSomethingStruct* rdss, SomeRenderingData** srd) {
    assert(srd0);
    assert(srd0->vert_count);

    const uint32_t vert_count = srd0->vert_count;

    typedef int (__cdecl *cDrawcDrawSubPrepVertsGuiAndCharacters)(RDrawSomethingStruct *a1, int vert_index, SomeRenderingData** a3);
    cDrawcDrawSubPrepVertsGuiAndCharacters prep_verts_gui_and_characters = (cDrawcDrawSubPrepVertsGuiAndCharacters)0x006DD4A0;

    typedef void(__fastcall * rPrepDrawDataSub6DF5A0)(unsigned int a1, int start_offset, int tri_strip_count);
    static rPrepDrawDataSub6DF5A0 r_prep_draw_data_sub_6DF5A0 = (rPrepDrawDataSub6DF5A0)0x6DF5A0;
    static int* some_graphic_struct_dword_252F490_ = (int*)0x0252F490;
    ptrdiff_t start_offset = 0;
    DWORD tri_strip_count = 0;
    uint8_t* g_render_ui_flag_byte_252F48C = (uint8_t*)0x0252F48C;

    if (*g_render_ui_flag_byte_252F48C) {

        for (size_t vert = 0; vert < vert_count; vert = vert + 1) {
            if (prep_verts_gui_and_characters(rdss, vert, srd)) {
                r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, start_offset, tri_strip_count);
                start_offset    = 0;
                tri_strip_count = 0;
            } else {
                if (!tri_strip_count) {
                    start_offset = rdss->verts_or_whatever;
                    //printf("|start_offset=%d\n", start_offset);
                }
                ++tri_strip_count;
            }
        }
        r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, start_offset, tri_strip_count);

        return;
    }

    uint8_t value    = EVertexBufferType::WORLD;
    size_t hash = std::hash<uintptr_t>()((uintptr_t)srd0->vert_count + (uintptr_t)srd0->tex_ind + (uintptr_t)srd0->model_mesh_hdr_ptr);
    if (auto& it = g_dip_map.find(hash); it != g_dip_map.end()) {

        DIPData* render_info = it->second;
        render_info->ref_count -= 1;
        g_dyn_ren_buf->check_and_flag_for_resize(
            render_info->vertex_buffer_ours.size(),
            render_info->index_buffer_ours.size());

        r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, g_vertices_offset | (value << 24), (int)it->second);
        return;
    }
    DIPData* render_info;
        render_info = new DIPData;
        render_info->vertex_buffer_ours.reserve(vert_count);

        for (size_t vert = 0; vert < vert_count; vert = vert + 1) {
            prep_verts_gui_and_characters(rdss, vert, srd);
            render_info->vertex_buffer_ours.emplace_back();
            memcpy(&render_info->vertex_buffer_ours[vert], g_current_vertex_data_pointer, 0x38);
        }

        if (!srd0->index_buffer) {
            srd0->index_buffer = render_info;
        }
        IndexData result = make_index_buffer(srd0);

        render_info->index_count_ours     = result.index_count;
        render_info->primitive_count_ours = result.prim_count;
        render_info->index_buffer_ours    = std::move(result.ib);
        render_info->vertex_count_ours    = srd0->vert_count;
        render_info->ref_count -= 1;

    g_dyn_ren_buf->check_and_flag_for_resize(
        render_info->vertex_buffer_ours.size(),
        render_info->index_buffer_ours.size());

    g_dip_map.emplace(hash, render_info);
    return;

    r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, g_vertices_offset | (value << 24), (int)render_info);

    //r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, start_offset, tri_strip_count);
}

static uintptr_t r_render_chars_jmp = 0x006DD43B;
// clang-format off
static __declspec(naked) void r_render_chars_detour() {
    __asm {
        
        pushad
        lea eax, [esp + 7Ch] ;v42
        lea edx, [esp + 0B0h/*20h + 0ACh + -1Ch*/] ;rdss
        push eax
        push edx
        push esi
        call r_render_chars_intercept
        add esp, 0Ch
        popad
        add esp, 10h // adjust stack memes
        mov esi,[esp+0000024h]

    originalCode:
        jmp dword ptr[r_render_chars_jmp]
    }
}
// clang-format on

static uintptr_t r_render_world_jmp = 0x006DE417;
// clang-format off
static __declspec(naked) void r_render_world_detour() {
    __asm {
        pushad
        lea edi, [esp+8ch]
        push edi
        call r_render_world_intercept
        add esp, 4
        popad
        add esp, 18h // adjust stack memes
        mov esi,[esp+000001BCh]

    originalCode:
        jmp dword ptr[r_render_world_jmp]
    }
}
// clang-format on

static void r_reset_rendering_globals() {
    g_indices_offset           = 0;
    g_vertices_offset          = 0;
    g_index_buffer.curr_offset = 0;
    g_index_buffer.prev_offset = 0;

    g_dyn_ren_buf->reset();
    // clean up all unreferenced models
    for (auto it = g_dip_map.begin(); it != g_dip_map.end();) {
        it->second->ref_count += 1;
        if (!it->second || it->second->ref_count > 32) {
            delete it->second;
            it = g_dip_map.erase(it);
        }
        else {
            ++it;
        }
    }
}

static uintptr_t r_reset_rendering_globals_jmp = 0x006E17F4;
// clang-format off
static __declspec(naked) void r_reset_rendering_globals_detour() {
    __asm {
        pushad
        call r_reset_rendering_globals
        popad

    originalCode:
        jmp dword ptr[r_reset_rendering_globals_jmp]
    }
}
// clang-format on

RendererReplace::~RendererReplace() {
    free(g_index_buffer_backing_area);
}


std::unique_ptr<FunctionHook> g_d3d_upload_to_vram_sub_6e18f0_hook;

static int __cdecl d3d_switch_on_buffer_type_and_upload_to_vram_sub_6E18F0(int a1) {
    if (!g_d3d_upload_to_vram_sub_6e18f0_hook) { return D3D_OK; }

    static D3DDevicePtr* device_ptr  = (D3DDevicePtr*)0x0252F374;
    static IDirect3DDevice9* pdevice = device_ptr->device;

    uint8_t  type = (a1 >> 24) & 0xFF;
    if (type == EVertexBufferType::WORLD) {
#if 0
        
        UINT indices_size = g_index_buffer.curr_offset;
        if (FAILED(pdevice->CreateIndexBuffer(indices_size,
            0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pib, NULL))) {
            return E_FAIL;
        }
        void* p_indices;
        if (FAILED(g_pib->Lock(0, indices_size, (void**)&p_indices, 0))) {
            return E_FAIL;
        }
        memcpy(p_indices, g_index_buffer.buf, indices_size);
        g_pib->Unlock();
#endif
    }
    auto result = g_d3d_upload_to_vram_sub_6e18f0_hook->get_original<decltype(d3d_switch_on_buffer_type_and_upload_to_vram_sub_6E18F0)>()(type);
    return result;
}


std::unique_ptr<FunctionHook> g_d3d_draw_primitive_sub_6E1C00_hook;
static void __cdecl d3d_draw_primitive_sub_6E1C00(unsigned int vertexCount, int primitve_count, char a3) {
    if(!g_d3d_draw_primitive_sub_6E1C00_hook) {return;}

    static D3DDevicePtr* device_ptr  = (D3DDevicePtr*)0x0252F374;
    static IDirect3DDevice9* pdevice = device_ptr->device;

    uint8_t  type = (vertexCount >> 24) & 0xFF;
    uint32_t vert_count = vertexCount & 0x00FFFFFF;
    if (type == EVertexBufferType::WORLD) {
#if 0
        typedef int (__cdecl *D3DCreateVB)(int a1);
        static D3DCreateVB create_vb_fptr = (D3DCreateVB)0x006E18F0;
        create_vb_fptr(vertexCount);
#endif

        DIPData* dd = (DIPData*)primitve_count;

        // upload data to vb & ib
        g_dyn_ren_buf->re_create_buffers(pdevice);
        g_dyn_ren_buf->upload(dd->vertex_buffer_ours, dd->index_buffer_ours);

        // draw
        pdevice->SetStreamSource(0, g_dyn_ren_buf->p_vb, 0, sizeof(VertexDef));
        pdevice->SetFVF(vertex_format_dmc3_3d);
        pdevice->SetIndices(g_dyn_ren_buf->p_ib);

        auto hr = pdevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, dd->vertex_buffer_ours.size(), 0, dd->primitive_count_ours);
        if (FAILED(hr)) {
            fprintf(stderr, "Error: %s error description: %s\n",
                    DXGetErrorString(hr), DXGetErrorDescription(hr));
        }
        dd->ref_count -= 1;

#if 0
        LPDIRECT3DINDEXBUFFER9 pIB  = NULL;
        LPDIRECT3DVERTEXBUFFER9 pVB = NULL;

        DIPData* dd = (DIPData*)primitve_count;

        // create and lock vb
        UINT vbsize = dd->vertex_buffer_ours.size() * 0x38;
        constexpr auto vertex_format_dmc3_3d = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 | D3DFVF_TEX2;
        if (FAILED(pdevice->CreateVertexBuffer( vbsize,
            0, vertex_format_dmc3_3d, D3DPOOL_DEFAULT, &pVB, NULL))) {
            return;
        }
        void* pvertices;
        if (FAILED(pVB->Lock(0, vbsize, (void**)&pvertices, 0))) {
            return;
        }
        memcpy(pvertices, dd->vertex_buffer_ours.data(), vbsize);
        pVB->Unlock();

        // create and lock ib
        UINT ibsize = dd->index_buffer_ours.size() * sizeof(std::uint16_t);
        if (FAILED(pdevice->CreateIndexBuffer(ibsize,
            0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pIB, NULL))) {
            return;
        }

        void* pindices;
        if (FAILED(pIB->Lock(0, ibsize, (void**)&pindices, 0))) {
            return ;
        }
        memcpy(pindices, dd->index_buffer_ours.data(), ibsize);
        pIB->Unlock();

        // draw
        pdevice->SetStreamSource(0, pVB, 0, sizeof(VertexDef));
        pdevice->SetFVF(vertex_format_dmc3_3d);
        pdevice->SetIndices(pIB);

        auto hr = pdevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, dd->vertex_buffer_ours.size(), 0, dd->primitive_count_ours);
        if (FAILED(hr)) {
            fprintf(stderr, "Error: %s error description: %s\n",
                    DXGetErrorString(hr), DXGetErrorDescription(hr));
        }

        pVB->Release();
        pIB->Release();
        dd->ref_count += 1;
#endif
    }
    else {
        //g_d3d_draw_primitive_sub_6E1C00_hook->get_original<decltype(d3d_draw_primitive_sub_6E1C00)>()(vertexCount, primitve_count, a3);
    }
}

std::optional<std::string> RendererReplace::on_initialize() {
    // WARNING(): dirty hack to only init once here:
    static bool init = false;
    if (init) {
        return Mod::on_initialize();
    }
    init = true;
    g_rreplace = this;

    g_dyn_ren_buf = new DynamicRenderBuffer();

    //g_index_buffer_backing_area = malloc(INDEX_BUFFER_SIZE);
    //utility::arena_init(&g_index_buffer, g_index_buffer_backing_area, INDEX_BUFFER_SIZE);

#if 0
    m_cdraw_set_render_data_hook = std::make_unique<FunctionHook>(0x0068BE0A, &cdraw_set_render_data_struct);
    if (!m_cdraw_set_render_data_hook->create()) {
        return "failed to hook prep mesh datas";
    }
#endif

    m_r_render_prep_world_hook = std::make_unique<FunctionHook>(0x006DE3BC, &r_render_world_detour);
    if (!m_r_render_prep_world_hook->create()) {
        return "failed to hook draw world";
    }

    m_r_render_prep_articulated_models_hook = std::make_unique<FunctionHook>(0x006DD3D3, &r_render_chars_detour);
    if (!m_r_render_prep_articulated_models_hook->create()) {
        return "failed to hook render characters and guis";
    }
    g_d3d_upload_to_vram_sub_6e18f0_hook = std::make_unique<FunctionHook>(0x006E18F0, &d3d_switch_on_buffer_type_and_upload_to_vram_sub_6E18F0);
    if (!g_d3d_upload_to_vram_sub_6e18f0_hook->create()) {
        return "failed to hook d3d_updload_to_vram";
    }
    g_d3d_draw_primitive_sub_6E1C00_hook = std::make_unique<FunctionHook>(0x006E1C00, &d3d_draw_primitive_sub_6E1C00);
    if (!g_d3d_draw_primitive_sub_6E1C00_hook->create()) {
        return "failed to hook d3d_draw_primitive";
    }
    m_r_render_reset_globals_hook = std::make_unique<FunctionHook>(0x006E17EF, &r_reset_rendering_globals_detour);
    if (!m_r_render_reset_globals_hook->create()) {
        return "failed to create render globals reset hook";
    }
    m_r_render_vertex_data_hook = std::make_unique<FunctionHook>(0x006E15A7, &r_malloc_vertex_datas_detour);
    if (!m_r_render_vertex_data_hook->create()) {
        return "failed to create render globals reset hook";
    }

    return Mod::on_initialize();


    m_d3d_dispatch_drawcall_006DF65D = std::make_unique<FunctionHook>(0x006DF65D, &d3d_dispatch_drawcall);
    if (!m_d3d_dispatch_drawcall_006DF65D->create()) {
        return "Failed to hook m_d3d_dispatch_drawcall_006DF65D";
    }

    m_d3d_before_unlock_006E1B54 = std::make_unique<FunctionHook>(0x006E1B54, &d3d_before_unlock_naked);
    if (!m_d3d_before_unlock_006E1B54->create()) {
        return "Failed to hook m_d3d_before_unlock_006E1B54";
    }

    m_d3d_release_vb_and_unlock_sub_6E1220 = std::make_unique<FunctionHook>(0x006E124F, &d3d_release_ib);
    if (!m_d3d_release_vb_and_unlock_sub_6E1220) {
        return "Failed to hook m_d3d_release_vb_and_unlock_sub_6E1220";
    }
    

    m_d3d_init_hook_sub_408DE0 = std::make_unique<FunctionHook>(0x0408DE0, &init_d3d9_sub_408DE0);
    if (!m_d3d_init_hook_sub_408DE0->create()) {
        return "Failed to hook m_d3d_init_hook_sub_408DE0";
    }
#if 0
    // set transform world
    m_d3d_set_transform_world_006DBCB0 = std::make_unique<FunctionHook>(0x006DBCB0, &d3d_set_transform_world);
    if (!m_d3d_set_transform_world_006DBCB0->create()) {
        return "m_d3d_set_transform_world_006DBCB0";
    }

    // begin scene
    m_d3d_begin_scene_sub_006DC3CD = std::make_unique<FunctionHook>(0x006DC3CD, &d3d_begin_scene);
    if (!m_d3d_begin_scene_sub_006DC3CD->create()) {
        return "m_d3d_begin_scene_sub_006DC3CD";
    }

    // end scene
    m_d3d_end_scene_sub_006DC483 = std::make_unique<FunctionHook>(0x006DC483, &d3d_end_scene);
    if (!m_d3d_end_scene_sub_006DC483->create()) {
        return "m_d3d_end_scene_sub_006DC483";
    }

    // vb update
    m_d3d_create_vb_and_lock_sub_006E1ADF = std::make_unique<FunctionHook>(0x006E1ADF, &d3d_create_vb_and_lock);
    #if 1
    if (!m_d3d_create_vb_and_lock_sub_006E1ADF->create()) {
        return "Failed to hook m_d3d_init_hook_sub_408DE0";
    }
    #endif

    // vb unlock
    m_d3d_unlock_vb_006E1B59 = std::make_unique<FunctionHook>(0x006E1B59, &d3d_unlock_buffs);
    if (!m_d3d_unlock_vb_006E1B59->create()) {
        return "Failed to hook m_d3d_init_hook_sub_408DE0";
    }

    // draw primitive
    m_d3d_draw_primitive_006E1C5B = std::make_unique<FunctionHook>(0x006E1C5B, &d3d_draw_primitive);
    if (!m_d3d_draw_primitive_006E1C5B->create()) {
        return "Failed to hook m_d3d_draw_primitive_006E1C5B";
    }
    #endif
    return Mod::on_initialize();
}

void RendererReplace::on_draw_ui() {
    ImGui::InputInt("base_vertex_index", &g_temp_dip_things.base_vertex_index);
    ImGui::InputInt("min_vertex_index", (int*) &g_temp_dip_things.min_vertex_index);
    ImGui::InputInt("num_vertices", (int*) &g_temp_dip_things.num_vertices);
    ImGui::InputInt("start_index", (int*) &g_temp_dip_things.start_index);
    ImGui::InputInt("prim_count", (int*) &g_temp_dip_things.prim_count);
}

//TODO
void RendererReplace::d3d_create_and_lock_vb_sub_6E1260_internal() noexcept
{
}

void RendererReplace::d3d_create_and_lock_vb_sub_6E1260() noexcept
{
}
void RendererReplace::init_d3d9_sub_408DE0_internal() noexcept
{
#if 0
    constexpr auto vertex_format_dmc3_3d = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 | D3DFVF_TEX2;
    m_d3d_init_hook_sub_408DE0->get_original<decltype(init_d3d9_sub_408DE0)>()();
    //return m_d3d_init_hook_sub_408DE0->get_original<decltype(init_d3d9_sub_408DE0)>()();
    wi::graphics::GraphicsDevice* dev = wi::graphics::GetDevice();

    HWND game_window = *(HWND*)0x00832DB4;
    application.SetWindow(game_window);
    //application.Initialize();
    // just show some basic info:
    application.infoDisplay.active = true;
    application.infoDisplay.watermark = true;
    application.infoDisplay.resolution = true;
    application.infoDisplay.fpsinfo = true;
    if (!application.initialized) {

        auto& shaderPath = wi::renderer::GetShaderSourcePath();
        wi::renderer::SetShaderSourcePath(wi::helper::GetCurrentPath() + "/");

        if(!wi::renderer::LoadShader(wi::graphics::ShaderStage::VS, debug_vs, "debugVS.cso")) {
            printf("error compiling debugVS.cso");
        }
        if (!wi::renderer::LoadShader(wi::graphics::ShaderStage::PS, debug_ps, "debugPS.cso")) {
            printf("error compiling debugPS.cso");
        }

        wi::renderer::SetShaderSourcePath(shaderPath);

        application.Initialize();
        application.initialized = true;
    }
    if(!create_device_objects()) {
        printf("error creating device objects");
    }
    #if 0
    std::thread wm_handler([]() {
        MSG msg = { 0 };
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else {

                application.Run(); // run the update - render loop (mandatory)

            }
        }
        });
    wm_handler.detach();
    #endif
    return m_d3d_init_hook_sub_408DE0->get_original<decltype(init_d3d9_sub_408DE0)>()();
#endif
}

void RendererReplace::init_d3d9_sub_408DE0() noexcept
{
    g_rreplace->init_d3d9_sub_408DE0_internal();
}

// during load
//void RendererReplace::on_config_load(const utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_load(cfg);
//	}
//}
// during save
//void RendererReplace::on_config_save(utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_save(cfg);
//	}
//}
// do something every frame
//void RendererReplace::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void RendererReplace::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
//void RendererReplace::on_draw_ui() {}
