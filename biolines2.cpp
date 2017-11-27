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

BioLines2::BioLines2(QWidget *parent)
	: QMainWindow(parent), algo(parent), saveSettingsOnQuit(true), closeOnFinish(false)
{
	ui.setupUi(this);

	// TODO Q_PROPERTY DESIGNABLE doesn't work, this could be done in QtDesigner
	ui.class1ColorButton->setColor(Qt::red);
	ui.class2ColorButton->setColor(Qt::gray);
	ui.class3ColorButton->setColor(Qt::green);

	// preview widget
	connect(ui.class1ColorButton, &ColorButton::colorChanged, ui.previewWidget, &PreviewWidget::setColor1);
	connect(ui.class2ColorButton, &ColorButton::colorChanged, ui.previewWidget, &PreviewWidget::setColor2);
	connect(ui.class3ColorButton, &ColorButton::colorChanged, ui.previewWidget, &PreviewWidget::setColor3);
	connect(ui.colorTreshold1SpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui.previewWidget, &PreviewWidget::setAngle1);
	connect(ui.colorTreshold2SpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui.previewWidget, &PreviewWidget::setAngle2);

	// angle ranges
	connect(ui.colorTreshold1SpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui.colorTreshold1repeatSpinBox, &QSpinBox::setValue);
	connect(ui.colorTreshold2SpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui.colorTreshold2repeatSpinBox, &QSpinBox::setValue);
	connect(ui.colorTreshold1SpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &BioLines2::adjust2ndangle);

	// class names
	connect(ui.class1NameEdit, &QLineEdit::textChanged, ui.class1GroupBox, &QGroupBox::setTitle);
	connect(ui.class2NameEdit, &QLineEdit::textChanged, ui.class2GroupBox, &QGroupBox::setTitle);
	connect(ui.class3NameEdit, &QLineEdit::textChanged, ui.class3GroupBox, &QGroupBox::setTitle);

	// cell walls removal
	connect(ui.removeCellEdgesCheckBox, &QCheckBox::toggled, ui.cellEdgesRemovalGroupBox, &QGroupBox::setEnabled);

	// dialogs
	inputDialog.setFileMode(QFileDialog::ExistingFiles);
	inputDialog.setNameFilter("Images (*.png *.jpg *.jpeg *.tif *.lsm)");
	outputDialog.setFileMode(QFileDialog::DirectoryOnly);
	connect(ui.selectImagesButton, &QPushButton::clicked, this, &BioLines2::selectImages);
	connect(ui.selectOutputDirButton, &QPushButton::clicked, this, &BioLines2::selectOutputDirectory);
	connect(ui.outputDirSameAsInputCheckBox, &QCheckBox::toggled, ui.selectOutputDirButton, &QPushButton::setDisabled);
	connect(ui.outputDirSameAsInputCheckBox, &QCheckBox::toggled, this, &BioLines2::setOutDirSameAsInput);

	// algorithm
	connect(&algo, &Algorithm::progressMade, ui.progressBar, &QProgressBar::setValue);
	connect(&algo, &Algorithm::etaUpdated, ui.etaLabel, &QLabel::setText);
	connect(&algo, &Algorithm::finished, this, &BioLines2::onAlgoFinished);
	connect(ui.runButton, &QPushButton::clicked, this, &BioLines2::startStopAlgorithm);

	readSettings();
}

BioLines2::~BioLines2()
{

}

