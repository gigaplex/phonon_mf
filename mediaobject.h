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

#ifndef PHONON_MF_MEDIAOBJECT_H
#define PHONON_MF_MEDIAOBJECT_H

#include <phonon/mediaobjectinterface.h>

#include <QTimer>

//#include <windows.h>
#include "mfsession.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
	namespace MF
	{
		class AudioOutput;
		class VideoWidget;

		class MediaObject : public QObject, public Phonon::MediaObjectInterface
		{
			Q_OBJECT
			Q_INTERFACES(Phonon::MediaObjectInterface)

		public:
			MediaObject(QObject* parent);
			virtual ~MediaObject();
			Phonon::State state() const;
			bool hasVideo() const;
			bool isSeekable() const;
			qint64 currentTime() const;
			qint32 tickInterval() const;

			void setTickInterval(qint32 newTickInterval);
			void play();
			void pause();
			void stop();
			void seek(qint64 time);

			QString errorString() const;
			Phonon::ErrorType errorType() const;
			qint64 totalTime() const;
			qint32 prefinishMark() const;
			void setPrefinishMark(qint32 newPrefinishMark);
			qint32 transitionTime() const;
			void setTransitionTime(qint32);
			qint64 remainingTime() const;
			MediaSource source() const;
			void setSource(const MediaSource& source);
			void setNextSource(const MediaSource& source);

			void addVideoWidget(VideoWidget* videoWidget);
			void removeVideoWidget(VideoWidget* videoWidget);

			void addAudioOutput(AudioOutput* audioOutput);
			void removeAudioOutput(AudioOutput* audioOutput);

		Q_SIGNALS:
			void stateChanged(Phonon::State, Phonon::State);
			void tick(qint64 time);
			void metaDataChanged(QMultiMap<QString, QString>);
			void seekableChanged(bool);
			void hasVideoChanged(bool);
			void bufferStatus(int);
			void finished();
			void prefinishMarkReached(qint32);
			void aboutToFinish();
			void totalTimeChanged(qint64) const;
			void currentSourceChanged(const MediaSource&);

		private Q_SLOTS:
			void onTick();
			void setHasVideo(bool hasVideo);
			void setSeekable(bool seekable);
			void setTotalTime(qint64 totalTime);
			void started();
			void paused();
			void stopped();

		private:
			QTimer m_ticker;
			QString m_errorString;
			ErrorType m_errorType;

			MediaSource m_source;
			MediaSource m_nextSource;
			AudioOutput *m_audioOutput;

			MFSession m_session;

//			qint64 m_mediaSize;
			qint64 m_totalTime;
			qint64 m_currentTime;
//			qint64 m_transitionTime;
//			qint64 m_prefinishMark;
//			qint64 m_tickIntervalResolution;
			qint32 m_tickInterval;
//			qint32 m_tick;
//			Phonon::State m_state;
			bool m_hasVideo;
			bool m_seekable;
			bool m_seeking;
			qint64 m_queuedSeek;

//			bool m_bufferingFinished;
//			bool m_paused;
//			bool m_stopped;            
//			bool m_hasNextSource; 
//			bool m_hasSource;
//			bool m_sourceIsValid;
//			bool m_bufferPrepared;

			friend class Backend;
		};
	}
}

QT_END_NAMESPACE

#endif // PHONON_MF_MEDIAOBJECT_H
