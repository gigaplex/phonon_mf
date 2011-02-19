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

#include "mfcallback.h"

#include <memory>
#include <mfapi.h>

#include "compointer.h"
#include "mfsession.h"

#pragma comment(lib, "mfuuid.lib")

QT_BEGIN_NAMESPACE

namespace Phonon
{
	namespace MF
	{
		MFCallback::MFCallback() : m_refCount(1)
		{
		}

		MFCallback::~MFCallback()
		{
		}

		HRESULT MFCallback::CreateInstance(MFSession* session, IMFAsyncCallback** p)
		{
			if (!session || !p)
				return E_POINTER;

			// Starts with a refcount of 1
			MFCallback* newCallback = new(std::nothrow) MFCallback();

			if (!newCallback)
				return E_OUTOFMEMORY;

			HRESULT hr = newCallback->Initialise(session);

			if (FAILED(hr))
			{
				newCallback->Release();
				return hr;
			}

			(*p) = newCallback;

			return S_OK;
		}

		HRESULT MFCallback::Initialise(MFSession* parent)
		{
			qRegisterMetaType<ComPointer<IMFMediaSource>>("ComPointer<IMFMediaSource>");

			// TODO choose correct threading model
			connect(this, SIGNAL(sourceReady(ComPointer<IMFMediaSource>)), parent, SLOT(sourceReady(ComPointer<IMFMediaSource>)), Qt::QueuedConnection);
			connect(this, SIGNAL(topologyLoaded()), parent, SLOT(topologyLoaded()), Qt::QueuedConnection);
			connect(this, SIGNAL(canSeek(bool)), parent, SIGNAL(canSeek(bool)), Qt::QueuedConnection);
			connect(this, SIGNAL(started()), parent, SLOT(onStarted()), Qt::QueuedConnection);
			connect(this, SIGNAL(paused()), parent, SIGNAL(paused()), Qt::QueuedConnection);
			connect(this, SIGNAL(stopped()), parent, SIGNAL(stopped()), Qt::QueuedConnection);
			connect(this, SIGNAL(ended()), parent, SLOT(onEnded()), Qt::QueuedConnection);

			// Must be a direct connection to set the event the thread is waiting on
			connect(this, SIGNAL(sessionClosed()), parent, SLOT(sessionClosed()), Qt::DirectConnection);

			return S_OK;
		}

		STDMETHODIMP_(ULONG) MFCallback::AddRef()
		{
			return InterlockedIncrement(&m_refCount);
		}

		STDMETHODIMP_(ULONG) MFCallback::Release()
		{
			ULONG temp = InterlockedDecrement(&m_refCount);
			
			if (temp == 0)
			{
				delete this;
			}

			return temp;
		}

		STDMETHODIMP MFCallback::QueryInterface(const IID& id, void** p)
		{
			if (!p)
			{
				return E_POINTER;
			}

			if (id == IID_IUnknown)
			{
				*p = static_cast<IUnknown*>(this);
			}
			else if (id == IID_IMFAsyncCallback)
			{
				*p = static_cast<IMFAsyncCallback*>(this);
			}
			else
			{
				*p = NULL;
				return E_NOINTERFACE;
			}

			AddRef();
			return S_OK;
		}

		STDMETHODIMP MFCallback::GetParameters(DWORD*, DWORD*)
		{
			return E_NOTIMPL;
		}

		STDMETHODIMP MFCallback::Invoke(IMFAsyncResult* result)
		{
			ComPointer<IUnknown> stateUnk;
			result->GetState(stateUnk.p());

			ComPointer<IMFMediaSession> session(stateUnk);

			if (session)
			{
				ComPointer<IMFMediaEvent> event;
				HRESULT hr = session->EndGetEvent(result, event.p());
				Q_ASSERT(SUCCEEDED(hr));

				if (event)
				{
					MediaEventType eventType = MEUnknown;
					event->GetType(&eventType);

					if (eventType == MESessionClosed)
					{
						qDebug("MESessionClosed event");
						emit sessionClosed();
					}
					else
					{
						switch (eventType)
						{
						case MESessionTopologySet:
							//emit topologyLoaded();
							qDebug("MESessionTopologySet event");
							break;
						case MESessionNotifyPresentationTime:
							// TODO debug info
							qDebug("MESessionNotifyPresentationTime event");
							break;
						case MESessionTopologyStatus:
							{
								UINT32 status = 0;
								event->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&status);
								switch (status)
								{
								case MF_TOPOSTATUS_READY:
									qDebug("MESessionTopologyStatus event with MF_TOPOSTATUS_READY status");
									emit topologyLoaded();
									break;
								case MF_TOPOSTATUS_STARTED_SOURCE:
									qDebug("MESessionTopologyStatus event with MF_TOPOSTATUS_STARTED_SOURCE status");
									break;
#if defined(_WIN32_WINNT_WIN7) && (WINVER >= _WIN32_WINNT_WIN7)
								case MF_TOPOSTATUS_DYNAMIC_CHANGED:
									qDebug("MESessionTopologyStatus event with MF_TOPOSTATUS_DYNAMIC_CHANGED status");
									break;
#endif // defined(_WIN32_WINNT_WIN7) && (WINVER >= _WIN32_WINNT_WIN7)
								case MF_TOPOSTATUS_SINK_SWITCHED:
									qDebug("MESessionTopologyStatus event with MF_TOPOSTATUS_SINK_SWITCHED status");
									break;
								case MF_TOPOSTATUS_ENDED:
									qDebug("MESessionTopologyStatus event with MF_TOPOSTATUS_ENDED status");
									break;
								default:
									qDebug("MESessionTopologyStatus event with invalid/unknown status");
									break;
								}
							}
							break;
						case MESessionCapabilitiesChanged:
							{
								UINT32 capabilities = 0;
								UINT32 delta = 0;
								event->GetUINT32(MF_EVENT_SESSIONCAPS_DELTA, &delta);
								event->GetUINT32(MF_EVENT_SESSIONCAPS, &capabilities);
								qDebug("MESessionCapabilitiesChanged event: MF_EVENT_SESSIONCAPS = %d, MF_EVENT_SESSIONCAPS_DELTA = %d", capabilities, delta);
								if (MFSESSIONCAP_SEEK & delta)
								{
									emit canSeek(MFSESSIONCAP_SEEK & capabilities);
								}
								emit capabilitiesChanged();
							}
							break;
						case MESessionStarted:
							// TODO info on attributes
							qDebug("MESessionStarted event");
							emit started();
							break;
						case MESessionPaused:
							qDebug("MESessionPaused event");
							emit paused();
							break;
						case MESessionStopped:
							qDebug("MESessionStopped event");
							emit stopped();
							break;
						case MESessionEnded:
							qDebug("MESessionEnded event");
							emit ended();
							break;
						case MEEndOfPresentation:
							qDebug("MEEndOfPresentation event");
							break;
						default:
							qDebug("Unknown MF event: %d", eventType);
							break;
						}

						session->BeginGetEvent(this, session);
					}
				}

				return hr;
			}

			ComPointer<IMFSourceResolver> sourceResolver(stateUnk);

			if (sourceResolver)
			{
				MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
				ComPointer<IUnknown> sourceUnk;
				sourceResolver->EndCreateObjectFromURL(result, &objectType, sourceUnk.p());

				ComPointer<IMFMediaSource> source(sourceUnk);

				qDebug("MFCallback: Resolved source");
				emit sourceReady(source);

				return S_OK;
			}

			return E_NOTIMPL;
		}
	}
}

QT_END_NAMESPACE