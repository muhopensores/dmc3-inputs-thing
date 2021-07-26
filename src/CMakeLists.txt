set(MODS_SRC
    mods/SimpleMod.hpp
    mods/UIButton.hpp
    mods/UIButton.cpp
    mods/BulletStop.hpp
    mods/BulletStop.cpp
    mods/PracticeMode.hpp
    mods/PracticeMode.cpp
    mods/StyleSwitchFX.hpp
    mods/StyleSwitchFX.cpp
    mods/InertiaThings.hpp
    mods/InertiaThings.cpp
    mods/QuicksilverShader.hpp
    mods/QuicksilverShader.cpp
    mods/SimpleMod.cpp
)

set(HOOKS_SRC
    DInputHook.hpp
    DInputHook.cpp
    D3D9Hook.hpp
    D3D9Hook.cpp
    WindowsMessageHook.hpp
    WindowsMessageHook.cpp
)

set(SDK_SRC
    sdk/ReClass.hpp
    sdk/ReClass_Internal.hpp
    sdk/Enums_Internal.hpp
    sdk/Offsets.hpp
    sdk/Math.hpp
    sdk/Devil3.hpp
    sdk/Devil3.cpp
)

set(IMGUI_SRC
    fw-imgui/imgui_impl_dx9.h
    fw-imgui/imgui_impl_dx9.cpp
    fw-imgui/imgui_impl_win32.h
    fw-imgui/imgui_impl_win32.cpp
)

set(UTILITY_SRC
    utility/ChronoAlias.hpp
    utility/Compressed.hpp
    utility/Compressed.cpp
    utility/Address.hpp
    utility/Address.cpp
    utility/Config.hpp
    utility/Config.cpp
    utility/FunctionHook.hpp
    utility/FunctionHook.cpp
    utility/Memory.hpp
    utility/Memory.cpp
    utility/Module.hpp
    utility/Module.cpp
    utility/Patch.hpp
    utility/Patch.cpp
    utility/Pattern.hpp
    utility/Pattern.cpp
    utility/Scan.hpp
    utility/Scan.cpp
    utility/String.hpp
    utility/String.cpp
)

set(FRAMEWORK_SRC
    Sfx.cpp
    Mod.hpp
    Mods.hpp
    Mods.cpp
    ModFramework.hpp
    ModFramework.cpp
    Config.hpp
)

source_group("imgui", FILES ${IMGUI_SRC})
source_group("Hooks" FILES ${HOOKS_SRC})
source_group("Mods" FILES ${MODS_SRC})
source_group("SDK" FILES ${SDK_SRC})
source_group("Utility" FILES ${UTILITY_SRC})
source_group("Framework" FILES ${FRAMEWORK_SRC})

string(REGEX REPLACE "/W0" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_library(${CMAKE_PROJECT_NAME} SHARED
            Main.cpp
            LicenseStrings.hpp
            ${SDK_SRC}
            ${MODS_SRC}
            ${HOOKS_SRC}
            ${IMGUI_SRC}
            ${UTILITY_SRC}
            ${FRAMEWORK_SRC}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC imgui minhook glm_static shlwapi D3dx9 d3d9 )
target_compile_features(${CMAKE_PROJECT_NAME} PUBLIC cxx_std_17)

include_directories(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME dinput8)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(DEVELOPER_MODE)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC DEVELOPER)
    target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC /EHa)
endif()