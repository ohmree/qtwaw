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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "webenginepage.h"

#include <QMainWindow>
#include <QSettings>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineNotification>
#include <QSettings>
#include <KF5/KNotifications/KStatusNotifierItem>
#include <QNetworkConfigurationManager>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void online_status_changed(bool is_online);

    void FIXME_trigger_permission_request();

    void feature_request(const QUrl &securityOrigin,
                         QWebEnginePage::Feature feature);

    void raise_main_window();

    void zoom_in();

    void zoom_out();

    void zoom_original();

    void reload();

    void start_minimized_toggled(bool checked);

    void close_to_tray_toggled(bool checked);

    void title_changed(const QString &title);

    void notification_presenter(QWebEngineNotification *notification);

    void download_requested(QWebEngineDownloadItem *download);

    void download_finished();

    void closeEvent(QCloseEvent *event);

    void quit();

private:
    QSettings *m_settings;

    QNetworkConfigurationManager * m_net_manager;
    QWebEngineView m_view;
    QWebEngineProfile *m_profile;
    WebEnginePage *m_page;
    KStatusNotifierItem *m_status_notifier;
    QIcon icon;
};

#endif // MAINWINDOW_H