void BioLines2::setArgs(const QStringList& args) {
	QCommandLineParser parser;

					    // optional parameters with value
	QCommandLineOption	outputDirOption("outputDir", "Output directory", "directory"),
						color1option("color1", "1st class color, hexadecimal eg. ff0000", "color"),
						color2option("color2", "2nd class color, hexadecimal eg. 00ff00", "color"),
						color3option("color3", "3rd class color, hexadecimal eg. 0000ff", "color"),
						lineLengthOption("length", "Detected lines length in pixels", "pixels"),
						thicknessOption("thickness", "Detected lines thickness in pixels", "pixels"),
						iterationsOption("iter", "Number of iterations beforethe algorithm is stopped", "number"),
						angle1option("angle1", "1st angle", "degrees"),
						angle2option("angle2", "2nd angle", "degrees"),
						coverageOption("coverage", "%% of the non-black pixels under the line to mark it", "%%"),
						class1nameOption("class1", "1st class name", "name"),
						class2nameOption("class2", "2nd class name", "name"),
						class3nameOption("class3", "3rd class name", "name"),
						cellWallsLineLengthOption("cellWallLength", "Detected lines length in pixels for cell walls removal preprocessing", "pixels"),
						cellWallsThicknessOption("cellWallThickness", "Detected lines thickness in pixels for cell walls removal preprocessing", "pixels"),
						cellWallsCoverageOption("cellWallCoverage", "%% of the non-black pixels under the line to mark it for cell walls removal preprocessing", "%%"),
						// optional, no value parameters
						autoRotateOption("autoRotate", "Preprocessing: auto-rotate image to vertical position"),
						removeCellEdgesOption("removeCellEdges", "Preprocessing: remove cell edges"),
						removedCellEdgesPreviewOption("removedCellEdgesPreview", "Preprocessing: output removed cell edges into file"),
						outputCombinedOption("outputCombined", "Output mask of all classes together"),
						outputCombinedWithSrcOption("outputCombinedWithSrc", "Output all classes together on source image"),
						outputClass1Option("outputClass1", "Output 1st class mask"),
						outputClass1WithSrcOption("outputClass1WithSrc", "Output 1st class on source image"),
						outputClass2Option("outputClass2", "Output2nd class mask"),
						outputClass2WithSrcOption("outputClass2WithSrc", "Output 2nd class on source image"),
						outputClass3Option("outputClass3", "Output 3rd class mask"),
						outputClass3WithSrcOption("outputClass3WithSrc", "Output 3rd class on source image"),
						autoStartOption("autoStart", "Start algorithm at program start"),
						autoCloseOption("autoClose", "Close the program when the algorithm finishes"),
						noSaveOption("noSave", "Don't restore parameters values on next program run");

	// setup all options
	parser.setApplicationDescription("BioLines");
	parser.addHelpOption();
	parser.addPositionalArgument("images", "Input images to be processed");
	parser.addOption(outputDirOption);
	parser.addOption(color1option);
	parser.addOption(color2option);
	parser.addOption(color3option);
	parser.addOption(lineLengthOption);
	parser.addOption(thicknessOption);
	parser.addOption(iterationsOption);
	parser.addOption(angle1option);
	parser.addOption(angle2option);
	parser.addOption(coverageOption);
	parser.addOption(class1nameOption);
	parser.addOption(class2nameOption);
	parser.addOption(class3nameOption);
	parser.addOption(cellWallsLineLengthOption),
	parser.addOption(cellWallsThicknessOption),
	parser.addOption(cellWallsCoverageOption),
	parser.addOption(autoRotateOption);
	parser.addOption(removeCellEdgesOption);
	parser.addOption(removedCellEdgesPreviewOption);
	parser.addOption(outputCombinedOption);
	parser.addOption(outputCombinedWithSrcOption);
	parser.addOption(outputClass1Option);
	parser.addOption(outputClass1WithSrcOption);
	parser.addOption(outputClass2Option);
	parser.addOption(outputClass2WithSrcOption);
	parser.addOption(outputClass3Option);
	parser.addOption(outputClass3WithSrcOption);
	parser.addOption(autoStartOption);
	parser.addOption(autoCloseOption);
	parser.addOption(noSaveOption);

	// parse & set values
	if (parser.parse(args)) {

		selectedImages = parser.positionalArguments();
		ui.selectedImagesLabel->setText(QString("%1 images selected").arg(selectedImages.size()));

		if (parser.isSet(outputDirOption)) {
			outputDir = parser.value(outputDirOption);
			ui.outputDirLabel->setText(outputDir);
		}

		if(parser.isSet(color1option))
			ui.class1ColorButton->setColor(QColor(parser.value(color1option)));
		if (parser.isSet(color2option))
			ui.class2ColorButton->setColor(QColor(parser.value(color2option)));
		if (parser.isSet(color3option))
			ui.class3ColorButton->setColor(QColor(parser.value(color3option)));

		if(parser.isSet(lineLengthOption))
			ui.lineLengthSpinBox->setValue(parser.value(lineLengthOption).toInt());
		if (parser.isSet(thicknessOption))
			ui.lineThicknessSpinBox->setValue(parser.value(thicknessOption).toInt());
		if (parser.isSet(iterationsOption))
			ui.iterationsSpinBox->setValue(parser.value(iterationsOption).toInt());
		if (parser.isSet(angle1option))
			ui.colorTreshold1SpinBox->setValue(parser.value(angle1option).toInt());
		if (parser.isSet(angle2option))
			ui.colorTreshold2SpinBox->setValue(parser.value(angle2option).toInt());
		if (parser.isSet(coverageOption))
			ui.coverageSpinBox->setValue(parser.value(coverageOption).toInt());

		if (parser.isSet(cellWallsLineLengthOption))
			ui.cellWallsLineLengthSpinBox->setValue(parser.value(cellWallsLineLengthOption).toInt());
		if (parser.isSet(cellWallsThicknessOption))
			ui.cellWallsLineThicknessSpinBox->setValue(parser.value(cellWallsThicknessOption).toInt());
		if (parser.isSet(cellWallsCoverageOption))
			ui.cellWallsCoverageSpinBox->setValue(parser.value(cellWallsCoverageOption).toInt());

		ui.autoRotateCheckBox->setChecked(parser.isSet(autoRotateOption));
		ui.removeCellEdgesCheckBox->setChecked(parser.isSet(removeCellEdgesOption));
		ui.removedCellEdgesPreviewCheckBox->setChecked(parser.isSet(removedCellEdgesPreviewOption));
		ui.allClassesCheckBox->setChecked(parser.isSet(outputCombinedOption));
		ui.allClassesWithSrcCheckbox->setChecked(parser.isSet(outputCombinedWithSrcOption));
		ui.class1CheckBox->setChecked(parser.isSet(outputClass1Option));
		ui.class1withSrcCheckbox->setChecked(parser.isSet(outputClass1WithSrcOption));
		ui.class2CheckBox->setChecked(parser.isSet(outputClass2Option));
		ui.class2withSrcCheckbox->setChecked(parser.isSet(outputClass2WithSrcOption));
		ui.class3CheckBox->setChecked(parser.isSet(outputClass3Option));
		ui.class3withSrcCheckbox->setChecked(parser.isSet(outputClass3WithSrcOption));

		if(parser.isSet(class1nameOption))
			ui.class1NameEdit->setText(parser.value(class1nameOption));
		if (parser.isSet(class2nameOption))
			ui.class2NameEdit->setText(parser.value(class2nameOption));
		if (parser.isSet(class3nameOption))
			ui.class3NameEdit->setText(parser.value(class3nameOption));

		saveSettingsOnQuit = !parser.isSet(noSaveOption);
		closeOnFinish = parser.isSet(autoCloseOption);

		// start algorithm if autostart enabled
		if (parser.isSet(autoStartOption))
			startStopAlgorithm();
	}
	else
		parser.showHelp();
}

