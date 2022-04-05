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
    QStringList keys = str.split('+');
    for (int i = 0; i < keys.size(); i++) {
        addKey(keys[i]);
    }
}

QString UKeySequence::toString()
{
    QVector<Qt::Key> simpleKeys = getSimpleKeys();
    QVector<Qt::Key> modifiers = getModifiers();
    QStringList result;
    for (int i = 0; i < modifiers.size(); i++) {
        result.push_back(keyToStr(modifiers[i]));
    }
    for (int i = 0; i < simpleKeys.size(); i++) {
        result.push_back(keyToStr(simpleKeys[i]));
    }
    return result.join('+');
}

QVector<Qt::Key> UKeySequence::getSimpleKeys() const
{
    QVector<Qt::Key> result;
    for (int i = 0; i < mKeys.size(); i++) {
        if (!isModifier(mKeys[i])) {
            result.push_back(mKeys[i]);
        }
    }
    return result;
}

QVector<Qt::Key> UKeySequence::getModifiers() const
{
    QVector<Qt::Key> result;
    for (int i = 0; i < mKeys.size(); i++) {
        if (isModifier(mKeys[i])) {
            result.push_back(mKeys[i]);
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
    if (key.contains("+") || key.contains(",")) {
        qWarning() << "Wrong key";
        return;
    }

    QString mod = key.toLower();
    // qDebug() << "mod: " << mod;
    if (mod == "alt") {
        addKey(Qt::Key_Alt);
        return;
    }
    if (mod == "shift" || mod == "shft") {
        addKey(Qt::Key_Shift);
        return;
    }
    if (mod == "control" || mod == "ctrl") {
        addKey(Qt::Key_Control);
        return;
    }
    if (mod == "win" || mod == "meta") {
        addKey(Qt::Key_Meta);
        return;
    }
    QKeySequence seq(key);
    if (seq.count() != 1) {
        qWarning() << "Wrong key";
        return;
    }

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    addKey((Qt::Key) seq[0]);
#else
    addKey(seq[0].key());
#endif
}

void UKeySequence::addKey(Qt::Key key)
{
    if (key <= 0) {
        return;
    }
    for (int i = 0; i < mKeys.size(); i++) {
        if (mKeys[i] == key) {
            return;
        }
    }
    // qDebug() << "Key added: " << key;
    mKeys.push_back(key);
}

void UKeySequence::addKey(const QKeyEvent *event)
{
    addKey((Qt::Key) event->key());
    addModifiers(event->modifiers());
}
