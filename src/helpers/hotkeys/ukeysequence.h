#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QStringList>
#include <QKeyEvent>

#include "uglobal.h"

class UGLOBALHOTKEY_EXPORT UKeySequence : public QObject
{
    Q_OBJECT

public:
    explicit UKeySequence(QObject *parent = 0);
    explicit UKeySequence(const QString &str, QObject *parent = 0);

    void fromString(const QString &str);
    QString toString();
    void addKey(Qt::Key key);
    void addKey(const QString &key);
    void addModifiers(Qt::KeyboardModifiers mod);
    void addKey(const QKeyEvent *event);

    inline size_t size() const
    {
        return mKeys.size();
    }
    inline Qt::Key operator [](size_t n) const
    {
        if ((int)n > mKeys.size()) {
            return Qt::Key_unknown;
        }

        return mKeys[n];
    }

    QVector<Qt::Key> getSimpleKeys() const;
    QVector<Qt::Key> getModifiers() const;

private:
    QVector<Qt::Key> mKeys;

    inline static bool isModifier(Qt::Key key)
    {
        return (key == Qt::Key_Shift ||
                key == Qt::Key_Control ||
                key == Qt::Key_Alt ||
                key == Qt::Key_Meta);
    }

    inline static QString keyToStr(int key)
    {
        if (key == Qt::Key_Shift) {
            return "Shift";
        }
        if (key == Qt::Key_Control) {
            return "Ctrl";
        }
        if (key == Qt::Key_Alt) {
            return "Alt";
        }
        if (key == Qt::Key_Meta) {
            return "Meta";
        }

        QKeySequence seq(key);
        return seq.toString();
    }

};

