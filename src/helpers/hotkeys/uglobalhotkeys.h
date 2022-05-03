#pragma once

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QSet>

#if defined(Q_OS_LINUX)
#include "xcb/xcb.h"
#include "xcb/xcb_keysyms.h"
#elif defined(Q_OS_MAC)
#include <Carbon/Carbon.h>
#endif

#include "ukeysequence.h"

#if defined(Q_OS_LINUX)
struct UHotkeyData {
    xcb_keycode_t keyCode;
    int mods;
    bool operator ==(const UHotkeyData &data) const
    {
        return data.keyCode == this->keyCode && data.mods == this->mods;
    }
};
#endif

class UGlobalHotkeys : public QObject
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
    , public QAbstractNativeEventFilter
#endif
{
    Q_OBJECT

public:
    explicit UGlobalHotkeys(QObject *parent = 0);
    bool registerHotkey(const QString &keySeq, size_t id = 1);
    bool registerHotkey(const UKeySequence &keySeq, size_t id = 1);
    void unregisterHotkey(size_t id = 1);
    void unregisterAllHotkeys();
    ~UGlobalHotkeys();
#if defined (Q_OS_MAC)
    void onHotkeyPressed(size_t id);
#endif

#if (defined (Q_OS_WIN) || defined (Q_OS_LINUX))
    typedef qintptr RESULT_TYPE;
#endif

protected:
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    bool nativeEventFilter(const QByteArray &eventType, void *message, RESULT_TYPE *result);
#endif

#if defined (Q_OS_WIN)
    bool winEvent(MSG *message, RESULT_TYPE *result);
#elif defined(Q_OS_LINUX)
    bool linuxEvent(xcb_generic_event_t *message);
    void regLinuxHotkey(const UKeySequence &keySeq, size_t id);
    void unregLinuxHotkey(size_t id);
#endif

signals:
    void activated(size_t id);

private:
#if defined(Q_OS_WIN)
    QSet<size_t> Registered;
#elif defined(Q_OS_LINUX)
    QHash<size_t, UHotkeyData> Registered;
    xcb_connection_t *X11Connection;
    xcb_window_t X11Wid;
    xcb_key_symbols_t *X11KeySymbs;
#elif defined(Q_OS_MAC)
    QHash<size_t, EventHotKeyRef> HotkeyRefs;
#endif
};
