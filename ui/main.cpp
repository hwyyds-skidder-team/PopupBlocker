#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <shellapi.h>

#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <optional>
#include <algorithm>
#include <cmath>

#include <nlohmann/json.hpp>

#include <imgui.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "RulesShared.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using nlohmann::json;

namespace Icons {
    const char* Rules = "[R]";
    const char* Events = "[E]";
    const char* Picker = "[P]";
    const char* Settings = "[S]";
    const char* Add = "+";
    const char* Delete = "X";
    const char* Up = "^";
    const char* Down = "v";
    const char* Check = "*";
    const char* Shield = "#";
    const char* Warning = "!";
    const char* Info = "i";
}


namespace Theme {
   
    ImVec4 Primary() { return ImVec4(0.35f, 0.45f, 0.95f, 1.00f); }
    ImVec4 PrimaryHover() { return ImVec4(0.45f, 0.55f, 1.00f, 1.00f); }
    ImVec4 PrimaryActive() { return ImVec4(0.30f, 0.40f, 0.85f, 1.00f); }

   
    ImVec4 Success() { return ImVec4(0.25f, 0.80f, 0.50f, 1.00f); }
    ImVec4 Warning() { return ImVec4(0.95f, 0.75f, 0.25f, 1.00f); }
    ImVec4 Error() { return ImVec4(0.95f, 0.35f, 0.35f, 1.00f); }
    ImVec4 Info() { return ImVec4(0.35f, 0.70f, 0.95f, 1.00f); }

    
    ImVec4 BgDark() { return ImVec4(0.06f, 0.06f, 0.09f, 1.00f); }
    ImVec4 BgMid() { return ImVec4(0.09f, 0.09f, 0.13f, 1.00f); }
    ImVec4 BgLight() { return ImVec4(0.14f, 0.14f, 0.19f, 1.00f); }
    ImVec4 BgCard() { return ImVec4(0.11f, 0.11f, 0.16f, 1.00f); }

   
    ImVec4 Border() { return ImVec4(0.22f, 0.22f, 0.30f, 0.60f); }

   
    ImVec4 Text() { return ImVec4(0.92f, 0.93f, 0.95f, 1.00f); }
    ImVec4 TextDim() { return ImVec4(0.55f, 0.58f, 0.65f, 1.00f); }
    ImVec4 TextDisabled() { return ImVec4(0.40f, 0.42f, 0.48f, 1.00f); }
}


namespace UI {

    // Toggle 
    bool ToggleSwitch(const char* strId, bool* v) {
        ImGui::PushID(strId);

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        float height = 20.0f;
        float width = 40.0f;
        float radius = height * 0.5f;

        bool changed = false;

        ImGui::InvisibleButton("##toggle", ImVec2(width, height));
        if (ImGui::IsItemClicked()) {
            *v = !*v;
            changed = true;
        }

        ImU32 bgColor = *v ? ImGui::ColorConvertFloat4ToU32(Theme::Success())
            : ImGui::ColorConvertFloat4ToU32(ImVec4(0.3f, 0.3f, 0.35f, 1.0f));

        dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), bgColor, radius);

        float knobX = *v ? (pos.x + width - radius - 2) : (pos.x + radius + 2);
        dl->AddCircleFilled(ImVec2(knobX, pos.y + radius), radius - 3, IM_COL32(255, 255, 255, 255));

        ImGui::PopID();
        return changed;
    }

    // ButtonPrimary
    bool ButtonPrimary(const char* label, const ImVec2& size = ImVec2(0, 0)) {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::Primary());
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::PrimaryHover());
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::PrimaryActive());
        bool clicked = ImGui::Button(label, size);
        ImGui::PopStyleColor(3);
        return clicked;
    }

    // ButtonSecondary
    bool ButtonSecondary(const char* label, const ImVec2& size = ImVec2(0, 0)) {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::BgLight());
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.18f, 0.24f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::BgMid());
        bool clicked = ImGui::Button(label, size);
        ImGui::PopStyleColor(3);
        return clicked;
    }

    // ButtonDanger
    bool ButtonDanger(const char* label, const ImVec2& size = ImVec2(0, 0)) {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::Error());
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.45f, 0.45f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.25f, 0.25f, 1.0f));
        bool clicked = ImGui::Button(label, size);
        ImGui::PopStyleColor(3);
        return clicked;
    }

    // Badge
    void Badge(const char* text, const ImVec4& color) {
        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float padding = 6.0f;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 rectMax = ImVec2(pos.x + textSize.x + padding * 2, pos.y + textSize.y + 4);

        dl->AddRectFilled(pos, rectMax, ImGui::ColorConvertFloat4ToU32(color), 4.0f);

        ImGui::SetCursorScreenPos(ImVec2(pos.x + padding, pos.y + 2));
        ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", text);
        ImGui::SetCursorScreenPos(ImVec2(rectMax.x + 8, pos.y));
    }

    // Separator
    void Separator() {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

} // namespace UI



