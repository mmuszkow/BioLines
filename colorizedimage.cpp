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
#include "colorizedimage.h"

ColorizedImage::ColorizedImage(const cv::Mat& grayscale, const cv::Mat& colorMask) : cv::Mat(grayscale.rows, grayscale.cols, CV_8UC3, cv::Scalar(0, 0, 0)) {
	if (grayscale.size != colorMask.size) throw;
	if (grayscale.type() != CV_8UC1) throw;
	if (colorMask.type() != CV_8UC3) throw;

	double minIntensity, maxIntensity;
	cv::minMaxIdx(grayscale, &minIntensity, &maxIntensity);
	if (maxIntensity == 0) return;

	for (int y = 0; y < rows; y++)
		for (int x = 0; x < cols; x++) {
			double intensity = grayscale.at<uchar>(y, x);
			const cv::Vec3b& color = colorMask.at<cv::Vec3b>(y, x);
			if (color != BLACK) {
				float multiplier = intensity / maxIntensity;
				this->at<cv::Vec3b>(y, x) = cv::Vec3b(multiplier * color[0], multiplier * color[1], multiplier * color[2]);
			}
			else
				this->at<cv::Vec3b>(y, x) = cv::Vec3b(intensity, intensity, intensity);
		}

}
