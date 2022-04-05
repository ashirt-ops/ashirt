#include <QtCore>

#if defined(Q_OS_WIN)
inline size_t QtKeyToWin(Qt::Key key)
{
    switch ((Qt::Key)key) {
    case Qt::Key_Escape:
        return VK_ESCAPE;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        return VK_TAB;
    case Qt::Key_Backspace:
        return VK_BACK;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return VK_RETURN;
    case Qt::Key_Insert:
        return VK_INSERT;
    case Qt::Key_Delete:
        return VK_DELETE;
    case Qt::Key_Pause:
        return VK_PAUSE;
    case Qt::Key_Print:
        return VK_SNAPSHOT;
    case Qt::Key_Clear:
        return VK_CLEAR;
    case Qt::Key_Home:
        return VK_HOME;
    case Qt::Key_End:
        return VK_END;
    case Qt::Key_Left:
        return VK_LEFT;
    case Qt::Key_Up:
        return VK_UP;
    case Qt::Key_Right:
        return VK_RIGHT;
    case Qt::Key_Down:
        return VK_DOWN;
    case Qt::Key_PageUp:
        return VK_PRIOR;
    case Qt::Key_PageDown:
        return VK_NEXT;
    case Qt::Key_F1:
        return VK_F1;
    case Qt::Key_F2:
        return VK_F2;
    case Qt::Key_F3:
        return VK_F3;
    case Qt::Key_F4:
        return VK_F4;
    case Qt::Key_F5:
        return VK_F5;
    case Qt::Key_F6:
        return VK_F6;
    case Qt::Key_F7:
        return VK_F7;
    case Qt::Key_F8:
        return VK_F8;
    case Qt::Key_F9:
        return VK_F9;
    case Qt::Key_F10:
        return VK_F10;
    case Qt::Key_F11:
        return VK_F11;
    case Qt::Key_F12:
        return VK_F12;
    case Qt::Key_F13:
        return VK_F13;
    case Qt::Key_F14:
        return VK_F14;
    case Qt::Key_F15:
        return VK_F15;
    case Qt::Key_F16:
        return VK_F16;
    case Qt::Key_F17:
        return VK_F17;
    case Qt::Key_F18:
        return VK_F18;
    case Qt::Key_F19:
        return VK_F19;
    case Qt::Key_F20:
        return VK_F20;
    case Qt::Key_F21:
        return VK_F21;
    case Qt::Key_F22:
        return VK_F22;
    case Qt::Key_F23:
        return VK_F23;
    case Qt::Key_F24:
        return VK_F24;
    case Qt::Key_Space:
        return VK_SPACE;
    case Qt::Key_Asterisk:
        return VK_MULTIPLY;
    case Qt::Key_Plus:
        return VK_ADD;
    case Qt::Key_Comma:
        return VK_SEPARATOR;
    case Qt::Key_Minus:
        return VK_SUBTRACT;
    case Qt::Key_Slash:
        return VK_DIVIDE;
    case Qt::Key_MediaNext:
        return VK_MEDIA_NEXT_TRACK;
    case Qt::Key_MediaPrevious:
        return VK_MEDIA_PREV_TRACK;
    case Qt::Key_MediaPlay:
        return VK_MEDIA_PLAY_PAUSE;
    case Qt::Key_MediaStop:
        return VK_MEDIA_STOP;
    case Qt::Key_VolumeDown:
        return VK_VOLUME_DOWN;
    case Qt::Key_VolumeUp:
        return VK_VOLUME_UP;
    case Qt::Key_VolumeMute:
        return VK_VOLUME_MUTE;
    }

    if (key >= 0x01000030 && key <= 0x01000047) {
        return VK_F1 + (key - Qt::Key_F1);
    }

    return key;
}
#elif defined(Q_OS_LINUX)

#include "ukeysequence.h"
#include <unordered_map>
#include "xcb/xcb.h"
#include "xcb/xcb_keysyms.h"
#include "X11/keysym.h"

struct UKeyData {
    int key;
    int mods;
};

static std::unordered_map<uint32_t, uint32_t> KEY_MAP = {
    {Qt::Key_Escape, XK_Escape},
    {Qt::Key_Tab, XK_Tab},
    {Qt::Key_Backspace, XK_BackSpace},
    {Qt::Key_Return, XK_Return},
    {Qt::Key_Enter, XK_Return},
    {Qt::Key_Insert, XK_Insert},
    {Qt::Key_Delete, XK_Delete},
    {Qt::Key_Pause, XK_Pause},
    {Qt::Key_Print, XK_Print},
    {Qt::Key_SysReq, XK_Sys_Req},
    {Qt::Key_Clear, XK_Clear},
    {Qt::Key_Home, XK_Home},
    {Qt::Key_End, XK_End},
    {Qt::Key_Left, XK_Left},
    {Qt::Key_Up, XK_Up},
    {Qt::Key_Right, XK_Right},
    {Qt::Key_Down, XK_Down},
    {Qt::Key_PageUp, XK_Page_Up},
    {Qt::Key_PageDown, XK_Page_Down}
};

