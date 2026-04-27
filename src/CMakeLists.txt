set(MODS_SRC
    mods/SimpleMod.hpp
    mods/RendererReplace.hpp
    mods/RendererReplace.cpp
    mods/CustomAlolcator.hpp
    mods/CustomAlolcator.cpp
    mods/EnemySpawnRate.hpp
    mods/EnemySpawnRate.cpp
    mods/RgTimer.hpp
    mods/RgTimer.cpp
    mods/DebugDraw.hpp
    mods/DebugDraw.cpp
    mods/PrintfDebugging.hpp
    mods/PrintfDebugging.cpp
    mods/AudioStutterFix.hpp
    mods/AudioStutterFix.cpp
 #   mods/CameraHack.hpp
 #   mods/CameraHack.cpp
    mods/InputLog.hpp
    mods/InputLog.cpp
    mods/GamepadsFix.hpp
    mods/GamepadsFix.cpp
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
    mods/StyleSwitcherInfo.hpp 
    mods/StyleSwitcherInfo.cpp 
    mods/NoHeightRestriction.hpp 
    mods/NoHeightRestriction.cpp 
    mods/EnemySoulEaterNoInvis.hpp 
    mods/EnemySoulEaterNoInvis.cpp 
    mods/TurnSpeed.hpp 
    mods/TurnSpeed.cpp 
    mods/EnemyStepCooldown.hpp 
    mods/EnemyStepCooldown.cpp 
    mods/EnemyStates.hpp 
    mods/EnemyStates.cpp 
    mods/AreaJump.hpp
    mods/AreaJump.cpp
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
    sdk/VoxObj.hpp
)

set(IMGUI_SRC
    fw-imgui/imgui_impl_dx9.h
    fw-imgui/imgui_impl_dx9.cpp
    fw-imgui/imgui_impl_win32.h
    fw-imgui/imgui_impl_win32.cpp
    fw-imgui/sm.cpp
    fw-imgui/gs.cpp
    fw-imgui/ts.cpp
    fw-imgui/rg.cpp
    fw-imgui/qs.cpp
    fw-imgui/dg.cpp
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
    utility/FixedQueue.hpp
    utility/FixedQueue.cpp
    utility/ExceptionHandler.hpp
    utility/ExceptionHandler.cpp
    utility/Thread.hpp
    utility/Thread.cpp
    utility/SDLGamepad.hpp
    utility/MemArena.hpp
    utility/MemArena.cpp
    #utility/tgc.h
    #utility/tgc.c
)

set(FRAMEWORK_SRC
    PromptFontGlyphs.hpp
    PromptFontCompressed.cpp
    Mod.hpp
    Mods.hpp
    Mods.cpp
    ModFramework.hpp
    ModFramework.cpp
    Config.hpp
#    Effekseer_DX9.h
#    Effekseer_DX9.cpp
)

source_group("imgui" FILES ${IMGUI_SRC})
source_group("Hooks" FILES ${HOOKS_SRC})
source_group("Mods" FILES ${MODS_SRC})
source_group("SDK" FILES ${SDK_SRC})
source_group("Utility" FILES ${UTILITY_SRC})
source_group("Framework" FILES ${FRAMEWORK_SRC})

#string(REGEX REPLACE "/W0" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(COMMON_SRC
    Main.cpp
    LicenseStrings.hpp
    ${SDK_SRC}
    ${MODS_SRC}
    ${HOOKS_SRC}
    ${IMGUI_SRC}
    ${UTILITY_SRC}
    ${FRAMEWORK_SRC}
)

function(configure_target TARGET_NAME OUTPUT_NAME EXTRA_DEFINES)
    add_library(${TARGET_NAME} SHARED ${COMMON_SRC})

    target_link_libraries(${TARGET_NAME} PUBLIC imgui minhook glm_static shlwapi D3dx9 d3d9 SDL2main SDL2 meshoptimizer legacy_stdio_definitions)
    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)

    include_directories(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})
    target_compile_definitions(${TARGET_NAME} PUBLIC ${EXTRA_DEFINES})
    if(DEVELOPER_MODE)
        target_compile_definitions(${TARGET_NAME} PUBLIC DEVELOPER)
        target_compile_options(${TARGET_NAME} PUBLIC /EHa)
    endif()
endfunction()

configure_target(${CMAKE_PROJECT_NAME} dinput8 "DINPUT_HOOK;DINPUT_MODE")
configure_target(dsound dsound "DSOUND_MODE")
configure_target(dinput8_speedrun dinput8_speedrun "DINPUT_HOOK;DINPUT_MODE;GAMEPLAY_HOOKS=0")
configure_target(dsound_speedrun dsound_speedrun "DSOUND_MODE;GAMEPLAY_HOOKS=0")
