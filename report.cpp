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

#include "report.h"
#include <algorithm>

void Report::reinit(
	const QString& dirPath,
	const QString& class1, const QString& class2, const QString& class3) {
	results.clear();
	_filePath = QString("%1/BioLines2.txt").arg(dirPath);
	_className[0] = class1;
	_className[1] = class2;
	_className[2] = class3;
}

// thread-safe
void Report::addResult(const QString& fileName, int color1count, int color2count, int color3count) {
	float totalCount = color1count + color2count + color3count;
	_mutex.lock();
	if (totalCount > 0) {
		Result res = {
			fileName,
			((color1count*100.0f) / totalCount),
			((color2count*100.0f) / totalCount),
			((color3count*100.0f) / totalCount)
		};
		results.push_back(res);
	}
	else {
		Result res = { fileName, 0.0f, 0.0f, 0.0f };
		results.push_back(res);
	}
	_mutex.unlock();
}

void Report::saveToDisk() {
	if (_filePath.isEmpty()) return;

	// open file
	QFile report(_filePath);
	report.open(QIODevice::WriteOnly);
	QTextStream reportStream(&report);

	// file header
	reportStream << "Image\t" <<
		_className[0] << "\t" <<
		_className[1] << "\t" <<
		_className[2] << endl;

	// sort results by filename
	std::sort(results.begin(), results.end());

	// write results to file
	std::vector<Result>::const_iterator it = results.begin(), end = results.end();
	QLocale& locale = QLocale::system();
	while (it != end) {
		reportStream << 
			it->fileName << "\t" << 
			locale.toString(it->percent[0]) << "\t" <<
			locale.toString(it->percent[1]) << "\t" <<
			locale.toString(it->percent[2]) << endl;
		++it;
	}
	reportStream.flush();
}
