/*  This file is part of the KDE project.

Copyright (C) 2009 Tim Boundy

This library is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 or 3 of the License.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PHONON_MF_VIDEOWIDGET_H
#define PHONON_MF_VIDEOWIDGET_H

#include <phonon/videowidgetinterface.h>

#include <evr.h>

#include "compointer.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
	namespace MF
	{
		class VideoWidget : public QWidget, public Phonon::VideoWidgetInterface
		{
			Q_OBJECT
			Q_INTERFACES(Phonon::VideoWidgetInterface)

		public:
			VideoWidget(QWidget* parent) : QWidget(parent)
			{
			}

			Phonon::VideoWidget::AspectRatio aspectRatio() const{return Phonon::VideoWidget::AspectRatioAuto;}
			void setAspectRatio(Phonon::VideoWidget::AspectRatio){}
			qreal brightness() const{return 0;}
			void setBrightness(qreal){}
			Phonon::VideoWidget::ScaleMode scaleMode() const{return Phonon::VideoWidget::FitInView;}
			void setScaleMode(Phonon::VideoWidget::ScaleMode){}
			qreal contrast() const{return 0;}
			void setContrast(qreal){}
			qreal hue() const{return 0;}
			void setHue(qreal){}
			qreal saturation() const{return 0;}
			void setSaturation(qreal){}
			QWidget *widget(){return this;}

			QSize sizeHint() const
			{
				return QSize(800, 600);
			}

			void resizeEvent(QResizeEvent* event);

			void topologyLoaded(IMFMediaSession* mediaSession);

		private:
			ComPointer<IMFVideoDisplayControl> m_videoControl;
		};
	}
}

QT_END_NAMESPACE

#endif // PHONON_MF_VIDEOWIDGET_H