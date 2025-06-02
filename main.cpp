// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "locationmodel.h"
#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>
QGuiApplication *app = nullptr;
LocationModel *model;

void quit(int code)
{
    if(app)
        app->exit(code);
}

int main(int argc, char **argv)
{
    app = new QGuiApplication(argc,argv);
    app->setWindowIcon(QIcon("qrc://icons/icons8-god-of-war-64.png"));

    QQuickView view;
    model = new LocationModel;
    model->setLocations({
        {
            "REMOTE85uvnz",
            "#highConfidence",
            "Network ID: 00:1D:C9:09:46:4A \nEncryption: WPA2 \nTime: 2025-05-29T17:39:32.000-07:00 \nSignal: -94.0\nAccuracy: 3.3\nType: WIFI",
            1,
            {
                52.02301407,
                -2.91925073
            }
        }
    });

    view.engine()->rootContext()->setContextProperty("locationModel", model);
    view.setSource(QUrl(QStringLiteral("qrc:///Main.qml")));
    view.showMaximized();

    QObject::connect(view.engine(), &QQmlEngine::exit, &quit);

    int ret = app->exec();

    delete model;
    delete app;

    return ret;
}
