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

#ifndef PHONON_MF_AUDIOOUTPUT_H
#define PHONON_MF_AUDIOOUTPUT_H

#include <phonon/audiooutputinterface.h>

#include "backend.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
	namespace MF
	{
		class AudioOutput : public QObject, public Phonon::AudioOutputInterface
		{
			Q_OBJECT
			Q_INTERFACES(Phonon::AudioOutputInterface)

		public:
			AudioOutput(Backend* backend, QObject* parent);
			~AudioOutput();

			qreal volume() const;
			int outputDevice() const;
			void setVolume(qreal newVolume);
			bool setOutputDevice(int newDevice);
			bool setOutputDevice(const AudioOutputDevice& newDevice);

		Q_SIGNALS:
			void audioDeviceFailed();
			void volumeChanged(qreal);

		private:
			unsigned int m_volume;
		};
	}
}

QT_END_NAMESPACE

#endif // PHONON_MF_AUDIOOUTPUT_H
