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

#include "webenginepage.h"

#include <QDesktopServices>

WebEnginePage::WebEnginePage(QWebEngineProfile *profile, QObject *parent) :
    QWebEnginePage(profile, parent),
    m_new_tab_page(nullptr)
{

}

QWebEnginePage *WebEnginePage::createWindow(QWebEnginePage::WebWindowType type)
{
    Q_UNUSED(type)

    if (m_new_tab_page == nullptr)
    {
        m_new_tab_page = new QWebEnginePage();
        connect(m_new_tab_page,
                SIGNAL(loadStarted()),
                SLOT(new_tab_load_started()));
        return m_new_tab_page;
    }

    return nullptr;
}

void WebEnginePage::new_tab_load_started()
{
    QDesktopServices::openUrl(m_new_tab_page->requestedUrl());

    delete m_new_tab_page;
    m_new_tab_page = nullptr;
}
