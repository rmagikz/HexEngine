#include "platform/platform.h"

#if HPLATFORM_WINDOWS

#include <core/logger.h>
#include <core/input.h>
#include <core/event.h>

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

typedef struct platform_state
{
    HINSTANCE h_instance;
    HWND hwnd;
    HDC window_device_handle;
    HGLRC gl_context;
    f64 clock_frequency;
    LARGE_INTEGER start_time;
} platform_state;

static platform_state* state_ptr;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

b8 platform_initialize(u64* memory_requirement, void* state, const char* application_name, i32 x, i32 y, i32 width, i32 height)
{
    *memory_requirement = sizeof(platform_state);
    if (!state) return FALSE;

    state_ptr = state;

    state_ptr->h_instance = GetModuleHandleA(0);

    HICON icon = LoadIcon(state_ptr->h_instance, IDI_APPLICATION);

    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = win32_process_message;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state_ptr->h_instance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "engine_window_class";

    if (!RegisterClassA(&wc))
    {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    u32 client_x = x;
    u32 client_y = y;
    u32 client_width = width;
    u32 client_height = height;

    u32 window_x = client_x;
    u32 window_y = client_y;
    u32 window_width = client_width;
    u32 window_height = client_height;

    u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 window_ex_style = WS_EX_APPWINDOW;

    window_style |= WS_MAXIMIZEBOX;
    window_style |= WS_MINIMIZEBOX;
    window_style |= WS_THICKFRAME;

    RECT border_rect = {0,0,0,0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    window_x += border_rect.left;
    window_y += border_rect.top;

    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    HWND handle = CreateWindowExA(window_ex_style, "engine_window_class", application_name, window_style, window_x, window_y, window_width, window_height, 0, 0, state_ptr->h_instance, 0);

    if (handle == 0)
    {
        MessageBoxA(NULL, "Window creation failed!", "Error", MB_ICONEXCLAMATION | MB_OK);

        HFATAL("Window creation failed!");
        return FALSE;
    }

    state_ptr->hwnd = handle;

    b32 should_activate = 1;
    i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;

    ShowWindow(state_ptr->hwnd, show_window_command_flags);

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    state_ptr->clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&state_ptr->start_time);

    return TRUE;
}

void platform_shutdown(void* state)
{
    if (state_ptr->hwnd)
    {
        DestroyWindow(state_ptr->hwnd);
        state_ptr->hwnd = 0;
    }
}

b8 platform_pump_messages(void* state)
{
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return TRUE;
}

void* platform_allocate(u64 size, b8 aligned)
{
    return malloc(size);
}

void platform_free(void* block, b8 aligned)
{
    free(block);
}

void* platform_zero_memory(void* block, u64 size)
{
    return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* src, u64 size)
{
    return memcpy(dest, src, size);
}

void* platform_set_memory(void* block, i32 value, u64 size)
{
    return memset(block, value, size);
}

void platform_console_write(const char* message, u8 color)
{
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);

    OutputDebugStringA(message);

    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(console_handle, message, (DWORD)length, number_written, 0);
}

void platform_console_write_error(const char* message, u8 color)
{
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);

    OutputDebugStringA(message);

    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(console_handle, message, (DWORD)length, number_written, 0);
}

f64 platform_get_absolute_time()
{
    if (state_ptr)
    {
        LARGE_INTEGER now_time;
        QueryPerformanceCounter(&now_time);
        return (f64)now_time.QuadPart * state_ptr->clock_frequency;
    }
    return 0;
}

