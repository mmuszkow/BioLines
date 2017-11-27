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
#include "algorithm.h"
#include "colorizedimage.h"
#include "lsm.h"

void Algorithm::run() {
	emit progressMade(0);

	// output report
	_report.reinit(
		_params.out_dir, 
		_params.class1_name, _params.class2_name, _params.class3_name);

	// for estimating time left
	_timer.start();

	// schedule worker threads
	QStringList::ConstIterator img = _images.begin();
	QThreadPool* pool = QThreadPool::globalInstance();
	int maxThreads = std::max(1, QThread::idealThreadCount() - 1);
	pool->setMaxThreadCount(maxThreads);
	while (img != _images.end()) {
		AlgorithmWorker* worker = new AlgorithmWorker(*img, _report, _params, _shouldStop);
		connect(worker, &AlgorithmWorker::finished, this, &Algorithm::workerFinished);
		pool->start(worker);
		++img;
	}
	pool->waitForDone();

	// finish
	_report.saveToDisk();
	if (!_shouldStop) emit progressMade(100);
	emit etaUpdated("");
}

void Algorithm::workerFinished() {	
	if (_shouldStop) return;
	
	_done++;

	// if we are done
	if (_done == _images.size()) {
		emit progressMade(100);
		emit etaUpdated("");
		return;
	}

	// compute progress, first percent
	emit progressMade(_done * 100.0f / static_cast<float>(_images.size()));
	// eta
	float timePerImage = _timer.elapsed() / 1000.0f / static_cast<float>(_done);
	int eta = static_cast<int>(std::roundf((_images.size() - _done) * timePerImage));
	char eta_str[128];
	sprintf_s(eta_str, 128, "%d:%.2d:%.2d left", eta / 3600, (eta / 60) % 60, eta % 60);
	emit etaUpdated(eta_str);
}

cv::Mat AlgorithmWorker::readLSM() {
	// get file handle
	FILE* flsm;
	fopen_s(&flsm, _image.toStdString().c_str(), "rb");
	if (!flsm) return cv::Mat();

	// parse .lsm file
	struct lsm_file* lsm = lsm_open(flsm);
	if (!lsm) {
		fclose(flsm);
		return cv::Mat();
	}

	// verify if there is any IFD and any strip
	if (lsm->ifd_length == 0) return cv::Mat();
	if (lsm->ifd[0].tag_strip_offsets_length == 0) return cv::Mat();

	// read raw pixels (8-bit grayscale)
	struct tiff_ifd* ifd = &lsm->ifd[0];
	void* pixels = lsm_read_pixel_data(flsm, ifd, 0);
	if (!pixels) {
		free(lsm);
		fclose(flsm);
	}
	
	// copy pixels data into new cv::Mat
	cv::Mat gray = cv::Mat(ifd->tag_image_length, ifd->tag_image_width, CV_8UC1, pixels).clone();

	// free & return
	free(pixels);
	free(lsm);
	fclose(flsm);
	return gray;
}

cv::Mat AlgorithmWorker::autoRotate(cv::Mat& gray) {
	// blur
	cv::Mat blurred;
	cv::GaussianBlur(gray, blurred, cv::Size(0, 0), 3);
#ifdef _DEBUG
	imwrite((_params.out_dir + "/ar_1_GaussianBlur.png").toStdString(), blurred);
#endif

	// binarize
	cv::Mat bin;
	cv::adaptiveThreshold(blurred, bin, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 19, -2);
#ifdef _DEBUG
	imwrite((_params.out_dir + "/ar_2_adaptiveThreshold.png").toStdString(), bin);
#endif

	// dilate, concates pixel groups
	cv::Mat dilated;
	int dilation_size = 3;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
		cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
		cv::Point(dilation_size, dilation_size));
	cv::dilate(bin, dilated, kernel, cv::Point(-1, -1), 5);
#ifdef _DEBUG
	imwrite((_params.out_dir + "/ar_3_dilate.png").toStdString(), dilated);