static std::wstring ExeDir() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    wchar_t* p = wcsrchr(path, L'\\');
    if (p) *p = 0;
    return path;
}

static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring out(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), len);
    return out;
}

static std::string WideToUtf8(const std::wstring& ws) {
    if (ws.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    std::string out(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), out.data(), len, nullptr, nullptr);
    return out;
}

static void WcsCopy(wchar_t* dst, size_t dstCch, const std::wstring& src) {
    if (!dst || dstCch == 0) return;
    dst[0] = 0;
    wcsncpy_s(dst, dstCch, src.c_str(), _TRUNCATE);
}

static uint32_t ParseU32(const json& j) {
    if (j.is_number_unsigned()) return j.get<uint32_t>();
    if (j.is_number_integer()) return (uint32_t)j.get<int64_t>();
    if (j.is_string()) {
        auto s = j.get<std::string>();
        return (uint32_t)std::stoul(s, nullptr, 0);
    }
    return 0;
}

static std::wstring GetProcessBaseName(DWORD pid) {
    std::wstring out;
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) return out;
    wchar_t buf[MAX_PATH];
    DWORD size = MAX_PATH;
    if (QueryFullProcessImageNameW(h, 0, buf, &size)) {
        const wchar_t* p = wcsrchr(buf, L'\\');
        out = p ? (p + 1) : buf;
    }
    CloseHandle(h);
    return out;
}

// ==================== DX11 Host ====================

class Dx11Host {
public:
    bool Create(HWND hwnd) {
        DXGI_SWAP_CHAIN_DESC sd{};
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT flags = 0;
#ifdef _DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL fl;
        const D3D_FEATURE_LEVEL fls[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
            fls, 2, D3D11_SDK_VERSION,
            &sd, &swap_, &dev_, &fl, &ctx_
        );
        if (FAILED(hr)) return false;
        return CreateRenderTarget();
    }

    void Cleanup() {
        CleanupRenderTarget();
        if (swap_) { swap_->Release(); swap_ = nullptr; }
        if (ctx_) { ctx_->Release();  ctx_ = nullptr; }
        if (dev_) { dev_->Release();  dev_ = nullptr; }
    }

    void Resize(UINT w, UINT h) {
        if (!swap_ || w == 0 || h == 0) return;
        CleanupRenderTarget();
        swap_->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
        CreateRenderTarget();
    }

    void BeginFrame() {
        float clear[4] = { 0.05f, 0.05f, 0.08f, 1.0f };
        ctx_->OMSetRenderTargets(1, &rtv_, nullptr);
        ctx_->ClearRenderTargetView(rtv_, clear);
    }

    void Present() { swap_->Present(1, 0); }

    ID3D11Device* Device() const { return dev_; }
    ID3D11DeviceContext* Ctx() const { return ctx_; }

private:
    bool CreateRenderTarget() {
        ID3D11Texture2D* back = nullptr;
        if (FAILED(swap_->GetBuffer(0, IID_PPV_ARGS(&back)))) return false;
        bool ok = SUCCEEDED(dev_->CreateRenderTargetView(back, nullptr, &rtv_));
        back->Release();
        return ok;
    }

    void CleanupRenderTarget() {
        if (rtv_) { rtv_->Release(); rtv_ = nullptr; }
    }

    ID3D11Device* dev_ = nullptr;
    ID3D11DeviceContext* ctx_ = nullptr;
    IDXGISwapChain* swap_ = nullptr;
    ID3D11RenderTargetView* rtv_ = nullptr;
};

// ==================== Hook Controller ====================

class HookController {
public:
    bool LoadFromDir(const std::wstring& dir) {
        dllPath_ = dir + L"\\PopupBlockerHook.dll";
        mod_ = LoadLibraryW(dllPath_.c_str());
        if (!mod_) return false;
        install_ = (PFN)GetProcAddress(mod_, "InstallGlobalCbtHook");
        uninstall_ = (PFN)GetProcAddress(mod_, "UninstallGlobalCbtHook");
        return install_ && uninstall_;
    }

    bool Install() {
        if (!install_) return false;
        installed_ = install_() ? true : false;
        return installed_;
    }

    bool Uninstall() {
        if (!uninstall_) return false;
        bool ok = uninstall_() ? true : false;
        installed_ = false;
        return ok;
    }

    bool Installed() const { return installed_; }

    ~HookController() {
        if (installed_) Uninstall();
        if (mod_) FreeLibrary(mod_);
    }

private:
    using PFN = BOOL(WINAPI*)();
    std::wstring dllPath_;
    HMODULE mod_ = nullptr;
    PFN install_ = nullptr;
    PFN uninstall_ = nullptr;
    bool installed_ = false;
};

// ==================== Shared Rules Writer ====================

struct RuleCfg {
    bool enabled = true;
    std::wstring processName;
    std::wstring className;
    uint32_t styleMask = 0, styleValue = 0;
    uint32_t exStyleMask = 0, exStyleValue = 0;
    int minW = 0, minH = 0, maxW = 0, maxH = 0;
    Action action = Action::Hide;
};

