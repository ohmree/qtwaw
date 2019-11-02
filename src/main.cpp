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
#include <QLibraryInfo>
#include <QLockFile>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QDir tmp_dir(QDir::tempPath());
    QLockFile lock_file(tmp_dir.absoluteFilePath("QtWAW.lock"));
    if(!lock_file.tryLock(100))
        return 0;

    QCoreApplication::setOrganizationName("QtWAW");
    QCoreApplication::setOrganizationDomain("scarpetta.eu");
    QCoreApplication::setApplicationName("qtwaw");
    QCoreApplication::setApplicationVersion("1.0");

    // Set application informations
    QApplication app(argc, argv);

    app.setApplicationDisplayName("QtWAW");
    app.setDesktopFileName("eu.scarpetta.QtWAW");

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
    main_window->show();

    return app.exec();
}
