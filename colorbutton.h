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

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>
#include <QColor>

class ColorButton : public QPushButton
{
	Q_OBJECT
	Q_PROPERTY(QColor color MEMBER currentColor NOTIFY colorChanged DESIGNABLE true)
private:
	QColor currentColor;

public:
	ColorButton(QWidget *parent = 0) : QPushButton(parent) {
		connect(this, &QPushButton::clicked, this, &ColorButton::chooseColor);
	}
	~ColorButton() {}

	inline const QColor& getColor() const {
		return currentColor;
	}

	inline cv::Vec3b getOpenCvColor() const {
		const QRgba64 rgba = currentColor.rgba64();
		return cv::Vec3b(rgba.blue8(), rgba.green8(), rgba.red8());
	}

public slots:
	void setColor(const QColor& color);
	void chooseColor();

signals:
	void colorChanged(QColor);

protected:
	void paintEvent(QPaintEvent *event) override;

};

#endif // COLORBUTTON_H
