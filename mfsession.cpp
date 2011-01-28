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

#include "mfsession.h"
#include "videowidget.h"

#include <QWidget>

#include <mfapi.h>
#include <mferror.h>
#pragma comment(lib, "mf.lib")

QT_BEGIN_NAMESPACE

namespace Phonon
{
	namespace MF
	{
		MFSession::MFSession() : 
			m_state(Phonon::LoadingState),
			m_nextState(Phonon::StoppedState),
			m_topoDirty(true)
		{
			m_closedEvent = ::CreateEvent(0, FALSE, FALSE, 0);
			MFCallback::CreateInstance(this, m_callback.p());
		}

		MFSession::~MFSession()
		{
			CloseSession();
			::CloseHandle(m_closedEvent);
		}

		HRESULT MFSession::LoadURL(const wchar_t* url)
		{
			CreateSession();
			BeginCreateSource(url);
			return E_NOTIMPL;
		}

		HRESULT MFSession::Play()
		{
			switch (m_state)
			{
			case PlayingState:
			case BufferingState: // Need to revise buffering, perhaps treat like paused
				return S_FALSE;

			case LoadingState:
				m_nextState = PlayingState;
				return S_FALSE;

			case StoppedState:
				if (m_topoDirty)
				{
					EnsureSinksConnected();
				}
				// Fall through

			case PausedState:
				{
					Phonon::State oldState = m_state;
					m_state = PlayingState;
					if (oldState != m_state)
					{
						emit stateChanged(m_state, oldState);
					}
					PROPVARIANT startParam;
					PropVariantInit(&startParam);
					return m_session->Start(0, &startParam);
					PropVariantClear(&startParam);
				}

			case ErrorState:
			default:
				return E_FAIL;
			}
		}

		HRESULT MFSession::Pause()
		{
			if (m_state == PlayingState)
			{
				HRESULT hr = m_session->Pause();
				m_state = PausedState;
				emit stateChanged(m_state, PlayingState);
				return hr;
			}
			else
			{
				return E_NOTIMPL;
			}
		}

		HRESULT MFSession::Stop()
		{
			HRESULT hr = m_session->Stop();
			Phonon::State oldState = m_state;
			m_state = StoppedState;
			if (oldState != m_state)
			{
				emit stateChanged(m_state, oldState);
			}
			return hr;
		}

		HRESULT MFSession::CreateSession()
		{
			CloseSession();

			MFCreateMediaSession(0, m_session.p());

			m_session->BeginGetEvent(m_callback, m_session);

			return E_NOTIMPL;
		}

		HRESULT MFSession::CloseSession()
		{
			m_audioSources.clear();
			m_videoSources.clear();

			if (m_session)
			{
				m_session->Close();

				DWORD wait = ::WaitForSingleObject(m_closedEvent, 1000);

				if (wait != WAIT_OBJECT_0)
				{
					qWarning() << "Wait on CloseSession failed:" << wait;
				}
			}

			if (m_source)
			{
				m_source->Shutdown();
			}

			if (m_session)
			{
				m_session->Shutdown();
			}

			return E_NOTIMPL;
		}

		HRESULT MFSession::BeginCreateSource(const wchar_t* url)
		{
			setState(LoadingState);
			m_nextState = StoppedState;
			m_topoDirty = true;

			m_audioSources.clear();
			m_videoSources.clear();

			ComPointer<IMFSourceResolver> sourceResolver;
			MFCreateSourceResolver(sourceResolver.p());

			sourceResolver->BeginCreateObjectFromURL(url, MF_RESOLUTION_MEDIASOURCE, 0, 0, m_callback, sourceResolver);
			return E_NOTIMPL;
		}

		HRESULT MFSession::EnsureSinksConnected()
		{
			LARGE_INTEGER start;
			LARGE_INTEGER end;
			LARGE_INTEGER freq;

			QueryPerformanceCounter(&start);
			ComPointer<IMFTopology> topology;
			MFCreateTopology(topology.p());

			DWORD streamCount = 0;
			m_presentation->GetStreamDescriptorCount(&streamCount);

			for (DWORD i = 0; i < streamCount; i++)
			{
				//m_presentation->SelectStream(i);
			}
			
			for (int i = 0; i < m_audioSources.count(); i++)
			{
				ComPointer<IMFTopologyNode> sourceNode = m_audioSources.at(i);
				/*const StreamNode& streamNode = m_audioSources.at(i);
				
				ComPointer<IMFTopologyNode> sourceNode;
				MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, sourceNode.p());

				sourceNode->SetUnknown(MF_TOPONODE_SOURCE, m_source);
				sourceNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, streamNode.m_stream);
				sourceNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, streamNode.m_presentation);*/

				ComPointer<IMFTopologyNode> outputNode;
				MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, outputNode.p());

				ComPointer<IMFActivate> activate;
				MFCreateAudioRendererActivate(activate.p());
				outputNode->SetObject(activate);

				topology->AddNode(sourceNode);
				topology->AddNode(outputNode);
				sourceNode->ConnectOutput(0, outputNode, 0);
			}

			for (int i = 0; i < m_videoSources.count(); i++)
			{
				HWND hWnd = 0;

				VideoWidget* videoWidget = 0;

				if (!m_videoSinks.isEmpty())
				{
					videoWidget = m_videoSinks.front();
				}

				if (videoWidget)
				{
					hWnd = videoWidget->widget()->winId();
				}

				ComPointer<IMFTopologyNode> sourceNode = m_videoSources.at(i);
				//const StreamNode& streamNode = m_videoSources.at(i);

				//ComPointer<IMFTopologyNode> sourceNode;
				//MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, sourceNode.p());

				//sourceNode->SetUnknown(MF_TOPONODE_SOURCE, m_source);
				//sourceNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, streamNode.m_stream);
				//sourceNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, streamNode.m_presentation);

				ComPointer<IMFTopologyNode> outputNode;
				MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, outputNode.p());

