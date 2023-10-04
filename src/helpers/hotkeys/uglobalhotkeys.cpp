#include <QtCore>
#if defined(Q_OS_WIN)
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <QWindow>
#include <qpa/qplatformnativeinterface.h>
#include <QApplication>
#endif

#include "hotkeymap.h"
#include "uglobalhotkeys.h"

UGlobalHotkeys::UGlobalHotkeys(QObject *parent)
    : QObject(parent)
{
#if defined(Q_OS_LINUX)
    qApp->installNativeEventFilter(this);
    QWindow wndw;
    void *v = qApp->platformNativeInterface()->nativeResourceForWindow("connection", &wndw);
    X11Connection = (xcb_connection_t *)v;
    X11Wid = xcb_setup_roots_iterator(xcb_get_setup(X11Connection)).data->root;
    X11KeySymbs = xcb_key_symbols_alloc(X11Connection);
#elif defined(Q_OS_WIN)
    qApp->installNativeEventFilter(this);
#endif
}

bool UGlobalHotkeys::registerHotkey(const QString &keySeq, size_t id)
{
    return registerHotkey(UKeySequence(keySeq), id);
}

#if defined(Q_OS_MAC)
OSStatus macHotkeyHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void *userData)
{
    Q_UNUSED(nextHandler);
    EventHotKeyID hkCom;
    GetEventParameter(theEvent, kEventParamDirectObject, typeEventHotKeyID, NULL,
                      sizeof(hkCom), NULL, &hkCom);
    size_t id = hkCom.id;

    UGlobalHotkeys *caller = (UGlobalHotkeys *)userData;
    caller->onHotkeyPressed(id);
    return noErr;
}
#endif

bool UGlobalHotkeys::registerHotkey(const UKeySequence &keySeq, size_t id)
{
    if (keySeq.size() == 0) {
        return false;
    }
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    if (Registered.find(id) != Registered.end()) {
        unregisterHotkey(id);
    }
#endif
#if defined(Q_OS_WIN)
    size_t winMod = MOD_NOREPEAT;
    size_t key = VK_F2;
    for (size_t i = 0; i != keySeq.size(); i++) {
        if (keySeq[i] == Qt::Key_Control) {
            winMod |= MOD_CONTROL;
        } else if (keySeq[i] == Qt::Key_Alt) {
            winMod |= MOD_ALT;
        } else if (keySeq[i] == Qt::Key_Shift) {
            winMod |= MOD_SHIFT;
        } else if (keySeq[i] == Qt::Key_Meta) {
            winMod |= MOD_WIN;
        } else {
            key = QtKeyToWin(keySeq[i]);
        }
    }

    if (!RegisterHotKey(nullptr, id, winMod, key)) {
        return false;
    } else {
        Registered.insert(id);
    }
#elif defined(Q_OS_LINUX)
    regLinuxHotkey(keySeq, id);
#elif defined(Q_OS_MAC)
    unregisterHotkey(id);

    EventHotKeyRef gMyHotKeyRef;
    EventHotKeyID gMyHotKeyID;
    EventTypeSpec eventType;
    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyPressed;

    InstallApplicationEventHandler(&macHotkeyHandler, 1, &eventType, this, NULL);

    gMyHotKeyID.signature = uint32_t(id);
    gMyHotKeyID.id = uint32_t(id);

    UKeyData macKey = QtKeyToMac(keySeq);

    RegisterEventHotKey(macKey.key, macKey.mods, gMyHotKeyID,
                        GetApplicationEventTarget(), 0, &gMyHotKeyRef);

    HotkeyRefs[id] = gMyHotKeyRef;
#endif

    return true;
}

void UGlobalHotkeys::unregisterHotkey(size_t id)
{
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    if(Registered.find(id) == Registered.end()) {
        return;
    }
#endif
#if defined(Q_OS_WIN)
    UnregisterHotKey(nullptr, id);
#elif defined(Q_OS_LINUX)
    unregLinuxHotkey(id);
#endif
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    Registered.remove(id);
#elif defined(Q_OS_MAC)
    if (HotkeyRefs.find(id) != HotkeyRefs.end()) {
        UnregisterEventHotKey(HotkeyRefs[id]);
    }
#endif
}

