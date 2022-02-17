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

#include "mainwindow.h"

#include <KNotifications/KNotification>
#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QMenu>
#include <QPainterPath>
#include <QStandardPaths>
#include <QTimer>

const QString MainWindow::userAgent() const
{
    return QStringLiteral("Mozilla/5.0 (X11; Linux x86_64; rv:95.0) Gecko/20100101 Firefox/96.0");
}
const QString MainWindow::whatsappUrl() const
{
    return QStringLiteral("https://web.whatsapp.com");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_settings(new QSettings(this))
    , m_connectionTimeout(250)
    , m_statusNotifier(new KStatusNotifierItem(this))
{
    m_profile = new QWebEngineProfile(QStringLiteral("QtWAW"), this);
    m_page = new WebEnginePage(m_profile, this);

    m_icon = QIcon::fromTheme(QStringLiteral("whatsapp"));

    //    // workaround: remove "Service Worker" directory
    //    QDir web_engine_dir(m_profile->persistentStoragePath());
    //    web_engine_dir.cd("Service Worker");
    //    web_engine_dir.removeRecursively();

    // Main winow properties
    this->setMinimumSize(400, 400);
    this->setWindowIcon(QIcon::fromTheme(QStringLiteral("whatsapp")));
    this->setWindowTitle(QStringLiteral("QtWAW"));
    this->restoreGeometry(m_settings->value(QStringLiteral("main_window_geometry")).toByteArray());

    QAction *quit_action = new QAction(QIcon::fromTheme(QStringLiteral("application-exit")), tr("Quit"), this);
    quit_action->setShortcut(QKeySequence::Quit);
    this->addAction(quit_action);
    connect(quit_action, &QAction::triggered, this, &MainWindow::quit);

    QAction *zoom_in_action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), tr("Zoom in"), this);
    zoom_in_action->setShortcut(QKeySequence::ZoomIn);
    this->addAction(zoom_in_action);
    connect(zoom_in_action, &QAction::triggered, this, &MainWindow::zoomIn);

    QAction *zoom_out_action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), tr("Zoom out"), this);
    zoom_out_action->setShortcut(QKeySequence::ZoomOut);
    this->addAction(zoom_out_action);
    connect(zoom_out_action, &QAction::triggered, this, &MainWindow::zoomOut);

    QAction *zoom_original_action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-original")), tr("Original size"), this);
    zoom_original_action->setShortcut(QKeySequence(tr("Ctrl+0")));
    this->addAction(zoom_original_action);
    connect(zoom_original_action, &QAction::triggered, this, &MainWindow::zoomOriginal);

    QAction *refresh_action = new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")), tr("Refresh"), this);
    refresh_action->setShortcut(QKeySequence::Refresh);
    this->addAction(refresh_action);
    connect(refresh_action, &QAction::triggered, this, &MainWindow::reload);

    QAction *start_minimized_action = new QAction(tr("Start minimized"), this);
    start_minimized_action->setCheckable(true);
    start_minimized_action->setChecked(m_settings->value(QStringLiteral("start_minimized"), false).toBool());
    connect(start_minimized_action, &QAction::toggled, this, &MainWindow::startMinimizedToggled);

    QAction *close_to_tray_action = new QAction(tr("Close to tray"), this);
    close_to_tray_action->setCheckable(true);
    close_to_tray_action->setChecked(m_settings->value(QStringLiteral("close_to_tray"), true).toBool());
    connect(close_to_tray_action, &QAction::toggled, this, &MainWindow::closeToTrayToggled);

    QMenu *tray_menu = new QMenu(this);
    tray_menu->addAction(refresh_action);
    tray_menu->addAction(zoom_in_action);
    tray_menu->addAction(zoom_out_action);
    tray_menu->addAction(zoom_original_action);
    tray_menu->addAction(start_minimized_action);
    tray_menu->addAction(close_to_tray_action);
    tray_menu->addAction(quit_action);

    m_statusNotifier->setContextMenu(tray_menu);
    m_statusNotifier->setStandardActionsEnabled(false);
    m_statusNotifier->setCategory(KStatusNotifierItem::Communications);
    titleChanged(QLatin1String(""));

    this->setCentralWidget(&m_view);

    if (m_profile->httpUserAgent() != userAgent())
        m_profile->setHttpUserAgent(userAgent());

    MainWindow *mw = this;
    std::function<void(std::unique_ptr<QWebEngineNotification>)> presenter = [mw](const std::unique_ptr<QWebEngineNotification> &notification) {
        mw->notificationPresenter(notification.get());
    };

    m_profile->setNotificationPresenter(presenter);
    m_view.setPage(m_page);

    connect(m_page, &QWebEnginePage::titleChanged, this, &MainWindow::titleChanged);

    connect(m_page, &QWebEnginePage::featurePermissionRequested, this, &MainWindow::featureRequested);

    connect(m_profile, &QWebEngineProfile::downloadRequested, this, &MainWindow::downloadRequested);

    connect(m_page, &QWebEnginePage::loadFinished, this, &MainWindow::loadFinished);

    // load whatsapp web
    QFile loading_page(QStringLiteral("%1/../share/qtwaw/loading.html").arg(qApp->applicationDirPath()));
    loading_page.open(QFile::ReadOnly);
    m_page->setContent(loading_page.readAll(), QStringLiteral("text/html"));
    loading_page.close();

    connect(&m_networkAccessManager, &QNetworkAccessManager::finished, this, &MainWindow::replyFinished);
    loadWhatsapp();

    if (m_settings->value(QStringLiteral("start_minimized"), false).toBool())
        this->hide();
    else
        this->show();
}