#endif

	// find pixel groups contours
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(dilated, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	// fit ellipses for big groups and check their angles
	double wagedBeanFeretAngle = 0;
	double totalArea = 0;
	for (std::vector<std::vector<cv::Point> >::const_iterator it = contours.begin(); it != contours.end(); it++) {
		if (it->size() > 314) { // 314 = radius of 50 pixels circle, 2 * pi * r
			cv::RotatedRect r = cv::fitEllipse(*it);
			if (r.angle > 90)
				wagedBeanFeretAngle += (r.angle - 180) * r.size.area();
			else if (r.angle < -90)
				wagedBeanFeretAngle += (r.angle + 180) * r.size.area();
			else
				wagedBeanFeretAngle += r.angle * r.size.area();
			totalArea += r.size.area();
		}
	}
#ifdef _DEBUG
	cv::Mat contoursImg(dilated.size(), dilated.type(), BLACK);
	cv::Mat contoursEllipses(dilated.size(), CV_8UC3, BLACK);
	cv::Mat inputEllipses;
	cvtColor(gray, inputEllipses, CV_GRAY2RGB);
	for (int i = 0; i < contours.size(); i++) {
		if (contours[i].size() > 314) { // 314 = radius of 50 pixels circle, 2 * pi * r
			cv::drawContours(contoursImg, contours, i, WHITE, 3);
			cv::drawContours(contoursEllipses, contours, i, WHITE, 3);
			cv::RotatedRect r = cv::fitEllipse(contours[i]);
			cv::ellipse(contoursEllipses, r, cv::Vec3b(0, 0, 255), 3);
			cv::ellipse(inputEllipses, r, cv::Vec3b(0, 0, 255), 3);
		}
	}
	imwrite((_params.out_dir + "/ar_4_contours.png").toStdString(), contoursImg);
	imwrite((_params.out_dir + "/ar_5_ellipses.png").toStdString(), contoursEllipses);
#endif

	if (totalArea == 0)
		return gray;

	// rotate
	wagedBeanFeretAngle /= totalArea;
	cv::Mat rotMat = cv::getRotationMatrix2D(cv::Point2f(gray.cols/2.0f, gray.rows/2.0f), wagedBeanFeretAngle, 1.0);
	cv::Mat rotated;
	cv::warpAffine(gray, rotated, rotMat, gray.size());
#ifdef _DEBUG
	cv::Mat ellipsesRotated;
	cv::warpAffine(inputEllipses, ellipsesRotated, rotMat, inputEllipses.size());
	imwrite((_params.out_dir + "/ar_6_ellipses_rotated.png").toStdString(), ellipsesRotated);
#endif

	return rotated;
}

cv::Mat AlgorithmWorker::removeCellEdges(cv::Mat& gray, const QFileInfo& fi) {
	// binarize
	cv::Mat bin;
	cv::adaptiveThreshold(gray, bin, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 19, -2);

	// process using main algorithm
	LinesOutput output = detectLines(bin, _params.cellWalls);
	if (_shouldStop) return bin;

	// thicken the output lines for class1 and class3 by 1 pixel
	cv::Mat dilated1, dilated3;
	int dilation_size = 3;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
		cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
		cv::Point(dilation_size, dilation_size));
	cv::dilate(output.color1, dilated1, kernel);
	cv::dilate(output.color3, dilated3, kernel);

	// split per channel
	cv::Mat split1[3], split3[3];
	cv::split(dilated1, split1);
	cv::split(dilated3, split3);

	// merge red and blue channels (which are now "white") and substract it from binarized image
	cv::Mat merged, masked;
	cv::add(split1[0], split3[2], merged);
	cv::subtract(bin, merged, masked);
	
	// preview of what was removed
	if (_params.removedCellEdgesPreview) {
		QString fn = QString("%1/%2_no_edges.%3").arg(_params.out_dir).arg(fi.completeBaseName()).arg(outExt(fi));
		imwrite(fn.toStdString(), masked);
	}

	return masked;
}

AlgorithmWorker::LinesOutput AlgorithmWorker::detectLines(const cv::Mat& bin, const LinesParameters& params) {

	LinesOutput output(bin);
	float whiteCount = output.bin.count(WHITE);
#ifdef _DEBUG
	int dbgImgDumpIter[9];
	for (int dd = 1; dd <= 9; dd++)
		dbgImgDumpIter[dd-1] = dd * (params.iterations / 10);
	imwrite(QString("%1/dl_%2_input.png")
		.arg(_params.out_dir)
		.arg(params.line_length)
		.toStdString(), bin);
#endif

	for (int i = 0; i < params.iterations; i++) {
		if (_shouldStop) return output;
		int x1 = rand() % bin.cols;
		int y1 = rand() % bin.rows;
		int x2, y2, angle;
		do {
			angle = rand() % 360;
			x2 = x1 + params.line_length * cos(deg2rad(angle));
			y2 = y1 + params.line_length * sin(deg2rad(angle));
		} while ((x1 == x2 && y1 == y2)
			|| x1 == x2
			|| x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0
			|| x1 >= bin.cols || y1 >= bin.rows || x2 >= bin.cols || y2 >= bin.rows);

		if (output.bin.coverage(x1, y1, x2, y2, params.line_thickness) > params.min_coverage) {

			cv::Vec3b color;
			if ((angle < params.angle1)
				|| (angle >= 360 - params.angle1)
				|| (angle >= 180 - params.angle1 && angle < 180 + params.angle1)) {
				// class 1
				color = params.color1;
				output.color1.line(x1, y1, x2, y2, params.line_thickness, params.color1);
			}
			else if ((angle < params.angle2)
				|| (angle >= 360 - params.angle2)
				|| (angle >= 180 - params.angle2 && angle < 180 + params.angle2)) {
				// class 2
				color = params.color2;
				output.color2.line(x1, y1, x2, y2, params.line_thickness, params.color2);
			}
			else {
				color = params.color3;
				output.color3.line(x1, y1, x2, y2, params.line_thickness, params.color3);
			}
			output.all.line(x1, y1, x2, y2, params.line_thickness, color);
		}

#ifdef _DEBUG		
		for(int dd = 0; dd < 9; dd++)
			if (i == dbgImgDumpIter[dd]) {
				imwrite(QString("%1/dl_%2_%3.png")
					.arg(_params.out_dir)
					.arg(params.line_length)
					.arg(i)
					.toStdString(), output.all);
				break;
			}
#endif

		// cutoff
		if (i % 100000 == 0) {
			float nonWhiteCount = output.all.count(params.color1) + output.all.count(params.color2) + output.all.count(params.color3);
			if (nonWhiteCount / (whiteCount + 1) >= 0.95) break;
		}
	}

	return output;
}