inline UKeyData QtKeyToLinux(const UKeySequence &keySeq)
{
    UKeyData data = {0, 0};

    auto key = keySeq.getSimpleKeys();
    if (key.size() > 0) {
        data.key = key[0];
    } else {
        qWarning() << "Invalid hotkey";
        return data;
    }
    // Key conversion
    // Misc Keys
    if (KEY_MAP.find(key[0]) != KEY_MAP.end()) {
        data.key = KEY_MAP[key[0]];
    } else if (data.key >= Qt::Key_F1 && data.key <= Qt::Key_F35) { // Qt's F keys need conversion
        const size_t DIFF = Qt::Key_F1 - XK_F1;
        data.key -= DIFF;
    } else if (data.key >= Qt::Key_Space && data.key <= Qt::Key_QuoteLeft) {
        // conversion is not necessary, if the value in the range Qt::Key_Space - Qt::Key_QuoteLeft
    } else {
        qWarning() << "Invalid hotkey: key conversion is not defined";
        return data;
    }

    // Modifiers conversion
    auto mods = keySeq.getModifiers();

    for (auto i : mods) {
        if (i == Qt::Key_Shift)
            data.mods |= XCB_MOD_MASK_SHIFT;
        else if (i == Qt::Key_Control)
            data.mods |= XCB_MOD_MASK_CONTROL;
        else if (i == Qt::Key_Alt)
            data.mods |= XCB_MOD_MASK_1;
        else if (i == Qt::Key_Meta)
            data.mods |= XCB_MOD_MASK_4; // !
    }

    return data;
}
#elif defined(Q_OS_MAC)

#include "ukeysequence.h"
#include <Carbon/Carbon.h>
#include <unordered_map>

struct UKeyData {
    uint32_t key;
    uint32_t mods;
};

static std::unordered_map<uint32_t, uint32_t> KEY_MAP = {
    {Qt::Key_A, kVK_ANSI_A},
    {Qt::Key_B, kVK_ANSI_B},
    {Qt::Key_C, kVK_ANSI_C},
    {Qt::Key_D, kVK_ANSI_D},
    {Qt::Key_E, kVK_ANSI_E},
    {Qt::Key_F, kVK_ANSI_F},
    {Qt::Key_G, kVK_ANSI_G},
    {Qt::Key_H, kVK_ANSI_H},
    {Qt::Key_I, kVK_ANSI_I},
    {Qt::Key_J, kVK_ANSI_J},
    {Qt::Key_K, kVK_ANSI_K},
    {Qt::Key_L, kVK_ANSI_L},
    {Qt::Key_M, kVK_ANSI_M},
    {Qt::Key_N, kVK_ANSI_N},
    {Qt::Key_O, kVK_ANSI_O},
    {Qt::Key_P, kVK_ANSI_P},
    {Qt::Key_Q, kVK_ANSI_Q},
    {Qt::Key_R, kVK_ANSI_R},
    {Qt::Key_S, kVK_ANSI_S},
    {Qt::Key_T, kVK_ANSI_T},
    {Qt::Key_U, kVK_ANSI_U},
    {Qt::Key_V, kVK_ANSI_V},
    {Qt::Key_W, kVK_ANSI_W},
    {Qt::Key_X, kVK_ANSI_X},
    {Qt::Key_Y, kVK_ANSI_Y},
    {Qt::Key_Z, kVK_ANSI_Z},
    {Qt::Key_0, kVK_ANSI_0},
    {Qt::Key_1, kVK_ANSI_1},
    {Qt::Key_2, kVK_ANSI_2},
    {Qt::Key_3, kVK_ANSI_3},
    {Qt::Key_4, kVK_ANSI_4},
    {Qt::Key_5, kVK_ANSI_5},
    {Qt::Key_6, kVK_ANSI_6},
    {Qt::Key_7, kVK_ANSI_7},
    {Qt::Key_8, kVK_ANSI_8},
    {Qt::Key_9, kVK_ANSI_9},
    {Qt::Key_F1, kVK_F1},
    {Qt::Key_F2, kVK_F2},
    {Qt::Key_F3, kVK_F3},
    {Qt::Key_F4, kVK_F4},
    {Qt::Key_F5, kVK_F5},
    {Qt::Key_F6, kVK_F6},
    {Qt::Key_F7, kVK_F7},
    {Qt::Key_F8, kVK_F8},
    {Qt::Key_F9, kVK_F9},
    {Qt::Key_F10, kVK_F10},
    {Qt::Key_F11, kVK_F11},
    {Qt::Key_F12, kVK_F12},
    {Qt::Key_F13, kVK_F13},
    {Qt::Key_F14, kVK_F14},
    {Qt::Key_Print, kVK_F14},
};

static std::unordered_map<uint32_t, uint32_t> MOD_MAP = {
    {Qt::Key_Shift, shiftKey},
    {Qt::Key_Alt, optionKey},
    {Qt::Key_Control, controlKey},
    {Qt::Key_Option, optionKey},
    {Qt::Key_Meta, cmdKey},
};

inline UKeyData QtKeyToMac(const UKeySequence &keySeq)
{
    UKeyData data = {0, 0};
    auto key = keySeq.getSimpleKeys();
    auto mods = keySeq.getModifiers();

    if (key.size() == 1 && KEY_MAP.find(key[0]) != KEY_MAP.end()) {
        data.key = KEY_MAP[key[0]];
    } else {
        qWarning() << "Invalid hotkey";
        return data;
    }

    for (auto && mod : mods) {
        if (MOD_MAP.find(mod) == MOD_MAP.end())
            return data;

        data.mods += MOD_MAP[mod];
    }
    return data;
}

#endif
