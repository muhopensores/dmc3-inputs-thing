#include "RendererReplace.hpp"
// #include "WickedEngine.h"
#include "D3dx9math.h"
#include "meshoptimizer.h"
#include "numeric"
#include "utility/MemArena.hpp"
#include <array>
#include <cstring>
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

static RendererReplace* g_rreplace{nullptr};

static bool g_3d_draw_cmd{false};

class DrawCommandMaybe {
public:
    class DrawCommandMaybe* m_ptr_next_draw_cmd;           // 0x0000
    class MeshDrawDataMaybe* m_ptr_mesh_draw_data;         // 0x0004
    uint32_t unk;                                          // 0x0008
    class SomeOtherTextureData* m_ptr_other_texture_stuff; // 0x000C
    class ZAndScrBlend* m_ptr_z_and_blendstate;            // 0x0010
    class MvpMatrix* m_ptr_mvp_matrix;                     // 0x0014
    void* m_unk_ptr;                                       // 0x0018
    class TextureData* m_ptr_texture_data;                 // 0x001C
    uint32_t unk_uint32;                                   // 0x0020
    uint32_t unk_uint32_0;                                 // 0x0024
    char pad_0028[32];                                     // 0x0028
}; // Size: 0x0048

class MeshDrawDataMaybe {
public:
    class MeshDrawDataMaybe* m_next_mesh_draw_data; // 0x0000
    uint32_t start_vertex;                          // 0x0004
    uint32_t prim_count;                            // 0x0008
}; // Size: 0x000C

class MvpMatrix {
public:
    Matrix4x4 model; // 0x0000
    Matrix4x4 view;  // 0x0040
    Matrix4x4 proj;  // 0x0080
}; // Size: 0x00C0

class ZAndScrBlend {
public:
    bool unk_bool_0;   // 0x0000
    bool unk_bool_1;   // 0x0001
    bool unk_bool_2;   // 0x0002
    bool unk_bool_3;   // 0x0003
    char pad_0004[64]; // 0x0004
}; // Size: 0x0044

#if 0
class TextureData
{
public:
    char pad_0000[4]; //0x0000
}; //Size: 0x0004
#endif

class SomeOtherTextureData {
public:
    char pad_0000[4]; // 0x0000
}; // Size: 0x0004
static DWORD g_vertices{0};
struct D3DDevicePtr {
    IDirect3DDevice9* device;
};

struct TempDIPThings {
    INT base_vertex_index{0};
    UINT min_vertex_index{0};
    UINT num_vertices{30438};
    UINT start_index{3321};
    UINT prim_count{24};
};

static TempDIPThings g_temp_dip_things{};
std::unordered_map<int, int> g_index_lookup_table;

