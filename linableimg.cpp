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
#include "linableimg.h"

void LinableImg::_handle(Line& res, int y, int x, const cv::Vec3b& color, bool draw) {
	if (x < 0 || y < 0 || x >= this->cols || y >= this->rows) return;
	res.total++;
	if (this->at<cv::Vec3b>(y, x) == WHITE) res.white++;
	cv::Vec3b& current = this->at<cv::Vec3b>(y, x);
	/*if (current != WHITE && current != BLACK) {
	res.conflict = true;
	return;
	}*/
	if (draw) this->at<cv::Vec3b>(y, x) = color;
}

LinableImg::Line LinableImg::_line(int x0, int y0, int x1, int y1, const cv::Vec3b& color, int w, bool draw) {
	Line res;
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx - dy, e2, x2, y2;                           /* error value e_xy */
	float ed = dx + dy == 0 ? 1 : sqrt((float)dx*dx + (float)dy*dy);
	float wd = static_cast<float>(w);
	for (wd = (wd + 1) / 2;;) {                                    /* pixel loop */
		_handle(res, y0, x0, color, draw);
		e2 = err; x2 = x0;
		if (2 * e2 >= -dx) {                                            /* x step */
			for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
				_handle(res, y2 += sy, x0, color, draw);
			if (x0 == x1) break;
			e2 = err; err -= dy; x0 += sx;
		}
		if (2 * e2 <= dy) {                                             /* y step */
			for (e2 = dx - e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
				_handle(res, y0, x2 += sx, color, draw);
			if (y0 == y1) break;
			err += dx; y0 += sy;
		}
	}
	return res;
}

int LinableImg::count(const cv::Vec3b& color) const {
	int count = 0;
	for (int y = 0; y < this->rows; y++)
		for (int x = 0; x < this->cols; x++)
			if (this->at<cv::Vec3b>(y, x) == color)
				count++;
	return count;
}
