#include <windows.h>
#include <xinput.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

// Real system XInput library handle and function pointers
static HMODULE g_real_xinput = nullptr;

typedef DWORD (WINAPI *PFN_XInputGetState)(DWORD, XINPUT_STATE*);
typedef DWORD (WINAPI *PFN_XInputSetState)(DWORD, XINPUT_VIBRATION*);
typedef DWORD (WINAPI *PFN_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);
typedef DWORD (WINAPI *PFN_XInputGetBatteryInformation)(DWORD, BYTE, XINPUT_BATTERY_INFORMATION*);
typedef DWORD (WINAPI *PFN_XInputGetKeystroke)(DWORD, DWORD, PXINPUT_KEYSTROKE);
typedef DWORD (WINAPI *PFN_XInputGetAudioDeviceIds)(DWORD, LPWSTR, UINT*, LPWSTR, UINT*);
typedef void  (WINAPI *PFN_XInputEnable)(BOOL);

static PFN_XInputGetState              g_pfn_get_state    = nullptr;
static PFN_XInputSetState              g_pfn_set_state    = nullptr;
static PFN_XInputGetCapabilities       g_pfn_get_caps     = nullptr;
static PFN_XInputGetBatteryInformation  g_pfn_get_bat      = nullptr;
static PFN_XInputGetKeystroke          g_pfn_get_keys     = nullptr;
static PFN_XInputGetState              g_pfn_get_state_ex = nullptr;

// ----------------------------------------------------------------------------
// Configuration Settings
// ----------------------------------------------------------------------------
static int   g_input_mode        = 0;       // 0 = Keyboard + Mouse, 1 = Keyboard Only
static float g_mouse_steer_sens  = 1200.0f; // Left stick steering sensitivity
static float g_mouse_cursor_sens = 1.6f;    // Right stick menu cursor sensitivity
static bool  g_mouse_capture     = true;
static bool  g_mouse_locked      = true;    // Cursor locked to window for PC play
static DWORD g_last_packet       = 1;

// Non-overlapping Hotkeys (defaults: F8 and F9 to avoid Xenia F1-F6/F11 overlaps)
static int   g_hotkey_mouse_lock = VK_F8;
static int   g_hotkey_input_mode = VK_F9;

// Analog Stick Smoothing and Accumulators
static float g_steer_accum     = 0.0f;
static float g_cursor_accum_x  = 0.0f;
static float g_cursor_accum_y  = 0.0f;

// Mouse wheel accumulator for rotating 3D carousel menu
static volatile int g_wheel_delta = 0;
static HHOOK g_mouse_hook = nullptr;

// Drift boost pulse accumulator
static bool g_drift_l_prev     = false;
static bool g_drift_r_prev     = false;
static int  g_drift_boost_fade = 0;

// ----------------------------------------------------------------------------
// Custom Keybinding Lists
// ----------------------------------------------------------------------------
struct KeyBinds {
    std::vector<int> steer_left;
    std::vector<int> steer_right;
    std::vector<int> lean_forward;
    std::vector<int> lean_back;
    std::vector<int> drift_left;
    std::vector<int> drift_right;
    std::vector<int> jump_crouch;
    std::vector<int> kick_dash;
    std::vector<int> confirm;
    std::vector<int> cancel_back;
    std::vector<int> use_item;
    std::vector<int> tag_mode;
    std::vector<int> pause;
    std::vector<int> reset_cursor;
    std::vector<int> toggle_cursor;
    std::vector<int> menu_cursor_up;
    std::vector<int> menu_cursor_down;
    std::vector<int> menu_cursor_left;
    std::vector<int> menu_cursor_right;
    std::vector<int> menu_ring_left;
    std::vector<int> menu_ring_right;
    std::vector<int> menu_option_up;
    std::vector<int> menu_option_down;
} g_binds;

