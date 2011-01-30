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

#include "backend.h"

#include "audiooutput.h"
#include "mediaobject.h"
#include "videowidget.h"

#include <QtCore/QSet>
#include <QtCore/QVariant>
#include <QtCore/QStringList>
#include <QtCore/QtPlugin>

#include <mfapi.h>
#pragma comment(lib, "mfplat.lib")


QT_BEGIN_NAMESPACE

Q_EXPORT_PLUGIN2(phonon_mf, Phonon::MF::Backend);

namespace Phonon
{
	namespace MF
	{
		Backend::Backend(QObject* parent, const QVariantList&) : QObject(parent)
		{
			MFStartup(MF_VERSION, 0);
		}

		Backend::~Backend()
		{
			MFShutdown();
		}

		QObject* Backend::createObject(BackendInterface::Class c, QObject* parent, const QList<QVariant>& args)
		{
			Q_UNUSED(args);
			switch (c)
			{
			case MediaObjectClass:
				return new MediaObject(parent);
			case AudioOutputClass:
				return new AudioOutput(this, parent);
			case VideoWidgetClass:
				return new VideoWidget(qobject_cast<QWidget*>(parent));
			default:
				return 0;
			}
		}

		QStringList Backend::availableMimeTypes() const
		{
			// TODO
			QStringList ret;
			return ret;
		}


		QList<int> Backend::objectDescriptionIndexes(Phonon::ObjectDescriptionType type) const
		{
			// TODO
			QList<int> r;
			if (type == Phonon::AudioOutputDeviceType)
				r.append(0);
			return r;
		}

		QHash<QByteArray, QVariant> Backend::objectDescriptionProperties(Phonon::ObjectDescriptionType type, int index) const
		{
			// TODO
			Q_UNUSED(index);
			QHash<QByteArray, QVariant> r;
			if (type == Phonon::AudioOutputDeviceType) 
				r["name"] = QLatin1String("default audio device");
			return r;
		}


		bool Backend::connectNodes(QObject* node1, QObject* node2)
		{
			// TODO
			MediaObject* mediaObject = qobject_cast<MediaObject*>(node1);
			VideoWidget* videoWidget = qobject_cast<VideoWidget*>(node2);
			AudioOutput* audioOutput = qobject_cast<AudioOutput*>(node2);

			if (mediaObject && videoWidget)
			{
				mediaObject->addVideoWidget(videoWidget);
			}

			if (mediaObject && audioOutput)
			{
				mediaObject->addAudioOutput(audioOutput);
			}

			// TODO
			//MediaObject *mediaObject = qobject_cast<MediaObject*> (node1);
			//AudioOutput *audioOutput = qobject_cast<AudioOutput*> (node2);

			//if (mediaObject && audioOutput)
				//mediaObject->setAudioOutput(audioOutput);
			return true;
		}

		bool Backend::disconnectNodes(QObject* node1, QObject* node2)
		{
			// TODO
			MediaObject* mediaObject = qobject_cast<MediaObject*>(node1);
			VideoWidget* videoWidget = qobject_cast<VideoWidget*>(node2);
			AudioOutput* audioOutput = qobject_cast<AudioOutput*>(node2);

			if (mediaObject && videoWidget)
			{
				mediaObject->removeVideoWidget(videoWidget);
			}

			if (mediaObject && audioOutput)
			{
				mediaObject->removeAudioOutput(audioOutput);
			}

			return true;
		}

		bool Backend::startConnectionChange(QSet<QObject*>)
		{
			return true;
		}

		bool Backend::endConnectionChange(QSet<QObject*>)
		{
			return true;
		}
	}
}

QT_END_NAMESPACE