cmake_minimum_required(VERSION 3.13)
project("3VG3")


# raylib
set(RAYLIB_VERSION 5.0)
find_package(raylib ${RAYLIB_VERSION} QUIET)
if (NOT raylib_FOUND) # If not found, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
	set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

# raylib-cpp
find_package(raylib_cpp QUIET)
if (NOT raylib_cpp_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib_cpp
        GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
        GIT_TAG "master"
		SYSTEM # suppress compiler warnings
    )
    FetchContent_MakeAvailable(raylib_cpp)
endif()

# raygui
add_library(raygui INTERFACE)
target_include_directories(raygui SYSTEM INTERFACE external)

# ImGui
set(IMGUI_PATH "external/imgui")
file(GLOB IMGUI_SOURCES "${IMGUI_PATH}/*.cpp") 
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${IMGUI_PATH})

# ImGui raylib backend
set(IMGUI_RAYLIB_DIR "external/rlImGui")
add_library(imgui_raylib STATIC "${IMGUI_RAYLIB_DIR}/rlImGui.cpp")
target_compile_definitions(imgui_raylib PUBLIC NO_FONT_AWESOME)
target_include_directories(imgui_raylib SYSTEM PUBLIC ${IMGUI_RAYLIB_DIR})
target_link_libraries(imgui_raylib imgui raylib)

# MAIN FILES
add_executable(${PROJECT_NAME})
add_subdirectory(src)
target_link_libraries(${PROJECT_NAME} raylib raylib_cpp raygui imgui imgui_raylib)

# Copy resources into build
set(RESOURCES_DIR "resources")
file(COPY ${RESOURCES_DIR} DESTINATION ${CMAKE_BINARY_DIR})

# Web Configurations
if (${PLATFORM} STREQUAL "Web" OR WEB_PRESET)
    # Tell Emscripten to build an example.html file.
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

	# Required linker flags for using Raylib with Emscripten
	target_link_options(${PROJECT_NAME} PRIVATE -sEXPORTED_FUNCTIONS=['_main','_malloc'] -sEXPORTED_RUNTIME_METHODS=ccall -sUSE_GLFW=3 --preload-file ${RESOURCES_DIR})
	#target_compile_options(${PROJECT_NAME} PRIVATE -Os)
endif()

# Raylib MacOS dependencies
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()