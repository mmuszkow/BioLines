#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <Windows.h>
#include <time.h>
#include <fstream>

using namespace cv;
using namespace std;

#define DEG2RAD(x) ((x) * M_PI / 180.0f)
static const Vec3b WHITE(255, 255, 255);
static const Vec3b BLACK(0, 0, 0);
static const Vec3b GREEN(0, 255, 0);
static const Vec3b RED(0, 0, 255);

class LinableImg : public Mat {
	struct Line {
		int white; // while pixels covered by line
		int total; // total pixels covered by line
		bool conflict; // another line already in that place
		Line() : white(0), total(0), conflict(false) {}
	};

	void _handle(Line& res, int y, int x, const Vec3b& color, bool draw) {
		if (x < 0 || y < 0 || x >= this->cols || y >= this->rows) return;
		res.total++;
		if (this->at<Vec3b>(y, x) == WHITE) res.white++;
		Vec3b& current = this->at<cv::Vec3b>(y, x);
		/*if (current != WHITE && current != BLACK) {
		res.conflict = true;
		return;
		}*/
		if (draw) this->at<cv::Vec3b>(y, x) = color;
	}

	//Line _line(int x0, int y0, int x1, int y1, const Vec3b& color, bool draw) {
	//	Line res;
	//	int dx = abs(x1 - x0), sx = x0<x1 ? 1 : -1;
	//	int dy = abs(y1 - y0), sy = y0<y1 ? 1 : -1;
	//	int err = dx - dy, e2, x2;                       /* error value e_xy */
	//	int ed = dx + dy == 0 ? 1 : sqrt((float)dx*dx + (float)dy*dy);
	//	for (;;){                                         /* pixel loop */
	//		_handle(res, y0, x0, color, draw);
	//		e2 = err; x2 = x0;
	//		if (2 * e2 >= -dx) {                                    /* x step */
	//			if (x0 == x1) break;
	//			if (e2 + dy < ed) _handle(res, y0 + sy, x0, color, draw);
	//			err -= dy; x0 += sx;
	//		}
	//		if (2 * e2 <= dy) {                                     /* y step */
	//			if (y0 == y1) break;
	//			if (dx - e2 < ed) _handle(res, y0, x2 + sx, color, draw);
	//			err += dx; y0 += sy;
	//		}
	//	}
	//	return res;
	//}

	Line _line(int x0, int y0, int x1, int y1, const Vec3b& color, int w, bool draw) {
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

public:
	LinableImg(const Mat& img) : Mat(img) {
		cvtColor(img, *this, COLOR_GRAY2BGR);
	}

	LinableImg(int width, int height) : Mat(height, width, CV_8UC3, Scalar(0, 0, 0)) {}

	inline void line(int x0, int y0, int x1, int y1, int w, const Vec3b& color) {
		_line(x0, y0, x1, y1, color, w, true);
	}

	inline float coverage(int x0, int y0, int x1, int y1, int w) {
		Line res = _line(x0, y0, x1, y1, Vec3b(0, 255, 0), w, false);
		if (res.conflict) return -1.0f;
		return res.white / static_cast<float>(res.total);
	}

	int count(const Vec3b& color) const {
		int count = 0;
		for (int y = 0; y < this->rows; y++)
			for (int x = 0; x < this->cols; x++)
				if (this->at<Vec3b>(y, x) == color)
					count++;
		return count;
	}
};

class IniFile {
	std::string _fileName;
public:
	IniFile(const char* fileName) {
        char curr_dir[MAX_PATH+1];
        GetCurrentDirectoryA(MAX_PATH, &curr_dir[0]);
        _fileName = curr_dir;
        _fileName += "\\";
        _fileName += fileName;
    }

	int getInt(const char* section, const char* name, int defaultVal) {
		return GetPrivateProfileIntA(section, name, defaultVal, _fileName.c_str());
	}

	float getFloat(const char* section, const char* name, const char* defaultVal) {
		char buff[256];
		GetPrivateProfileStringA(section, name, defaultVal, buff, 255, _fileName.c_str());
		return atof(buff);
	}
};

class AlgoPic {
	int line_length, line_thickness, iterations, angle1, angle2, angle3, angle4;
	float coverage;

	LinableImg bin;    
public:
	LinableImg red, green, redGreen;
    int angle_count[360];

	AlgoPic(const Mat& binary) : bin(binary), red(binary.cols, binary.rows), 
        green(binary.cols, binary.rows), redGreen(binary.cols, binary.rows) {
            IniFile cfg("params.ini");
            iterations = cfg.getInt("algorithm", "iterations", 10000000);
		    coverage = cfg.getFloat("algorithm", "coverage", "0.7");
		    line_length = cfg.getInt("algorithm", "line_length", 30);
		    line_thickness = cfg.getInt("algorithm", "line_thickness", 1);
            angle1 = cfg.getInt("coloring", "angle1", 315);
            angle2 = cfg.getInt("coloring", "angle2", 45);
            angle3 = cfg.getInt("coloring", "angle3", 135);
            angle4 = cfg.getInt("coloring", "angle4", 225);
            memset(&angle_count[0], 0, 360 * sizeof(int));            
    }

	void run() {
		for (int i = 0; i < iterations; i++) {
			int x1 = rand() % bin.cols;
			int y1 = rand() % bin.rows;
			int x2, y2, angle;
			do {
				angle = rand() % 360;
				x2 = x1 + line_length * cos(DEG2RAD(angle));
				y2 = y1 + line_length * sin(DEG2RAD(angle));
			} while ((x1 == x2 && y1 == y2)
				|| x1 == x2
				|| x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0
				|| x1 >= bin.cols || y1 >= bin.rows || x2 >= bin.cols || y2 >= bin.rows);

			if (bin.coverage(x1, y1, x2, y2, line_thickness) > coverage) {
				Vec3b color = (angle > angle1 || angle < angle2) || (angle > angle3 && angle < angle4) ? GREEN : RED;
				redGreen.line(x1, y1, x2, y2, line_thickness, color);
				if (color == RED)
					red.line(x1, y1, x2, y2, line_thickness, color);
				if (color == GREEN)
					green.line(x1, y1, x2, y2, line_thickness, color);
                angle_count[angle]++;
			}
		}
	}
};

int main(int argc, char** argv) {		
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " <input file>" << std::endl;
		return 1;
	}
	
	Mat src, gray, bin, binLines;
	src = imread(argv[1], CV_LOAD_IMAGE_COLOR);
	cvtColor(src, gray, COLOR_BGR2GRAY);
	adaptiveThreshold(gray, bin, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 19, -2);
	srand(time(NULL));
	AlgoPic algo(bin);
	algo.run();

	char out[256];
	//sprintf_s(out, 255, "%s_bin.png", argv[1]);
	//imwrite(out, bin);
	sprintf_s(out, 255, "%s_src.png", argv[1]);
	imwrite(out, src);
	sprintf_s(out, 255, "%s_rg.png", argv[1]);	
	imwrite(out, algo.redGreen);
	sprintf_s(out, 255, "%s_r.png", argv[1]);
	imwrite(out, algo.red);
	sprintf_s(out, 255, "%s_g.png", argv[1]);
	imwrite(out, algo.green);

    std::ofstream f("BioLines_out.txt", fstream::app | fstream::out);
	f << argv[1] << "\t" << algo.red.count(RED) << "\t" << algo.green.count(GREEN);
    for(int i=0; i<360; i++)
        f << "\t" << algo.angle_count[i];
    f << std::endl;
    f.close();

	return 0;
}
