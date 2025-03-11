/*
    Copyright 2020 - 2025 Christian R. Halaszovich

     This file is part of PMbrowser.

    PMbrowser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PMbrowser is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PMbrowser.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <locale>
#include "pmbrowserwindow.h"
#include <QApplication>
#include <QSettings>
#include <QDebug>

int main(int argc, char *argv[])
{
    //QApplication::setStyle("fusion");
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("CRHalaszovichMD");
    QCoreApplication::setOrganizationDomain("halaszovich.de");
    QCoreApplication::setApplicationName("PM browser");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QApplication::setWindowIcon(QIcon(QString(":/myappico.ico"))); // sets icon in OS X dock

    {
        QSettings settings;
        const bool use_C_locale = settings.value("Preferences/use_C_locale", false).toBool();
        if (use_C_locale) {
            QLocale::setDefault(QLocale::c());
        }
        QLocale loc;
        std::locale::global(std::locale(loc.name().toUtf8()));
        qDebug() << std::locale().name();
    }
    PMbrowserWindow w;
    w.show();
    if (argc > 1) {
        w.loadFile(argv[1]);
    }
    return a.exec();
}