void BioLines2::selectImages() {
	if (inputDialog.exec()) {
		selectedImages = inputDialog.selectedFiles();
		ui.selectedImagesLabel->setText(QString("%1 images selected").arg(selectedImages.size()));
		setOutDirSameAsInput(); // if checkbox checked
	}
}

void BioLines2::setOutDirSameAsInput() {
	if (ui.outputDirSameAsInputCheckBox->isChecked() && selectedImages.size() > 0) {
		QFileInfo fi(selectedImages.first());
		outputDir = fi.absoluteDir().absolutePath();
		ui.outputDirLabel->setText(outputDir);
	}
}

void BioLines2::selectOutputDirectory() {
	if (outputDialog.exec()) {
		outputDir = *outputDialog.selectedFiles().begin();
		ui.outputDirLabel->setText(outputDir);
	}
}

void BioLines2::adjust2ndangle() {
	int angle1 = ui.colorTreshold1SpinBox->value();
	int angle2 = ui.colorTreshold2SpinBox->value();
	ui.colorTreshold1SpinBox->setValue(std::min(angle1, angle2));
	ui.colorTreshold2SpinBox->setValue(std::max(angle1, angle2));
}

void BioLines2::startStopAlgorithm() {
	if (selectedImages.size() > 0 && outputDir != "") {
		if (algo.isRunning()) {
			algo.stop();
			ui.progressBar->setValue(0);
			ui.etaLabel->setText("");
			ui.runButton->setText("Stopping ...");
		}
		else {
			AlgorithmWorker::LinesParameters cellWalls = {
				cv::Vec3b(0xFF, 0, 0),
				cv::Vec3b(0, 0xFF, 0),
				cv::Vec3b(0, 0, 0xFF),
				ui.cellWallsLineLengthSpinBox->value(),
				ui.cellWallsLineThicknessSpinBox->value(),
				ui.iterationsSpinBox->value(),
				ui.colorTreshold1SpinBox->value(),
				ui.colorTreshold2SpinBox->value(),
				ui.cellWallsCoverageSpinBox->value() / 100.0f
			};
			AlgorithmWorker::LinesParameters mainAlgo = {
				ui.class1ColorButton->getOpenCvColor(),
				ui.class2ColorButton->getOpenCvColor(),
				ui.class3ColorButton->getOpenCvColor(),
				ui.lineLengthSpinBox->value(),
				ui.lineThicknessSpinBox->value(),
				ui.iterationsSpinBox->value(),
				ui.colorTreshold1SpinBox->value(),
				ui.colorTreshold2SpinBox->value(),
				ui.coverageSpinBox->value() / 100.0f
			};
			AlgorithmWorker::Parameters params = {
				outputDir,
				cellWalls,
				mainAlgo,
				ui.autoRotateCheckBox->isChecked(),
				ui.removeCellEdgesCheckBox->isChecked(),
				ui.removedCellEdgesPreviewCheckBox->isChecked(),
				ui.allClassesCheckBox->isChecked(),
				ui.allClassesWithSrcCheckbox->isChecked(),
				ui.class1CheckBox->isChecked(),
				ui.class1withSrcCheckbox->isChecked(),
				ui.class2CheckBox->isChecked(),
				ui.class2withSrcCheckbox->isChecked(),
				ui.class3CheckBox->isChecked(),
				ui.class3withSrcCheckbox->isChecked(),
				ui.class1NameEdit->text(),
				ui.class2NameEdit->text(),
				ui.class3NameEdit->text()
			};
			algo.start(selectedImages, params);
			ui.runButton->setText("Stop");
		}
	}
	else {
		QMessageBox::warning(this, "No data", "Select input images and output directory");
	}
}

