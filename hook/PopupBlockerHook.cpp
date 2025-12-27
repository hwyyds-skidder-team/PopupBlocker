#include <windows.h>
#include <cwctype>
#include<stdlib.h>
#include "RulesShared.h"

static HMODULE g_hModule = nullptr;
static HHOOK   g_hHook = nullptr;

static INIT_ONCE  g_initOnce = INIT_ONCE_STATIC_INIT;
static SharedRules* g_shared = nullptr;

static bool iequals(const wchar_t* a, const wchar_t* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (towlower(*a) != towlower(*b)) return false;
        ++a; ++b;
    }
    return *a == 0 && *b == 0;
}

static void get_basename(const wchar_t* full, wchar_t* out, size_t outCount) {
    if (!out || outCount == 0) return;
    out[0] = 0;
    if (!full) return;
    const wchar_t* p = wcsrchr(full, L'\\');
    const wchar_t* base = p ? (p + 1) : full;
    wcsncpy_s(out, outCount, base, _TRUNCATE);
}

static bool get_process_basename(DWORD pid, wchar_t* out, size_t outCount) {
    if (!out || outCount == 0) return false;
    out[0] = 0;

    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) return false;

    wchar_t buf[MAX_PATH];
    DWORD size = (DWORD)(sizeof(buf) / sizeof(buf[0]));
    BOOL ok = QueryFullProcessImageNameW(h, 0, buf, &size);
    CloseHandle(h);

    if (!ok) return false;
    get_basename(buf, out, outCount);
    return out[0] != 0;
}

static BOOL CALLBACK InitShared(PINIT_ONCE, PVOID, PVOID*) {
    HANDLE hMap = OpenFileMappingW(FILE_MAP_READ, FALSE, kRulesMappingName);
    if (!hMap) return TRUE;

    void* p = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(SharedRules));
    CloseHandle(hMap);

    if (p) g_shared = (SharedRules*)p;
    return TRUE;
}

static bool read_snapshot(SharedRules& out) {
    if (!g_shared || g_shared->magic != kRulesMagic) return false;

    LONG s1 = g_shared->seq;
    if (s1 & 1) return false;

    out.magic = g_shared->magic;
    out.paused = g_shared->paused;
    out.ruleCount = g_shared->ruleCount;
    if (out.ruleCount > kMaxRules) out.ruleCount = kMaxRules;

    for (uint32_t i = 0; i < out.ruleCount; ++i) out.rules[i] = g_shared->rules[i];

    LONG s2 = g_shared->seq;
    return (s1 == s2) && !(s2 & 1);
}

static bool match_rule(const Rule& r,
    const wchar_t* procBase,
    const wchar_t* cls,
    uint32_t style,
    uint32_t exStyle,
    int w, int h) {
    if (!r.enabled) return false;

   
    if (r.processName[0] == 0) return false;
    if (!procBase || !iequals(r.processName, procBase)) return false;

    if (r.className[0] != 0) {
        if (!cls || !iequals(r.className, cls)) return false;
    }

    if ((style & r.styleMustHaveMask) != r.styleMustHaveValue) return false;
    if ((exStyle & r.exStyleMustHaveMask) != r.exStyleMustHaveValue) return false;

    if (r.minW > 0 && w < r.minW) return false;
    if (r.minH > 0 && h < r.minH) return false;
    if (r.maxW > 0 && w > r.maxW) return false;
    if (r.maxH > 0 && h > r.maxH) return false;

    return true;
}

static bool get_window_info(HWND hwnd,
    wchar_t* procBase, size_t procCount,
    wchar_t* cls, size_t clsCount,
    uint32_t& style, uint32_t& exStyle,
    int& w, int& h) {
    if (!hwnd) return false;

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (!get_process_basename(pid, procBase, procCount)) return false;

    if (!GetClassNameW(hwnd, cls, (int)clsCount)) return false;

    style = (uint32_t)GetWindowLongPtrW(hwnd, GWL_STYLE);
    exStyle = (uint32_t)GetWindowLongPtrW(hwnd, GWL_EXSTYLE);

    RECT rc{};
    if (GetWindowRect(hwnd, &rc)) {
        w = rc.right - rc.left;
        h = rc.bottom - rc.top;
    }
    else {
        w = 0;
        h = 0;
    }

    return true;
}

