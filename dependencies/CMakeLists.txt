add_subdirectory(spdlog)

SET(GLM_TEST_ENABLE OFF CACHE BOOL "Build unit tests" FORCE)
add_subdirectory(glm)

#add_subdirectory(rpclib)

project(minhook)
set(CompilerFlags
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELEASE
        )
foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

add_library(minhook STATIC
            minhook/src/buffer.c
            minhook/src/buffer.h
            minhook/src/hook.c
            minhook/src/trampoline.c
            minhook/src/trampoline.h
            minhook/src/hde/hde32.c
            minhook/src/hde/hde32.h
            minhook/src/hde/hde64.c
            minhook/src/hde/hde64.h
            minhook/src/hde/pstdint.h
            minhook/src/hde/table32.h
            minhook/src/hde/table64.h)

project(imgui)
add_library(imgui STATIC
            imgui/imconfig.h
            imgui/imgui_demo.cpp
            imgui/imgui_widgets.cpp
            imgui/imgui_tables.cpp
            imgui/imgui_draw.cpp
            imgui/imgui.cpp)

target_compile_definitions(imgui PUBLIC IMGUI_USER_CONFIG="../src/fw-imgui/imgui_imconfig.hpp")

add_subdirectory(DebugDraw)
add_subdirectory(meshoptimizer)

# sdl3 crap
set(SDL_SHARED OFF CACHE BOOL "Build shared SDL library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build static SDL library" FORCE)

set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build SDL test library" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Build SDL test programs" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

set(SDL_AUDIO OFF CACHE BOOL "" FORCE)
set(SDL_VIDEO OFF CACHE BOOL "" FORCE)
set(SDL_RENDER OFF CACHE BOOL "" FORCE)

add_subdirectory(SDL)