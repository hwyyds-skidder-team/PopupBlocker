cmake_minimum_required(VERSION 3.21)
project(PopupBlocker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# Hook DLL
add_library(PopupBlockerHook SHARED
  hook/PopupBlockerHook.cpp
)
target_include_directories(PopupBlockerHook PRIVATE shared)
target_link_libraries(PopupBlockerHook PRIVATE)

# UI (Win32 + DX11)
add_executable(PopupBlockerUI WIN32
  ui/main.cpp

  imgui/backends/imgui_impl_win32.cpp
  imgui/backends/imgui_impl_dx11.cpp
)
target_include_directories(
PopupBlockerUI PRIVATE shared
imgui/backends
)
target_link_libraries(PopupBlockerUI PRIVATE
  imgui::imgui
  nlohmann_json::nlohmann_json
  d3d11 dxgi
  imm32
)

# Test target
add_executable(TestPopup WIN32
  test/TestPopup.cpp
)