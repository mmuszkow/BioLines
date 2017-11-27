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


#pragma once

#include "stdafx.h"

class LinableImg : public cv::Mat {
	struct Line {
		int white; // while pixels covered by line
		int total; // total pixels covered by line
		bool conflict; // another line already in that place
		Line() : white(0), total(0), conflict(false) {}
	};

	void _handle(Line& res, int y, int x, const cv::Vec3b& color, bool draw);
	Line _line(int x0, int y0, int x1, int y1, const cv::Vec3b& color, int w, bool draw);

public:
	LinableImg(const Mat& img) : Mat(img) {
		cvtColor(img, *this, cv::COLOR_GRAY2BGR);
	}

	LinableImg(const cv::MatSize& size) : LinableImg(size[1], size[0]) {}

	LinableImg(int width, int height) : Mat(height, width, CV_8UC3, cv::Scalar(0, 0, 0)) {}

	inline void line(int x0, int y0, int x1, int y1, int w, const cv::Vec3b& color) {
		_line(x0, y0, x1, y1, color, w, true);
	}

	inline float coverage(int x0, int y0, int x1, int y1, int w) {
		Line res = _line(x0, y0, x1, y1, cv::Vec3b(0, 255, 0), w, false);
		if (res.conflict) return -1.0f;
		return res.white / static_cast<float>(res.total);
	}

	int count(const cv::Vec3b& color) const;
};