void BioLines2::onAlgoFinished() {
	ui.runButton->setText("Run");
	if (closeOnFinish)
		close();
}

void BioLines2::readSettings() {
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "BioLines2");

	settings.beginGroup("Window");
	if (settings.contains("pos"))
		move(settings.value("pos").toPoint());
	settings.endGroup();

	settings.beginGroup("Algorithm");
	selectedImages = settings.value("in_imgs", QStringList()).toStringList();
	ui.selectedImagesLabel->setText(QString("%1 images selected").arg(selectedImages.size()));
	outputDir = settings.value("out_dir", "").toString();
	ui.outputDirSameAsInputCheckBox->setChecked(settings.value("output_dir_same_as_input", false).toBool());
	ui.outputDirLabel->setText(outputDir);
	ui.class1ColorButton->setColor(settings.value("color1", QColor(Qt::green)).value<QColor>());
	ui.class2ColorButton->setColor(settings.value("color2", QColor(Qt::yellow)).value<QColor>());
	ui.class3ColorButton->setColor(settings.value("color3", QColor(Qt::red)).value<QColor>());
	ui.lineLengthSpinBox->setValue(settings.value("line_length", 20).toInt());
	ui.lineThicknessSpinBox->setValue(settings.value("line_thickness", 1).toInt());
	ui.iterationsSpinBox->setValue(settings.value("iterations", 2000000).toInt());
	ui.colorTreshold1SpinBox->setValue(settings.value("treshold1", 30).toInt());
	ui.colorTreshold2SpinBox->setValue(settings.value("treshold2", 60).toInt());
	ui.coverageSpinBox->setValue(settings.value("coverage", 70).toInt());
	ui.cellWallsLineLengthSpinBox->setValue(settings.value("cell_walls_line_length", 100).toInt());
	ui.cellWallsLineThicknessSpinBox->setValue(settings.value("cell_walls_line_thickness", 5).toInt());
	ui.cellWallsCoverageSpinBox->setValue(settings.value("cell_walls_coverage", 70).toInt());
	ui.autoRotateCheckBox->setChecked(settings.value("auto_rotate", false).toBool());
	ui.removeCellEdgesCheckBox->setChecked(settings.value("remove_cell_edges", false).toBool());
	ui.removedCellEdgesPreviewCheckBox->setChecked(settings.value("removed_cell_edges_preview", false).toBool());
	ui.allClassesCheckBox->setChecked(settings.value("output_combined", true).toBool());
	ui.allClassesWithSrcCheckbox->setChecked(settings.value("output_src_combined", true).toBool());
	ui.class1CheckBox->setChecked(settings.value("output_class1", false).toBool());
	ui.class1withSrcCheckbox->setChecked(settings.value("output_src_class1", false).toBool());
	ui.class2CheckBox->setChecked(settings.value("output_class2", false).toBool());
	ui.class2withSrcCheckbox->setChecked(settings.value("output_src_class2", false).toBool());
	ui.class3CheckBox->setChecked(settings.value("output_class3", false).toBool());
	ui.class3withSrcCheckbox->setChecked(settings.value("output_src_class3", false).toBool());
	ui.class1NameEdit->setText(settings.value("class1_name", "Transverse").toString());
	ui.class2NameEdit->setText(settings.value("class2_name", "Oblique").toString());
	ui.class3NameEdit->setText(settings.value("class3_name", "Longitudinal").toString());
	settings.endGroup();
}

