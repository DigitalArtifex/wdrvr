#ifndef ICONMODEL_H
#define ICONMODEL_H

#include <QObject>
#include <QGuiApplication>
#include <QApplication>
#include <QQmlEngine>
#include <QIcon>

class IconModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit IconModel(QObject *parent = nullptr);

    QString themeName() const;
    void setThemeName(const QString &themeName);
    Q_INVOKABLE QString fromTheme(const QString &icon);

    QStringList availableThemes() const;

signals:

    void themeNameChanged();

private:
    QString m_themeName = "";

    const QStringList m_availableThemes {
        "win11",
        "glyph"
    };

    Q_PROPERTY(QString themeName READ themeName WRITE setThemeName NOTIFY themeNameChanged FINAL)
    Q_PROPERTY(QStringList availableThemes READ availableThemes CONSTANT FINAL)
};

Q_DECLARE_METATYPE(IconModel)
#endif // ICONMODEL_H