class SharedRulesWriter {
public:
    bool OpenOrCreate() {
        hMap_ = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
            (DWORD)sizeof(SharedRules), kRulesMappingName);
        if (!hMap_) return false;

        bool existed = (GetLastError() == ERROR_ALREADY_EXISTS);

        shared_ = (SharedRules*)MapViewOfFile(hMap_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedRules));
        if (!shared_) return false;

        hMutex_ = CreateMutexW(nullptr, FALSE, kRulesMutexName);
        if (!hMutex_) return false;

        if (!existed || shared_->magic != kRulesMagic) {
            WaitForSingleObject(hMutex_, INFINITE);
            shared_->magic = kRulesMagic;
            shared_->seq = 0;
            shared_->paused = 0;
            shared_->ruleCount = 0;
            ReleaseMutex(hMutex_);
        }
        return true;
    }

    void Publish(bool paused, const std::vector<RuleCfg>& rules) {
        if (!shared_ || shared_->magic != kRulesMagic) return;

        WaitForSingleObject(hMutex_, INFINITE);

        InterlockedIncrement(&shared_->seq);
        shared_->paused = paused ? 1u : 0u;

        uint32_t n = (uint32_t)std::min<size_t>(rules.size(), kMaxRules);
        shared_->ruleCount = n;

        for (uint32_t i = 0; i < n; ++i) {
            Rule r{};
            r.enabled = rules[i].enabled ? 1u : 0u;
            WcsCopy(r.processName, _countof(r.processName), rules[i].processName);
            WcsCopy(r.className, _countof(r.className), rules[i].className);
            r.styleMustHaveMask = rules[i].styleMask;
            r.styleMustHaveValue = rules[i].styleValue;
            r.exStyleMustHaveMask = rules[i].exStyleMask;
            r.exStyleMustHaveValue = rules[i].exStyleValue;
            r.minW = rules[i].minW; r.minH = rules[i].minH;
            r.maxW = rules[i].maxW; r.maxH = rules[i].maxH;
            r.action = rules[i].action;
            shared_->rules[i] = r;
        }

        InterlockedIncrement(&shared_->seq);
        ReleaseMutex(hMutex_);
    }

    ~SharedRulesWriter() {
        if (shared_) UnmapViewOfFile(shared_);
        if (hMap_) CloseHandle(hMap_);
        if (hMutex_) CloseHandle(hMutex_);
    }

private:
    HANDLE hMap_ = nullptr;
    HANDLE hMutex_ = nullptr;
    SharedRules* shared_ = nullptr;
};

// ==================== Rules Store ====================

class RulesStore {
public:
    bool Load(const std::wstring& path) {
        path_ = path;
        std::ifstream in(WideToUtf8(path), std::ios::binary);
        if (!in) return false;

        json j;
        try {
            in >> j;
        }
        catch (...) {
            return false;
        }

        paused_ = j.value("paused", false);

        std::vector<RuleCfg> out;
        if (j.contains("rules") && j["rules"].is_array()) {
            for (auto& it : j["rules"]) {
                RuleCfg r;
                r.enabled = it.value("enabled", true);
                r.processName = Utf8ToWide(it.value("processName", ""));
                r.className = Utf8ToWide(it.value("className", ""));

                r.styleMask = ParseU32(it.value("styleMask", "0"));
                r.styleValue = ParseU32(it.value("styleValue", "0"));
                r.exStyleMask = ParseU32(it.value("exStyleMask", "0"));
                r.exStyleValue = ParseU32(it.value("exStyleValue", "0"));

                r.minW = it.value("minW", 0);
                r.minH = it.value("minH", 0);
                r.maxW = it.value("maxW", 0);
                r.maxH = it.value("maxH", 0);

                std::string act = it.value("action", "Hide");
                if (_stricmp(act.c_str(), "Hide") == 0) r.action = Action::Hide;
                else if (_stricmp(act.c_str(), "RemoveTopmost") == 0) r.action = Action::RemoveTopmost;
                else if (_stricmp(act.c_str(), "Block") == 0) r.action = Action::Block;
                else r.action = Action::None;

                if (r.processName.empty()) r.enabled = false;
                out.push_back(std::move(r));
            }
        }
        rules_ = std::move(out);
        return true;
    }

