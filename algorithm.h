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

#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <QThread>
#include <QStack>
#include "linableimg.h"
#include "report.h"

class AlgorithmWorker : public QObject, public QRunnable
{
	Q_OBJECT

public:
	struct LinesParameters {
		// colors
		cv::Vec3b color1, color2, color3;
		// drawn line length
		int line_length;
		// how thick is the drawn line
		int line_thickness;
		// how many times we try to draw lines
		int iterations;
		// angles ranges for colors
		int angle1, angle2;
		// min white pixels under the line to keep it
		float min_coverage;
	};
	struct Parameters {
		// output directory
		QString out_dir;
		// parameters for cell walls filtering
		LinesParameters cellWalls;
		// parameters for main algorithm
		LinesParameters mainAlgo;
		// preprocessing - autorotation
		bool autoRotate;
		// preprocessing - removing of cell edges
		bool removeCellEdges;
		// preprocessing - removing of cell edges, output what was removed into image file
		bool removedCellEdgesPreview;
		// if we should output all classes together on single img
		bool output_combined_img, output_combined_img_with_src;
		// which classes images to output
		bool output_class1_img, output_class1_img_with_src, output_class2_img, output_class2_img_with_src, output_class3_img, output_class3_img_with_src;
		// classes names
		QString class1_name, class2_name, class3_name;
	};

private:
	bool& _shouldStop;
	const QString& _image;
	const AlgorithmWorker::Parameters& _params;
	Report& _report;

	// main algorithm output
	struct LinesOutput {
		LinableImg 	bin, color1, color2, color3, all;

		LinesOutput(const cv::Mat& bin) :
			bin(bin),
			color1(bin.size), color2(bin.size), color3(bin.size), all(bin.size) { }
	};

public:
	AlgorithmWorker(const QString& image, Report& report, const AlgorithmWorker::Parameters& params, bool& stopFlag) : 
		_shouldStop(stopFlag), _image(image), _report(report), _params(params) {}
	~AlgorithmWorker() {}

signals:
	void finished();

protected:
	void run() override;

private:
	// degrees to radians
	static inline float deg2rad(float degrees) {
		return (degrees * M_PI) / 180.0f;
	}

	// lsm files we output as tif, other in same format as input image
	static inline QString outExt(const QFileInfo& fi) {
		if (fi.suffix().toLower() == "lsm")
			return "tif";
		return fi.suffix();
	}

	// reads image in Zeiss confocal microscope format (.lsm)
	cv::Mat readLSM();
	// auto-rotate the image to vertical position before processing
	cv::Mat autoRotate(cv::Mat& gray);
	// remove cell walls before processing, returns binary image
	cv::Mat removeCellEdges(cv::Mat& gray, const QFileInfo& fi);
	// main algorithm, used also in removeCellWalls
	AlgorithmWorker::LinesOutput detectLines(const cv::Mat& bin, const LinesParameters& params);
};

class Algorithm : public QThread
{
	Q_OBJECT

private:
	bool _shouldStop;
	QStringList _images;
	AlgorithmWorker::Parameters _params;

	// for estimating time left
	QElapsedTimer _timer;
	int _done;
	Report _report;

public:
	Algorithm(QObject *parent = 0) : QThread(parent), _shouldStop(false) {}
	~Algorithm() {}

	inline void start(const QStringList& images, const AlgorithmWorker::Parameters& params) {
		if (isRunning()) return;
		_images = images;
		_params = params;
		_shouldStop = false;
		_done = 0;
		QThread::start();
	}

	inline void stop() {
		_shouldStop = true;
	}

protected slots:
	void workerFinished();

signals:
	void progressMade(int progress);
	void etaUpdated(const QString& eta);

protected:
	void run() override;

};

#endif // ALGORITHM_H
