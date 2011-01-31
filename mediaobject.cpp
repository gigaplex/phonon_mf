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

#include "mediaobject.h"
#include "audiooutput.h"
#include "videowidget.h"

//#include <QtCore/QVector>
//#include <QtCore/QTimerEvent>
//#include <QtCore/QTimer>
//#include <QtCore/QTime>
//#include <QtCore/QLibrary>
#include <QtCore/QUrl>
//#include <QtCore/QWriteLocker>

//#include <phonon/streaminterface.h>

#include "compointer.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace MF
    {        
		MediaObject::MediaObject(QObject *parent) : m_ticker(this),//m_file(0), m_stream(0),
                                                    //m_hWaveOut(0), m_nextBufferIndex(1), 
                                                    //m_mediaSize(-1), m_bufferingFinished(0),
                                                    //m_paused(0),
                                                    //m_hasNextSource(0), m_hasSource(0),
                                                    //m_sourceIsValid(0),
													m_hasVideo(false),
													m_seekable(false),
													m_errorType(Phonon::NoError),
													m_totalTime(0),
													m_tickInterval(0),
                                                    m_currentTime(0),
													m_seeking(false),
													m_queuedSeek(-1)//, m_transitionTime(0),
                                                    //m_tick(0), m_prefinishMark(0),
                                                    //m_tickIntervalResolution(0), m_bufferPrepared(0),
                                                    //m_stopped(0)
        {
            setParent(parent);

			connect(&m_ticker, SIGNAL(timeout()), this, SLOT(onTick()));

			connect(&m_session, SIGNAL(hasVideo(bool)), this, SLOT(setHasVideo(bool)));
			connect(&m_session, SIGNAL(canSeek(bool)), this, SLOT(setSeekable(bool)));
			connect(&m_session, SIGNAL(totalTimeChanged(qint64)), this, SLOT(setTotalTime(qint64)));
			connect(&m_session, SIGNAL(started()), this, SLOT(started()));
			connect(&m_session, SIGNAL(paused()), this, SLOT(paused()));
			connect(&m_session, SIGNAL(stopped()), this, SLOT(stopped()));
			connect(&m_session, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
        }

		MediaObject::~MediaObject()
		{
			stop();
		}

		Phonon::State MediaObject::state() const
		{
			return m_session.state();
		}

		bool MediaObject::hasVideo() const
		{
			return m_hasVideo;
		}

		bool MediaObject::isSeekable() const
		{
			return m_seekable;
		}

		qint64 MediaObject::totalTime() const
		{
			return m_totalTime;
		}

		qint64 MediaObject::currentTime() const
		{
			return m_currentTime;
		}

		qint32 MediaObject::tickInterval() const
		{
			return m_tickInterval;
		}

		void MediaObject::setTickInterval(qint32 newTickInterval)
		{
			if (newTickInterval != m_tickInterval)
			{
				m_tickInterval = newTickInterval;

				if (m_tickInterval == 0)
				{
					m_ticker.stop();
				}
				else
				{
					m_ticker.start(m_tickInterval);
				}
			}
		}

		void MediaObject::pause()
		{
			m_session.Pause();
			// TODO
		}

		void MediaObject::stop()
		{
			m_session.Stop();
			// TODO
		}

		void MediaObject::play()
		{
			m_session.Play();
			// TODO
		}

		QString MediaObject::errorString() const
		{
			return m_errorString;
		}

		Phonon::ErrorType MediaObject::errorType() const
		{
			return Phonon::ErrorType();
		}

		qint32 MediaObject::prefinishMark() const
		{
			// TODO
			return 0;//m_prefinishMark;
		}

		void MediaObject::setPrefinishMark(qint32 /*newPrefinishMark*/)
		{
			// TODO
			//m_prefinishMark = newPrefinishMark;
		}

		qint32 MediaObject::transitionTime() const
		{
			// TODO
			return 0;//m_transitionTime;
		}

		void MediaObject::setTransitionTime(qint32 /*time*/)
		{
			// TODO
			//m_transitionTime = time;
		}

		qint64 MediaObject::remainingTime() const
		{
			return m_totalTime - m_currentTime;
		}

		Phonon::MediaSource MediaObject::source() const
		{
			// TODO
			return Phonon::MediaSource();
		}

		void MediaObject::setNextSource(const Phonon::MediaSource &source)
		{
			m_nextSource = source;
			//m_hasNextSource = true;
		}

		void MediaObject::setSource(const Phonon::MediaSource& source)
		{
			// TODO
			m_source = source;
			m_session.LoadURL(source.url().toString().utf16());
			emit currentSourceChanged(source);
		}

		void MediaObject::seek(qint64 time)
		{
			if (m_seeking)
			{
				m_queuedSeek = time;
				return;
			}

			m_ticker.stop();

			m_queuedSeek = -1;

			m_seeking = true;
			// Convert from milliseconds to 100-nanoseconds
			m_session.Seek(time * 10000);
		}

		void MediaObject::onTick()
		{
			if (m_seeking)
				return;

			// Convert 100-nanosecond to millisecond
			m_currentTime = m_session.GetCurrentTime() / 10000;

			emit tick(m_currentTime);
		}

		void MediaObject::addVideoWidget(VideoWidget* videoWidget)
		{
			m_session.addVideoWidget(videoWidget);
		}

		void MediaObject::removeVideoWidget(VideoWidget* videoWidget)
		{
			m_session.removeVideoWidget(videoWidget);
		}

		void MediaObject::addAudioOutput(AudioOutput* audioOutput)
		{
			m_session.addAudioOutput(audioOutput);
		}

		void MediaObject::removeAudioOutput(AudioOutput* audioOutput)
		{
			m_session.removeAudioOutput(audioOutput);
		}

		void MediaObject::setHasVideo(bool hasVideo)
		{
			if (m_hasVideo != hasVideo)
			{
				m_hasVideo = hasVideo;
				emit hasVideoChanged(m_hasVideo);
			}
		}

		void MediaObject::setSeekable(bool seekable)
		{
			if (m_seekable != seekable)
			{
				m_seekable = seekable;
				emit seekableChanged(m_seekable);
			}
		}

		void MediaObject::setTotalTime(qint64 totalTime)
		{
			if (m_totalTime != totalTime)
			{
				m_totalTime = totalTime;
				emit totalTimeChanged(totalTime);
			}
		}

		void MediaObject::started()
		{
			m_seeking = false;

			if (m_queuedSeek != -1)
			{
				seek(m_queuedSeek);
			}

			if (m_session.state() == Phonon::PlayingState && m_tickInterval)
			{
				m_ticker.start(m_tickInterval);
			}
		}

		void MediaObject::paused()
		{
			m_ticker.stop();
		}

		void MediaObject::stopped()
		{
			// TODO
			m_ticker.stop();
		}
	}
}

QT_END_NAMESPACE