/* Copyright (C) 2019 Marco Scarpetta
 *
 * This file is part of QtWAW.
 *
 * QtWAW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QtWAW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QtWAW. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QTranslator>
#include <KDBusAddons/KDBusService>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("QtWAW");
    QCoreApplication::setOrganizationDomain("scarpetta.eu");
    QCoreApplication::setApplicationName("qtwaw");
    QCoreApplication::setApplicationVersion("1.6.4");

    // Set application informations
    QApplication app(argc, argv);

    app.setApplicationDisplayName("QtWAW");
    app.setDesktopFileName("eu.scarpetta.QtWAW");

    KDBusService service(KDBusService::Unique);

    // Set up translations
    QTranslator translator;

    bool ok = translator.load(
                QString("qtwaw_%1.qm").arg(QLocale::system().name()),
                QString("%1/../share/qtwaw/translations").arg(
                    qApp->applicationDirPath()
                    )
                );

    if (ok) app.installTranslator(&translator);

    // Create and show the main window
    MainWindow *main_window = new MainWindow();

    QObject::connect(&service,
                     SIGNAL(activateRequested(const QStringList &,
                                              const QString &)),
                     main_window,
                     SLOT(raise_main_window()));
    return app.exec();
}
