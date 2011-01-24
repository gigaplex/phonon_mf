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

#pragma comment(lib, "strmiids.lib")

QT_BEGIN_NAMESPACE

namespace Phonon
{
	namespace MF
	{
		void VideoWidget::resizeEvent(QResizeEvent* event)
		{
			if (event && m_videoControl)
			{
				RECT destRect = {0, 0, event->size().width(), event->size().height()};
				HRESULT hr = m_videoControl->SetVideoPosition(0, &destRect);
				hr = hr;
			}
		}

		void VideoWidget::topologyLoaded(IMFMediaSession *mediaSession)
		{
			HRESULT hr = MFGetService(mediaSession, MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), (void**)m_videoControl.p());
			hr = hr;
		}
	}
}

QT_END_NAMESPACE