static void d3d_draw_prim_wrapper(std::vector<std::pair<DWORD, DWORD>>& batches) {
    static D3DDevicePtr* device_ptr  = (D3DDevicePtr*)0x0252F374;
    static IDirect3DDevice9* pdevice = device_ptr->device;
    d3d_vbuffer* static_vbuffer      = (d3d_vbuffer*)0x0252FFA8;

    // std::reverse(batches.begin(), batches.end());
    DWORD primitve_count = std::accumulate(batches.begin(), batches.end(), 0, [](DWORD acc, const std::pair<DWORD, DWORD>& pair) {
        return acc + pair.second;
    });
    UINT numvertices     = batches.back().first - batches.front().first;
    if (static_vbuffer->pindexbuffer) {
        pdevice->SetIndices(static_vbuffer->pindexbuffer);
    }
    UINT index_count = 0;
    for (const auto& batch : batches) {
        // HRESULT result = pdevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, batch.first, batch.second);
        HRESULT result = pdevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, batch.first, 0, batch.second, 0, batch.second);
        assert(result == S_OK);
        // printf("hehe\n");
    }
    // HRESULT result = pdevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, batches.front().first, primitve_count);
}
void d3d_emplace_back_triangles(std::vector<std::pair<DWORD, DWORD>>& batches, UINT vertexCount, UINT primitve_count, char a3) {
    typedef int (*d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t)(int a1);
    d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0 = (d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t)0x006E1C00;
    DWORD benis                                                                                                                       = *(DWORD*)0x0252FFF0;

    if ((vertexCount & 3) == benis ||
        d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0(vertexCount & 3) >= 0) {
        DWORD start_vertex = vertexCount >> 2;
        if (a3) {
            auto v5 = 2;
            if (start_vertex < v5) {
                return;
            }
            start_vertex -= v5;
            primitve_count += v5;
        }
        batches.emplace_back(start_vertex, primitve_count - 2);
        // pdevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, start_vertex, primitve_count - 2);
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
    d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0 = (d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0_t)0x006E1C00;
    DWORD benis                                                                                                                       = *(DWORD*)0x0252FFF0;
    static D3DDevicePtr* device_ptr                                                                                                   = (D3DDevicePtr*)0x0252F374;
    static IDirect3DDevice9* pdevice                                                                                                  = device_ptr->device;

    if ((vertexCount & 3) == benis ||
        d3d_switch_on_buffer_types_copy_data_and_lock_dmc3se_sub_6E18F0(vertexCount & 3) >= 0) {
        DWORD start_vertex = vertexCount >> 2;
        if (a3) {
            auto v5 = 2;
            if (start_vertex < v5) {
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

    DWORD start_vertex{mdd->start_vertex};
    DWORD prim_count{mdd->prim_count};
    MeshDrawDataMaybe* m_next_draw_data               = mdd->m_next_mesh_draw_data;
    static constexpr void* d3d_draw_prim_func_address = &d3d_draw_primitive_maybe_sub_6E1C00;

    std::vector<std::pair<DWORD, DWORD>> batches;
#if 1
    do {
        if ((start_vertex & 3) != 1) {
            d3d_draw_primitive_maybe_sub_6E1C00(start_vertex, prim_count, a3);
            batches.clear();
        } else {
            d3d_emplace_back_triangles(batches, start_vertex, prim_count, a3);
        }
        if (m_next_draw_data) {
            start_vertex     = m_next_draw_data->start_vertex;
            prim_count       = m_next_draw_data->prim_count;
            m_next_draw_data = m_next_draw_data->m_next_mesh_draw_data;
        }
    } while (m_next_draw_data != NULL);
    if (batches.size() > 0) {
        d3d_draw_prim_wrapper(batches);
    }
#else
    do {
        d3d_draw_primitive_maybe_sub_6E1C00(start_vertex, prim_count, a3);
        if (m_next_draw_data) {
            start_vertex     = m_next_draw_data->start_vertex;
            prim_count       = m_next_draw_data->prim_count;
            m_next_draw_data = m_next_draw_data->m_next_mesh_draw_data;
        }
    } while (m_next_draw_data != NULL);
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

class VertexDef {
public:
    Vector3 pos;    // 0x0000
    Vector3 norm;   // 0x000C
    uint8_t diff_r; // 0x0018
    uint8_t diff_g; // 0x0019
    uint8_t diff_b; // 0x001A
    uint8_t diff_a; // 0x001B
    uint8_t spec_r; // 0x001C
    uint8_t spec_g; // 0x001D
    uint8_t spec_b; // 0x001E
    uint8_t spec_a; // 0x001F
    Vector2 uv0;    // 0x0020
    Vector2 uv1;    // 0x0028
    Vector2 uv2;    // 0x0030
}; // Size: 0x0038

#pragma region cdraw_reclass
// Created with ReClass.NET 1.2 by KN4CK3R

class cDrawSub {
public:
    class ModTansformsMatrices* obj_transforms_matrices_ptr;        // 0x0000
    uint8_t* skel_hierarchy;                                        // 0x0004
    uint8_t* skel_indicies_ptr;                                     // 0x0008
    uint8_t* skel_object_roots;                                     // 0x000C
    class ModTransformVec* skel_bones_ptr;                          // 0x0010
    class IDraw* IDrawVtable;                                       // 0x0014
    class IDrawOperate* IDrawOperateVtable;                         // 0x0018
    char pad_001C[132];                                             // 0x001C
    uint32_t draw_flag_or_something;                                // 0x00A0
    uint32_t ass;                                                   // 0x00A4
    uint16_t object_count;                                          // 0x00A8
    uint16_t bone_count;                                            // 0x00AA
    uint16_t tex_count;                                             // 0x00AC
    uint16_t idk3;                                                  // 0x00AE
    uint32_t unk1;                                                  // 0x00B0
    uint32_t unk2;                                                  // 0x00B4
    uint16_t unk3;                                                  // 0x00B8
    uint16_t unk4;                                                  // 0x00BA
    class SomeModelData* modnrender_data_ptr;                       // 0x00BC
    class rModel* model_ptr;                                        // 0x00C0
    class rTim2* texture_ptr;                                       // 0x00C4
    class TextureStuff0 (*texture_stuff0_ptr)[5];                   // 0x00C8
    class TextureStuff1* texture_stuff1_ptr;                        // 0x00CC
    class cDrawSubUnk2* cdrawsub2_unk0_ptr;                         // 0x00D0
    class cDrawSubUnk2* cdrawsub2_unk1_ptr;                         // 0x00D4
    class cDrawSubUnk2* cdrawsub2_unk2_ptr;                         // 0x00D8
    class cDrawSubUnk2* cdrawsub2_unk3_ptr;                         // 0x00DC
    class cDrawSubUnk2* cdrawsub2_unk4_ptr;                         // 0x00E0
    class cDrawSubUnk2* cdrawsub2_unk5_ptr;                         // 0x00E4
    class SomeTextureUintsIdk (*some_texture_uints_ptr)[2];         // 0x00E8
    class cDrawSubUnk6* cdrawsub_unk6_ptr;                          // 0x00EC
    class SomeTextureAlphaThing (*some_texture_alpha_thing_ptr)[1]; // 0x00F0
    class SomeTextureAlphaThing* some_texture_alpha_thing_ptr1;     // 0x00F4
    Matrix4x4* bone_matrices_ptr;                                   // 0x00F8
    class cDrawSubUnk7* cdrawsub_unk7_ptr;                          // 0x00FC
    Matrix4x4* some_matrix_ptr;                                     // 0x0100
    class UnkVecMaybe* some_ptr_vec_maybe;                          // 0x0104
    char pad_0108[8];                                               // 0x0108
    Matrix4x4 some_matrix0;                                         // 0x0110
    Matrix4x4 some_matrix1;                                         // 0x0150
}; // Size: 0x0190

class cDrawSCM {
public:
    char pad_0000[44];        // 0x0000
    class cDrawSub N0000005A; // 0x002C
    char pad_01BC[4116];      // 0x01BC
}; // Size: 0x11D0

class rTim2 {
public:
    char pad_0000[2048]; // 0x0000
    char tim2header[4];  // 0x0800
    char pad_0804[2048]; // 0x0804
}; // Size: 0x1004

class rModelObjectHeaders {
public:
    uint8_t meshCount;              // 0x0000
    uint8_t unk;                    // 0x0001
    uint16_t vertCount;             // 0x0002
    class rModelMesh* objectOffset; // 0x0004
    uint32_t objFlags;              // 0x0008
    uint8_t reserved[20];           // 0x000C
    Vector3 spherePos;              // 0x0020
    float radius;                   // 0x002C
}; // Size: 0x0030

class rModel {
public:
    char modfourcc[4];                   // 0x0000
    float version;                       // 0x0004
    uint64_t reserved;                   // 0x0008
    uint8_t objectCount;                 // 0x0010
    uint8_t boneCount;                   // 0x0011
    uint8_t texCount;                    // 0x0012
    uint8_t unk[9];                      // 0x0013
    uint32_t skelOffset;                 // 0x001C
    char pad_0020[16];                   // 0x0020
    class rModelObjectHeaders N00000E39; // 0x0030
    char pad_0060[220];                  // 0x0060
}; // Size: 0x013C

class cDraw {
public:
    char pad_0000[8];                 // 0x0000
    uint32_t some_field;              // 0x0008
    uint32_t pad;                     // 0x000C
    uint32_t draw_flag;               // 0x0010
    class rTim2* tim2;                // 0x0014
    uint32_t pad1;                    // 0x0018
    void* some_allocator_type_thing1; // 0x001C
    void* some_allocator_type_thing;  // 0x0020
    uint32_t notSure;                 // 0x0024
    uint32_t notSure1;                // 0x0028
    class cDrawSub cDrawSub;          // 0x002C
}; // Size: 0x01BC

class ModTransformVec {
public:
    Vector4 pos; // 0x0000
    Vector4 rot; // 0x0010
}; // Size: 0x0020

class ModTansformsMatrices {
public:
    Matrix4x4 unk;                    // 0x0000
    Matrix4x4 world_maybe;            // 0x0040
    class ModTansformsMatrices* next; // 0x0080
    uint32_t idk;                     // 0x0084
}; // Size: 0x0088

class TextureStuff1 {
public:
    char pad_0000[4100]; // 0x0000
}; // Size: 0x1004

class TextureStuff0 {
public:
    void* some_texture_offset; // 0x0000
    uint32_t field_04;         // 0x0004
    uint32_t field_08;         // 0x0008
    uint32_t field_0C;         // 0x000C
    uint32_t field_10;         // 0x0010
    uint32_t field_14;         // 0x0014
    uint32_t field_18;         // 0x0018
    uint32_t field_1C;         // 0x001C
    uint32_t field_20;         // 0x0020
    uint32_t field_24;         // 0x0024
    uint32_t field_28;         // 0x0028
    uint32_t field_2c;         // 0x002C
}; // Size: 0x0030

class UnkVecMaybe {
public:
    Vector4 maybe_vec; // 0x0000
}; // Size: 0x0010

class SomeTextureUintsIdk {
public:
    uint32_t field0;  // 0x0000
    uint32_t field4;  // 0x0004
    uint32_t field8;  // 0x0008
    uint32_t fieldC;  // 0x000C
    char pad_0010[8]; // 0x0010
    uint32_t field18; // 0x0018
    uint32_t field1C; // 0x001C
    char pad_0020[8]; // 0x0020
    uint32_t field28; // 0x0028
    char pad_002C[4]; // 0x002C
}; // Size: 0x0030

class SomeTextureAlphaThing {
public:
    float flt0;  // 0x0000
    float flt4;  // 0x0004
    float flt8;  // 0x0008
    float fltC;  // 0x000C
    float flt10; // 0x0010
    float flt14; // 0x0014
    float flt18; // 0x0018
    float flt1C; // 0x001C
}; // Size: 0x0020

class rModelHeader {
public:
    char fourcc[4];                  // 0x0000
    float version;                   // 0x0004
    uint64_t reserved;               // 0x0008
    uint8_t objectCount;             // 0x0010
    uint8_t boneCount;               // 0x0011
    uint8_t texCount;                // 0x0012
    uint8_t unk[9];                  // 0x0013
    void* skelOffs;                  // 0x001C
    void* someOffset;                // 0x0020
    void* someOtherOffset;           // 0x0024
    void* someOtherOtherOffset;      // 0x0028
    void* someOtherOtherOtherOffset; // 0x002C
}; // Size: 0x0030

class rModelSCM : public rModelHeader {
public:
    class rModelObjectHeaders objects[114]; // 0x0030 size = header.objectCount
    char pad_1590[5524];                    // 0x1590
}; // Size: 0x2B24

class rModelMeshHdr {
public:
    uint16_t vertCount;                 // 0x0000
    uint16_t texInd;                    // 0x0002
    uint64_t reserved;                  // 0x0004
    Vector3* vertsOffset;               // 0x000C
    Vector3* normalsOffs;               // 0x0010
    class rModelUVData* uvsOffset;      // 0x0014
    void* someOffset;                   // 0x0018
    void* someOtherOffset;              // 0x001C
    class rModelSCMData* scmDataOffset; // 0x0020
    uint32_t idk00;                     // 0x0024
    uint32_t idk01;                     // 0x0028
    uint32_t idk02;                     // 0x002C
}; // Size: 0x0030

class rModelMesh : public rModelMeshHdr {
public:
}; // Size: 0x0030

class Object : public rModelMesh {
public:
}; // Size: 0x0030

class rModelUVData {
public:
    int16_t u; // 0x0000
    int16_t t; // 0x0002
}; // Size: 0x0004

class rModelVertColor {
public:
    uint8_t red;   // 0x0000
    uint8_t green; // 0x0001
    uint8_t blue;  // 0x0002
}; // Size: 0x0003

class rModelSCMData : public rModelVertColor {
public:
    uint8_t triSkipFlag; // 0x0003
}; // Size: 0x0004

class rModelBone {
public:
    Vector3 pos;    // 0x0000
    float length;   // 0x000C
    Vector3 unkVec; // 0x0010
    float unk;      // 0x001C
}; // Size: 0x0020

class rModelSkeleton {
public:
    void* hierarchyOffset;           // 0x0000
    void* indexOffset;               // 0x0004
    void* objectRootsOffset;         // 0x0008
    void* boneTransformOffset;       // 0x000C
    uint32_t notBoneCount;           // 0x0010
    uint8_t unk[12];                 // 0x0014
    uint8_t hierarchy[116];          // 0x0020 length == notBoneCount?
    uint8_t indicies[116];           // 0x0094
    uint8_t objectRoots[116];        // 0x0108
    uint32_t reserved;               // 0x017C
    class rModelBone N000010A8[115]; // 0x0180
}; // Size: 0x0FE0

class SomeRenderDataIdkAnonStruct {
public:
    uint32_t some_int;                     // 0x0000
    uint32_t N00001DEE;                    // 0x0004
    uint32_t eeee_int;                     // 0x0008
    char pad_000C[4];                      // 0x000C
    uint32_t some_table_lookup_int0;       // 0x0010
    uint32_t some_table_lookup_int1;       // 0x0014
    uint32_t n_int;                        // 0x0018
    char pad_001C[4];                      // 0x001C
    uint32_t table_lookup_int2;            // 0x0020
    uint32_t table_lookup_int3;            // 0x0024
    uint32_t g_int;                        // 0x0028
    uint32_t N00001DF8;                    // 0x002C
    uint32_t some_int1;                    // 0x0030
    uint32_t N00001392_;                   // 0x0034
    uint32_t b_int;                        // 0x0038
    uint32_t N00001DFC;                    // 0x003C
    uint32_t some_obj_flags_bitwise_thing; // 0x0040
    uint32_t some_int_unk;                 // 0x0044
    char pad_0048[4];                      // 0x0048
    uint32_t N00001E07;                    // 0x004C
}; // Size: 0x0050

class SmdScratchBufferMaybe {
public:
    char pad_0000[144]; // 0x0000
}; // Size: 0x0090

class SomeModelData {
public:
    uint32_t fld0;                                         // 0x0000
    uint8_t flag0;                                         // 0x0004
    uint8_t flag1;                                         // 0x0005
    uint8_t flag2;                                         // 0x0006
    uint8_t flag3;                                         // 0x0007
    uint32_t vert_accumulator;                             // 0x0008
    uint8_t mesh_count;                                    // 0x000C
    uint8_t N000019A8;                                     // 0x000D
    uint8_t some_skel_stuff;                               // 0x000E
    uint8_t N000019A9;                                     // 0x000F
    uint32_t N00001380;                                    // 0x0010
    uint32_t some_rendering_flags_again;                   // 0x0014
    class rModelObjectHeaders* r_model_obj_headers_ptr;    // 0x0018
    class SomeRenderingData* ptr_has_some_rendering_data;  // 0x001C
    Vector4 sphere;                                        // 0x0020
    class cDrawSubUnk2* cdrawsubs_array[6];                // 0x0030
    uint32_t N0000138B;                                    // 0x0048
    uint32_t N0000138C;                                    // 0x004C
    uint32_t some_table_lookup_int0;                       // 0x0050
    uint32_t some_table_lookup_int1;                       // 0x0054
    uint32_t some_table_lookup_int2;                       // 0x0058
    uint32_t some_table_lookup_int3;                       // 0x005C
    uint32_t some_int1;                                    // 0x0060
    uint32_t N00001392;                                    // 0x0064
    uint32_t some_obj_flags_bitwise_thing;                 // 0x0068
    uint32_t some_int_unk;                                 // 0x006C
    uint32_t some_result_int;                              // 0x0070
    char pad_0074[12];                                     // 0x0074
    class SomeRenderDataIdkAnonStruct some_anon_struct0;   // 0x0080
    class SomeRenderDataIdkAnonStruct some_anon_struct1;   // 0x00D0
    uint32_t N000013C1;                                    // 0x0120
    uint32_t N000013C2;                                    // 0x0124
    uint32_t N000013C3;                                    // 0x0128
    uint32_t N000013C4;                                    // 0x012C
    Vector4 emissive;                                      // 0x0130
    uint32_t N000013C9;                                    // 0x0140
    uint32_t N000013CA;                                    // 0x0144
    uint32_t N000013CB;                                    // 0x0148
    uint32_t some_flag0;                                   // 0x014C
    char pad_0150[16];                                     // 0x0150
    Vector4 emissive_copy;                                 // 0x0160
    char pad_0170[12];                                     // 0x0170
    uint32_t some_flag1;                                   // 0x017C
    char pad_0180[304];                                    // 0x0180
    class SmdScratchBufferMaybe some_scratch_buffer_maybe; // 0x02B0
}; // Size: 0x0340

class cDrawSubUnk2 {
public:
    uint32_t f0;                                           // 0x0000
    class cDrawSubUnk2* cdsu_next_ptr;                     // 0x0004
    uint32_t f8;                                           // 0x0008
    uint32_t fc;                                           // 0x000C
    uint32_t f10;                                          // 0x0010
    class SomeTextureAlphaThing* some_tex_alpha_thing_ptr; // 0x0014
    uint32_t some_static_memory_stuff;                     // 0x0018
    uint32_t f1c;                                          // 0x001C
    uint32_t field_20;                                     // 0x0020
    Matrix4x4* projection_maybe;                           // 0x0024
    uint32_t field_28;                                     // 0x0028
    uint32_t field_2C;                                     // 0x002C
}; // Size: 0x0030

class SomeRenderingData {
public:
    uint32_t batches_maybe;                      // 0x0000
    uint32_t vert_count;                         // 0x0004
    uint32_t tex_ind;                            // 0x0008
    class rModelMeshHdr* model_mesh_hdr_ptr;     // 0x000C
    uint32_t offset_0x10;                        // 0x0010
    uint32_t offset_0x14;                        // 0x0014
    uint32_t offset_0x18;                        // 0x0018
    uint32_t offset_0x1C;                        // 0x001C
    uint32_t offset_0x20;                        // 0x0020
    uint32_t offset_0x24;                        // 0x0024
    uint32_t offset_0x28;                        // 0x0028
    uint32_t offset_0x2C;                        // 0x002C
    uint8_t offset_0x30;                         // 0x0030
    char pad_0031[3];                            // 0x0031
    uint32_t bone_count;                         // 0x0034
    Matrix4x4* world;                            // 0x0038
    char pad_003C[4];                            // 0x003C
    class SomeRenderDataIdkAnonStruct N00001A0B; // 0x0040
    class SomeRenderDataIdkAnonStruct N00001A0C; // 0x0090
    uint32_t offset_0xE0;                        // 0x00E0
    uint32_t offset_0xE4;                        // 0x00E4
    uint32_t offset_0xE8;                        // 0x00E8
    uint32_t offset_0xEC;                        // 0x00EC
    void* anon_struct0;                          // 0x00F0
    void* anon_struct1;                          // 0x00F4
    Vector3* verts_ptr0;                         // 0x00F8
    Vector3* verts_ptr1;                         // 0x00FC
    Vector3* normals_ptr;                        // 0x0100
    Vector3* normals_ptr1;                       // 0x0104
    class rModelUVData* uvdata_ptr;              // 0x0108
    class rModelUVData* uvdata_ptr1;             // 0x010C
    class rModelBoneIndices* boindata_ptr0;      // 0x0110
    class rModelBoneIndices* boindata_ptr1;      // 0x0114
    class rModelWeightsData* weightdata_ptr0;    // 0x0118
    class rModelWeightsData* weightdata_ptr1;    // 0x011C
    class rModelSCMData* scmdata_ptr;            // 0x0120
    class rModelSCMData* scmdata_ptr1;           // 0x0124
    class RenderUVFudge* render_uv_fudge0;       // 0x0128
    class RenderUVFudge* render_uv_fudge1;       // 0x012C
    Vector3* verts_ptr2;                         // 0x0130
}; // Size: 0x0134

class RenderUVFudge {
public:
    int32_t some_uvx_fudge; // 0x0000
    int32_t some_uvy_fude;  // 0x0004
    uint32_t N00001CC7;     // 0x0008
    uint32_t N00001CC8;     // 0x000C
    uint32_t N00001CC9;     // 0x0010
    char pad_0014[4];       // 0x0014
}; // Size: 0x0018

class cDrawSubUnk6 {
public:
    char pad_0000[260]; // 0x0000
}; // Size: 0x0104

class cDrawSubUnk7 {
public:
    char pad_0000[4]; // 0x0000
}; // Size: 0x0004

class IDraw {
public:
    void* N00001BDA[57]; // 0x0000
}; // Size: 0x00E4

class IDrawOperate {
public:
    void* N00001F51[45]; // 0x0000
}; // Size: 0x00B4

class N00002118 {
public:
    Matrix4x4 N00002119; // 0x0000
    char pad_0040[288];  // 0x0040
}; // Size: 0x0160

class rModelWeightsData {
public:
    uint16_t bone_weights; // 0x0000
}; // Size: 0x0004

class SomeStackThing {
public:
    class SomeRenderingData* p_some_rendering_data; // 0x0000
    Matrix4x4* p_matrix;                            // 0x0004
    uint8_t some_uints0[4];                         // 0x0008
    uint8_t some_uints1[4];                         // 0x000C
    char pad_0010[4];                               // 0x0010
    class N00001713* p_c_light_mgr;                 // 0x0014
    char pad_0018[296];                             // 0x0018
}; // Size: 0x0140

class rModelBoneIndices {
public:
    uint8_t bn_ind[4]; // 0x0000
}; // Size: 0x0004
#pragma endregion

struct IndexData {
    std::int32_t ref_count{};
    std::uint32_t prim_count;
    std::uint32_t index_count;
    std::vector<uint16_t> ib;
};

IndexData make_index_buffer(SomeRenderingData* srd) {
    assert(srd->vert_count);
#ifndef NDEBUG
    // printf("make_index_buffer() vert_count:%d\n", srd->vert_count);
#endif // !NDEBUG

    IndexData result;
    result.prim_count = 0;

    std::uint16_t p1 = 0, p2 = 1;
    bool wnd = 1; // winding order
    std::vector<uint16_t> indices;
    indices.reserve(srd->vert_count * 3);

    for (uint32_t i = 2; i < srd->vert_count; ++i) {
        std::uint16_t p3 = i;

        if (p1 >= srd->vert_count || p2 >= srd->vert_count || p3 >= srd->vert_count) {
            printf("Bad index detected: %d, %d, %d\n", p1, p2, p3);
        }

        bool tri_skip{false};
        if (srd->scmdata_ptr) { // static meshes
            tri_skip = srd->scmdata_ptr[p3].triSkipFlag;
            if (srd->weightdata_ptr0) {
                tri_skip = ((srd->weightdata_ptr0[p3].bone_weights >> 0xF) & 1);
            }
        } else { // stored in bone weight for characters
            tri_skip = ((srd->weightdata_ptr0[p3].bone_weights >> 0xF) & 1);
        }
        if (!tri_skip) {
            // compute triangle normal and winding order
            Vector3 v1 = srd->verts_ptr0[p1];
            Vector3 v2 = srd->verts_ptr0[p2];
            Vector3 v3 = srd->verts_ptr0[p3];

            Vector3 face_edge1 = v3 - v1;
            Vector3 face_edge2 = v2 - v1;
            face_edge1         = glm::normalize(face_edge1);
            face_edge2         = glm::normalize(face_edge2);

            Vector3 z = glm::cross(face_edge1, face_edge2);
            z         = glm::normalize(z);

            Vector3 normal1 = srd->normals_ptr[p1];
            Vector3 normal2 = srd->normals_ptr[p2];
            Vector3 normal3 = srd->normals_ptr[p3];
            Vector3 normal  = glm::normalize(normal1 + normal2 + normal3);

            // Add indices in the correct winding order
            // wnd = glm::dot(normal, z) > 0.0f;
            // ratmix normals look inverted with code above for some reason
            wnd = glm::dot(normal, z) < 0.0f;
            if (wnd) {
                indices.push_back(p1);
                indices.push_back(p3);
                indices.push_back(p2);
            } else {
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
    // srd->starting_index = num_indices;
    result.index_count = num_indices;
    result.ib          = std::move(indices);
    return result;
}

void cdraw_set_render_data_hook(SomeRenderingData* srd) {
    if (!srd) {
        return;
    }
    assert(srd->vert_count);
    // srd->starting_index = 0xDEADBEEF; // test

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

struct RDrawSomethingStruct {
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
    Matrix4x4* p_some_matrix;
    struct SomeRenderingData* srd;
    uint32_t some_flags;
};

size_t g_vertices_offset{0};
size_t g_indices_offset{0};

enum EVertexBufferType {
    ARTICULATED_MODELS = 1, // unused i think
    GUI_QUADS          = 2,
    WORLD              = 3,
};

struct MatrixIndices {
    union {
        BYTE index[4];
        DWORD indices;
    };
};
static_assert(sizeof(MatrixIndices) == sizeof(DWORD));

struct VertexDefDynamic {
    Vector3 pos;
    Vector3 weight;
    MatrixIndices mat_indices;
    Vector3 normal;

    uint8_t diff_r; // 0x0018
    uint8_t diff_g; // 0x0019
    uint8_t diff_b; // 0x001A
    uint8_t diff_a; // 0x001B
    uint8_t spec_r; // 0x001C
    uint8_t spec_g; // 0x001D
    uint8_t spec_b; // 0x001E
    uint8_t spec_a; // 0x001F

    Vector2 uv0; // 0x0020
};

// static_assert(sizeof(VertexDefDynamic) == sizeof(VertexDef));
//  TODO(): find where to stick this
// static constexpr DWORD vertex_format_dmc3_3d = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 | D3DFVF_TEX2;
static constexpr DWORD vertex_format_dmc3_3d_ffp_skinning = D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;

struct DIPData {
    std::uint32_t vertex_count_ours;
    std::uint32_t primitive_count_ours;
    std::uint32_t index_count_ours;
    std::uint32_t base_vertex_index;
    std::uint32_t start_index;
    Matrix4x4* matrices{};
    Matrix4x4* world{};
    std::uint16_t bone_count;
    std::uint16_t source_bone_count;
    std::vector<uint8_t> palette_to_global;
    // std::vector<std::uint16_t> index_buffer_ours;
    // std::vector<VertexDef> vertex_buffer_ours;

    // LPDIRECT3DVERTEXBUFFER9 p_vb = NULL;
    // LPDIRECT3DINDEXBUFFER9  p_ib = NULL;
};

#if 0
static void release_buffers(DIPData& dip) {
    if (dip.p_vb) {
        dip.p_vb->Release();
        dip.p_vb = nullptr;
    }
    if (dip.p_ib) {
        dip.p_ib->Release();
        dip.p_ib = nullptr;
    }
}

static void create_buffers(DIPData& dip, D3DPOOL pool = D3DPOOL_DEFAULT) {
    static D3DDevicePtr* device_ptr  = (D3DDevicePtr*)0x0252F374;
    static IDirect3DDevice9* pdevice = device_ptr->device;
    // (re)create vertex and idex buffer
    {
        // release existing
        release_buffers(dip);

        // create vb
        if (FAILED(pdevice->CreateVertexBuffer(dip.vertex_buffer_ours.size() * sizeof(VertexDef),
                                               0, vertex_format_dmc3_3d, pool, &dip.p_vb, NULL))) {
            assert("create_vb failed!");
            return;
        }
        // create ib
        if (FAILED(pdevice->CreateIndexBuffer(dip.index_buffer_ours.size() * sizeof(WORD),
                                              0, D3DFMT_INDEX16, pool, &dip.p_ib, NULL))) {
            assert("create_ib failed!");
            return;
        }
    }
    // upload data to gpu
    {
        assert(dip.p_vb);
        // upload vert data
        void* pvertices;
        const size_t vbsize = dip.vertex_buffer_ours.size() * sizeof(VertexDef);
        if (FAILED(dip.p_vb->Lock(0, vbsize, (void**)&pvertices, 0))) {
            return;
        }
        memcpy(pvertices, dip.vertex_buffer_ours.data(), vbsize);
        dip.p_vb->Unlock();

        assert(dip.p_ib);
        // upload index data
        void* pindices;
        const size_t ibsize = dip.index_buffer_ours.size() * sizeof(WORD);
        if (FAILED(dip.p_ib->Lock(0, ibsize, (void**)&pindices, 0))) {
            return;
        }
        memcpy(pindices, dip.index_buffer_ours.data(), ibsize);
        dip.p_ib->Unlock();
    }
}
#endif

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
                                                   0, vertex_format_dmc3_3d_ffp_skinning, D3DPOOL_DEFAULT, &p_vb, NULL))) {
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
        m_vb_cur_size  = 0;
        m_ib_cur_size  = 0;
        m_is_dirty     = true;
        m_create_flags = CREATE_FLAGS::NONE;
    }

    void upload(std::vector<VertexDefDynamic>& verts, std::vector<WORD>& inds) {
        if (m_is_dirty == false) {
            return;
        }
        assert(p_vb);
        // upload vert data
        void* pvertices;
        const size_t vbsize = verts.size() * sizeof(VertexDefDynamic);
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
            return;
        }
        memcpy(pindices, inds.data(), ibsize);
        p_ib->Unlock();
        m_is_dirty = false;
    }

    void check_and_flag_for_resize(size_t verts, size_t indexs) {
        m_vb_cur_size += (verts * sizeof(VertexDefDynamic));
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
            new_vert_capacity = round_to_multiple(new_vert_capacity, sizeof(VertexDefDynamic));
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
        NONE          = 0,
        VERTEX_BUFFER = 1 << 0,
        INDEX_BUFFER  = 1 << 1,
    };
    uint32_t m_create_flags = NONE;

    bool m_is_dirty = true;

    LPDIRECT3DVERTEXBUFFER9 p_vb = NULL;
    LPDIRECT3DINDEXBUFFER9 p_ib  = NULL;

    size_t m_vb_cur_size{0};
    size_t m_ib_cur_size{0};

    size_t m_vb_capacity{0};
    size_t m_ib_capacity{0};
};

static DynamicRenderBuffer* g_dyn_ren_buf{nullptr};

static std::map<size_t, IndexData> g_ib_map;
static int g_current_obj{-1};
static int g_skipdraw{-1};

static uint16_t get_ffp_palette_limit(IDirect3DDevice9* pdevice);

std::vector<VertexDefDynamic> g_vertex_data_world;
std::vector<WORD> g_index_data_world;

// TODO() imgui remove
bool g_skipdraw_static{false};

int g_bcount{24};
int g_bindex_0{0};
int g_bindex_1{0};
int g_bindex_2{1};
int g_bindex_3{2};

static constexpr std::array<std::array<int, 4>, 6> g_bindex_presets{{
    {{0, 0, 1, 2}},
    {{0, 1, 2, 0}},
    {{0, 1, 2, 2}},
    {{2, 1, 3, 0}},
    {{2, 1, 0, 3}},
    {{3, 2, 1, 0}},
}};
static int g_bindex_preset_idx = 0;

static void apply_bindex_preset(int preset_index) {
    if (preset_index < 0 || preset_index >= (int)g_bindex_presets.size()) {
        return;
    }
    g_bindex_preset_idx = preset_index;
    const auto& p       = g_bindex_presets[(size_t)preset_index];
    g_bindex_0          = p[0];
    g_bindex_1          = p[1];
    g_bindex_2          = p[2];
    g_bindex_3          = p[3];
}

static void cycle_bindex_preset(int step) {
    const int preset_count = (int)g_bindex_presets.size();
    int next_idx           = g_bindex_preset_idx + step;
    while (next_idx < 0) {
        next_idx += preset_count;
    }
    next_idx %= preset_count;
    apply_bindex_preset(next_idx);
}

static VertexDef* g_current_vertex_data_pointer = 0x0;
static uintptr_t r_malloc_vertex_datas_jmp      = 0x006E15AC;
// clang-format off
static __declspec(naked) void r_malloc_vertex_datas_detour() {
    __asm {
        mov eax, DWORD PTR [g_current_vertex_data_pointer]
        test eax,eax
        jne originalCode
        mov eax,ecx
        //mov dword ptr [g_current_vertex_data_pointer], eax
    originalCode:
        add ecx,38h
        jmp DWORD PTR [r_malloc_vertex_datas_jmp]
    }
}
// clang-format on

static void r_render_world_intercept(RDrawSomethingStruct* rdss) {
    if (!rdss) {
        return;
    }
    if (g_skipdraw_static) {
        return;
    }
    // printf("|rdss->srd->vert_count=%d\n", rdss->srd->vert_count);

    typedef void(__fastcall * rPrepDrawDataSub6DF5A0)(unsigned int a1, int start_offset, int tri_strip_count);
    static rPrepDrawDataSub6DF5A0 r_prep_draw_data_sub_6DF5A0 = (rPrepDrawDataSub6DF5A0)0x6DF5A0;
    static int* some_graphic_struct_dword_252F490_            = (int*)0x0252F490;
    ptrdiff_t start_offset                                    = 0;
    DWORD tri_strip_count                                     = 0;

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

    DIPData* render_info = new DIPData;
    size_t vertex_offset = g_vertex_data_world.size();

    assert(vertex_offset < USHRT_MAX);

    for (size_t resb = 0; resb < rdss->srd->vert_count; resb = (resb + 1)) {
        typedef int(__fastcall * cDrawVertexDataSetSub6DE480)(size_t vert_index, RDrawSomethingStruct * a2);
        static cDrawVertexDataSetSub6DE480 cDraw_vertex_data_set_sub_6DE480 = (cDrawVertexDataSetSub6DE480)0x6DE480;
        VertexDef svtx{}; // visual studio, bro, i know its not initialized
        g_current_vertex_data_pointer = &svtx;
        auto& new_vert                = g_vertex_data_world.emplace_back();
        cDraw_vertex_data_set_sub_6DE480(resb, rdss);
        new_vert.pos    = svtx.pos;
        new_vert.normal = svtx.norm;

        new_vert.diff_a = svtx.diff_a;
        new_vert.diff_r = svtx.diff_r;
        new_vert.diff_g = svtx.diff_g;
        new_vert.diff_b = svtx.diff_b;
        new_vert.spec_a = svtx.spec_a;
        new_vert.spec_r = svtx.spec_r;
        new_vert.spec_g = svtx.spec_g;
        new_vert.spec_b = svtx.spec_b;

        new_vert.mat_indices.indices = 0;
        new_vert.weight.x            = 0.0f;
        new_vert.weight.y            = 0.0f;
        new_vert.weight.z            = 0.0f;

        new_vert.uv0 = svtx.uv0;
        // new_vert.uv1 = svtx.uv1;

        g_current_vertex_data_pointer = 0;
        // memcpy(&render_info->vertex_buffer_ours[resb], g_current_vertex_data_pointer, 0x38);
    }

    IndexData* result{};
#if 0
    size_t hash = std::hash<uintptr_t>()((uintptr_t)rdss->srd->vert_count + (uintptr_t)rdss->srd->tex_ind + (uintptr_t)rdss->srd->model_mesh_hdr_ptr);
    if (auto& it = g_ib_map.find(hash); it != g_ib_map.end()) {
        result = &it->second;
        result->ref_count -= 1;
    } 
    else {
        auto& oops    = g_ib_map.emplace(hash, std::move(ass));
        result        = &oops.first->second;
    }
#endif

    IndexData ass = make_index_buffer(rdss->srd);
    result        = &ass;
    assert(result);

    render_info->index_count_ours     = result->index_count;
    render_info->primitive_count_ours = result->prim_count;
    render_info->base_vertex_index    = vertex_offset; // g_index_data_world.size();
    render_info->start_index          = g_index_data_world.size();

    // render_info->index_buffer_ours    = std::move(result.ib);
    for (WORD index : result->ib) {
        g_index_data_world.push_back(index /* + vertex_offset */);
    }
    render_info->vertex_count_ours = rdss->srd->vert_count;
    result->ref_count -= 1;

    g_dyn_ren_buf->check_and_flag_for_resize(
        render_info->vertex_count_ours,
        render_info->index_count_ours);

    // create_buffers(*render_info);

#if 0
    g_dyn_ren_buf->check_and_flag_for_resize(
        render_info->vertex_buffer_ours.size(),
        render_info->index_buffer_ours.size());
#endif

    r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, g_vertices_offset | (value << 24), (int)render_info);
    g_current_obj += 1;
#if 0
    g_dip_map.emplace(hash, render_info);
#endif

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

static void r_render_chars_intercept(cDrawSub* cds, SomeRenderingData* srd0, RDrawSomethingStruct* rdss, SomeStackThing* stack) {
    assert(srd0);
    assert(srd0->vert_count);

    if ((g_current_obj == g_skipdraw)) {
        return;
    }
    const uint32_t vert_count = srd0->vert_count;

    typedef int(__cdecl * cDrawcDrawSubPrepVertsGuiAndCharacters)(RDrawSomethingStruct * a1, int vert_index, SomeStackThing* a3);
    cDrawcDrawSubPrepVertsGuiAndCharacters prep_verts_gui_and_characters = (cDrawcDrawSubPrepVertsGuiAndCharacters)0x006DD4A0;

    typedef void(__fastcall * rPrepDrawDataSub6DF5A0)(unsigned int a1, int start_offset, int tri_strip_count);
    static rPrepDrawDataSub6DF5A0 r_prep_draw_data_sub_6DF5A0 = (rPrepDrawDataSub6DF5A0)0x6DF5A0;
    static int* some_graphic_struct_dword_252F490_            = (int*)0x0252F490;
    ptrdiff_t start_offset                                    = 0;
    DWORD tri_strip_count                                     = 0;
    uint8_t* g_render_ui_flag_byte_252F48C                    = (uint8_t*)0x0252F48C;

    if (*g_render_ui_flag_byte_252F48C) {

        for (size_t vert = 0; vert < vert_count; vert = vert + 1) {
            if (prep_verts_gui_and_characters(rdss, vert, stack)) {
                r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, start_offset, tri_strip_count);
                start_offset    = 0;
                tri_strip_count = 0;
            } else {
                if (!tri_strip_count) {
                    start_offset = rdss->verts_or_whatever;
                    // printf("|start_offset=%d\n", start_offset);
                }
                ++tri_strip_count;
            }
        }
        r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, start_offset, tri_strip_count);

        return;
    }

    uint8_t value                    = EVertexBufferType::ARTICULATED_MODELS;
    static D3DDevicePtr* device_ptr  = (D3DDevicePtr*)0x0252F374;
    static IDirect3DDevice9* pdevice = device_ptr->device;

    std::vector<VertexDefDynamic> local_vertices;
    local_vertices.reserve(vert_count);

    for (size_t vert = 0; vert < vert_count; vert = vert + 1) {
        VertexDef cpuvtx{};
        g_current_vertex_data_pointer = &cpuvtx;
        prep_verts_gui_and_characters(rdss, vert, stack);
        VertexDefDynamic& dvtx = local_vertices.emplace_back();

        dvtx.pos    = srd0->verts_ptr0[vert];
        dvtx.normal = srd0->normals_ptr[vert];

        auto clamp_bone_index = [cds](uint8_t idx) -> uint8_t {
            return (idx < cds->bone_count) ? idx : 0;
        };
        const uint8_t decoded_bones[4] = {
            clamp_bone_index(srd0->boindata_ptr0[vert].bn_ind[2] >> 2),
            clamp_bone_index(srd0->boindata_ptr0[vert].bn_ind[1] >> 2),
            clamp_bone_index(srd0->boindata_ptr0[vert].bn_ind[3] >> 2),
            clamp_bone_index(srd0->boindata_ptr0[vert].bn_ind[0] >> 2),
        };

        dvtx.mat_indices.index[g_bindex_0] = decoded_bones[0];
        dvtx.mat_indices.index[g_bindex_1] = decoded_bones[1];
        dvtx.mat_indices.index[g_bindex_2] = decoded_bones[2];
        dvtx.mat_indices.index[g_bindex_3] = decoded_bones[3];

        const bool writes_index3 =
            (g_bindex_0 == 3) || (g_bindex_1 == 3) || (g_bindex_2 == 3) || (g_bindex_3 == 3);
        if (!writes_index3) {
            dvtx.mat_indices.index[3] = decoded_bones[0];
        }

        WORD weight_packed = srd0->weightdata_ptr0[vert].bone_weights;
        const float w1     = ((float)((weight_packed >> 0) & 0x1F) / 31.0f);
        const float w2     = ((float)((weight_packed >> 5) & 0x1F) / 31.0f);
        const float w3     = ((float)((weight_packed >> 10) & 0x1F) / 31.0f);
        dvtx.weight.x      = w1;
        dvtx.weight.y      = w2;
        dvtx.weight.z      = w3;
        assert((w1 + w2 + w3) <= 1.0f);

        dvtx.diff_r = cpuvtx.diff_r;
        dvtx.diff_g = cpuvtx.diff_g;
        dvtx.diff_b = cpuvtx.diff_b;
        dvtx.diff_a = cpuvtx.diff_a;

        dvtx.spec_r = cpuvtx.spec_r;
        dvtx.spec_g = cpuvtx.spec_g;
        dvtx.spec_b = cpuvtx.spec_b;
        dvtx.spec_a = cpuvtx.spec_a;

        dvtx.uv0                      = cpuvtx.uv0;
        g_current_vertex_data_pointer = 0;
    }

    IndexData* result{};
    size_t hash = (size_t)srd0; // std::hash<uintptr_t>()((uintptr_t)srd0->vert_count + (uintptr_t)srd0->tex_ind + (uintptr_t)srd0->model_mesh_hdr_ptr);
    if (auto& it = g_ib_map.find(hash); it != g_ib_map.end()) {
        result = &it->second;
        result->ref_count -= 1;
    } else {
        IndexData idata    = make_index_buffer(srd0);
        auto& ib_map_entry = g_ib_map.emplace(hash, std::move(idata));
        result             = &ib_map_entry.first->second;
    }
    assert(result);

    const uint16_t palette_limit = get_ffp_palette_limit(pdevice);
    if (result->ib.size() % 3 != 0) {
        result->ref_count -= 1;
        return;
    }

    std::vector<std::array<WORD, 3>> current_batch;
    current_batch.reserve(result->ib.size() / 3);
    bool batch_bones[256]{};
    uint16_t batch_unique_bones = 0;

    auto add_triangle_bones = [&](const std::array<WORD, 3>& tri, bool dry_run) {
        bool tri_bones[256]{};
        uint16_t additional = 0;
        for (WORD idx : tri) {
            const VertexDefDynamic& v = local_vertices[idx];
            const uint8_t bones[4]    = {
                v.mat_indices.index[0],
                v.mat_indices.index[1],
                v.mat_indices.index[2],
                v.mat_indices.index[3],
            };
            for (uint8_t b : bones) {
                if (!tri_bones[b]) {
                    tri_bones[b] = true;
                    if (!batch_bones[b]) {
                        ++additional;
                    }
                }
            }
        }
        if (dry_run) {
            return additional;
        }
        for (size_t b = 0; b < 256; ++b) {
            if (tri_bones[b] && !batch_bones[b]) {
                batch_bones[b] = true;
                ++batch_unique_bones;
            }
        }
        return additional;
    };

    auto emit_batch = [&]() {
        if (current_batch.empty()) {
            return;
        }

        std::vector<uint8_t> palette;
        palette.reserve(batch_unique_bones);
        uint8_t remap_table[256];
        memset(remap_table, 0xFF, sizeof(remap_table));
        for (size_t b = 0; b < 256; ++b) {
            if (batch_bones[b]) {
                remap_table[b] = (uint8_t)palette.size();
                palette.push_back((uint8_t)b);
            }
        }

        std::unordered_map<WORD, WORD> vertex_remap;
        std::vector<WORD> batch_indices;
        batch_indices.reserve(current_batch.size() * 3);
        const size_t batch_vertex_offset = g_vertex_data_world.size();

        for (const auto& tri : current_batch) {
            for (WORD src_idx : tri) {
                auto it      = vertex_remap.find(src_idx);
                WORD dst_idx = 0;
                if (it == vertex_remap.end()) {
                    VertexDefDynamic v     = local_vertices[src_idx];
                    v.mat_indices.index[0] = remap_table[v.mat_indices.index[0]];
                    v.mat_indices.index[1] = remap_table[v.mat_indices.index[1]];
                    v.mat_indices.index[2] = remap_table[v.mat_indices.index[2]];
                    v.mat_indices.index[3] = remap_table[v.mat_indices.index[3]];
                    dst_idx                = (WORD)vertex_remap.size();
                    g_vertex_data_world.push_back(v);
                    vertex_remap.emplace(src_idx, dst_idx);
                } else {
                    dst_idx = it->second;
                }
                batch_indices.push_back(dst_idx);
            }
        }

        DIPData* render_info              = new DIPData;
        render_info->source_bone_count    = cds->bone_count;
        render_info->bone_count           = (uint16_t)palette.size();
        render_info->palette_to_global    = std::move(palette);
        render_info->world                = &cds->obj_transforms_matrices_ptr->world_maybe;
        render_info->matrices             = stack->p_matrix;
        render_info->vertex_count_ours    = (uint32_t)vertex_remap.size();
        render_info->index_count_ours     = (uint32_t)batch_indices.size();
        render_info->primitive_count_ours = render_info->index_count_ours / 3;
        render_info->base_vertex_index    = (uint32_t)batch_vertex_offset;
        render_info->start_index          = (uint32_t)g_index_data_world.size();

        g_index_data_world.insert(g_index_data_world.end(), batch_indices.begin(), batch_indices.end());

        g_dyn_ren_buf->check_and_flag_for_resize(
            render_info->vertex_count_ours,
            render_info->index_count_ours);

        g_current_obj += 1;
        r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, g_vertices_offset | (value << 24), (int)render_info);

        current_batch.clear();
        memset(batch_bones, 0, sizeof(batch_bones));
        batch_unique_bones = 0;
    };

    for (size_t i = 0; i < result->ib.size(); i += 3) {
        std::array<WORD, 3> tri = {
            result->ib[i + 0],
            result->ib[i + 1],
            result->ib[i + 2],
        };
        const uint16_t extra = add_triangle_bones(tri, true);
        if (!current_batch.empty() && (batch_unique_bones + extra) > palette_limit) {
            emit_batch();
        }
        add_triangle_bones(tri, false);
        current_batch.push_back(tri);
    }
    emit_batch();
    result->ref_count -= 1;

    // r_prep_draw_data_sub_6DF5A0(*some_graphic_struct_dword_252F490_, start_offset, tri_strip_count);
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
        mov eax, [esp + 40h] ; cDrawSub*
        push eax
        call r_render_chars_intercept
        add esp, 10h
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
    g_indices_offset  = 0;
    g_vertices_offset = 0;
    g_current_obj     = 0;
#if 0
    g_bindex_0 = g_combinations[g_iter][0];
    g_bindex_1 = g_combinations[g_iter][1];
    g_bindex_2 = g_combinations[g_iter][2];
    g_bindex_3 = g_combinations[g_iter][3];
#endif

    g_vertex_data_world.clear();
    g_index_data_world.clear();

    g_dyn_ren_buf->reset();

    for (auto it = g_ib_map.begin(); it != g_ib_map.end();) {
        it->second.ref_count += 1;
        if (it->second.ref_count > 32) {
            it = g_ib_map.erase(it);
        } else {
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
}

static bool IsMatrixZero(const D3DMATRIX& mat) {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (mat.m[r][c] != 0.0f) {
                return false;
            }
        }
    }
    return true;
}

static uint16_t get_ffp_palette_limit(IDirect3DDevice9* pdevice) {
    static uint16_t cached_limit = 0;
    if (cached_limit != 0) {
        return cached_limit;
    }

    cached_limit = 8;
    if (!pdevice) {
        return cached_limit;
    }

    D3DCAPS9 caps{};
    if (SUCCEEDED(pdevice->GetDeviceCaps(&caps))) {
        const uint32_t from_caps = caps.MaxVertexBlendMatrixIndex;
        if (from_caps > 0 && from_caps <= 255) {
            cached_limit = (uint16_t)from_caps;
        }
    }
    return cached_limit;
}

static bool g_skinning_validator_enabled = true;

static void validate_ffp_skinning_draw(IDirect3DDevice9* pdevice, const DIPData* dd) {
    if (!g_skinning_validator_enabled || !pdevice || !dd) {
        return;
    }

    static bool caps_cached = false;
    static D3DCAPS9 caps{};
    if (!caps_cached) {
        pdevice->GetDeviceCaps(&caps);
        caps_cached = true;
    }

    static uint32_t draw_counter = 0;
    ++draw_counter;

    const size_t start = dd->start_index;
    const size_t end   = start + (size_t)dd->index_count_ours;
    if (end > g_index_data_world.size()) {
        fprintf(stderr,
                "[ffp-skin-validate] draw=%u bad index range start=%zu end=%zu ib_size=%zu\n",
                draw_counter,
                start,
                end,
                g_index_data_world.size());
        return;
    }

    bool used_bones[256]{};
    uint32_t unique_bones      = 0;
    uint32_t out_of_range_refs = 0;

    const uint8_t max_allowed_by_draw = dd->bone_count > 0 ? (uint8_t)(dd->bone_count - 1) : 0;
    const uint8_t max_allowed_by_caps = caps.MaxVertexBlendMatrixIndex < 255 ? (uint8_t)caps.MaxVertexBlendMatrixIndex : 255;
    const uint8_t max_allowed         = max_allowed_by_draw < max_allowed_by_caps ? max_allowed_by_draw : max_allowed_by_caps;

    for (size_t i = start; i < end; ++i) {
        const size_t vertex_index = (size_t)dd->base_vertex_index + (size_t)g_index_data_world[i];
        if (vertex_index >= g_vertex_data_world.size()) {
            fprintf(stderr,
                    "[ffp-skin-validate] draw=%u vertex OOB: base=%u idx=%u final=%zu vb_size=%zu\n",
                    draw_counter,
                    (unsigned)dd->base_vertex_index,
                    (unsigned)g_index_data_world[i],
                    vertex_index,
                    g_vertex_data_world.size());
            return;
        }

        const VertexDefDynamic& v     = g_vertex_data_world[vertex_index];
        const uint8_t bone_indices[4] = {
            v.mat_indices.index[0],
            v.mat_indices.index[1],
            v.mat_indices.index[2],
            v.mat_indices.index[3],
        };

        for (uint8_t bi : bone_indices) {
            if (!used_bones[bi]) {
                used_bones[bi] = true;
                ++unique_bones;
            }
            if (bi > max_allowed) {
                ++out_of_range_refs;
            }
        }
    }

    if (out_of_range_refs > 0) {
        fprintf(stderr,
                "[ffp-skin-validate] draw=%u out_of_range_refs=%u unique_bones=%u max_allowed=%u draw_bones=%u cap_idx=%u\n",
                draw_counter,
                out_of_range_refs,
                unique_bones,
                (unsigned)max_allowed,
                (unsigned)dd->bone_count,
                (unsigned)caps.MaxVertexBlendMatrixIndex);
    } else if ((draw_counter % 120) == 0) {
        fprintf(stderr,
                "[ffp-skin-validate] draw=%u ok unique_bones=%u draw_bones=%u cap_idx=%u\n",
                draw_counter,
                unique_bones,
                (unsigned)dd->bone_count,
                (unsigned)caps.MaxVertexBlendMatrixIndex);
    }
}

std::unique_ptr<FunctionHook> g_d3d_draw_primitive_sub_6E1C00_hook;
static void __cdecl d3d_draw_primitive_sub_6E1C00(unsigned int vertexCount, int primitve_count, char a3) {
    if (!g_d3d_draw_primitive_sub_6E1C00_hook) {
        return;
    }

    static D3DDevicePtr* device_ptr  = (D3DDevicePtr*)0x0252F374;
    static IDirect3DDevice9* pdevice = device_ptr->device;

    uint8_t type        = (vertexCount >> 24) & 0xFF;
    uint32_t vert_count = vertexCount & 0x00FFFFFF;
    if (type == EVertexBufferType::WORLD) {

        DIPData* dd = (DIPData*)primitve_count;
        // goto earlie;

        // upload data to vb & ib
        g_dyn_ren_buf->re_create_buffers(pdevice);
        g_dyn_ren_buf->upload(g_vertex_data_world, g_index_data_world);

        pdevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
        pdevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);

        // draw
        pdevice->SetStreamSource(0, g_dyn_ren_buf->p_vb, 0, sizeof(VertexDefDynamic));
        pdevice->SetFVF(vertex_format_dmc3_3d_ffp_skinning);
        pdevice->SetIndices(g_dyn_ren_buf->p_ib);

        // auto hr = pdevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, dd->base_index, dd->vertex_count_ours, dd->base_index, dd->primitive_count_ours);
        HRESULT hr = pdevice->DrawIndexedPrimitive(
            D3DPT_TRIANGLELIST,
            dd->base_vertex_index,   // BaseVertexIndex
            0,                       // MinIndex
            dd->vertex_count_ours,   // NumVertices
            dd->start_index,         // StartIndex
            dd->primitive_count_ours // PrimCount
        );
        if (FAILED(hr)) {
            fprintf(stderr, "Error: %s error description: %s\n",
                    DXGetErrorString(hr), DXGetErrorDescription(hr));
        }
#if 0
        // draw
        pdevice->SetStreamSource(0, g_dyn_ren_buf->p_vb, 0, sizeof(VertexDefDynamic));
        pdevice->SetFVF(vertex_format_dmc3_3d_ffp_skinning);
        pdevice->SetIndices(g_dyn_ren_buf->p_ib);

        auto hr = pdevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, dd->vertex_count_ours, dd->base_vertex_index, dd->primitive_count_ours);
        if (FAILED(hr)) {
            fprintf(stderr, "Error: %s error description: %s\n",
                    DXGetErrorString(hr), DXGetErrorDescription(hr));
        }
#endif

    earlie:
        delete dd;
    } else if (type == EVertexBufferType::ARTICULATED_MODELS) {

        DIPData* dd = (DIPData*)primitve_count;

#if 0
        for (size_t i = dd->base_index; i < dd->base_index + dd->index_count_ours; ++i) {
            WORD idx = g_index_data_world[i];
            assert(idx < g_vertex_data_world.size() && "Index exceeds vertex buffer size!");
        }

        // Find min and max vertex index used by this batched mesh
        WORD min_index = UINT16_MAX;
        WORD max_index = 0;
        
        for (size_t i = dd->base_index; i < dd->base_index + dd->index_count_ours; ++i) {
            WORD idx = g_index_data_world[i];
            if (idx < min_index) min_index = idx;
            if (idx > max_index) max_index = idx;
        }

        // Calculate how many vertices are actually referenced
        UINT num_vertices = max_index - min_index + 1;

        // Adjust base vertex index so we don't have to re-upload the whole buffer
        UINT base_vertex_index = min_index;
#endif

        Matrix4x4 world;
        Matrix4x4* ass = dd->matrices;
        if (dd->world) {
            world = *dd->world;
            // pdevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&woerld);

            // world = glm::transpose(world);
            // pdevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&world);
        }

        // upload data to vb & ib
        g_dyn_ren_buf->re_create_buffers(pdevice);
        g_dyn_ren_buf->upload(g_vertex_data_world, g_index_data_world);

        uint16_t bones = dd->bone_count;

        // printf("bones=%d\n", bones);

        for (uint16_t i = 0; i < bones; ++i) {
            uint16_t src_bone = i;
            if (!dd->palette_to_global.empty()) {
                assert(i < dd->palette_to_global.size());
                src_bone = dd->palette_to_global[i];
            }
            if (src_bone >= dd->source_bone_count) {
                src_bone = 0;
            }
            D3DMATRIX& mat = *(D3DMATRIX*)&ass[src_bone];
            pdevice->SetTransform(D3DTS_WORLDMATRIX(i), &mat);
        }

#if 0
        for (uint16_t i = 0; i < bones; ++i) {

            printf("matrix%d:\n", i);
            for (int row = 0; row < 4; ++row) {
                printf("  | ");
                for (int col = 0; col < 4; ++col) {
                    printf("%8.4f ", ass[i][row][col]);
                }
                printf("|\n");
            }
            printf("\n");
            pdevice->SetTransform(D3DTS_WORLDMATRIX(i), (CONST D3DMATRIX*) & ass[i]);
        }
#endif

        pdevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS);
        pdevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE);
        validate_ffp_skinning_draw(pdevice, dd);

        // draw
        pdevice->SetStreamSource(0, g_dyn_ren_buf->p_vb, 0, sizeof(VertexDefDynamic));
        pdevice->SetFVF(vertex_format_dmc3_3d_ffp_skinning);
        pdevice->SetIndices(g_dyn_ren_buf->p_ib);

        // auto hr = pdevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, dd->base_index, dd->vertex_count_ours, dd->base_index, dd->primitive_count_ours);
        HRESULT hr = pdevice->DrawIndexedPrimitive(
            D3DPT_TRIANGLELIST,
            dd->base_vertex_index,   // BaseVertexIndex
            0,                       // MinIndex
            dd->vertex_count_ours,   // NumVertices
            dd->start_index,         // StartIndex
            dd->primitive_count_ours // PrimCount
        );
        if (FAILED(hr)) {
            fprintf(stderr, "Error: %s error description: %s\n",
                    DXGetErrorString(hr), DXGetErrorDescription(hr));
        }