				ComPointer<IMFActivate> activate;
				MFCreateVideoRendererActivate(hWnd, activate.p());
				outputNode->SetObject(activate);
				topology->AddNode(sourceNode);
				topology->AddNode(outputNode);
				sourceNode->ConnectOutput(0, outputNode, 0);
			}

			HRESULT hr = m_session->SetTopology(MFSESSION_SETTOPOLOGY_IMMEDIATE, topology);

			Q_ASSERT(SUCCEEDED(hr));

			QueryPerformanceCounter(&end);
			QueryPerformanceFrequency(&freq);

			double diff = double(end.QuadPart - start.QuadPart) / freq.QuadPart;

			m_topoDirty = false;

			return E_NOTIMPL;
		}

		void MFSession::setState(Phonon::State state)
		{
			if (m_state != state)
			{
				Phonon::State oldState = m_state;
				m_state = state;
				emit stateChanged(m_state, oldState);
			}
		}

		void MFSession::sourceReady(ComPointer<IMFMediaSource> source)
		{
			m_audioSources.clear();
			m_videoSources.clear();

			m_source = source;
			// TODO kick off topology creation
			m_topoDirty = true;

			if (!m_source)
			{
				setState(ErrorState);

				return;
			}
						
			ComPointer<IMFPresentationDescriptor> presentation;
			m_source->CreatePresentationDescriptor(presentation.p());

			m_presentation = presentation;

			DWORD streamCount = 0;
			presentation->GetStreamDescriptorCount(&streamCount);

			for (DWORD i = 0; i < streamCount; i++)
			{
				BOOL isSelected = FALSE;
				ComPointer<IMFStreamDescriptor> stream;
				presentation->GetStreamDescriptorByIndex(i, &isSelected, stream.p());

				// Need to re-evaluate this, if playing a video without a videowidget then adding a
				// videowidget later, the video stream is unselected by MS internal code
				if (!isSelected)
				{
					isSelected = presentation->SelectStream(i) == S_OK;
				}

				if (isSelected)
				{
					ComPointer<IMFTopologyNode> sourceNode;
					MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, sourceNode.p());

					//StreamNode sourceNode;
					//sourceNode.m_presentation = presentation;
					//sourceNode.m_stream = stream;

					sourceNode->SetUnknown(MF_TOPONODE_SOURCE, m_source);
					sourceNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, stream);
					sourceNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, presentation);

					ComPointer<IMFMediaTypeHandler> mediaType;
					stream->GetMediaTypeHandler(mediaType.p());
					GUID typeId;
					mediaType->GetMajorType(&typeId);

					if (typeId == MFMediaType_Audio)
					{
						m_audioSources.append(sourceNode);
					}

					if (typeId == MFMediaType_Video)
					{
						m_videoSources.append(sourceNode);
					}
				}
			}

			emit hasVideo(!m_videoSources.isEmpty());

			setState(StoppedState);

			if (m_nextState == PlayingState)
			{
				Play();
				m_nextState = StoppedState;
			}
		}

		void MFSession::topologyLoaded()
		{
			foreach(VideoWidget* videoWidget, m_videoSinks)
			{
				//ComPointer<IUnknown> object;
				//g_videoNode->GetObject(object.p());

				//ComPointer<IMFGetService> service(object);

				//ComPointer<IMFVideoDisplayControl> control;
				//service->GetService(MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), (void**)control.p());

				videoWidget->topologyLoaded(m_session);
			}

			// Convert from 100-nanosecond to millisecond
			qint64 duration = GetDuration() / 10000;
			emit totalTimeChanged(duration);

			ComPointer<IMFClock> clock;
			m_session->GetClock(clock.p());

			m_clock = clock;

			/*m_state = StoppedState;

			if (m_nextState == PlayingState)
			{
				Play();
				m_nextState = StoppedState;
			}*/
		}

		void MFSession::sessionClosed()
		{
			::SetEvent(m_closedEvent);
		}

		void MFSession::setVideoWidget(VideoWidget* videoWidget)
		{
			m_topoDirty = true;
			m_videoSinks.push_back(videoWidget);
		}

		Phonon::State MFSession::state() const
		{
			return m_state;
		}

		MFTIME MFSession::GetDuration() const
		{
			MFTIME duration = 0;

			if (m_presentation)
			{
				HRESULT hr = m_presentation->GetUINT64(MF_PD_DURATION, (UINT64*)&duration);

				Q_ASSERT(SUCCEEDED(hr));
			}

			return duration;
		}

		HRESULT MFSession::Seek(MFTIME pos)
		{
			PROPVARIANT startParam;
			PropVariantInit(&startParam);

			startParam.vt = VT_I8;
			startParam.hVal.QuadPart = pos;

			return m_session->Start(0, &startParam);
			//return m_clock->Start(pos);
			PropVariantClear(&startParam);
		}

		MFTIME MFSession::GetCurrentTime() const
		{
			MFTIME currentTime = 0;

			if (m_clock)
			{
				HRESULT hr = m_clock->GetTime(&currentTime);
				//Q_ASSERT(SUCCEEDED(hr));
			}

			return currentTime;
		}
	}
}

QT_END_NAMESPACE