void AlgorithmWorker::run() {
	if (_shouldStop) return;

	// read file info
	QFileInfo fi(_image);
	if (!fi.isReadable()) return;

	// load image in grayscale
	cv::Mat gray;
	if (fi.suffix().toLower() == "lsm") { // handle Zeiss files as well
		gray = readLSM();
		if (gray.rows == 0) return;
	} else
		gray = cv::imread(_image.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
#ifdef _DEBUG
	imwrite((_params.out_dir + "/1_grayscale.png").toStdString(), gray);
#endif

	// rotate
	if(_params.autoRotate)
		gray = autoRotate(gray);

	// remove cell edges
	cv::Mat bin;
	if (_params.removeCellEdges) {
		bin = removeCellEdges(gray, fi);
		if (_shouldStop) return;
	} else
		cv::adaptiveThreshold(gray, bin, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 19, -2);
#ifdef _DEBUG
	imwrite((_params.out_dir + "/2_bin.png").toStdString(), bin);
#endif

	// main algorithm
	LinesOutput output = detectLines(bin, _params.mainAlgo);
	if (_shouldStop) return;

	// write output images
	if (_params.output_combined_img) {
		QString fn = QString("%1/%2_combined.%3").arg(_params.out_dir).arg(fi.completeBaseName()).arg(outExt(fi));
		imwrite(fn.toStdString(), output.all);
	}
	if (_params.output_combined_img_with_src) {
		ColorizedImage colorized(gray, output.all);
		QString fn = QString("%1/%2_src_combined.%3").arg(_params.out_dir).arg(fi.completeBaseName()).arg(outExt(fi));
		imwrite(fn.toStdString(), colorized);
	}
	if (_params.output_class1_img) {
		QString fn = QString("%1/%2_%3.%4").arg(_params.out_dir).arg(fi.completeBaseName()).arg(_params.class1_name).arg(outExt(fi));
		imwrite(fn.toStdString(), output.color1);
	}
	if (_params.output_class1_img_with_src) {
		ColorizedImage colorized(gray, output.color1);
		QString fn = QString("%1/%2_src_%3.%4").arg(_params.out_dir).arg(fi.completeBaseName()).arg(_params.class1_name).arg(outExt(fi));
		imwrite(fn.toStdString(), colorized);
	}
	if (_params.output_class2_img) {
		QString fn = QString("%1/%2_%3.%4").arg(_params.out_dir).arg(fi.completeBaseName()).arg(_params.class2_name).arg(outExt(fi));
		imwrite(fn.toStdString(), output.color2);
	}
	if (_params.output_class2_img_with_src) {
		ColorizedImage colorized(gray, output.color2);
		QString fn = QString("%1/%2_src_%3.%4").arg(_params.out_dir).arg(fi.completeBaseName()).arg(_params.class2_name).arg(outExt(fi));
		imwrite(fn.toStdString(), colorized);
	}
	if (_params.output_class3_img) {
		QString fn = QString("%1/%2_%3.%4").arg(_params.out_dir).arg(fi.completeBaseName()).arg(_params.class3_name).arg(outExt(fi));
		imwrite(fn.toStdString(), output.color3);
	}
	if (_params.output_class3_img_with_src) {
		ColorizedImage colorized(gray, output.color3);
		QString fn = QString("%1/%2_src_%3.%4").arg(_params.out_dir).arg(fi.completeBaseName()).arg(_params.class3_name).arg(outExt(fi));
		imwrite(fn.toStdString(), colorized);
	}

	// add line to the report file
	int color1count = output.color1.count(_params.mainAlgo.color1);
	int color2count = output.color2.count(_params.mainAlgo.color2);
	int color3count = output.color3.count(_params.mainAlgo.color3);
	_report.addResult(fi.fileName(), color1count, color2count, color3count);

	emit finished();
}