#if 0
        pdevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
        pdevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
#endif

    early_out:
        delete dd;
    } else {
        g_d3d_draw_primitive_sub_6E1C00_hook->get_original<decltype(d3d_draw_primitive_sub_6E1C00)>()(vertexCount, primitve_count, a3);
    }
}

static void cDraw_write_bone_count(cDrawSub* cds) {
    uint16_t bone_count = cds->bone_count;
    if (cds->modnrender_data_ptr) {
        if (cds->modnrender_data_ptr->ptr_has_some_rendering_data) {
            cds->modnrender_data_ptr->ptr_has_some_rendering_data->bone_count = bone_count;
            cds->modnrender_data_ptr->ptr_has_some_rendering_data->world      = &cds->obj_transforms_matrices_ptr->world_maybe;
        }
    }
}

std::optional<std::string> RendererReplace::on_initialize() {
    // WARNING(): dirty hack to only init once here:
    static bool init = false;
    if (init) {
        return Mod::on_initialize();
    }
    init       = true;
    g_rreplace = this;

    g_vertex_data_world.reserve(128_MiB / sizeof(VertexDef));
    g_index_data_world.reserve(128_MiB / sizeof(WORD));

    g_dyn_ren_buf = new DynamicRenderBuffer();

    // g_index_buffer_backing_area = malloc(INDEX_BUFFER_SIZE);
    // utility::arena_init(&g_index_buffer, g_index_buffer_backing_area, INDEX_BUFFER_SIZE);

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

#if 0
    g_d3d_upload_to_vram_sub_6e18f0_hook = std::make_unique<FunctionHook>(0x006E18F0, &d3d_switch_on_buffer_type_and_upload_to_vram_sub_6E18F0);
    if (!g_d3d_upload_to_vram_sub_6e18f0_hook->create()) {
        return "failed to hook d3d_updload_to_vram";
    }
#endif
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

#if 0
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
#endif
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
#if 0
    static constexpr float w1 = 0.71f;
    static constexpr float w2 = 0.29f;

    static const D3DXVECTOR3 vec(-13.87f, 148.90f, 8.57f);
    static D3DXMATRIX wew(
        0.69f, -0.07f, -0.72f, 0.00f, -0.08f, 0.98f, -0.18f, 0.00f, 0.72f, 0.18f, 0.67f, 0.00f, 1520.71f, -0.46f, 2014.77f, 1.00f
    );
    static Matrix4x4 wew1(
        0.69f, -0.07f, -0.72f, 0.00f, -0.08f, 0.98f, -0.18f, 0.00f, 0.72f, 0.18f, 0.67f, 0.00f, 1520.71f, -0.46f, 2014.77f, 1.00f
    );

    D3DXVECTOR3 result;
    D3DXVec3TransformCoord(&result, &vec, &wew);

    static glm::vec4 vecglm_(vec.x, vec.y, vec.z, 1.0f);
    const glm::vec4 vecglm = (wew1 * vecglm_) * w1;
    ImGui::Text("result: %f, %f, %f", result.x, result.y, result.z);
    ImGui::Text("result1: %f, %f, %f", vecglm.x, vecglm.y, vecglm.z);

    static D3DXMATRIX mat2(
        0.84f, -0.11f, -0.53f, 0.00f, 0.10f, 0.99f, -0.06f, 0.00f, 0.53f, 0.00f, 0.85f, 0.00f, 1494.46f, -2.74f, 1998.60f, 1.00f
    );

    D3DXVec3TransformCoord(&result, &vec, &mat2);
    ImGui::Text("result: %f, %f, %f", result.x * w2, result.y * w2, result.z * w2);
#endif

    if (ImGui::IsKeyPressed(VK_F8)) {
        cycle_bindex_preset(+1);
    }

    if (ImGui::Button("Cycle bindex preset (F8)")) {
        cycle_bindex_preset(+1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Prev preset")) {
        cycle_bindex_preset(-1);
    }

    const auto& active_preset = g_bindex_presets[(size_t)g_bindex_preset_idx];
    ImGui::Text("Preset %d/%d: [%d, %d, %d, %d]",
                g_bindex_preset_idx + 1,
                (int)g_bindex_presets.size(),
                active_preset[0],
                active_preset[1],
                active_preset[2],
                active_preset[3]);

    ImGui::InputInt("g_bindex_0", &g_bindex_0);
    ImGui::InputInt("g_bindex_1", &g_bindex_1);
    ImGui::InputInt("g_bindex_2", &g_bindex_2);
    ImGui::InputInt("g_bindex_3", &g_bindex_3);

    for (int i = 0; i < (int)g_bindex_presets.size(); ++i) {
        const auto& p = g_bindex_presets[(size_t)i];
        if (g_bindex_0 == p[0] && g_bindex_1 == p[1] && g_bindex_2 == p[2] && g_bindex_3 == p[3]) {
            g_bindex_preset_idx = i;
            break;
        }
    }

    ImGui::InputInt("skip draw dynamic", &g_skipdraw);
    ImGui::Checkbox("skip drawing static meshes", &g_skipdraw_static);

    ImGui::InputInt("g_bone_count", &g_bcount);

#if 0
    ImGui::InputInt("base_vertex_index", &g_temp_dip_things.base_vertex_index);
    ImGui::InputInt("min_vertex_index", (int*) &g_temp_dip_things.min_vertex_index);
    ImGui::InputInt("num_vertices", (int*) &g_temp_dip_things.num_vertices);
    ImGui::InputInt("start_index", (int*) &g_temp_dip_things.start_index);
    ImGui::InputInt("prim_count", (int*) &g_temp_dip_things.prim_count);
#endif
}

// TODO
void RendererReplace::d3d_create_and_lock_vb_sub_6E1260_internal() noexcept {
}

void RendererReplace::d3d_create_and_lock_vb_sub_6E1260() noexcept {
}
void RendererReplace::init_d3d9_sub_408DE0_internal() noexcept {
}

void RendererReplace::init_d3d9_sub_408DE0() noexcept {
    g_rreplace->init_d3d9_sub_408DE0_internal();
}

// during load
// void RendererReplace::on_config_load(const utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_load(cfg);
//	}
//}
// during save
// void RendererReplace::on_config_save(utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_save(cfg);
//	}
//}
// do something every frame
// void RendererReplace::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void RendererReplace::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void RendererReplace::on_draw_ui() {}
