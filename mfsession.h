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

#ifndef PHONON_MF_MFSESSION_H
#define PHONON_MF_MFSESSION_H

#include <QtCore/QObject>
#include <phononnamespace.h>

#include <mfidl.h>

#include "compointer.h"
#include "mfcallback.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
	namespace MF
	{
		class VideoWidget;

		//class StreamNode
		//{
		//public:
		//	ComPointer<IMFPresentationDescriptor> m_presentation;
		//	ComPointer<IMFStreamDescriptor> m_stream;
		//};

		class MFSession : public QObject
		{
			Q_OBJECT

		public:
			MFSession();
			~MFSession();
			HRESULT LoadURL(const wchar_t* url);
			HRESULT Play();
			HRESULT Pause();
			HRESULT Stop();

			Phonon::State state() const;
			void setVideoWidget(VideoWidget* videoWidget);

		protected:
			HRESULT CreateSession();
			HRESULT CloseSession();
			HRESULT BeginCreateSource(const wchar_t* url);
			HRESULT EnsureSinksConnected();

			HRESULT CreateTopology();

		protected Q_SLOTS:
			void sourceReady(ComPointer<IMFMediaSource> source);
			void sessionClosed();
			void topologyLoaded();

		Q_SIGNALS:
			void stateChanged(Phonon::State, Phonon::State);
			void hasVideo(bool);

		private:
			Phonon::State m_state;
			Phonon::State m_nextState;

			ComPointer<IMFMediaSession> m_session;
			ComPointer<IMFMediaSource> m_source;
			ComPointer<IMFAsyncCallback> m_callback;
			ComPointer<IMFPresentationDescriptor> m_presentation;

			QList<ComPointer<IMFTopologyNode>> m_audioSources;
			QList<ComPointer<IMFTopologyNode>> m_videoSources;

			QList<VideoWidget*> m_videoSinks;

			HANDLE m_closedEvent;
			bool m_topoDirty;

			friend MFCallback;
		};
	}
}

QT_END_NAMESPACE

#endif // PHONON_MF_MFSESSION_H