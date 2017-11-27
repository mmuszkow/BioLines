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
#include "colorbutton.h"

void ColorButton::paintEvent(QPaintEvent *event) {
	QPushButton::paintEvent(event);
	int colorPadding = 5;
	QRect rect = event->rect();
	QPainter painter(this);
	painter.setBrush(QBrush(currentColor));
	painter.setPen("#CECECE");
	rect.adjust(colorPadding, colorPadding, -1 - colorPadding, -1 - colorPadding);
	painter.drawRect(rect);
}

void ColorButton::setColor(const QColor& color) {
	currentColor = color;
	colorChanged(currentColor);
}

void ColorButton::chooseColor() {
	QColor color = QColorDialog::getColor(currentColor, this);
	if (color.isValid())
		setColor(color);
}
