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

#ifndef PHONON_MF_MFCALLBACK_H
#define PHONON_MF_MFCALLBACK_H

#include <QtCore/QObject>
#include <QtCore/QMetaType>

#include <mfidl.h>

#include "compointer.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
	namespace MF
	{
		class MFSession;

		class MFCallback : public QObject, public IMFAsyncCallback
		{
			Q_OBJECT

		public:
			static HRESULT CreateInstance(MFSession* session, IMFAsyncCallback** p);

			// IUnknown
			STDMETHOD_(ULONG, AddRef)();
			STDMETHOD_(ULONG, Release)();
			STDMETHOD(QueryInterface)(const IID& id, void** p);

			// IMFAsyncCallback
			STDMETHOD(GetParameters)(DWORD*, DWORD*);
			STDMETHOD(Invoke)(IMFAsyncResult* result);

		Q_SIGNALS:
			void sourceReady(ComPointer<IMFMediaSource>);
			void sessionClosed();
			void topologyLoaded();
			void capabilitiesChanged();
			void canSeek(bool);
			void started();
			void paused();
			void ended();

		private:
			MFCallback();
			virtual ~MFCallback();

			HRESULT Initialise(MFSession* parent);

			LONG m_refCount;
		};
	}
}

// For queued thread signaling
Q_DECLARE_METATYPE(Phonon::MF::ComPointer<IMFMediaSource>)

QT_END_NAMESPACE

#endif // PHONON_MF_MFCALLBACK_H