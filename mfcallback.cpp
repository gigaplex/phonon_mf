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
				session->EndGetEvent(result, event.p());

				MediaEventType eventType = MEUnknown;
				event->GetType(&eventType);
				
				if (eventType == MESessionClosed)
				{
					emit sessionClosed();
				}
				else
				{
					switch (eventType)
					{
					case MESessionTopologySet:
						emit topologyLoaded();
						break;
					case MESessionCapabilitiesChanged:
						{
							UINT32 delta = 0;
							event->GetUINT32(MF_EVENT_SESSIONCAPS_DELTA, &delta);
							emit capabilitiesChanged();
						}
						break;
					case MESessionEnded:
						__asm{nop};
						break;
					case MEEndOfPresentation:
						__asm{nop};
						break;
					}

					session->BeginGetEvent(this, session);
				}

				return E_NOTIMPL;
			}

			ComPointer<IMFSourceResolver> sourceResolver(stateUnk);

			if (sourceResolver)
			{
				MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
				ComPointer<IUnknown> sourceUnk;
				sourceResolver->EndCreateObjectFromURL(result, &objectType, sourceUnk.p());

				ComPointer<IMFMediaSource> source(sourceUnk);

				emit sourceReady(source);

				return S_OK;
			}

			return E_NOTIMPL;
		}
	}
}

QT_END_NAMESPACE