void MainWindow::loadWhatsapp()
{
    if (m_connectionTimeout < 4001)
        m_connectionTimeout *= 2;

    // check connectivity
    m_networkAccessManager.get(QNetworkRequest(QUrl(QStringLiteral("http://www.whatsapp.com"))));
}

void MainWindow::replyFinished(QNetworkReply *reply)
{
    // load whatsapp web if there is connectivity,
    // re-test after the timeout if there isn't connectivity

    if (reply->error() == QNetworkReply::NoError)
        m_page->load(QUrl(whatsappUrl()));
    else
        QTimer::singleShot(m_connectionTimeout, this, &MainWindow::loadWhatsapp);

    reply->deleteLater();
}

void MainWindow::loadFinished()
{
    m_page->setZoomFactor(m_settings->value(QStringLiteral("zoom_factor")).toReal());
    QTimer::singleShot(60000, this, &MainWindow::FIXME_triggerPermissionRequest);
}

void MainWindow::FIXME_triggerPermissionRequest()
{
    QString script =
        ""
        "var el = document.querySelector('[data-icon=\"chevron-right-text\"]');"
        "var ev = document.createEvent('Events');"
        "ev.initEvent('click', true, false);"
        "el.dispatchEvent(ev);";

    m_page->runJavaScript(script);
}

void MainWindow::raiseMainWindow()
{
    this->show();
    this->activateWindow();
    this->raise();
}

void MainWindow::featureRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature)
{
    if (securityOrigin.host() == QStringLiteral("web.whatsapp.com") && securityOrigin.scheme() == QStringLiteral("https")
        && (feature == QWebEnginePage::Notifications || feature == QWebEnginePage::MediaAudioCapture))
        m_page->setFeaturePermission(securityOrigin, feature, QWebEnginePage::PermissionGrantedByUser);
}

void MainWindow::zoomIn()
{
    m_page->setZoomFactor(m_page->zoomFactor() + 0.1);
}

void MainWindow::zoomOut()
{
    m_page->setZoomFactor(m_page->zoomFactor() - 0.1);
}

void MainWindow::zoomOriginal()
{
    m_page->setZoomFactor(1.0);
}

