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

#include <vector>

// Text report on how many % each class occupies, thread-safe
class Report {

	struct Result {
		QString fileName;
		float percent[3];

		// for sorting
		inline bool operator<(const Result& other) const {
			return fileName < other.fileName;
		}
	};

	QString _filePath;
	QString _className[3];
	QMutex _mutex;
public:
	std::vector<Result> results;

	Report() {}
	Report(
		const QString& dirPath,
		const QString& class1, const QString& class2, const QString& class3) {
		reinit(dirPath, class1, class2, class3);
	}

	// constructor
	void reinit(
		const QString& dirPath,
		const QString& class1, const QString& class2, const QString& class3);

	// adds a single result to the list, thread-safe
	void addResult(const QString& fileName, int color1count, int color2count, int color3count);

	// saves .txt file with sorted results to disk
	void saveToDisk();
};
