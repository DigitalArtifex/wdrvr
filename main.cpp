// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "locationmodel.h"
#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>
// QGuiApplication *app = nullptr;
// LocationModel *model;

void quit(int code)
{
    // if(app)
    //     app->exit(code);
}

int main(int argc, char **argv)
{
    // app = new QGuiApplication(argc,argv);
    QGuiApplication app(argc,argv);

    QQuickView view;
    LocationModel model;

    QIcon::setThemeName("svgrepo");
    app.setWindowIcon(QIcon("qrc://icons/icons8-god-of-war-64.png"));

    view.engine()->rootContext()->setContextProperty("locationModel", &model);
    view.setSource(QUrl(QStringLiteral("qrc:///Main.qml")));
    view.showMaximized();

    QObject::connect(view.engine(), &QQmlEngine::exit, [&app](int code) { app.exit(code); });

    int ret = app.exec();

    // delete model;
    // delete app;

    return ret;
}