    bool Save() const {
        if (path_.empty()) return false;

        json j;
        j["version"] = 1;
        j["paused"] = paused_;
        j["rules"] = json::array();

        for (auto& r : rules_) {
            json it;
            it["enabled"] = r.enabled;
            it["processName"] = WideToUtf8(r.processName);
            it["className"] = WideToUtf8(r.className);
            it["styleMask"] = r.styleMask;
            it["styleValue"] = r.styleValue;
            it["exStyleMask"] = r.exStyleMask;
            it["exStyleValue"] = r.exStyleValue;
            it["minW"] = r.minW; it["minH"] = r.minH;
            it["maxW"] = r.maxW; it["maxH"] = r.maxH;

            const char* act = "None";
            if (r.action == Action::Hide) act = "Hide";
            else if (r.action == Action::RemoveTopmost) act = "RemoveTopmost";
            else if (r.action == Action::Block) act = "Block";
            it["action"] = act;

            j["rules"].push_back(it);
        }

        std::ofstream out(WideToUtf8(path_), std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out << j.dump(2);
        return true;
    }

    std::vector<RuleCfg>& Rules() { return rules_; }
    bool& Paused() { return paused_; }
    bool  Paused() const { return paused_; }
    const std::wstring& Path() const { return path_; }

private:
    std::wstring path_;
    bool paused_ = false;
    std::vector<RuleCfg> rules_;
};

// ==================== WinEvent Monitor ====================

struct WindowEvent {
    uint64_t id = 0;
    DWORD pid = 0;
    HWND hwnd = nullptr;
    std::wstring process;
    std::wstring cls;
    std::wstring title;
    DWORD style = 0;
    DWORD exStyle = 0;
    RECT rect{};
};

class WinEventMonitor {
public:
    bool Start() {
        std::lock_guard<std::mutex> _g(mu_);
        if (hook_) return true;
        self_ = this;

        hook_ = SetWinEventHook(
            EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW,
            nullptr,
            &WinEventMonitor::Callback,
            0, 0,
            WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
        );
        return hook_ != nullptr;
    }

    void Stop() {
        std::lock_guard<std::mutex> _g(mu_);
        if (hook_) {
            UnhookWinEvent(hook_);
            hook_ = nullptr;
        }
        self_ = nullptr;
    }

    std::vector<WindowEvent> Snapshot() const {
        std::lock_guard<std::mutex> _g(mu_);
        return events_;
    }

    void Clear() {
        std::lock_guard<std::mutex> _g(mu_);
        events_.clear();
    }

    size_t Count() const {
        std::lock_guard<std::mutex> _g(mu_);
        return events_.size();
    }

    ~WinEventMonitor() { Stop(); }

private:
    static void CALLBACK Callback(HWINEVENTHOOK, DWORD event, HWND hwnd,
        LONG idObject, LONG idChild, DWORD, DWORD) {
        if (!self_) return;
        if (event != EVENT_OBJECT_SHOW) return;
        if (!IsWindow(hwnd)) return;
        if (idObject != OBJID_WINDOW || idChild != 0) return;

        HWND top = GetAncestor(hwnd, GA_ROOT);
        if (!top) return;

        DWORD pid = 0;
        GetWindowThreadProcessId(top, &pid);
        if (!pid) return;

        WindowEvent e;
        e.id = ++self_->nextId_;
        e.pid = pid;
        e.hwnd = top;
        e.process = GetProcessBaseName(pid);

        wchar_t buf[256]{};
        GetClassNameW(top, buf, 256);
        e.cls = buf;

        GetWindowTextW(top, buf, 256);
        e.title = buf;

        e.style = (DWORD)GetWindowLongPtrW(top, GWL_STYLE);
        e.exStyle = (DWORD)GetWindowLongPtrW(top, GWL_EXSTYLE);
        GetWindowRect(top, &e.rect);

        if (e.style & WS_CHILD) return;

        std::lock_guard<std::mutex> _g(self_->mu_);
        self_->events_.push_back(std::move(e));
        if (self_->events_.size() > 300)
            self_->events_.erase(self_->events_.begin(), self_->events_.begin() + 50);
    }

    inline static WinEventMonitor* self_ = nullptr;

    mutable std::mutex mu_;
    HWINEVENTHOOK hook_ = nullptr;
    std::vector<WindowEvent> events_;
    uint64_t nextId_ = 0;
};

// ==================== 主题设置 ====================

static void ApplyModernTheme() {
    ImGuiStyle& s = ImGui::GetStyle();

    // 圆角
    s.WindowRounding = 8.0f;
    s.ChildRounding = 6.0f;
    s.FrameRounding = 4.0f;
    s.PopupRounding = 6.0f;
    s.ScrollbarRounding = 6.0f;
    s.GrabRounding = 4.0f;
    s.TabRounding = 4.0f;

    // 边距
    s.WindowPadding = ImVec2(12, 12);
    s.FramePadding = ImVec2(8, 4);
    s.ItemSpacing = ImVec2(8, 6);
    s.ItemInnerSpacing = ImVec2(6, 4);
    s.ScrollbarSize = 12.0f;

    // 边框
    s.WindowBorderSize = 1.0f;
    s.ChildBorderSize = 1.0f;
    s.FrameBorderSize = 0.0f;

    // 颜色
    ImVec4* c = s.Colors;

    c[ImGuiCol_WindowBg] = Theme::BgMid();
    c[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);  // 透明，避免重叠问题
    c[ImGuiCol_PopupBg] = Theme::BgCard();
    c[ImGuiCol_Border] = Theme::Border();

    c[ImGuiCol_TitleBg] = Theme::BgDark();
    c[ImGuiCol_TitleBgActive] = Theme::BgMid();
    c[ImGuiCol_TitleBgCollapsed] = Theme::BgDark();

    c[ImGuiCol_ScrollbarBg] = Theme::BgDark();
    c[ImGuiCol_ScrollbarGrab] = Theme::BgLight();
    c[ImGuiCol_ScrollbarGrabHovered] = Theme::Primary();
    c[ImGuiCol_ScrollbarGrabActive] = Theme::PrimaryActive();

    c[ImGuiCol_FrameBg] = Theme::BgLight();
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.24f, 1.0f);
    c[ImGuiCol_FrameBgActive] = Theme::BgCard();