void platform_sleep(u64 ms)
{
    Sleep(ms);
}

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg)
    {
        case WM_CREATE:
            {
                PIXELFORMATDESCRIPTOR pfd = // Creating pixel format descriptor for OpenGL
                {
                    sizeof(PIXELFORMATDESCRIPTOR),
                    1,
                    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
                    PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
                    32,                   // Colordepth of the framebuffer.
                    0, 0, 0, 0, 0, 0,
                    0,
                    0,
                    0,
                    0, 0, 0, 0,
                    24,                   // Number of bits for the depthbuffer
                    8,                    // Number of bits for the stencilbuffer
                    0,                    // Number of Aux buffers in the framebuffer.
                    PFD_MAIN_PLANE,
                    0,
                    0, 0, 0
                };

                state_ptr->window_device_handle = GetDC(hwnd);

                i32 pixel_format = ChoosePixelFormat(state_ptr->window_device_handle, &pfd); 
                SetPixelFormat(state_ptr->window_device_handle,pixel_format, &pfd);
            } break;
        case WM_ERASEBKGND:
            return 1;
        case WM_CLOSE:
            {
                event_context event = {};
                event_post(EVENT_APPLICATION_QUIT, 0, event);
                return 1;
            }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            {
                RECT r;
                GetClientRect(hwnd, &r);

                event_context event = {};
                event.data.u16[0] = (u16)(r.right - r.left);
                event.data.u16[1] = (u16)(r.bottom - r.top);

                event_post(EVENT_RESIZED, 0, event);
            } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            {
                b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
                keys key = (u16)w_param;

                // Left/Right Alt Key
                if (w_param == VK_MENU)
                {
                    if (GetKeyState(VK_RMENU) & 0x8000)
                    {
                        key = KEY_RALT;
                    }
                    else if (GetKeyState(VK_RMENU) & 0x8000)
                    {
                        key = KEY_RALT;
                    }
                }

                // Left/Right Shift Key
                if (w_param == VK_SHIFT)
                {
                    if (GetKeyState(VK_RSHIFT) & 0x8000)
                    {
                        key = KEY_RSHIFT;
                    }
                    else if (GetKeyState(VK_LSHIFT) & 0x8000)
                    {
                        key = KEY_LSHIFT;
                    }
                }

                // Left/Right Control Key
                if (w_param == VK_CONTROL)
                {
                    if (GetKeyState(VK_RCONTROL) & 0x8000)
                    {
                        key = KEY_RCONTROL;
                    }
                    else if (GetKeyState(VK_LCONTROL) & 0x8000)
                    {
                        key = KEY_LCONTROL;
                    }
                }

                input_process_key(key, pressed);
            } break;
        case WM_MOUSEMOVE:
            i32 x_position = GET_X_LPARAM(l_param);
            i32 y_position = GET_Y_LPARAM(l_param);
            input_process_mouse_move(x_position, y_position);
            break;
        case WM_MOUSEWHEEL:
            i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            if (z_delta != 0) 
            {
                z_delta = (z_delta < 0) ? -1 : 1;
                input_process_mouse_wheel(z_delta);
            }
            break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN;
            buttons button = BUTTON_MAX_BUTTONS;
            switch (msg)
            {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    button = BUTTON_LEFT;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    button = BUTTON_RIGHT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    button = BUTTON_LEFT;
                    break;
            }
            if (button != BUTTON_MAX_BUTTONS)
            {
                input_process_button(button, pressed);
            }
            break;
    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}
void* platform_opengl_context_create()
{
    // Create OpenGL context and make current.
    state_ptr->gl_context = wglCreateContext(state_ptr->window_device_handle);

    if (state_ptr->gl_context == 0)
    {
        int result = GetLastError();
        HFATAL("Failed to create OpenGL context, shutting down. Error code: %d", result);
        return 0;
    }

    if (wglMakeCurrent(state_ptr->window_device_handle, state_ptr->gl_context) == 0)
    {
        int result = GetLastError();
        HFATAL("Failed to make OpenGL context current, shutting down. Error code: %d", result);
        return 0;
    }

    return state_ptr->gl_context;
}

void platform_opengl_context_delete()
{
    wglDeleteContext(state_ptr->gl_context);
}

b8 platform_swap_buffers()
{
    return SwapBuffers(state_ptr->window_device_handle);
}

#endif // HPLATFORM_WINDOWS