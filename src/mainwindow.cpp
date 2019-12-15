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

#include "mainwindow.h"

#include <QApplication>
#include <QAction>
#include <QTimer>
#include <QMenu>
#include <QStandardPaths>
#include <QMimeDatabase>
#include <QDir>
#include <KNotifications/KNotification>
#include <QDesktopServices>

#define USER_AGENT "Mozilla/5.0 Gecko/20100101 Firefox/70.0"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_settings(new QSettings(this)),
    m_net_manager(new QNetworkConfigurationManager(this)),
    m_status_notifier(new KStatusNotifierItem(this))
{
    m_profile = new QWebEngineProfile("QtWAW", this);
    m_page = new WebEnginePage(m_profile, this);

    icon = QIcon(QString("%1/../share/icons/hicolor/256x256/apps/"
                         "eu.scarpetta.QtWAW.png").arg(
                     qApp->applicationDirPath()));

    // workaround: remove "Service Worker" directory
    QDir web_engine_dir(m_profile->persistentStoragePath());
    web_engine_dir.cd("Service Worker");
    web_engine_dir.removeRecursively();

    // Main winow properties
    this->setMinimumSize(400, 400);
    this->setWindowIcon(QIcon::fromTheme("eu.scarpetta.QtWAW"));
    this->setWindowTitle("QtWAW");
    this->restoreGeometry(
                m_settings->value("main_window_geometry").toByteArray()
                );

    QAction *quit_action = new QAction(QIcon::fromTheme("application-exit"),
                                       tr("Quit"),
                                       this);
    quit_action->setShortcut(QKeySequence::Quit);
    this->addAction(quit_action);
    connect(quit_action,
            SIGNAL(triggered()),
            SLOT(quit()));

    QAction *zoom_in_action = new QAction(QIcon::fromTheme("zoom-in"),
                                       tr("Zoom in"),
                                       this);
    zoom_in_action->setShortcut(QKeySequence::ZoomIn);
    this->addAction(zoom_in_action);
    connect(zoom_in_action,
            SIGNAL(triggered()),
            SLOT(zoom_in()));

    QAction *zoom_out_action = new QAction(QIcon::fromTheme("zoom-out"),
                                       tr("Zoom out"),
                                       this);
    zoom_out_action->setShortcut(QKeySequence::ZoomOut);
    this->addAction(zoom_out_action);
    connect(zoom_out_action,
            SIGNAL(triggered()),
            SLOT(zoom_out()));

    QAction *zoom_original_action = new QAction(
                QIcon::fromTheme("zoom-original"),
                tr("Original size"),
                this);
    zoom_original_action->setShortcut(QKeySequence(tr("Ctrl+0")));
    this->addAction(zoom_original_action);
    connect(zoom_original_action,
            SIGNAL(triggered()),
            SLOT(zoom_original()));

    QAction *refresh_action = new QAction(
                QIcon::fromTheme("view-refresh"),
                tr("Refresh"),
                this);
    refresh_action->setShortcut(QKeySequence::Refresh);
    this->addAction(refresh_action);
    connect(refresh_action,
            SIGNAL(triggered()),
            &m_view,
            SLOT(reload()));

    QAction *start_minimized_action = new QAction(
                tr("Start minimized"),
                this);
    start_minimized_action->setCheckable(true);
    start_minimized_action->setChecked(
                m_settings->value("start_minimized", false).toBool());
    connect(start_minimized_action,
            SIGNAL(toggled(bool)),
            SLOT(start_minimized_toggled(bool)));

    QMenu *tray_menu = new QMenu(this);
    tray_menu->addAction(refresh_action);
    tray_menu->addAction(zoom_in_action);
    tray_menu->addAction(zoom_out_action);
    tray_menu->addAction(zoom_original_action);
    tray_menu->addAction(start_minimized_action);
    tray_menu->addAction(quit_action);

    m_status_notifier->setContextMenu(tray_menu);
    m_status_notifier->setStandardActionsEnabled(false);
    m_status_notifier->setIconByName("eu.scarpetta.QtWAW");
    m_status_notifier->setCategory(KStatusNotifierItem::Communications);
    m_status_notifier->setStatus(KStatusNotifierItem::Passive);

    this->setCentralWidget(&m_view);

    if (m_profile->httpUserAgent() != USER_AGENT)
        m_profile->setHttpUserAgent(USER_AGENT);

    MainWindow *mw = this;
    std::function<void (std::unique_ptr<QWebEngineNotification>)> presenter =
            [mw] (std::unique_ptr<QWebEngineNotification> notification)
    {
        mw->notification_presenter(notification.get());
    };

    m_profile->setNotificationPresenter(presenter);
    m_page->load(QUrl("https://web.whatsapp.com"));
    m_page->setZoomFactor(m_settings->value("zoom_factor").toReal());
    m_view.setPage(m_page);

    QTimer timer;
    timer.singleShot(60000, this, SLOT(FIXME_trigger_permission_request()));

    connect(m_page,
            SIGNAL(titleChanged(const QString)),
            SLOT(title_changed(const QString)));

    connect(m_page,
            SIGNAL(featurePermissionRequested(
                       const QUrl&,
                       QWebEnginePage::Feature)),
            SLOT(feature_request(const QUrl&, QWebEnginePage::Feature)));

    connect(m_profile,
            SIGNAL(downloadRequested(QWebEngineDownloadItem *)),
            SLOT(download_requested(QWebEngineDownloadItem *)));

    // Check if the system is online
    if (!m_net_manager->isOnline())
        connect(m_net_manager,
                SIGNAL(onlineStateChanged(bool)),
                SLOT(online_status_changed(bool)));

    if (m_settings->value("start_minimized", false).toBool())
        this->hide();
    else
        this->show();
}