    c[ImGuiCol_Button] = Theme::BgLight();
    c[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.22f, 0.30f, 1.0f);
    c[ImGuiCol_ButtonActive] = Theme::Primary();

    c[ImGuiCol_CheckMark] = Theme::Success();

    c[ImGuiCol_Header] = Theme::BgCard();
    c[ImGuiCol_HeaderHovered] = Theme::BgLight();
    c[ImGuiCol_HeaderActive] = Theme::Primary();

    c[ImGuiCol_Separator] = Theme::Border();

    c[ImGuiCol_Tab] = Theme::BgDark();
    c[ImGuiCol_TabHovered] = Theme::Primary();
    c[ImGuiCol_TabActive] = Theme::PrimaryActive();

    c[ImGuiCol_TableHeaderBg] = Theme::BgCard();
    c[ImGuiCol_TableBorderStrong] = Theme::Border();
    c[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.20f, 0.28f, 0.40f);
    c[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.02f);

    c[ImGuiCol_Text] = Theme::Text();
    c[ImGuiCol_TextDisabled] = Theme::TextDisabled();
}

// ==================== App ====================

enum class Page { Rules, Events, Picker, Settings };

class App {
public:
    bool Init(HWND hwnd, Dx11Host* dx) {
        hwnd_ = hwnd;
        dx_ = dx;

        auto dir = ExeDir();
        rulesPath_ = dir + L"\\rules.json";

        if (!writer_.OpenOrCreate()) return false;

        store_.Load(rulesPath_);
        writer_.Publish(store_.Paused(), store_.Rules());

        if (hook_.LoadFromDir(dir)) {
            hook_.Install();
        }

        monitor_.Start();
        RegisterHotKey(hwnd_, kHotkeyId, MOD_CONTROL | MOD_ALT, 'X');

        return true;
    }

    void Shutdown() {
        UnregisterHotKey(hwnd_, kHotkeyId);
        monitor_.Stop();
    }

    void OnHotkey() {
        POINT pt{};
        GetCursorPos(&pt);
        HWND h = WindowFromPoint(pt);
        h = GetAncestor(h, GA_ROOT);
        if (!h) return;

        WindowEvent e;
        e.hwnd = h;
        GetWindowThreadProcessId(h, &e.pid);
        e.process = GetProcessBaseName(e.pid);

        wchar_t buf[256]{};
        GetClassNameW(h, buf, 256); e.cls = buf;
        GetWindowTextW(h, buf, 256); e.title = buf;
        e.style = (DWORD)GetWindowLongPtrW(h, GWL_STYLE);
        e.exStyle = (DWORD)GetWindowLongPtrW(h, GWL_EXSTYLE);
        GetWindowRect(h, &e.rect);

        picked_ = e;
        page_ = Page::Picker;
    }

    void Render() {
        DrawMainWindow();
    }

private:
    void ApplyNow() {
        writer_.Publish(store_.Paused(), store_.Rules());
    }

    // ==================== 顶部栏 ====================
    void DrawTopBar() {
        // 固定高度的顶部栏
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::BgCard());
        ImGui::BeginChild("##topbar", ImVec2(0, 56), true);

        // 标题
        ImGui::SetCursorPosY(16);
        ImGui::Text("PopupBlocker");

        // 状态
        ImGui::SameLine(0, 20);
        if (hook_.Installed()) {
            UI::Badge("ACTIVE", Theme::Success());
        }
        else {
            UI::Badge("INACTIVE", Theme::Error());
        }

        // 暂停开关
        ImGui::SameLine(0, 30);
        ImGui::SetCursorPosY(16);
        ImGui::Text("Paused:");
        ImGui::SameLine();
        bool paused = store_.Paused();
        if (UI::ToggleSwitch("##pause", &paused)) {
            store_.Paused() = paused;
            ApplyNow();
        }

