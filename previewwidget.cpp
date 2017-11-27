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
#include "previewwidget.h"

PreviewWidget::PreviewWidget(QWidget *parent) : QWidget(parent) {
	brush[0].setStyle(Qt::SolidPattern);
	brush[1].setStyle(Qt::SolidPattern);
	brush[2].setStyle(Qt::SolidPattern);
	setColor1(Qt::red);
	setColor2(Qt::gray);
	setColor3(Qt::green);
	setAngle1(20);
	setAngle2(70);
}

void PreviewWidget::setColor1(const QColor& color) {
	pen[0].setBrush(color);
	brush[0].setColor(color);
	repaint();
}

void PreviewWidget::setColor2(const QColor& color) {
	pen[1].setBrush(color);
	brush[1].setColor(color);
	repaint();
}

void PreviewWidget::setColor3(const QColor& color) {
	pen[2].setBrush(color);
	brush[2].setColor(color);
	repaint();
}

void PreviewWidget::setAngle1(int angle) {
	angle1 = angle;
	repaint();
}

void PreviewWidget::setAngle2(int angle) {
	angle2 = angle;
	repaint();
}

void PreviewWidget::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	const QRect& rect = event->rect();

	// full circle, class 3
	painter.setPen(pen[2]);
	painter.setBrush(brush[2]);
	painter.drawPie(rect, 0, 360 * 16);

	// class 2
	painter.setPen(pen[1]);
	painter.setBrush(brush[1]);
	painter.drawPie(rect, 0, angle2 * 16);
	painter.drawPie(rect, 0, -angle2 * 16);
	painter.drawPie(rect, 180 * 16, angle2 * 16);
	painter.drawPie(rect, 180 * 16, -angle2 * 16);

	// class 1
	painter.setPen(pen[0]);
	painter.setBrush(brush[0]);
	painter.drawPie(rect, 0, angle1 * 16);
	painter.drawPie(rect, 0, -angle1 * 16);
	painter.drawPie(rect, 180 * 16, angle1 * 16);
	painter.drawPie(rect, 180 * 16, -angle1 * 16);

	event->accept();
}
