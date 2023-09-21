#include "platform/platform.h"

#if HPLATFORM_WINDOWS

#include <core/logger.h>
#include <core/input.h>

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

typedef struct internal_state
{
    HINSTANCE h_instance;
    HWND hwnd;
} internal_state;

static f64 clock_frequency;
static LARGE_INTEGER start_time;
static HDC window_device_handle;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);
HDC get_window_device_handle(HWND hwnd);

b8 platform_initialize(platform_state* platform_state, const char* application_name, i32 x, i32 y, i32 width, i32 height)
{
    platform_state->internal_state = malloc(sizeof(internal_state));
    internal_state* state = (internal_state*)platform_state->internal_state;

    state->h_instance = GetModuleHandleA(0);

    HICON icon = LoadIcon(state->h_instance, IDI_APPLICATION);

    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = win32_process_message;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state->h_instance;
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

    HWND handle = CreateWindowExA(window_ex_style, "engine_window_class", application_name, window_style, window_x, window_y, window_width, window_height, 0, 0, state->h_instance, 0);

    if (handle == 0)
    {
        MessageBoxA(NULL, "Window creation failed!", "Error", MB_ICONEXCLAMATION | MB_OK);

        HFATAL("Window creation failed!");
        return FALSE;
    }

    state->hwnd = handle;

    b32 should_activate = 1;
    i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;

    ShowWindow(state->hwnd, show_window_command_flags);

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);

    return TRUE;
}

void platform_shutdown(platform_state* platform_state)
{
    internal_state* state = (internal_state*)platform_state->internal_state;

    if (state->hwnd)
    {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

b8 platform_pump_messages(platform_state* platform_state)
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
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
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

                window_device_handle = GetDC(hwnd);

                int pixel_format = ChoosePixelFormat(window_device_handle, &pfd); 
                SetPixelFormat(window_device_handle,pixel_format, &pfd);
            } break;
        case WM_ERASEBKGND:
            return 1;
        case WM_CLOSE:
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            // RECT r;
            // GetClientRect(hwnd, &r);
            // u32 width = r.right - r.left;
            // u32 height = r.bottom - r.top;
            break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            keys key = (u16)w_param;
            input_process_key(key, pressed);
            break;
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
            pressed = msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN;
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
    HGLRC gl_context = wglCreateContext(window_device_handle);

    if (gl_context == 0)
    {
        int result = GetLastError();
        HFATAL("Failed to create OpenGL context, shutting down. Error code: %d", result);
        return 0;
    }

    if (wglMakeCurrent(window_device_handle, gl_context) == 0)
    {
        int result = GetLastError();
        HFATAL("Failed to make OpenGL context current, shutting down. Error code: %d", result);
        return 0;
    }

    return gl_context;
}

void platform_opengl_context_delete(void* gl_context)
{
    wglDeleteContext(gl_context);
}

#endif // HPLATFORM_WINDOWS