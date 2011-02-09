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
#include "audiooutput.h"
#include "videowidget.h"
#include "mediaobject.h"

#include <QWidget>

#include <mfapi.h>
#include <mferror.h>
#pragma comment(lib, "mf.lib")

QT_BEGIN_NAMESPACE

namespace Phonon
{
	namespace MF
	{
		char* getStateString(Phonon::State state)
		{
			switch (state)
			{
			case LoadingState:
				return "LoadingState";
			case StoppedState:
				return "StoppedState";
			case PlayingState:
				return "PlayingState";
			case BufferingState:
				return "BufferingState";
			case PausedState:
				return "PausedState";
			case ErrorState:
				return "ErrorState";
			default:
				qFatal("Unknown state");
				return "Unknown state";
			}
		}

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
			case Phonon::PlayingState:
			case Phonon::BufferingState: // Need to revise buffering, perhaps treat like paused
				return S_FALSE;

			case Phonon::LoadingState:
				m_nextState = Phonon::PlayingState;
				return S_FALSE;

			case Phonon::StoppedState:
				if (m_topoDirty)
				{
					EnsureSinksConnected();
				}

				{
					// Play from beginning
					HRESULT hr = Seek(0);
					setState(Phonon::PlayingState);
					return hr;
				}
				
			case Phonon::PausedState:
				{
					PROPVARIANT startParam;
					PropVariantInit(&startParam);
					HRESULT hr = m_session->Start(0, &startParam);
					PropVariantClear(&startParam);

					setState(Phonon::PlayingState);

					return hr;
				}

