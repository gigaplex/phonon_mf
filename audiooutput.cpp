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

#include "audiooutput.h"
#include "mediaobject.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
	namespace MF
	{
		AudioOutput::AudioOutput(QObject* parent) : QObject(parent), m_volume(1.0)
		{
		}

		AudioOutput::~AudioOutput()
		{
		}

		int AudioOutput::outputDevice() const
		{
			// TODO
			return 0;
		}

		void AudioOutput::setVolume(qreal newVolume)
		{
			if (newVolume != m_volume)
			{
				m_volume = newVolume;

				if (m_audioControl)
				{
					m_audioControl->SetMasterVolume(m_volume);
				}

				emit volumeChanged(m_volume);
			}
		}

		bool AudioOutput::setOutputDevice(const AudioOutputDevice & newDevice)
		{
			return setOutputDevice(newDevice.index());
		}

		qreal AudioOutput::volume() const
		{
			return m_volume;
		}

		bool AudioOutput::setOutputDevice(int newDevice)
		{
			// TODO
			return (newDevice == 0);
		}

		void AudioOutput::reset()
		{
			m_topoNode.Release();
			m_audioControl.Release();
		}

		void AudioOutput::attach(IMFTopologyNode* node)
		{
			m_topoNode = node;
		}

		HRESULT AudioOutput::topologyLoaded()
		{
			// TODO
			HRESULT hr = E_NOINTERFACE;

			if (m_topoNode)
			{
				ComPointer<IUnknown> object;
				m_topoNode->GetObject(object.p());
				hr = MFGetService(object, MR_POLICY_VOLUME_SERVICE, __uuidof(IMFSimpleAudioVolume), (void**)m_audioControl.p());
				Q_ASSERT(SUCCEEDED(hr));
				if (m_audioControl)
				{
					hr = m_audioControl->SetMasterVolume(m_volume);
				}
			}

			return hr;
		}
	}
}

QT_END_NAMESPACE