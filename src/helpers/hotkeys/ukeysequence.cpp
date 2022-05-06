#include "ukeysequence.h"

#include <QDebug>

UKeySequence::UKeySequence(QObject *parent)
    : QObject(parent)
{
}

UKeySequence::UKeySequence(const QString &str, QObject *parent)
    : QObject(parent)
{
    fromString(str);
}

void UKeySequence::fromString(const QString &str)
{
    const QStringList keys = str.split('+');
    for (const QString & key : keys)
        addKey(key);
}

QString UKeySequence::toString()
{
    const QList<Qt::Key> simpleKeys = getSimpleKeys();
    const QList<Qt::Key> modifiers = getModifiers();
    QStringList result;
    for (auto mod : modifiers)
        result.append(keyToStr(mod));

    for (auto key : simpleKeys)
        result.append(keyToStr(key));

    return result.join('+');
}

QList<Qt::Key> UKeySequence::getSimpleKeys() const
{
    QList<Qt::Key> result;
    for (auto key : qAsConst(mKeys)) {
        if (!isModifier(key)) {
            result.append(key);
        }
    }
    return result;
}

QList<Qt::Key> UKeySequence::getModifiers() const
{
    QList<Qt::Key> result;
    for (auto key : qAsConst(mKeys)) {
        if (isModifier(key)) {
            result.append(key);
        }
    }
    return result;
}

void UKeySequence::addModifiers(Qt::KeyboardModifiers mod)
{
    if (mod == Qt::NoModifier) {
        return;
    }
    if (mod & Qt::ShiftModifier) {
        addKey(Qt::Key_Shift);
    }
    if (mod & Qt::ControlModifier) {
        addKey(Qt::Key_Control);
    }
    if (mod & Qt::AltModifier) {
        addKey(Qt::Key_Alt);
    }
    if (mod & Qt::MetaModifier) {
        addKey(Qt::Key_Meta);
    }
}

void UKeySequence::addKey(const QString &key)
{
    if (key.contains(QStringLiteral("+")) || key.contains(QStringLiteral(","))) {
        qWarning() << "Wrong key";
        return;
    }

    QString mod = key.toLower();
    if (mod == QStringLiteral("alt")) {
        addKey(Qt::Key_Alt);
        return;
    }
    if (mod == QStringLiteral("shift") || mod == QStringLiteral("shft")) {
        addKey(Qt::Key_Shift);
        return;
    }
    if (mod == QStringLiteral("control") || mod == QStringLiteral("ctrl")) {
        addKey(Qt::Key_Control);
        return;
    }
    if (mod == QStringLiteral("win") || mod == QStringLiteral("meta")) {
        addKey(Qt::Key_Meta);
        return;
    }
    QKeySequence seq(key);
    if (seq.count() != 1) {
        qWarning() << "Wrong key";
        return;
    }

    addKey(seq[0].key());
}

void UKeySequence::addKey(Qt::Key key)
{
    if (key <= 0)
        return;

    for (auto testKey : qAsConst(mKeys)) {
        if (testKey == key) {
            return;
        }
    }

    mKeys.append(key);
}

void UKeySequence::addKey(const QKeyEvent *event)
{
    addKey((Qt::Key) event->key());
    addModifiers(event->modifiers());
}
