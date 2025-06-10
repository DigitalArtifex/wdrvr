// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "iconmodel.h"
#include "locationmodel.h"
#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>
QGuiApplication *appRef = nullptr;
LocationModel *model;

#ifdef Q_OS_LINUX
#include <signal.h>
void quit(int signum)
{
    if(appRef)
        appRef->exit(0);
}
#elif Q_OS_WIN
#include <windows.h>

BOOL WINAPI WinHandler(DWORD CEvent)
{
    switch(CEvent)
    {
    case CTRL_C_EVENT:
        if(appRef)
            appRef->exit(0);
        break;
    }
    return TRUE;
}
#endif

int main(int argc, char **argv)
{
    QGuiApplication app(argc,argv);
    app.setOrganizationName("DigitalArtifex");
    app.setOrganizationName("digitalartifex.com");
    app.setApplicationName("wdrvr");
    app.setApplicationDisplayName("wdrvr");
    appRef = &app;

#ifdef Q_OS_LINUX
    signal(SIGINT, quit);
    signal(SIGKILL, quit);
    signal(SIGTERM, quit);
#elif Q_OS_WIN
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)WinHandler, TRUE);
#endif

    QQuickView view;
    LocationModel model;
    IconModel iconModel;

    iconModel.setThemeName("win11");
    app.setWindowIcon(QIcon::fromTheme("wdrvr"));
    view.setIcon(QIcon::fromTheme("wdrvr"));

    view.engine()->rootContext()->setContextProperty("locationModel", &model);
    view.engine()->rootContext()->setContextProperty("Icon", &iconModel);
    view.setSource(QUrl(QStringLiteral("qrc:///Main.qml")));
    view.showMaximized();

    QObject::connect(view.engine(), &QQmlEngine::exit, [&app](int code) { app.exit(code); });

    int ret = app.exec();

    return ret;
}