void MainWindow::reload()
{
    QString script =
        "try {"
        "navigator.serviceWorker.getRegistration()"
        ".then(function (r) {"
        "r.unregister();"
        "document.location.replace(\"https://web.whatsapp.com\");"
        "})"
        ".catch(function (reason) {"
        "document.location.replace(\"https://web.whatsapp.com\");"
        "});"
        "} catch(error) {"
        "document.location.replace(\"https://web.whatsapp.com\");"
        "}";

    m_page->runJavaScript(script);
}

void MainWindow::startMinimizedToggled(bool checked)
{
    m_settings->setValue(QStringLiteral("start_minimized"), checked);
}

void MainWindow::closeToTrayToggled(bool checked)
{
    m_settings->setValue(QStringLiteral("close_to_tray"), checked);
}

void MainWindow::titleChanged(const QString &title)
{
    QPixmap pixmap = m_icon.pixmap(256);

    int a = title.indexOf('(');
    if (a > -1) {
        QString count = title.mid(a + 1, title.indexOf(')') - a - 1);
        QPainter painter(&pixmap);

        int font_size = 150;
        int m = 20; // margins
        QFont font = painter.font();
        font.setPixelSize(font_size);
        font.setBold(true);
        QFontMetrics fm(font);
        QRect text_rect = fm.boundingRect(0, 0, 256, font_size, Qt::AlignCenter, count);

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

        m_statusNotifier->setIconByPixmap(pixmap);
        m_statusNotifier->setStatus(KStatusNotifierItem::Active);
    } else {
        // Use full the pixmap instead of name because it's better supported
        m_statusNotifier->setIconByPixmap(pixmap);
        m_statusNotifier->setStatus(KStatusNotifierItem::Passive);
    }
}

void MainWindow::notificationPresenter(QWebEngineNotification *notification)
{
    KNotification *n = new KNotification(QStringLiteral("new-message"), KNotification::CloseOnTimeout | KNotification::CloseWhenWidgetActivated);
    n->setWidget(this);
    n->setTitle(notification->title());
    n->setText(notification->message());
    n->setPixmap(QPixmap::fromImage(notification->icon()));
    n->setDefaultAction(tr("Open"));
    connect(n, &KNotification::defaultActivated, this, &MainWindow::raiseMainWindow);
    n->sendEvent();
}

void MainWindow::downloadRequested(QWebEngineDownloadItem *download)
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    if (!dir.exists(QStringLiteral("WhatsApp")))
        dir.mkdir(QStringLiteral("WhatsApp"));
    dir.cd(QStringLiteral("WhatsApp"));

    download->setDownloadDirectory(dir.path());
    download->setDownloadFileName(download->suggestedFileName());

    connect(download, &QWebEngineDownloadItem::finished, this, &MainWindow::downloadFinished);

    download->accept();
}

void MainWindow::downloadFinished()
{
    QWebEngineDownloadItem *download = static_cast<QWebEngineDownloadItem *>(QObject::sender());

    KNotification *n = new KNotification(QStringLiteral("download-finished"), KNotification::CloseOnTimeout | KNotification::CloseWhenWidgetActivated);
    n->setWidget(this);
    n->setTitle(tr("Download completed"));
    n->setText(tr("File %1 have beens successfully downloaded").arg(download->downloadFileName()));
    QDir dir = download->downloadDirectory();
    QList<QUrl> urls;
    urls.push_back(dir.absoluteFilePath(download->downloadFileName()));
    n->setUrls(urls);
    n->sendEvent();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    if (m_settings->value(QStringLiteral("close_to_tray"), true).toBool())
        this->hide();
    else
        this->quit();
}

void MainWindow::quit()
{
    m_settings->setValue(QStringLiteral("main_window_geometry"), this->saveGeometry());
    m_settings->setValue(QStringLiteral("zoom_factor"), m_page->zoomFactor());
    m_settings->sync();

    delete m_page;

    qApp->quit();
}
