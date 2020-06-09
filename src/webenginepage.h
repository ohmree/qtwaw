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

#ifndef WEBENGINEPAGE_H
#define WEBENGINEPAGE_H

#include <QWebEnginePage>

class WebEnginePage : public QWebEnginePage
{
    Q_OBJECT
public:
    WebEnginePage(QWebEngineProfile *profile, QObject *parent = nullptr);

public slots:
    void new_tab_load_started();

protected:
    QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type);

private:
    QWebEnginePage *m_new_tab_page;
};

#endif // WEBENGINEPAGE_H
