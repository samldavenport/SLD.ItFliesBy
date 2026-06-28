#ifndef IFB_HPP
#define IFB_HPP

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <stb_image.h>

#include <sld.hpp>
#include <sld-opengl.hpp>
#include <sld-math.hpp>

#include "ifb-types.hpp"
#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "ifb-engine.hpp"
#include "ifb-input.hpp"
#include "ifb-font-opensans.hpp"

using namespace sld;

extern IMGUI_IMPL_API LRESULT 
ImGui_ImplWin32_WndProcHandler(
    HWND   hWnd, 
    UINT   msg, 
    WPARAM wParam, 
    LPARAM lParam);

#endif //IFB_HPP