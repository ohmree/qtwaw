/* Copyright (C) 2019-2020 Marco Scarpetta
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

#include <KF5/KNotifications/KStatusNotifierItem>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QWebEngineNotification>
#include <QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineView>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void loadWhatsapp();

    void replyFinished(QNetworkReply *reply);

    void loadFinished();

    void FIXME_triggerPermissionRequest();

    void featureRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature);

    void raiseMainWindow();

    void zoomIn();

    void zoomOut();

    void zoomOriginal();

    void reload();

    void startMinimizedToggled(bool checked);

    void closeToTrayToggled(bool checked);

    void titleChanged(const QString &title);

    void notificationPresenter(QWebEngineNotification *notification);

    void downloadRequested(QWebEngineDownloadItem *download);

    void downloadFinished();

    void closeEvent(QCloseEvent *event);

    void quit();

private:
    const QString userAgent() const;
    const QString whatsappUrl() const;

    QSettings *m_settings;
    QNetworkAccessManager m_networkAccessManager;
    int m_connectionTimeout;
    QWebEngineView m_view;
    QWebEngineProfile *m_profile;
    WebEnginePage *m_page;
    KStatusNotifierItem *m_statusNotifier;
    QIcon m_icon;
};

#endif // MAINWINDOW_H
