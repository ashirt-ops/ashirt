#pragma once

#include <QObject>
#include <QKeyEvent>

class UKeySequence : public QObject
{
    Q_OBJECT

public:
    explicit UKeySequence(QObject *parent = nullptr);
    explicit UKeySequence(const QString &str, QObject *parent = nullptr);

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

    QList<Qt::Key> getSimpleKeys() const;
    QList<Qt::Key> getModifiers() const;

private:
    QList<Qt::Key> mKeys;

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
            return QStringLiteral("Shift");
        }
        if (key == Qt::Key_Control) {
            return QStringLiteral("Ctrl");
        }
        if (key == Qt::Key_Alt) {
            return QStringLiteral("Alt");
        }
        if (key == Qt::Key_Meta) {
            return QStringLiteral("Meta");
        }

        QKeySequence seq(key);
        return seq.toString();
    }

};

