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

#ifndef BIOLINES2_H
#define BIOLINES2_H

#include <QtWidgets/QMainWindow>
#include "ui_biolines2.h"
#include "algorithm.h"

class BioLines2 : public QMainWindow
{
	Q_OBJECT
private:
	Ui::BioLines2Class ui;
	QFileDialog inputDialog, outputDialog;
	QStringList selectedImages;
	QString outputDir;
	Algorithm algo;
	bool saveSettingsOnQuit;
	bool closeOnFinish;

public:
	BioLines2(QWidget *parent = 0);
	~BioLines2();

	// in case of running the program from commandline with parameters specified
	void setArgs(const QStringList& args);

protected slots:
	// opens images selection dialog
	void selectImages();
	// opens output directory selection dialog
	void selectOutputDirectory();
	// sets the output directory to same value as input one when the checkbox is checked
	void setOutDirSameAsInput();
	// makes sure that 2nd treshold angle is bigger than 1st
	void adjust2ndangle();
	// start/stop algorithm
	void startStopAlgorithm();
	// when algo finishes set button to Run again
	void onAlgoFinished();

protected:
	// on close e need to stop the algorithm and wait for it's thread to finish and also save changed fields values
	void closeEvent(QCloseEvent *event) override;

private:
	void readSettings();
	void writeSettings();

};

#endif // BIOLINES2_H
