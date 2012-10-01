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

#include "videowidget.h"

#include <qevent.h>
#include <qpainter.h>

#pragma comment(lib, "strmiids.lib")

QT_BEGIN_NAMESPACE

namespace Phonon
{
	namespace MF
	{
		VideoWidget::VideoWidget(QWidget* parent) : QWidget(parent), m_videoActive(false)
		{
			QPalette p(palette());
			p.setColor(backgroundRole(), Qt::black);
			setPalette(p);
		}

		QSize VideoWidget::sizeHint() const
		{
			if (m_videoControl)
			{
				SIZE videoSize = {0, 0};
				m_videoControl->GetNativeVideoSize(&videoSize, NULL);
				return QSize(videoSize.cx, videoSize.cy).expandedTo(QWidget::sizeHint());
			}
			return QWidget::sizeHint();
		}

		void VideoWidget::resizeEvent(QResizeEvent* event)
		{
			if (event && m_videoControl)
			{
				RECT destRect = {0, 0, event->size().width(), event->size().height()};
				HRESULT hr = m_videoControl->SetVideoPosition(0, &destRect);
				Q_ASSERT(SUCCEEDED(hr));
			}
		}

		void VideoWidget::paintEvent(QPaintEvent* /*event*/)
		{
			if (m_videoControl && m_videoActive)
			{
				m_videoControl->RepaintVideo();
			}
			else
			{
				QPainter painter(this);
				painter.eraseRect(rect());
			}
		}

		void VideoWidget::reset()
		{
			m_videoActive = false;
			m_topoNode.Release();
			m_videoControl.Release();
		}

		void VideoWidget::attach(IMFTopologyNode* node)
		{
			m_topoNode = node;
		}

		HRESULT VideoWidget::topologyLoaded()
		{
			HRESULT hr = E_NOINTERFACE;

			if (m_topoNode)
			{
				ComPointer<IUnknown> object;
				m_topoNode->GetObject(object.p());
				hr = MFGetService(object, MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), (void**)m_videoControl.p());
				Q_ASSERT(SUCCEEDED(hr));

				QResizeEvent re(geometry().size(), geometry().size());
				resizeEvent(&re);
			}

			return hr;
		}

		void VideoWidget::stateChanged(Phonon::State newState, Phonon::State /*oldState*/)
		{
			switch (newState)
			{
			case Phonon::PlayingState:
			case Phonon::PausedState:
				m_videoActive = true;
				break;

			default:
				m_videoActive = false;
				break;
			}
		}
	}
}

QT_END_NAMESPACE