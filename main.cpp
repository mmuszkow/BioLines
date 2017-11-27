/*
* BioLines (https://github.com/mmuszkow/BioLines)
* Detection of filamentous structures in biological microscopic images
* Copyright(C) 2017 Maciek Muszkowski
*
* This program is free software : you can redistribute it and / or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "biolines2.h"
#include <QtWidgets/QApplication>
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

// Qt5.9.2 compiled with
// configure -static -static-runtime -debug-and-release -mp -prefix "G:\Qt\5.9.2\msvc2015_64-static-mt" -opensource -confirm-license -make libs -make tools -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg -qt-freetype -no-dbus -no-gstreamer -no-opengl -no-sql-sqlite -no-sql-odbc -no-qml-debug -no-openssl -no-angle -no-icu -no-zlib -no-libjpeg -no-libpng -no-freetype -no-harfbuzz -skip qtactiveqt -skip qtenginio -skip qtlocation -skip qtserialport -skip qtquick1 -skip qtsensors -skip qtwebsockets -skip qtxmlpatterns -skip qt3d -nomake examples -nomake tests

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	BioLines2 w;

	a.setApplicationName("BioLines2");
	a.setApplicationVersion("2.2");

	w.show();

	if (argc > 1)
		w.setArgs(a.arguments());

	return a.exec();
}