        // 右侧按钮 - 使用固定位置
        float rightX = ImGui::GetWindowWidth() - 340;
        if (rightX > 300) {
            ImGui::SameLine();
            ImGui::SetCursorPosX(rightX);
            ImGui::SetCursorPosY(12);

            if (UI::ButtonSecondary("Apply", ImVec2(60, 28))) ApplyNow();
            ImGui::SameLine();
            if (UI::ButtonSecondary("Save", ImVec2(60, 28))) store_.Save();
            ImGui::SameLine();
            if (UI::ButtonSecondary("Reload", ImVec2(60, 28))) {
                store_.Load(rulesPath_);
                ApplyNow();
            }
            ImGui::SameLine();

            if (hook_.Installed()) {
                if (UI::ButtonDanger("Unhook", ImVec2(70, 28))) hook_.Uninstall();
            }
            else {
                if (UI::ButtonPrimary("Hook", ImVec2(70, 28))) hook_.Install();
            }
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    // ==================== 侧边栏 ====================
    void DrawSidebar() {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::BgDark());
        ImGui::BeginChild("##sidebar", ImVec2(180, 0), true);

        ImGui::Spacing();

        auto navButton = [&](Page p, const char* icon, const char* label) {
            bool active = (page_ == p);

            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Button, Theme::Primary());
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::Primary());
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::BgLight());
            }

            char buf[64];
            snprintf(buf, sizeof(buf), " %s  %s", icon, label);

            if (ImGui::Button(buf, ImVec2(-1, 36))) {
                page_ = p;
            }

            ImGui::PopStyleColor(2);
            };

        navButton(Page::Rules, Icons::Rules, "Rules");

        // Events 带数量显示
        {
            bool active = (page_ == Page::Events);
            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Button, Theme::Primary());
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::Primary());
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::BgLight());
            }

            size_t count = monitor_.Count();
            char buf[64];
            if (count > 0) {
                snprintf(buf, sizeof(buf), " %s  Events (%zu)", Icons::Events, count > 99 ? 99 : count);
            }
            else {
                snprintf(buf, sizeof(buf), " %s  Events", Icons::Events);
            }

            if (ImGui::Button(buf, ImVec2(-1, 36))) {
                page_ = Page::Events;
            }
            ImGui::PopStyleColor(2);
        }

        navButton(Page::Picker, Icons::Picker, "Picker");
        navButton(Page::Settings, Icons::Settings, "Settings");

        // 底部提示
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60);
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextDim());
        ImGui::TextWrapped("Tip: Ctrl+Alt+X to pick a window");
        ImGui::PopStyleColor();

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    // ==================== Rules 页面 ====================
    void DrawRulesPage() {
        // 工具栏
        if (UI::ButtonPrimary("+ Add Rule", ImVec2(100, 30))) {
            RuleCfg r;
            r.enabled = true;
            r.action = Action::Hide;
            store_.Rules().push_back(r);
        }
        ImGui::SameLine();
        ImGui::TextColored(Theme::TextDim(), "%zu rules", store_.Rules().size());

        UI::Separator();

        // 表格
        auto& rules = store_.Rules();

        ImGuiTableFlags tableFlags =
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_ScrollY;

        if (ImGui::BeginTable("##rules", 7, tableFlags, ImVec2(0, 0))) {
            ImGui::TableSetupColumn("On", ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableSetupColumn("Process", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Class", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 120);
            ImGui::TableSetupColumn("Topmost", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableSetupColumn("Ops", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            for (int i = 0; i < (int)rules.size(); ++i) {
                ImGui::TableNextRow();
                ImGui::PushID(i);

             
                ImGui::TableNextColumn();
                bool enabled = rules[i].enabled;
                if (UI::ToggleSwitch("##en", &enabled)) {
                    rules[i].enabled = enabled;
                }

              
                ImGui::TableNextColumn();
                std::string procUtf8 = WideToUtf8(rules[i].processName);
                char procBuf[128];
                strncpy_s(procBuf, procUtf8.c_str(), _TRUNCATE);
                ImGui::SetNextItemWidth(-1);
                if (ImGui::InputText("##proc", procBuf, 128)) {
                    rules[i].processName = Utf8ToWide(procBuf);
                }

               
                ImGui::TableNextColumn();
                std::string clsUtf8 = WideToUtf8(rules[i].className);
                char clsBuf[128];
                strncpy_s(clsBuf, clsUtf8.c_str(), _TRUNCATE);
                ImGui::SetNextItemWidth(-1);
                if (ImGui::InputText("##cls", clsBuf, 128)) {
                    rules[i].className = Utf8ToWide(clsBuf);
                }

                
                ImGui::TableNextColumn();
                int act = (rules[i].action == Action::Hide) ? 0 :
                    (rules[i].action == Action::RemoveTopmost ? 1 :
                        (rules[i].action == Action::Block ? 2 : 3));
                const char* items[] = { "Hide", "RemoveTopmost", "Block", "None" };
                ImGui::SetNextItemWidth(-1);
                if (ImGui::Combo("##act", &act, items, 4)) {
                    rules[i].action = (act == 0) ? Action::Hide :
                        (act == 1) ? Action::RemoveTopmost :
                        (act == 2) ? Action::Block : Action::None;
                }

                ImGui::TableNextColumn();
                bool wantTopmost = (rules[i].exStyleMask == 0x00000008 &&
                    rules[i].exStyleValue == 0x00000008);
                if (ImGui::Checkbox("##tm", &wantTopmost)) {
                    if (wantTopmost) {
                        rules[i].exStyleMask = 0x00000008;
                        rules[i].exStyleValue = 0x00000008;
                    }
                    else {
                        rules[i].exStyleMask = 0;
                        rules[i].exStyleValue = 0;
                    }
                }

             
                ImGui::TableNextColumn();
                if (rules[i].maxW > 0 || rules[i].maxH > 0) {
                    ImGui::Text("%d-%d", rules[i].minW, rules[i].maxW);
                }
                else {
                    ImGui::TextColored(Theme::TextDim(), "any");
                }

                
                ImGui::TableNextColumn();
                if (ImGui::SmallButton("^") && i > 0)
                    std::swap(rules[i], rules[i - 1]);
                ImGui::SameLine(0, 2);
                if (ImGui::SmallButton("v") && i + 1 < (int)rules.size())
                    std::swap(rules[i], rules[i + 1]);
                ImGui::SameLine(0, 2);
                ImGui::PushStyleColor(ImGuiCol_Button, Theme::Error());
                if (ImGui::SmallButton("X")) {
                    rules.erase(rules.begin() + i);
                    --i;
                }
                ImGui::PopStyleColor();

                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }


    void DrawEventsPage() {
        if (UI::ButtonSecondary("Clear All", ImVec2(80, 28)))
            monitor_.Clear();

        ImGui::SameLine();
        auto events = monitor_.Snapshot();
        ImGui::TextColored(Theme::TextDim(), "%zu events", events.size());

        UI::Separator();

        ImGuiTableFlags tableFlags =
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_ScrollY;

        if (ImGui::BeginTable("##events", 6, tableFlags, ImVec2(0, 0))) {
            ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Process", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Class", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("ExStyle", ImGuiTableColumnFlags_WidthFixed, 90);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 120);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

          
            for (int i = (int)events.size() - 1; i >= 0; --i) {
                auto& e = events[i];
                ImGui::TableNextRow();
                ImGui::PushID((int)e.id);

                ImGui::TableNextColumn();
                ImGui::Text("%lu", (unsigned long)e.pid);

                ImGui::TableNextColumn();
                ImGui::Text("%s", WideToUtf8(e.process).c_str());

                ImGui::TableNextColumn();
                ImGui::TextColored(Theme::Info(), "%s", WideToUtf8(e.cls).c_str());

                ImGui::TableNextColumn();
                std::string titleStr = WideToUtf8(e.title);
                if (titleStr.length() > 25) titleStr = titleStr.substr(0, 22) + "...";
                ImGui::Text("%s", titleStr.c_str());

                ImGui::TableNextColumn();
                if (e.exStyle & 0x00000008) {
                    ImGui::TextColored(Theme::Warning(), "0x%X", (unsigned)e.exStyle);
                }
                else {
                    ImGui::Text("0x%X", (unsigned)e.exStyle);
                }

                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Pick")) {
                    picked_ = e;
                    page_ = Page::Picker;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("+Rule")) {
                    RuleCfg r;
                    r.enabled = true;
                    r.processName = e.process;
                    r.className = e.cls;
                    if (e.exStyle & 0x00000008) {
                        r.exStyleMask = 0x00000008;
                        r.exStyleValue = 0x00000008;
                    }
                    r.action = Action::Hide;
                    if (!r.processName.empty())
                        store_.Rules().push_back(std::move(r));
                }

                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }


    void DrawPickerPage() {
        ImGui::TextColored(Theme::Info(), "Hotkey: Ctrl + Alt + X");
        ImGui::TextWrapped("Hover over any popup window and press the hotkey to capture its properties.");

        UI::Separator();

        if (!picked_.has_value()) {
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::TextColored(Theme::TextDim(), "No window picked yet.");
            ImGui::TextColored(Theme::TextDisabled(), "Use the hotkey to pick a window.");
            return;
        }

        auto& e = *picked_;

       
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::BgDark());
        ImGui::BeginChild("##info", ImVec2(0, 200), true);

        ImGui::Text("HWND:    0x%p", e.hwnd);
        ImGui::Text("PID:     %lu", (unsigned long)e.pid);
        ImGui::Text("Process: "); ImGui::SameLine();
        ImGui::TextColored(Theme::Success(), "%s", WideToUtf8(e.process).c_str());
        ImGui::Text("Class:   "); ImGui::SameLine();
        ImGui::TextColored(Theme::Warning(), "%s", WideToUtf8(e.cls).c_str());
        ImGui::Text("Title:   %s", WideToUtf8(e.title).c_str());
        ImGui::Text("Style:   0x%08X", (unsigned)e.style);
        ImGui::Text("ExStyle: 0x%08X", (unsigned)e.exStyle);
        if (e.exStyle & 0x00000008) {
            ImGui::SameLine();
            ImGui::TextColored(Theme::Warning(), "(TOPMOST)");
        }
        ImGui::Text("Rect:    (%ld,%ld)-(%ld,%ld) [%ldx%ld]",
            e.rect.left, e.rect.top, e.rect.right, e.rect.bottom,
            e.rect.right - e.rect.left, e.rect.bottom - e.rect.top);

        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::Spacing();

        
        if (UI::ButtonPrimary("Create Hide Rule", ImVec2(140, 36))) {
            RuleCfg r;
            r.enabled = true;
            r.processName = e.process;
            r.className = e.cls;
            if (e.exStyle & 0x00000008) {
                r.exStyleMask = 0x00000008;
                r.exStyleValue = 0x00000008;
            }
            r.action = Action::Hide;
            if (!r.processName.empty()) store_.Rules().push_back(std::move(r));
            page_ = Page::Rules;
        }

        ImGui::SameLine();

        if (UI::ButtonDanger("Create Block Rule", ImVec2(140, 36))) {
            RuleCfg r;
            r.enabled = true;
            r.processName = e.process;
            r.className = e.cls;
            if (e.exStyle & 0x00000008) {
                r.exStyleMask = 0x00000008;
                r.exStyleValue = 0x00000008;
            }
            r.action = Action::Block;
            if (!r.processName.empty()) store_.Rules().push_back(std::move(r));
            page_ = Page::Rules;
        }
    }


    void DrawSettingsPage() {
        ImGui::Text("Safety Guidelines");
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.10f, 0.08f, 1.0f));
        ImGui::BeginChild("##safety", ImVec2(0, 100), true);
        ImGui::BulletText("Rules with empty ProcessName are auto-disabled");
        ImGui::BulletText("Start with Action=Hide, then use Block if needed");
        ImGui::BulletText("For 32-bit apps, run the x86 version");
        ImGui::BulletText("Always test rules before saving");
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Text("Configuration");
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::BgDark());
        ImGui::BeginChild("##config", ImVec2(0, 60), true);
        ImGui::Text("Rules file: %s", WideToUtf8(rulesPath_).c_str());
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Text("Statistics");
        ImGui::Spacing();

        size_t enabledCount = 0;
        for (auto& r : store_.Rules()) if (r.enabled) enabledCount++;

        ImGui::Text("Total Rules: %zu", store_.Rules().size());
        ImGui::Text("Active Rules: %zu", enabledCount);
        ImGui::Text("Events Captured: %zu", monitor_.Count());
        ImGui::Text("Hook Status: %s", hook_.Installed() ? "Active" : "Inactive");
    }


    void DrawMainWindow() {
     
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("##main", nullptr, windowFlags);

        // TopBar
        DrawTopBar();
        ImGui::Spacing();
        float contentHeight = ImGui::GetContentRegionAvail().y;
        ImGui::BeginChild("##body", ImVec2(0, contentHeight), false);
        float sidebarWidth = 180.0f;
        // sidebar
        DrawSidebar();
        ImGui::SameLine();

       
        ImGui::BeginChild("##content", ImVec2(0, 0), false);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));

        
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::BgCard());
        ImGui::BeginChild("##contentCard", ImVec2(0, 0), true);

        switch (page_) {
        case Page::Rules:    DrawRulesPage(); break;
        case Page::Events:   DrawEventsPage(); break;
        case Page::Picker:   DrawPickerPage(); break;
        case Page::Settings: DrawSettingsPage(); break;
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::PopStyleVar();
        ImGui::EndChild();

        ImGui::EndChild();

        ImGui::End();
    }