// Trim whitespace from string
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Case-insensitive key string parser to Windows Virtual Key code
static int parse_key_code(std::string key) {
    key = trim(key);
    if (key.empty()) return 0;

    std::string ukey = key;
    for (char &c : ukey) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    // Hex codes (e.g. 0x20)
    if (ukey.rfind("0X", 0) == 0) {
        return static_cast<int>(std::strtoul(ukey.c_str(), nullptr, 16));
    }

    // Single character letters & digits
    if (ukey.length() == 1) {
        char c = ukey[0];
        if (c >= 'A' && c <= 'Z') return c;
        if (c >= '0' && c <= '9') return c;
        if (c == '`' || c == '~') return VK_OEM_3;
        if (c == '-') return VK_OEM_MINUS;
        if (c == '=' || c == '+') return VK_OEM_PLUS;
        if (c == '[') return VK_OEM_4;
        if (c == ']') return VK_OEM_6;
        if (c == ';') return VK_OEM_1;
        if (c == '\'') return VK_OEM_7;
        if (c == ',') return VK_OEM_COMMA;
        if (c == '.') return VK_OEM_PERIOD;
        if (c == '/') return VK_OEM_2;
        if (c == '\\') return VK_OEM_5;
    }

    // Function keys
    if (ukey == "F1") return VK_F1;
    if (ukey == "F2") return VK_F2;
    if (ukey == "F3") return VK_F3;
    if (ukey == "F4") return VK_F4;
    if (ukey == "F5") return VK_F5;
    if (ukey == "F6") return VK_F6;
    if (ukey == "F7") return VK_F7;
    if (ukey == "F8") return VK_F8;
    if (ukey == "F9") return VK_F9;
    if (ukey == "F10") return VK_F10;
    if (ukey == "F11") return VK_F11;
    if (ukey == "F12") return VK_F12;

    // Common navigation & action keys
    if (ukey == "SPACE") return VK_SPACE;
    if (ukey == "ENTER" || ukey == "RETURN") return VK_RETURN;
    if (ukey == "ESCAPE" || ukey == "ESC") return VK_ESCAPE;
    if (ukey == "TAB") return VK_TAB;
    if (ukey == "BACKSPACE" || ukey == "BACK") return VK_BACK;
    if (ukey == "SHIFT" || ukey == "LSHIFT") return VK_LSHIFT;
    if (ukey == "RSHIFT") return VK_RSHIFT;
    if (ukey == "CTRL" || ukey == "LCTRL" || ukey == "CONTROL") return VK_LCONTROL;
    if (ukey == "RCTRL") return VK_RCONTROL;
    if (ukey == "ALT" || ukey == "LALT") return VK_LMENU;
    if (ukey == "RALT") return VK_RMENU;
    if (ukey == "UP") return VK_UP;
    if (ukey == "DOWN") return VK_DOWN;
    if (ukey == "LEFT") return VK_LEFT;
    if (ukey == "RIGHT") return VK_RIGHT;
    if (ukey == "INSERT") return VK_INSERT;
    if (ukey == "DELETE" || ukey == "DEL") return VK_DELETE;
    if (ukey == "HOME") return VK_HOME;
    if (ukey == "END") return VK_END;
    if (ukey == "PAGEUP" || ukey == "PGUP") return VK_PRIOR;
    if (ukey == "PAGEDOWN" || ukey == "PGDN") return VK_NEXT;
    if (ukey == "CAPSLOCK") return VK_CAPITAL;
    if (ukey == "SCROLLLOCK") return VK_SCROLL;
    if (ukey == "PAUSE") return VK_PAUSE;
    if (ukey == "TILDE" || ukey == "GRAVE") return VK_OEM_3;

    // Numpad
    if (ukey == "NUMPAD0") return VK_NUMPAD0;
    if (ukey == "NUMPAD1") return VK_NUMPAD1;
    if (ukey == "NUMPAD2") return VK_NUMPAD2;
    if (ukey == "NUMPAD3") return VK_NUMPAD3;
    if (ukey == "NUMPAD4") return VK_NUMPAD4;
    if (ukey == "NUMPAD5") return VK_NUMPAD5;
    if (ukey == "NUMPAD6") return VK_NUMPAD6;
    if (ukey == "NUMPAD7") return VK_NUMPAD7;
    if (ukey == "NUMPAD8") return VK_NUMPAD8;
    if (ukey == "NUMPAD9") return VK_NUMPAD9;

    // Mouse buttons
    if (ukey == "MOUSE_LEFT" || ukey == "LCLICK" || ukey == "MOUSE1") return VK_LBUTTON;
    if (ukey == "MOUSE_RIGHT" || ukey == "RCLICK" || ukey == "MOUSE2") return VK_RBUTTON;
    if (ukey == "MOUSE_MIDDLE" || ukey == "MCLICK" || ukey == "MOUSE3") return VK_MBUTTON;
    if (ukey == "MOUSE_4" || ukey == "MOUSE_SIDE1" || ukey == "XBUTTON1") return VK_XBUTTON1;
    if (ukey == "MOUSE_5" || ukey == "MOUSE_SIDE2" || ukey == "XBUTTON2") return VK_XBUTTON2;

    return 0;
}

