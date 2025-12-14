#include <windows.h>
#include <stdio.h> 

static const wchar_t kCls[] = L"TestAdWndClass";

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }

    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.hInstance = hInst;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = kCls;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);


    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);


    ATOM atom = RegisterClassExW(&wc);
    if (!atom) {
        wchar_t buf[256];
        wsprintfW(buf, L"RegisterClassEx failed! Error: %lu", GetLastError());
        MessageBoxW(nullptr, buf, L"Debug", MB_ICONERROR);
        return 0;
    }

    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        kCls,
        L"Test Popup Window",
        WS_POPUP | WS_VISIBLE,
        200, 200, 420, 180,
        nullptr, nullptr, hInst, nullptr
    );


    if (!hwnd) {
        DWORD err = GetLastError();
        wchar_t buf[256];

        wsprintfW(buf, L"CreateWindowEx failed! GetLastError=%lu", err);
        MessageBoxW(nullptr, buf, L"TestPopup", MB_OK | MB_ICONERROR);
        return 0;
    }

    wchar_t buf[256];
    wsprintfW(buf, L"Success! Window created. HWND: %p", hwnd);
    MessageBoxW(nullptr, buf, L"TestPopup", MB_OK);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}