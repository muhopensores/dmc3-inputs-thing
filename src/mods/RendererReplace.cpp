#include "RendererReplace.hpp"
//#include "WickedEngine.h"
#include "D3dx9math.h"
#include "meshoptimizer.h"
#include "numeric"
#include <set>

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
    INT base_vertex_index {};
    UINT min_vertex_index {};
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

#if 1
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
#endif

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



std::optional<std::string> RendererReplace::on_initialize() {
    // WARNING(): dirty hack to only init once here:
    static bool init = false;
    if (init) {
        return Mod::on_initialize();
    }
    init = true;
    g_rreplace = this;
    return Mod::on_initialize();
    m_r_prep_mesh_datas_006DE498 = std::make_unique<FunctionHook>(0x006DE498, &r_prep_mesh_datas);
    if (!m_r_prep_mesh_datas_006DE498->create()) {
        return "failed to hook prep mesh datas";
    }


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
    ImGui::InputInt("DIP base_vertex_index", &g_temp_dip_things.base_vertex_index);
    ImGui::InputInt("DIP min_vertex_index", (int*)&g_temp_dip_things.min_vertex_index);
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