// Parse comma-separated key list: e.g. "SPACE, Z, MOUSE_LEFT"
static std::vector<int> parse_key_list(const std::string& line_val) {
    std::vector<int> res;
    size_t start = 0;
    while (start < line_val.length()) {
        size_t comma = line_val.find(',', start);
        std::string token = (comma == std::string::npos) ? line_val.substr(start) : line_val.substr(start, comma - start);
        int code = parse_key_code(token);
        if (code != 0) res.push_back(code);
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    return res;
}

// Set factory default bindings
static void set_default_bindings() {
    g_binds.steer_left        = { 'A', VK_LEFT };
    g_binds.steer_right       = { 'D', VK_RIGHT };
    g_binds.lean_forward      = { 'W', VK_UP };
    g_binds.lean_back         = { 'S', VK_DOWN };
    g_binds.drift_left        = { 'Q', VK_XBUTTON1 };
    g_binds.drift_right       = { 'E', VK_XBUTTON2 };
    g_binds.jump_crouch       = { VK_SPACE, 'Z', VK_LBUTTON };
    g_binds.kick_dash         = { VK_LSHIFT, VK_RSHIFT, 'X', VK_RBUTTON };
    g_binds.confirm           = { VK_RETURN, VK_SPACE, 'Z', VK_LBUTTON };
    g_binds.cancel_back       = { VK_ESCAPE, VK_BACK, 'C' };
    g_binds.use_item          = { 'F', 'R', VK_MBUTTON };
    g_binds.tag_mode          = { 'T', VK_LMENU };
    g_binds.pause             = { 'P', VK_PAUSE };
    g_binds.reset_cursor      = { VK_TAB };
    g_binds.toggle_cursor     = { 'Y', 'V' };
    g_binds.menu_cursor_up    = { VK_UP, 'I', VK_NUMPAD8 };
    g_binds.menu_cursor_down  = { VK_DOWN, 'K', VK_NUMPAD5, VK_NUMPAD2 };
    g_binds.menu_cursor_left  = { VK_LEFT, 'J', VK_NUMPAD4 };
    g_binds.menu_cursor_right = { VK_RIGHT, 'L', VK_NUMPAD6 };
    g_binds.menu_ring_left    = { '1', VK_LEFT, 'A' };
    g_binds.menu_ring_right   = { '2', VK_RIGHT, 'D' };
    g_binds.menu_option_up    = { '3', VK_UP, 'W' };
    g_binds.menu_option_down  = { '4', VK_DOWN, 'S' };
}

static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_MOUSEWHEEL) {
        MSLLHOOKSTRUCT* p = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
        short delta = static_cast<short>(HIWORD(p->mouseData));
        g_wheel_delta += delta;
    }
    return CallNextHookEx(g_mouse_hook, nCode, wParam, lParam);
}