			case Phonon::ErrorState:
			default:
				return E_FAIL;
			}
		}

		HRESULT MFSession::Pause()
		{
			if (m_state == Phonon::PlayingState)
			{
				if (m_session)
				{
					HRESULT hr = m_session->Pause();
					setState(Phonon::PausedState);
					return hr;
				}
				else
				{
					setState(Phonon::ErrorState);
					return E_FAIL;
				}
			}
	
			return E_NOTIMPL;
		}

		HRESULT MFSession::Stop()
		{
			HRESULT hr = E_FAIL;
			if (m_session)
			{
				hr = m_session->Stop();
			}

			setState(Phonon::StoppedState);

			return hr;
		}

		HRESULT MFSession::CreateSession()
		{
			CloseSession();

			HRESULT hr = MFCreateMediaSession(0, m_session.p());

			m_session->BeginGetEvent(m_callback, m_session);

			return hr;
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
				m_source.Release();
			}

			if (m_session)
			{
				m_session->Shutdown();
				m_session.Release();
			}

			return E_NOTIMPL;
		}

		HRESULT MFSession::BeginCreateSource(const wchar_t* url)
		{
			setState(Phonon::LoadingState);
			m_nextState = Phonon::StoppedState;
			m_topoDirty = true;

			m_audioSources.clear();
			m_videoSources.clear();

			ComPointer<IMFSourceResolver> sourceResolver;
			HRESULT hr = MFCreateSourceResolver(sourceResolver.p());

			sourceResolver->BeginCreateObjectFromURL(url, MF_RESOLUTION_MEDIASOURCE, 0, 0, m_callback, sourceResolver);
			return hr;
		}

		HRESULT MFSession::EnsureSinksConnected()
		{
			LARGE_INTEGER start;
			LARGE_INTEGER end;
			LARGE_INTEGER freq;

			QueryPerformanceCounter(&start);
			ComPointer<IMFTopology> topology;
			MFCreateTopology(topology.p());

			foreach(AudioOutput* audioOutput, m_audioSinks)
			{
				audioOutput->reset();
			}

			foreach(VideoWidget* videoWidget, m_videoSinks)
			{
				videoWidget->reset();
			}

			for (int i = 0; i < m_audioSources.count(); i++)
			{
				if (i >= m_audioSinks.count())
				{
					break;
				}

				ComPointer<IMFTopologyNode> sourceNode = m_audioSources.at(i);

				AudioOutput* audioOutput = m_audioSinks.at(i);

				ComPointer<IMFTopologyNode> outputNode;
				MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, outputNode.p());
				audioOutput->attach(outputNode);

				ComPointer<IMFActivate> activate;
				MFCreateAudioRendererActivate(activate.p());
				outputNode->SetObject(activate);

				topology->AddNode(sourceNode);
				topology->AddNode(outputNode);
				sourceNode->ConnectOutput(0, outputNode, 0);
			}

			for (int i = 0; i < m_videoSources.count(); i++)
			{
				if (i >= m_videoSinks.count())
				{
					break;
				}

				ComPointer<IMFTopologyNode> teeNode;
				MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, teeNode.p());
				topology->AddNode(teeNode);

				ComPointer<IMFTopologyNode> sourceNode = m_videoSources.at(i);
				topology->AddNode(sourceNode);
				sourceNode->ConnectOutput(0, teeNode, 0);

				for (int j = 0; j < m_videoSinks.count(); j++)
				{
					VideoWidget* videoWidget = m_videoSinks.at(j);
					HWND hWnd = videoWidget->widget()->winId();

					ComPointer<IMFTopologyNode> outputNode;
					MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, outputNode.p());
					videoWidget->attach(outputNode);

					ComPointer<IMFActivate> activate;
					MFCreateVideoRendererActivate(hWnd, activate.p());
					outputNode->SetObject(activate);
					topology->AddNode(outputNode);
					teeNode->ConnectOutput(j, outputNode, 0);
				}
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
				qDebug("Phonon state change from %s to %s", getStateString(oldState), getStateString(m_state));
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
				setState(Phonon::ErrorState);

				return;
			}
						
			m_source->CreatePresentationDescriptor(m_presentation.p());

			DWORD streamCount = 0;
			m_presentation->GetStreamDescriptorCount(&streamCount);

			for (DWORD i = 0; i < streamCount; i++)
			{
				BOOL isSelected = FALSE;
				ComPointer<IMFStreamDescriptor> stream;
				m_presentation->GetStreamDescriptorByIndex(i, &isSelected, stream.p());

				// Need to re-evaluate this, if playing a video without a videowidget then adding a
				// videowidget later, the video stream is unselected by MS internal code
				if (!isSelected)
				{
					isSelected = m_presentation->SelectStream(i) == S_OK;
				}

				if (isSelected)
				{
					ComPointer<IMFTopologyNode> sourceNode;
					MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, sourceNode.p());

					sourceNode->SetUnknown(MF_TOPONODE_SOURCE, m_source);
					sourceNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, stream);
					sourceNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, m_presentation);

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

			// Convert from 100-nanosecond to millisecond
			qint64 duration = GetDuration() / 10000;
			emit totalTimeChanged(duration);

			emit hasVideo(!m_videoSources.isEmpty());

			QMultiMap<QString, QString> metadataMap;

			ComPointer<IMFMetadataProvider> metadataProvider;
			if (SUCCEEDED(MFGetService(m_source, MF_METADATA_PROVIDER_SERVICE, __uuidof(IMFMetadataProvider), (void**)metadataProvider.p())))
			{
				ComPointer<IMFMetadata> metadata;
				metadataProvider->GetMFMetadata(m_presentation, 0, 0, metadata.p());

				PROPVARIANT meta;
				PropVariantInit(&meta);
				if (SUCCEEDED(metadata->GetAllPropertyNames(&meta)))
				{
					for (ULONG i = 0; i < meta.calpwstr.cElems; i++)
					{
						PROPVARIANT propVal;
						PropVariantInit(&propVal);
						if (SUCCEEDED(metadata->GetProperty(meta.calpwstr.pElems[i], &propVal)))
						{
							switch (propVal.vt)
							{
							case VT_LPWSTR:
								{
									QString key = QString::fromUtf16(meta.calpwstr.pElems[i]);
									QString value = QString::fromUtf16(propVal.pwszVal);

									if (key == "WM/AlbumArtist")
									{
										metadataMap.insert("ARTIST", value);
									}
									else if (key == "WM/AlbumTitle")
									{
										metadataMap.insert("ALBUM", value);
									}
									else if (key == "Title")
									{
										metadataMap.insert("TITLE", value);
									}
									else if (key == "WM/Year")
									{
										metadataMap.insert("DATE", value);
									}
									else if (key == "WM/Genre")
									{
										metadataMap.insert("GENRE", value);
									}
									else if (key == "WM/TrackNumber")
									{
										metadataMap.insert("TRACKNUMBER", value);
									}

									key += ": ";
									key += value;
									qDebug(key.toAscii());
									break;
								}
							default:
								qDebug(QString::fromUtf16(meta.calpwstr.pElems[i]).toAscii());
							}
						}
						PropVariantClear(&propVal);
					}
				}
				PropVariantClear(&meta);
			}

			emit metaDataChanged(metadataMap);

			if (m_nextState == Phonon::PlayingState)
			{
				EnsureSinksConnected();

				PROPVARIANT startParam;
				PropVariantInit(&startParam);
				m_session->Start(0, &startParam);
				PropVariantClear(&startParam);

				setState(Phonon::PlayingState);

				m_nextState = Phonon::StoppedState;
			}
			else
			{
				setState(Phonon::StoppedState);
			}
		}

		void MFSession::topologyLoaded()
		{
			foreach(VideoWidget* videoWidget, m_videoSinks)
			{
				videoWidget->topologyLoaded();
			}

			foreach(AudioOutput* audioOutput, m_audioSinks)
			{
				audioOutput->topologyLoaded();
			}

			ComPointer<IMFClock> clock;
			m_session->GetClock(clock.p());

			m_clock = clock;

			DWORD capabilities = 0;
			HRESULT hr = m_session->GetSessionCapabilities(&capabilities);
			Q_ASSERT(SUCCEEDED(hr));
			emit canSeek(capabilities & MFSESSIONCAP_SEEK);
		}

		void MFSession::onStarted()
		{
			if (m_state == Phonon::PausedState)
			{
				// Happens during seeking while paused
				m_session->Pause();
			}

			emit started();
		}

		void MFSession::onEnded()
		{
			emit ended();
		}

		void MFSession::sessionClosed()
		{
			::SetEvent(m_closedEvent);
		}

		void MFSession::addVideoWidget(VideoWidget* videoWidget)
		{
			m_topoDirty = true;
			m_videoSinks.push_back(videoWidget);

			connect(this, SIGNAL(stateChanged(Phonon::State, Phonon::State)), videoWidget, SLOT(stateChanged(Phonon::State, Phonon::State)));
		}

		void MFSession::removeVideoWidget(VideoWidget* videoWidget)
		{
			// TODO
			m_topoDirty = true;
			m_videoSinks.removeOne(videoWidget);
			videoWidget->reset();

			disconnect(this, SIGNAL(stateChanged(Phonon::State, Phonon::State)), videoWidget, SLOT(stateChanged(Phonon::State, Phonon::State)));
		}

		void MFSession::addAudioOutput(AudioOutput* audioOutput)
		{
			// TODO
			m_topoDirty = true;
			m_audioSinks.push_back(audioOutput);
		}

		void MFSession::removeAudioOutput(AudioOutput* audioOutput)
		{
			// TODO
			m_topoDirty = true;
			m_audioSinks.removeOne(audioOutput);
			audioOutput->reset();
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