void MainWindow::online_status_changed(bool is_online)
{
    if (is_online)
    {
        m_view.reload();

        disconnect(m_net_manager,
                   SIGNAL(onlineStateChanged(bool)),
                   this,
                   SLOT(gone_online()));
    }
}

void MainWindow::FIXME_trigger_permission_request()
{
    QString script = ""
    "var el = document.querySelector('[data-icon=\"chevron-right-text\"]');"
    "var ev = document.createEvent('Events');"
    "ev.initEvent('click', true, false);"
    "el.dispatchEvent(ev);";

    m_page->runJavaScript(script);
}

void MainWindow::raise_main_window()
{
    this->show();
    this->activateWindow();
    this->raise();
}

void MainWindow::feature_request(const QUrl &securityOrigin,
                                 QWebEnginePage::Feature feature)
{
    if (securityOrigin.host() == QString("web.whatsapp.com") &&
            securityOrigin.scheme() == QString("https") &&
            feature == QWebEnginePage::Notifications)
        m_page->setFeaturePermission(securityOrigin,
                                     feature,
                                     QWebEnginePage::PermissionGrantedByUser);
}

void MainWindow::zoom_in()
{
    m_page->setZoomFactor(m_page->zoomFactor() + 0.1);
}

void MainWindow::zoom_out()
{
    m_page->setZoomFactor(m_page->zoomFactor() - 0.1);
}

void MainWindow::zoom_original()
{
    m_page->setZoomFactor(1.0);
}

void MainWindow::start_minimized_toggled(bool checked)
{
    m_settings->setValue("start_minimized", checked);
}

void MainWindow::title_changed(const QString &title)
{
    int a = title.indexOf('(');
    if (a > -1)
    {
        QString count = title.mid(a + 1, title.indexOf(')') - a - 1);
        QPixmap pixmap = icon.pixmap(256);
        QPainter painter(&pixmap);

        int font_size = 150;
        int m = 20; //margins
        QFont font = painter.font();
        font.setPixelSize(font_size);
        font.setBold(true);
        QFontMetrics fm(font);
        QRect text_rect = fm.boundingRect(0, 0, 256, font_size,
                                     Qt::AlignCenter, count);

        QPainterPath path;
        QMargins margins(m, 0, m, 0);
        QRect rect = text_rect + margins;
        rect.translate(-rect.x(), -rect.y());
        path.addRoundedRect(rect, 3 * m, 3 * m);
        painter.fillPath(path, QColor(Qt::red));

        text_rect = rect - margins;
        painter.setFont(font);
        painter.setPen(QColor(Qt::white));
        painter.drawText(rect, count, QTextOption(Qt::AlignCenter));

        m_status_notifier->setIconByPixmap(pixmap);
        m_status_notifier->setStatus(KStatusNotifierItem::Active);
    }
    else
    {
        m_status_notifier->setIconByName("eu.scarpetta.QtWAW");
        m_status_notifier->setStatus(KStatusNotifierItem::Passive);
    }
}

void MainWindow::notification_presenter(QWebEngineNotification *notification)
{
    KNotification *n = new KNotification(
                "new-message",
                this,
                KNotification::CloseOnTimeout |
                KNotification::CloseWhenWidgetActivated);
    n->setTitle(notification->title());
    n->setText(notification->message());
    n->setPixmap(QPixmap::fromImage(notification->icon()));
    n->setDefaultAction(tr("Open"));
    connect(n, SIGNAL(activated()), SLOT(raise_main_window()));
    n->sendEvent();
}

void MainWindow::download_requested(QWebEngineDownloadItem *download)
{
    QMimeDatabase db;

    QDir dir(QStandardPaths::writableLocation(
                 QStandardPaths::DownloadLocation));
    if (!dir.exists("WhatsApp"))
        dir.mkdir("WhatsApp");
    dir.cd("WhatsApp");

    QMimeType mime = db.mimeTypeForName(download->mimeType());
    download->setPath(dir.absoluteFilePath(download->url().fileName() +
                                           "." +
                                           mime.suffixes().at(0)));

    connect(download, SIGNAL(finished()), SLOT(download_finished()));

    download->accept();
}

void MainWindow::download_finished()
{
    QWebEngineDownloadItem * download =
            static_cast<QWebEngineDownloadItem *>(QObject::sender());

    KNotification *n = new KNotification(
                "download-finished",
                this,
                KNotification::CloseOnTimeout |
                KNotification::CloseWhenWidgetActivated);
    n->setTitle(tr("Download completed"));
    n->setText(tr("File %1 have beens successfully downloaded")
               .arg(QUrl(download->path()).fileName()));
    QList<QUrl> urls;
    urls.push_back(QUrl(download->path()));
    n->setUrls(urls);
    n->sendEvent();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    this->hide();
}

void MainWindow::quit()
{
    m_settings->setValue("main_window_geometry", this->saveGeometry());
    m_settings->setValue("zoom_factor", m_page->zoomFactor());
    m_settings->sync();

    delete m_page;

    qApp->quit();
}