static void load_config() {
    set_default_bindings();

    FILE* f = std::fopen("sonic_kb_mouse.ini", "r");
    if (!f) {
        f = std::fopen("sonic_kb_mouse.ini", "w");
        if (f) {
            std::fprintf(f, "# ====================================================================\n");
            std::fprintf(f, "# Sonic Free Riders - PC Keyboard & Mouse Configuration\n");
            std::fprintf(f, "# Mod by @mrunemployed\n");
            std::fprintf(f, "# ====================================================================\n\n");
            std::fprintf(f, "[Controls]\n");
            std::fprintf(f, "input_mode = 0                 # 0 = Keyboard + Mouse, 1 = Keyboard Only\n");
            std::fprintf(f, "mouse_steering_sensitivity = 1200.0   # Left stick steering responsiveness\n");
            std::fprintf(f, "mouse_cursor_sensitivity   = 1.6      # Right stick cursor speed\n");
            std::fprintf(f, "mouse_capture              = 1        # 1 = Enable mouse tracking, 0 = Disable\n");
            std::fprintf(f, "mouse_locked_default       = 1        # 1 = Cursor locked on launch, 0 = Unlocked\n\n");

            std::fprintf(f, "[Hotkeys]\n");
            std::fprintf(f, "# Change hotkeys to avoid emulator key conflicts (e.g. F8, F9, TILDE)\n");
            std::fprintf(f, "hotkey_mouse_lock = F8         # Toggle Cursor Lock / Unlock\n");
            std::fprintf(f, "hotkey_input_mode = F9         # Toggle Keyboard+Mouse vs Keyboard-Only\n\n");

            std::fprintf(f, "[Keybindings]\n");
            std::fprintf(f, "# Comma-separated keys. Supported: A-Z, 0-9, F1-F12, SPACE, ENTER, ESC,\n");
            std::fprintf(f, "# TAB, BACKSPACE, SHIFT, CTRL, ALT, UP, DOWN, LEFT, RIGHT, TILDE,\n");
            std::fprintf(f, "# MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE, MOUSE_4, MOUSE_5\n\n");

            std::fprintf(f, "# --- Racing Controls ---\n");
            std::fprintf(f, "steer_left    = A, LEFT\n");
            std::fprintf(f, "steer_right   = D, RIGHT\n");
            std::fprintf(f, "lean_forward  = W, UP\n");
            std::fprintf(f, "lean_back     = S, DOWN\n");
            std::fprintf(f, "drift_left    = Q, MOUSE_4\n");
            std::fprintf(f, "drift_right   = E, MOUSE_5\n");
            std::fprintf(f, "jump_crouch   = SPACE, Z, MOUSE_LEFT\n");
            std::fprintf(f, "kick_dash     = LSHIFT, X, MOUSE_RIGHT\n");
            std::fprintf(f, "use_item      = F, R, MOUSE_MIDDLE\n");
            std::fprintf(f, "tag_mode      = T, LALT\n");
            std::fprintf(f, "pause         = P, PAUSE\n\n");

            std::fprintf(f, "# --- Menu & Dialog Controls ---\n");
            std::fprintf(f, "confirm       = ENTER, SPACE, Z, MOUSE_LEFT\n");
            std::fprintf(f, "cancel_back   = ESCAPE, BACKSPACE, C\n");
            std::fprintf(f, "reset_cursor  = TAB\n");
            std::fprintf(f, "toggle_cursor = Y, V\n");
            std::fprintf(f, "menu_cursor_up    = UP, I, NUMPAD8\n");
            std::fprintf(f, "menu_cursor_down  = DOWN, K, NUMPAD5\n");
            std::fprintf(f, "menu_cursor_left  = LEFT, J, NUMPAD4\n");
            std::fprintf(f, "menu_cursor_right = RIGHT, L, NUMPAD6\n");
            std::fprintf(f, "menu_ring_left    = 1, LEFT, A\n");
            std::fprintf(f, "menu_ring_right   = 2, RIGHT, D\n");
            std::fprintf(f, "menu_option_up    = 3, UP, W\n");
            std::fprintf(f, "menu_option_down  = 4, DOWN, S\n");

            std::fclose(f);
        }
        return;
    }

    char line[512];
    while (std::fgets(line, sizeof(line), f)) {
        std::string s(line);
        size_t comment_pos = s.find('#');
        if (comment_pos != std::string::npos) s = s.substr(0, comment_pos);
        s = trim(s);
        if (s.empty() || s[0] == '[') continue;

        size_t eq_pos = s.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key = trim(s.substr(0, eq_pos));
        std::string val = trim(s.substr(eq_pos + 1));
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

        if (key == "input_mode") g_input_mode = std::atoi(val.c_str());
        else if (key == "mouse_steering_sensitivity") g_mouse_steer_sens = static_cast<float>(std::atof(val.c_str()));
        else if (key == "mouse_cursor_sensitivity") g_mouse_cursor_sens = static_cast<float>(std::atof(val.c_str()));
        else if (key == "mouse_capture") g_mouse_capture = (std::atoi(val.c_str()) != 0);
        else if (key == "mouse_locked_default") g_mouse_locked = (std::atoi(val.c_str()) != 0);
        else if (key == "hotkey_mouse_lock") { int c = parse_key_code(val); if (c) g_hotkey_mouse_lock = c; }
        else if (key == "hotkey_input_mode") { int c = parse_key_code(val); if (c) g_hotkey_input_mode = c; }
        else if (key == "steer_left") g_binds.steer_left = parse_key_list(val);
        else if (key == "steer_right") g_binds.steer_right = parse_key_list(val);
        else if (key == "lean_forward") g_binds.lean_forward = parse_key_list(val);
        else if (key == "lean_back") g_binds.lean_back = parse_key_list(val);
        else if (key == "drift_left") g_binds.drift_left = parse_key_list(val);
        else if (key == "drift_right") g_binds.drift_right = parse_key_list(val);
        else if (key == "jump_crouch") g_binds.jump_crouch = parse_key_list(val);
        else if (key == "kick_dash") g_binds.kick_dash = parse_key_list(val);
        else if (key == "confirm") g_binds.confirm = parse_key_list(val);
        else if (key == "cancel_back") g_binds.cancel_back = parse_key_list(val);
        else if (key == "use_item") g_binds.use_item = parse_key_list(val);
        else if (key == "tag_mode") g_binds.tag_mode = parse_key_list(val);
        else if (key == "pause") g_binds.pause = parse_key_list(val);
        else if (key == "reset_cursor") g_binds.reset_cursor = parse_key_list(val);
        else if (key == "toggle_cursor") g_binds.toggle_cursor = parse_key_list(val);
        else if (key == "menu_cursor_up") g_binds.menu_cursor_up = parse_key_list(val);
        else if (key == "menu_cursor_down") g_binds.menu_cursor_down = parse_key_list(val);
        else if (key == "menu_cursor_left") g_binds.menu_cursor_left = parse_key_list(val);
        else if (key == "menu_cursor_right") g_binds.menu_cursor_right = parse_key_list(val);
        else if (key == "menu_ring_left") g_binds.menu_ring_left = parse_key_list(val);
        else if (key == "menu_ring_right") g_binds.menu_ring_right = parse_key_list(val);
        else if (key == "menu_option_up") g_binds.menu_option_up = parse_key_list(val);
        else if (key == "menu_option_down") g_binds.menu_option_down = parse_key_list(val);
    }
    std::fclose(f);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        load_config();

        // Load genuine Windows system XInput library
        char sys_path[MAX_PATH];
        GetSystemDirectoryA(sys_path, MAX_PATH);
        char full_path[MAX_PATH];
        std::snprintf(full_path, sizeof(full_path), "%s\\xinput1_4.dll", sys_path);
        g_real_xinput = LoadLibraryA(full_path);
        if (!g_real_xinput) {
            std::snprintf(full_path, sizeof(full_path), "%s\\xinput1_3.dll", sys_path);
            g_real_xinput = LoadLibraryA(full_path);
        }

        if (g_real_xinput) {
            g_pfn_get_state    = (PFN_XInputGetState)GetProcAddress(g_real_xinput, "XInputGetState");
            g_pfn_set_state    = (PFN_XInputSetState)GetProcAddress(g_real_xinput, "XInputSetState");
            g_pfn_get_caps     = (PFN_XInputGetCapabilities)GetProcAddress(g_real_xinput, "XInputGetCapabilities");
            g_pfn_get_bat      = (PFN_XInputGetBatteryInformation)GetProcAddress(g_real_xinput, "XInputGetBatteryInformation");
            g_pfn_get_keys     = (PFN_XInputGetKeystroke)GetProcAddress(g_real_xinput, "XInputGetKeystroke");
            g_pfn_get_state_ex = (PFN_XInputGetState)GetProcAddress(g_real_xinput, (LPCSTR)100);
        }

        // Install low-level mouse hook for wheel detection
        g_mouse_hook = SetWindowsHookExA(WH_MOUSE_LL, LowLevelMouseProc, hinstDLL, 0);
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        if (g_mouse_hook) {
            UnhookWindowsHookEx(g_mouse_hook);
            g_mouse_hook = nullptr;
        }
        if (g_real_xinput) {
            FreeLibrary(g_real_xinput);
            g_real_xinput = nullptr;
        }
    }
    return TRUE;
}