void UGlobalHotkeys::unregisterAllHotkeys()
{
#ifdef Q_OS_WIN
    const auto keys = Registered;
    for (const size_t key : keys) {
        unregisterHotkey(key);
    }
#elif defined(Q_OS_LINUX)
    const auto keys = Registered.keys();
    for (const size_t key : keys) {
        unregisterHotkey(key);
    }
#elif defined(Q_OS_MAC)
    const auto refs = HotkeyRefs;
    for (auto ref : refs) {
        UnregisterEventHotKey(ref);
    }
#endif
}

UGlobalHotkeys::~UGlobalHotkeys()
{
#if defined(Q_OS_WIN)
    for (auto hotKey : qAsConst(Registered)) {
        UnregisterHotKey(nullptr, hotKey);
    }
#elif defined(Q_OS_LINUX)
    xcb_key_symbols_free(X11KeySymbs);
#endif
}

#if defined(Q_OS_MAC)
void UGlobalHotkeys::onHotkeyPressed(size_t id)
{
    Q_EMIT activated(id);
}
#endif

#if defined(Q_OS_WIN)
bool UGlobalHotkeys::winEvent(MSG *message, RESULT_TYPE *result)
{
    if (message->message == WM_HOTKEY && result == nullptr) {
        size_t id = message->wParam;
        Q_ASSERT(Registered.find(id) != Registered.end() && "Unregistered hotkey");
        Q_EMIT activated(id);
    }
    return false;
}
#endif

#if defined (Q_OS_WIN) || defined( Q_OS_LINUX)
bool UGlobalHotkeys::nativeEventFilter(const QByteArray &eventType, void *message, RESULT_TYPE *result)
{
    Q_UNUSED(eventType);
#if defined(Q_OS_LINUX)
    Q_UNUSED(result);
    return linuxEvent(static_cast<xcb_generic_event_t *>(message));
#elif defined(Q_OS_WIN)
    return winEvent((MSG *)message, result);
#endif
}
#endif

#if defined(Q_OS_LINUX)
bool UGlobalHotkeys::linuxEvent(xcb_generic_event_t *message)
{
    if ((message->response_type & ~0x80) == XCB_KEY_PRESS) {
        xcb_key_press_event_t *ev = (xcb_key_press_event_t *)message;
        auto ind = Registered.key({ev->detail, (ev->state & ~XCB_MOD_MASK_2)});

        if (ind == 0) // this is not hotkeys
            return false;

        Q_EMIT activated(ind);
        return true;
    }
    return false;
}

void UGlobalHotkeys::regLinuxHotkey(const UKeySequence &keySeq, size_t id)
{
    UHotkeyData data;
    UKeyData keyData = QtKeyToLinux(keySeq);

    xcb_keycode_t *keyC = xcb_key_symbols_get_keycode(X11KeySymbs, keyData.key);

    if (keyC == XCB_NO_SYMBOL) { // 0x0
        qWarning() << "Cannot find symbol";
        return;
    }
    data.keyCode = *keyC;
    data.mods = keyData.mods;

    xcb_grab_key(X11Connection, 1, X11Wid, data.mods, data.keyCode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    // NumLk
    xcb_grab_key(X11Connection, 1, X11Wid, data.mods | XCB_MOD_MASK_2, data.keyCode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

    Registered.insert(id, data);
}

void UGlobalHotkeys::unregLinuxHotkey(size_t id)
{
    UHotkeyData data = Registered.take(id);
    xcb_ungrab_key(X11Connection, data.keyCode, X11Wid, data.mods);
    xcb_ungrab_key(X11Connection, data.keyCode, X11Wid, data.mods | XCB_MOD_MASK_2);
}
#endif
