#include "Window.h"

Window* window = nullptr;

Window::Window() {

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_CREATE:
            // event fired when window will be created
            window->onCreate();
            break;
        case WM_DESTROY:
            // event fired when window will be destroyed
            window->onDestroy();
            ::PostQuitMessage(0);
            break;
        default:
            return ::DefWindowProc(hwnd, msg, wparam, lparam);
    }

    return NULL;
}
bool Window::init() {
    // setting up WNDCLASSEXA object
    WNDCLASSEXA wc;
    wc.cbClsExtra = NULL;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = NULL;
    wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = NULL;
    wc.lpszClassName = "MyWindowClass";
    wc.lpszMenuName = "";
    wc.style = NULL;
    wc.lpfnWndProc = &WndProc;

    // if the registration of the class fails, return false
    if (!::RegisterClassExA(&wc)) {
        return false;
    }

    if (!window) {
        window = this;
    }

    m_hwnd = ::CreateWindowExA(WS_EX_OVERLAPPEDWINDOW, "MyWindowClass", "DirectX Application", WS_EX_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768, NULL, NULL, NULL, NULL);

    // if the creation fails, return false
    if (!m_hwnd) {
        return false;
    }

    //show the window
    ::ShowWindow(m_hwnd, SW_SHOW);
    ::UpdateWindow(m_hwnd);

    m_is_running = true;

    return true;
}

bool Window::broadcast() {
    MSG msg;

    while (::PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    window->onUpdate();

    Sleep(0);
}

bool Window::release() {
    // destroy the window
    if (m_hwnd && !::DestroyWindow(m_hwnd)) {
        return false;
    }

    m_is_running = false;

    return true;
}

bool Window::isRunning() {
    return m_is_running;
}

Window::~Window() {
    
}