static inline bool key_pressed(int vk) {
    if (vk <= 0) return false;
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

static inline bool any_key_pressed(const std::vector<int>& keys) {
    for (int vk : keys) {
        if (key_pressed(vk)) return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
// Internal Controller State Processor
// ----------------------------------------------------------------------------
static DWORD internal_get_state(DWORD dwUserIndex, XINPUT_STATE* pState, bool is_ex) {
    if (!pState) return ERROR_BAD_ARGUMENTS;

    // Passthrough for player 2, 3, 4
    if (dwUserIndex != 0) {
        if (is_ex && g_pfn_get_state_ex) {
            return g_pfn_get_state_ex(dwUserIndex, pState);
        }
        if (g_pfn_get_state) {
            return g_pfn_get_state(dwUserIndex, pState);
        }
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    // Emulating Player 1 (dwUserIndex == 0)
    XINPUT_STATE real_state;
    std::memset(&real_state, 0, sizeof(real_state));
    bool real_connected = false;
    if (is_ex && g_pfn_get_state_ex) {
        real_connected = (g_pfn_get_state_ex(0, &real_state) == ERROR_SUCCESS);
    } else if (g_pfn_get_state) {
        real_connected = (g_pfn_get_state(0, &real_state) == ERROR_SUCCESS);
    }

    std::memset(pState, 0, sizeof(XINPUT_STATE));
    pState->dwPacketNumber = ++g_last_packet;

    // Foreground window check
    HWND fg = GetForegroundWindow();
    DWORD fg_pid = 0;
    if (fg) GetWindowThreadProcessId(fg, &fg_pid);
    DWORD my_pid = GetCurrentProcessId();
    bool is_focused = (fg_pid == my_pid || fg == NULL);

    // Configurable Hotkey: Toggle Mouse Lock (Default: F8)
    static bool lock_hotkey_prev = false;
    bool lock_hotkey_now = key_pressed(g_hotkey_mouse_lock);
    if (lock_hotkey_now && !lock_hotkey_prev) {
        g_mouse_locked = !g_mouse_locked;
        MessageBeep(g_mouse_locked ? MB_OK : MB_ICONASTERISK);
    }
    lock_hotkey_prev = lock_hotkey_now;

    // Configurable Hotkey: Toggle Input Mode (Default: F9)
    static bool mode_hotkey_prev = false;
    bool mode_hotkey_now = key_pressed(g_hotkey_input_mode);
    if (mode_hotkey_now && !mode_hotkey_prev) {
        g_input_mode = (g_input_mode == 0) ? 1 : 0;
        MessageBeep(g_input_mode == 0 ? MB_OK : MB_ICONWARNING);
    }
    mode_hotkey_prev = mode_hotkey_now;

    // If user clicks inside window when unlocked, lock cursor automatically
    if (is_focused && !g_mouse_locked && g_input_mode == 0) {
        if (key_pressed(VK_LBUTTON) || key_pressed(VK_RBUTTON)) {
            g_mouse_locked = true;
        }
    }

    WORD buttons = 0;
    BYTE left_trigger = 0;
    BYTE right_trigger = 0;
    SHORT thumb_lx = 0;
    SHORT thumb_ly = 0;
    SHORT thumb_rx = 0;
    SHORT thumb_ry = 0;

    if (is_focused) {
        bool allow_mouse = (g_input_mode == 0);

        // ====================================================================
        // 1. CLASSIC SONIC RIDERS 1 & 2 DRIFT SYSTEM
        // ====================================================================
        bool ctrl_held   = key_pressed(VK_LCONTROL) || key_pressed(VK_RCONTROL);
        bool drift_left  = any_key_pressed(g_binds.drift_left) ||
                           (ctrl_held && any_key_pressed(g_binds.steer_left));
        bool drift_right = any_key_pressed(g_binds.drift_right) ||
                           (ctrl_held && any_key_pressed(g_binds.steer_right));

        // Corner exit boost pulse
        if ((g_drift_l_prev && !drift_left) || (g_drift_r_prev && !drift_right)) {
            g_drift_boost_fade = 6; // ~100ms boost surge
        }
        g_drift_l_prev = drift_left;
        g_drift_r_prev = drift_right;

        if (g_drift_boost_fade > 0) {
            buttons |= XINPUT_GAMEPAD_X; // Boost out of drift
            g_drift_boost_fade--;
        }

        // ====================================================================
        // 2. CONFIRM / SELECT / CROUCH & JUMP (A & X Buttons)
        // ====================================================================
        if (any_key_pressed(g_binds.confirm)) {
            buttons |= (XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_X); // Universal dialog confirm
        }
        if (any_key_pressed(g_binds.jump_crouch)) {
            buttons |= XINPUT_GAMEPAD_A; // Crouch / Jump
        }

        // ====================================================================
        // 3. KICK DASH / BOOST / SWIM (X Button)
        // ====================================================================
        if (any_key_pressed(g_binds.kick_dash)) {
            buttons |= XINPUT_GAMEPAD_X;
        }

        // ====================================================================
        // 4. CANCEL / DECLINE / BACK (B Button)
        // ====================================================================
        if (any_key_pressed(g_binds.cancel_back)) {
            buttons |= XINPUT_GAMEPAD_B;
        }

        // ====================================================================
        // 5. MENU CURSOR TOGGLE & DEBUG (Y Button)
        // ====================================================================
        if (any_key_pressed(g_binds.toggle_cursor)) {
            buttons |= XINPUT_GAMEPAD_Y;
        }

        // ====================================================================
        // 6. POWER PUNCH (LB / RB)
        // ====================================================================
        if (drift_left)  buttons |= XINPUT_GAMEPAD_LEFT_SHOULDER;
        if (drift_right) buttons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;

        // ====================================================================
        // 7. TAG MODE HOLD HANDS (LT)
        // ====================================================================
        if (any_key_pressed(g_binds.tag_mode)) {
            left_trigger = 255;
        }

        // ====================================================================
        // 8. USE ITEM / SHAKE ITEM / SCREEN WIPE (RT)
        // ====================================================================
        if (any_key_pressed(g_binds.use_item)) {
            right_trigger = 255;
        }

        // ====================================================================
        // 9. PAUSE GAME (Start)
        // ====================================================================
        if (any_key_pressed(g_binds.pause)) {
            buttons |= XINPUT_GAMEPAD_START;
        }

        // ====================================================================
        // 10. SWAP RELAY RACER / RESET CURSOR (Back)
        // ====================================================================
        if (any_key_pressed(g_binds.reset_cursor)) {
            buttons |= XINPUT_GAMEPAD_BACK;
        }

        // ====================================================================
        // 11. D-PAD & MENU CAROUSEL RING ROTATION
        // ====================================================================
        if (any_key_pressed(g_binds.menu_ring_left))   buttons |= XINPUT_GAMEPAD_DPAD_LEFT;
        if (any_key_pressed(g_binds.menu_ring_right))  buttons |= XINPUT_GAMEPAD_DPAD_RIGHT;
        if (any_key_pressed(g_binds.menu_option_up))   buttons |= XINPUT_GAMEPAD_DPAD_UP;
        if (any_key_pressed(g_binds.menu_option_down)) buttons |= XINPUT_GAMEPAD_DPAD_DOWN;

        // Mouse Wheel carousel rotation
        if (allow_mouse) {
            if (g_wheel_delta > 50) {
                buttons |= XINPUT_GAMEPAD_DPAD_LEFT;
                g_wheel_delta -= 50;
            } else if (g_wheel_delta < -50) {
                buttons |= XINPUT_GAMEPAD_DPAD_RIGHT;
                g_wheel_delta += 50;
            }
        }

        // Guide Button (0x0400)
        if (key_pressed(VK_HOME) || key_pressed(VK_F12)) {
            buttons |= 0x0400;
        }

        // ====================================================================
        // 12. MOUSE MOTION TRACKING
        // ====================================================================
        float mouse_dx = 0.0f;
        float mouse_dy = 0.0f;

        if (allow_mouse && g_mouse_capture && fg) {
            RECT rc;
            if (GetClientRect(fg, &rc)) {
                POINT center = { (rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2 };
                ClientToScreen(fg, &center);

                POINT cur;
                if (GetCursorPos(&cur)) {
                    mouse_dx = static_cast<float>(cur.x - center.x);
                    mouse_dy = static_cast<float>(cur.y - center.y);

                    if (g_mouse_locked) {
                        SetCursorPos(center.x, center.y);
                    }
                }
            }
        }

        // ====================================================================
        // 13. LEFT STICK: BOARD STEERING, LEAN & DRIFTING
        // ====================================================================
        float kb_steer_x = 0.0f;
        float kb_lean_y  = 0.0f;
        if (any_key_pressed(g_binds.steer_left))  kb_steer_x -= 1.0f;
        if (any_key_pressed(g_binds.steer_right)) kb_steer_x += 1.0f;
        if (any_key_pressed(g_binds.lean_forward)) kb_lean_y += 1.0f;
        if (any_key_pressed(g_binds.lean_back))    kb_lean_y -= 1.0f;

        // Smooth mouse steering
        if (allow_mouse && std::abs(mouse_dx) > 0.0f) {
            g_steer_accum += (mouse_dx / 40.0f) * (g_mouse_steer_sens / 1000.0f);
        }
        g_steer_accum *= 0.82f;

        float final_steer_x = kb_steer_x + (allow_mouse ? g_steer_accum : 0.0f);

        // Apply Drift Physics Override
        if (drift_left) {
            final_steer_x = -1.0f; // Maximum left carve
            if (!any_key_pressed(g_binds.lean_forward)) {
                kb_lean_y = -0.55f; // Tight carve angle weight shift
            }
        } else if (drift_right) {
            final_steer_x = 1.0f; // Maximum right carve
            if (!any_key_pressed(g_binds.lean_forward)) {
                kb_lean_y = -0.55f; // Tight carve angle weight shift
            }
        }

        final_steer_x = std::max(-1.0f, std::min(1.0f, final_steer_x));
        kb_lean_y     = std::max(-1.0f, std::min(1.0f, kb_lean_y));

        thumb_lx = static_cast<SHORT>(final_steer_x * 32767.0f);
        thumb_ly = static_cast<SHORT>(kb_lean_y * 32767.0f);

        // ====================================================================
        // 14. RIGHT STICK: MENU CURSOR & ARMS (Mouse Servo + Keyboard)
        // ====================================================================
        float kb_cursor_x = 0.0f;
        float kb_cursor_y = 0.0f;

        if (any_key_pressed(g_binds.menu_cursor_left))  kb_cursor_x -= 1.0f;
        if (any_key_pressed(g_binds.menu_cursor_right)) kb_cursor_x += 1.0f;
        if (any_key_pressed(g_binds.menu_cursor_up))    kb_cursor_y += 1.0f;
        if (any_key_pressed(g_binds.menu_cursor_down))  kb_cursor_y -= 1.0f;

        // Mouse Cursor Servo
        if (allow_mouse) {
            if (std::abs(mouse_dx) > 0.5f) {
                g_cursor_accum_x += mouse_dx * (g_mouse_cursor_sens * 60.0f);
            }
            if (std::abs(mouse_dy) > 0.5f) {
                g_cursor_accum_y -= mouse_dy * (g_mouse_cursor_sens * 60.0f);
            }

            g_cursor_accum_x = std::max(-400.0f, std::min(400.0f, g_cursor_accum_x));
            g_cursor_accum_y = std::max(-400.0f, std::min(400.0f, g_cursor_accum_y));
        } else {
            g_cursor_accum_x = 0.0f;
            g_cursor_accum_y = 0.0f;
        }

        float mouse_cursor_x = 0.0f;
        float mouse_cursor_y = 0.0f;

        if (allow_mouse) {
            if (std::abs(g_cursor_accum_x) > 2.0f) {
                float dir_x = (g_cursor_accum_x > 0.0f) ? 1.0f : -1.0f;
                float mag_x = std::min(1.0f, std::abs(g_cursor_accum_x) / 30.0f);
                mouse_cursor_x = dir_x * mag_x;
                g_cursor_accum_x *= 0.68f;
            } else {
                g_cursor_accum_x = 0.0f;
            }

            if (std::abs(g_cursor_accum_y) > 2.0f) {
                float dir_y = (g_cursor_accum_y > 0.0f) ? 1.0f : -1.0f;
                float mag_y = std::min(1.0f, std::abs(g_cursor_accum_y) / 30.0f);
                mouse_cursor_y = dir_y * mag_y;
                g_cursor_accum_y *= 0.68f;
            } else {
                g_cursor_accum_y = 0.0f;
            }
        }

        float final_cursor_x = kb_cursor_x + mouse_cursor_x;
        float final_cursor_y = kb_cursor_y + mouse_cursor_y;
        final_cursor_x = std::max(-1.0f, std::min(1.0f, final_cursor_x));
        final_cursor_y = std::max(-1.0f, std::min(1.0f, final_cursor_y));

        thumb_rx = static_cast<SHORT>(final_cursor_x * 32767.0f);
        thumb_ry = static_cast<SHORT>(final_cursor_y * 32767.0f);
    }

    // Merge with real controller if physically connected
    if (real_connected) {
        buttons |= real_state.Gamepad.wButtons;
        if (real_state.Gamepad.bLeftTrigger > left_trigger)   left_trigger = real_state.Gamepad.bLeftTrigger;
        if (real_state.Gamepad.bRightTrigger > right_trigger) right_trigger = real_state.Gamepad.bRightTrigger;
        if (std::abs(real_state.Gamepad.sThumbLX) > std::abs(thumb_lx)) thumb_lx = real_state.Gamepad.sThumbLX;
        if (std::abs(real_state.Gamepad.sThumbLY) > std::abs(thumb_ly)) thumb_ly = real_state.Gamepad.sThumbLY;
        if (std::abs(real_state.Gamepad.sThumbRX) > std::abs(thumb_rx)) thumb_rx = real_state.Gamepad.sThumbRX;
        if (std::abs(real_state.Gamepad.sThumbRY) > std::abs(thumb_ry)) thumb_ry = real_state.Gamepad.sThumbRY;
    }

    pState->Gamepad.wButtons      = buttons;
    pState->Gamepad.bLeftTrigger  = left_trigger;
    pState->Gamepad.bRightTrigger = right_trigger;
    pState->Gamepad.sThumbLX      = thumb_lx;
    pState->Gamepad.sThumbLY      = thumb_ly;
    pState->Gamepad.sThumbRX      = thumb_rx;
    pState->Gamepad.sThumbRY      = thumb_ry;

    return ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------
// EXPORTS
// ----------------------------------------------------------------------------
extern "C" {

__declspec(dllexport) DWORD WINAPI XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
    return internal_get_state(dwUserIndex, pState, false);
}

__declspec(dllexport) DWORD WINAPI XInputGetStateEx(DWORD dwUserIndex, XINPUT_STATE* pState) {
    return internal_get_state(dwUserIndex, pState, true);
}

__declspec(dllexport) DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
    if (g_pfn_set_state && dwUserIndex == 0) {
        return g_pfn_set_state(dwUserIndex, pVibration);
    }
    return ERROR_SUCCESS;
}

__declspec(dllexport) DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities) {
    if (!pCapabilities) return ERROR_BAD_ARGUMENTS;

    if (g_pfn_get_caps && g_pfn_get_caps(dwUserIndex, dwFlags, pCapabilities) == ERROR_SUCCESS) {
        return ERROR_SUCCESS;
    }

    if (dwUserIndex != 0) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    std::memset(pCapabilities, 0, sizeof(XINPUT_CAPABILITIES));
    pCapabilities->Type                  = XINPUT_DEVTYPE_GAMEPAD;
    pCapabilities->SubType               = XINPUT_DEVSUBTYPE_GAMEPAD;
    pCapabilities->Flags                 = 0;
    pCapabilities->Gamepad.wButtons      = 0xFFFF;
    pCapabilities->Gamepad.bLeftTrigger  = 255;
    pCapabilities->Gamepad.bRightTrigger = 255;
    pCapabilities->Gamepad.sThumbLX      = 32767;
    pCapabilities->Gamepad.sThumbLY      = 32767;
    pCapabilities->Gamepad.sThumbRX      = 32767;
    pCapabilities->Gamepad.sThumbRY      = 32767;

    return ERROR_SUCCESS;
}

__declspec(dllexport) DWORD WINAPI XInputGetCapabilitiesEx(DWORD dwReserved, DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES_EX* pCapabilitiesEx) {
    if (!pCapabilitiesEx) return ERROR_BAD_ARGUMENTS;
    return XInputGetCapabilities(dwUserIndex, dwFlags, reinterpret_cast<XINPUT_CAPABILITIES*>(pCapabilitiesEx));
}

__declspec(dllexport) DWORD WINAPI XInputGetBatteryInformation(DWORD dwUserIndex, BYTE devType, XINPUT_BATTERY_INFORMATION* pBattery) {
    if (!pBattery) return ERROR_BAD_ARGUMENTS;

    if (g_pfn_get_bat && g_pfn_get_bat(dwUserIndex, devType, pBattery) == ERROR_SUCCESS) {
        return ERROR_SUCCESS;
    }

    if (dwUserIndex != 0) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    pBattery->BatteryType  = BATTERY_TYPE_WIRED;
    pBattery->BatteryLevel = BATTERY_LEVEL_FULL;
    return ERROR_SUCCESS;
}

__declspec(dllexport) DWORD WINAPI XInputGetKeystroke(DWORD dwUserIndex, DWORD dwReserved, PXINPUT_KEYSTROKE pKeystroke) {
    if (g_pfn_get_keys) {
        return g_pfn_get_keys(dwUserIndex, dwReserved, pKeystroke);
    }
    return ERROR_EMPTY;
}

__declspec(dllexport) DWORD WINAPI XInputGetAudioDeviceIds(DWORD dwUserIndex, LPWSTR pRender, UINT* pRenderCount, LPWSTR pCapture, UINT* pCaptureCount) {
    return ERROR_NOT_SUPPORTED;
}

__declspec(dllexport) void WINAPI XInputEnable(BOOL enable) {
    // No-op
}

__declspec(dllexport) DWORD WINAPI XInputWaitForGuideButton(DWORD dwUserIndex, DWORD dwFlags, LPVOID pVoid) {
    return ERROR_NOT_SUPPORTED;
}

__declspec(dllexport) DWORD WINAPI XInputCancelGuideButtonWait(DWORD dwUserIndex) {
    return ERROR_SUCCESS;
}

__declspec(dllexport) DWORD WINAPI XInputPowerOffController(DWORD dwUserIndex) {
    return ERROR_SUCCESS;
}

__declspec(dllexport) DWORD WINAPI XInputGetBaseBusInformation(DWORD dwUserIndex, LPVOID pVoid) {
    return ERROR_NOT_SUPPORTED;
}

}