static void apply_action(HWND hwnd, Action action) {
    switch (action) {
    case Action::Hide:
        ShowWindow(hwnd, SW_HIDE);
        break;
    case Action::RemoveTopmost:
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        break;
    default:
        break;
    }
}

static LRESULT CALLBACK CbtProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HCBT_CREATEWND) {
        InitOnceExecuteOnce(&g_initOnce, InitShared, nullptr, nullptr);

        SharedRules snap{};
        if (!read_snapshot(snap)) return CallNextHookEx(g_hHook, code, wParam, lParam);
        if (snap.paused) return CallNextHookEx(g_hHook, code, wParam, lParam);
        if (snap.ruleCount == 0) return CallNextHookEx(g_hHook, code, wParam, lParam);

        CBT_CREATEWND* cbt = (CBT_CREATEWND*)lParam;
        if (!cbt || !cbt->lpcs) return CallNextHookEx(g_hHook, code, wParam, lParam);

        CREATESTRUCTW* cs = (CREATESTRUCTW*)cbt->lpcs;

       
        if ((cs->style & WS_CHILD) != 0) return CallNextHookEx(g_hHook, code, wParam, lParam);

        HWND hwnd = (HWND)wParam;

        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);

        wchar_t procBase[64]{};
        if (!get_process_basename(pid, procBase, _countof(procBase))) {
           
            return CallNextHookEx(g_hHook, code, wParam, lParam);
        }

        wchar_t cls[64]{};
        GetClassNameW(hwnd, cls, _countof(cls));

        const int w = (cs->cx > 0) ? cs->cx : 0;
        const int h = (cs->cy > 0) ? cs->cy : 0;

        for (uint32_t i = 0; i < snap.ruleCount; ++i) {
            const Rule& r = snap.rules[i];
            if (!match_rule(r, procBase, cls, cs->style, cs->dwExStyle, w, h)) continue;

            switch (r.action) {
            case Action::Hide:
                cs->style &= ~WS_VISIBLE;
                cs->dwExStyle &= ~WS_EX_TOPMOST;
                break;
            case Action::RemoveTopmost:
                cs->dwExStyle &= ~WS_EX_TOPMOST;
                break;
            case Action::Block:
                return 1; 
            default:
                break;
            }
            break; 
        }
    }
    else if (code == HCBT_ACTIVATE) {
        InitOnceExecuteOnce(&g_initOnce, InitShared, nullptr, nullptr);

        SharedRules snap{};
        if (!read_snapshot(snap)) return CallNextHookEx(g_hHook, code, wParam, lParam);
        if (snap.paused) return CallNextHookEx(g_hHook, code, wParam, lParam);
        if (snap.ruleCount == 0) return CallNextHookEx(g_hHook, code, wParam, lParam);

        HWND hwnd = (HWND)wParam;
        if (!hwnd) return CallNextHookEx(g_hHook, code, wParam, lParam);

        uint32_t style = 0;
        uint32_t exStyle = 0;
        int w = 0;
        int h = 0;
        wchar_t procBase[64]{};
        wchar_t cls[64]{};
        if (!get_window_info(hwnd, procBase, _countof(procBase), cls, _countof(cls),
                style, exStyle, w, h)) {
            return CallNextHookEx(g_hHook, code, wParam, lParam);
        }

        if ((style & WS_CHILD) != 0) return CallNextHookEx(g_hHook, code, wParam, lParam);

        for (uint32_t i = 0; i < snap.ruleCount; ++i) {
            const Rule& r = snap.rules[i];
            if (!match_rule(r, procBase, cls, style, exStyle, w, h)) continue;
            if (r.action == Action::Hide || r.action == Action::RemoveTopmost) {
                apply_action(hwnd, r.action);
            }
            break;
        }
    }

    return CallNextHookEx(g_hHook, code, wParam, lParam);
}

extern "C" __declspec(dllexport) BOOL InstallGlobalCbtHook() {
    if (g_hHook) return TRUE;
    g_hHook = SetWindowsHookExW(WH_CBT, CbtProc, g_hModule, 0);
    return g_hHook != nullptr;
}

extern "C" __declspec(dllexport) BOOL UninstallGlobalCbtHook() {
    BOOL ok = TRUE;
    if (g_hHook) {
        ok = UnhookWindowsHookEx(g_hHook);
        g_hHook = nullptr;
    }
    return ok;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hModule = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
    }
    return TRUE;
}
