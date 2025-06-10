#include "iconmodel.h"

IconModel::IconModel(QObject *parent)
    : QObject{parent}
{
}

QString IconModel::themeName() const
{
    return m_themeName;
}

void IconModel::setThemeName(const QString &themeName)
{
    if (m_themeName == themeName)
        return;

    QIcon::setThemeName(themeName);

    m_themeName = themeName;
    emit themeNameChanged();
}

QString IconModel::fromTheme(const QString &icon)
{
    QString result = QString("icons/%1/%2").arg(m_themeName, icon);

    return result;
}

QStringList IconModel::availableThemes() const
{
    return m_availableThemes;
}
