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

#ifndef PREWIEVWIDGET_H
#define PREWIEVWIDGET_H

#include <QWidget>

class PreviewWidget : public QWidget
{
	Q_OBJECT

private:
	QPen pen[3];
	QBrush brush[3];
	int angle1, angle2;

public:
	PreviewWidget(QWidget *parent = 0);
	~PreviewWidget() {}

public slots:
	void setColor1(const QColor& color);
	void setColor2(const QColor& color);
	void setColor3(const QColor& color);
	void setAngle1(int angle);
	void setAngle2(int angle);

protected:
	void paintEvent(QPaintEvent *event) override;

};

#endif // PREWIEVWIDGET_H