private:
    static constexpr int kHotkeyId = 0xBEEF;

    HWND hwnd_ = nullptr;
    Dx11Host* dx_ = nullptr;

    std::wstring rulesPath_;
    Page page_ = Page::Rules;

    SharedRulesWriter writer_;
    RulesStore store_;
    HookController hook_;
    WinEventMonitor monitor_;

    std::optional<WindowEvent> picked_;
};

// ==================== Win32 ====================

static Dx11Host g_dx;
static App g_app;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            UINT w = LOWORD(lParam);
            UINT h = HIWORD(lParam);
            if (w > 0 && h > 0) {
                g_dx.Resize(w, h);
            }
        }
        return 0;
    case WM_HOTKEY:
        g_app.OnHotkey();
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.hInstance = hInst;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = L"PopupBlockerUIWnd";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(15, 15, 20));
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
        0, wc.lpszClassName, L"PopupBlocker",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1100, 700,
        nullptr, nullptr, hInst, nullptr
    );
    if (!hwnd) return 0;

    if (!g_dx.Create(hwnd)) return 0;

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;


    io.Fonts->AddFontDefault();

    ApplyModernTheme();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_dx.Device(), g_dx.Ctx());

    if (!g_app.Init(hwnd, &g_dx)) return 0;

    MSG msg;
    while (true) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) goto exit;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        g_app.Render();

        ImGui::Render();
        g_dx.BeginFrame();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_dx.Present();
    }

exit:
    g_app.Shutdown();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    g_dx.Cleanup();
    return 0;
}