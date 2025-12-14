
# PopupBlocker

**PopupBlocker** is a lightweight, high-performance Windows utility designed to suppress annoying popups and unwanted windows at the system level.

Unlike traditional automation scripts that close windows after they appear, PopupBlocker utilizes a **Global CBT (Computer-Based Training) Hook** to intercept window creation events system-wide. This allows it to modify window attributes or prevent creation entirely *before* the window becomes visible or steals focus.

## 🚀 Key Features

*   **Zero Focus Interruption:** By intercepting the `HCBT_CREATEWND` event, the tool acts before the OS activates the new window. Your full-screen games, typing flow, or remote sessions remain uninterrupted.
*   **System-Wide Integration:** Uses standard Windows Hooks (`SetWindowsHookEx`) to inject logic into all running desktop applications automatically.
*   **High Performance IPC:** Uses **Shared Memory (File Mapping)** to read configuration rules. This ensures that the hook running inside other processes causes zero performance impact and requires no disk I/O during window creation.
*   **Rule-Based System:**
    *   **Process Name:** (e.g., `example.exe`)
    *   **Window Class:** (e.g., `TxGuiFoundation`)
    *   **Window Size:** Filter by min/max width and height.
    *   **Window Styles:** Filter by specific Style or ExStyle flags.
*   **Hot-Reload:** Rules are updated in shared memory instantly. No need to restart the blocker or the target applications.

## 🛠️ Technical Implementation

PopupBlocker differs from aggressive inline hooks (like Detours/MinHook). Instead, it relies on documented Windows mechanisms:

1.  **CBT Hook (`WH_CBT`):**
    The core logic is installed via `SetWindowsHookEx(WH_CBT, ...)`. The OS notifies our DLL whenever a window is about to be created (`HCBT_CREATEWND`).

2.  **Action Logic:**
    *   **Block:** Returns `1` to the OS callback, effectively canceling the window creation request immediately.
    *   **Hide:** Modifies the `CREATESTRUCT` parameters on the fly, removing the `WS_VISIBLE` style and `WS_EX_TOPMOST` flag. The window exists but remains invisible and doesn't steal focus.
    *   **Remove Topmost:** Strips the `WS_EX_TOPMOST` flag, preventing ads from forcing themselves over your other windows.

3.  **Targeting:**
    The hook automatically ignores child windows (`WS_CHILD`), ensuring it only targets top-level windows (popups, dialogs, main windows).

## 📦 Build Instructions

This project is built using **Visual Studio 2022** and **CMake**.

1.  Clone the repository.
2.  Open the folder in Visual Studio.
3.  Select **`x64-Release`** configuration.
4.  Build the project.
    *   *Artifacts produced:* `PopupBlockerUI.exe` (The interface) and `PopupBlockerHook.dll` (The engine).

## 📖 Usage

1.  Run `PopupBlockerUI.exe` as **Administrator** (required to install global hooks).
2.  The status should show **[ACTIVE]**.
3.  **To Add a Rule:**
    *   Hover your mouse over an unwanted window.
    *   Press **`Ctrl + Alt + X`**.
    *   Use the **Picker** tab to select the detected window and click **"+ Rule"**.
4.  **Configure Actions:**
    *   In the **Rules** tab, choose between **Block** (Destroy) or **Hide** (Invisible).
    *   *Tip:* Start with **Hide**. It is safer and less likely to crash the target application than blocking it outright.

## ⚠️ Notes

*   **Architecture:** To block windows in 32-bit applications, you must compile and run the x86 version. To block 64-bit applications, use the x64 version. (Recommended: Run x64 for modern systems).
*   **Font Support:** The UI uses a default ASCII font to remain lightweight. Non-English window titles may appear as `????` in the picker, but the internal matching logic works perfectly with Unicode process names and class names.

## 🤝 Disclaimer

This tool uses DLL injection via Windows Hooks. While this is a standard Windows feature used by accessibility tools and screen readers, some anti-cheat software or strict antivirus heuristics might flag it. Use at your own risk.