void BioLines2::writeSettings() {
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "BioLines2");

	settings.beginGroup("Window");
	settings.setValue("pos", pos());
	settings.endGroup();

	settings.beginGroup("Algorithm");
	settings.setValue("in_imgs", selectedImages);
	settings.setValue("out_dir", outputDir);
	settings.setValue("output_dir_same_as_input", ui.outputDirSameAsInputCheckBox->isChecked());
	settings.setValue("color1", ui.class1ColorButton->getColor());
	settings.setValue("color2", ui.class2ColorButton->getColor());
	settings.setValue("color3", ui.class3ColorButton->getColor());
	settings.setValue("line_length", ui.lineLengthSpinBox->value());
	settings.setValue("line_thickness", ui.lineThicknessSpinBox->value());
	settings.setValue("iterations", ui.iterationsSpinBox->value());
	settings.setValue("treshold1", ui.colorTreshold1SpinBox->value());
	settings.setValue("treshold2", ui.colorTreshold2SpinBox->value());
	settings.setValue("coverage", ui.coverageSpinBox->value());
	settings.setValue("cell_walls_line_length", ui.cellWallsLineLengthSpinBox->value());
	settings.setValue("cell_walls_line_thickness", ui.cellWallsLineThicknessSpinBox->value());
	settings.setValue("cell_walls_coverage", ui.cellWallsCoverageSpinBox->value());
	settings.setValue("auto_rotate", ui.autoRotateCheckBox->isChecked());
	settings.setValue("remove_cell_edges", ui.removeCellEdgesCheckBox->isChecked());
	settings.setValue("removed_cell_edges_preview", ui.removedCellEdgesPreviewCheckBox->isChecked());
	settings.setValue("output_combined", ui.allClassesCheckBox->isChecked());
	settings.setValue("output_src_combined", ui.allClassesWithSrcCheckbox->isChecked());
	settings.setValue("output_class1", ui.class1CheckBox->isChecked());
	settings.setValue("output_src_class1", ui.class1withSrcCheckbox->isChecked());
	settings.setValue("output_class2", ui.class2CheckBox->isChecked());
	settings.setValue("output_src_class2", ui.class2withSrcCheckbox->isChecked());
	settings.setValue("output_class3", ui.class3CheckBox->isChecked());
	settings.setValue("output_src_class3", ui.class3withSrcCheckbox->isChecked());
	settings.setValue("class1_name", ui.class1NameEdit->text());
	settings.setValue("class2_name", ui.class2NameEdit->text());
	settings.setValue("class3_name", ui.class3NameEdit->text());
	settings.endGroup();
}

void BioLines2::closeEvent(QCloseEvent *event) {
	// we need to stop the algorithm and wait for it's thread to finish
	algo.stop();
	QTime dieTime = QTime::currentTime().addMSecs(250);
	// save changed fields values
	if(saveSettingsOnQuit)
		writeSettings();
	// wait 250 ms
	while (QTime::currentTime() < dieTime)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	event->accept();
}
