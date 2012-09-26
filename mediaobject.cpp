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

#include <QtCore/QUrl>

#include "compointer.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace MF
    {        
		MediaObject::MediaObject(QObject *parent) : m_ticker(this),
													m_hasVideo(false),
													m_seekable(false),
													m_errorType(Phonon::NoError),
													m_totalTime(0),
													m_tickInterval(0),
													m_seeking(false),
													m_hasNextSource(false),
													m_prefinishEmitted(false),
													m_finishedEmitted(false),
													m_prefinishMark(0),
													m_aboutToFinishEmitted(false),
													m_queuedSeek(-1)
        {
            setParent(parent);

			connect(&m_ticker, SIGNAL(timeout()), this, SLOT(onTick()));

			connect(&m_session, SIGNAL(hasVideo(bool)), this, SLOT(setHasVideo(bool)));
			connect(&m_session, SIGNAL(canSeek(bool)), this, SLOT(setSeekable(bool)));
			connect(&m_session, SIGNAL(totalTimeChanged(qint64)), this, SLOT(setTotalTime(qint64)));
			connect(&m_session, SIGNAL(metaDataChanged(QMultiMap<QString, QString>)), this, SIGNAL(metaDataChanged(QMultiMap<QString, QString>)));
			connect(&m_session, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SIGNAL(stateChanged(Phonon::State, Phonon::State)));
			connect(&m_session, SIGNAL(started()), this, SLOT(started()));
			connect(&m_session, SIGNAL(paused()), this, SLOT(paused()));
			connect(&m_session, SIGNAL(stopped()), this, SLOT(stopped()));
			connect(&m_session, SIGNAL(ended()), this, SLOT(ended()));
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
			// Convert 100-nanosecond to millisecond
			return m_session.GetCurrentTime() / 10000;
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
			return m_errorType;
		}

		qint32 MediaObject::prefinishMark() const
		{
			// TODO
			return m_prefinishMark;
		}

		void MediaObject::setPrefinishMark(qint32 newPrefinishMark)
		{
			// TODO
			m_prefinishMark = newPrefinishMark;
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
			return totalTime() - currentTime();
		}

		Phonon::MediaSource MediaObject::source() const
		{
			// TODO
			return m_source;
		}

		void MediaObject::setNextSource(const Phonon::MediaSource &source)
		{
			// TODO
			if (state() == Phonon::StoppedState || state() == Phonon::ErrorState)
			{
				setSource(source);
			}
			else
			{
				m_nextSource = source;
				m_hasNextSource = true;
			}
		}

		void MediaObject::setSource(const Phonon::MediaSource& source)
		{
			// TODO
			m_source = source;
			m_prefinishEmitted = false;
			m_aboutToFinishEmitted = false;
			m_finishedEmitted = false;
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
			qint64 current = currentTime();
			qint64 total = totalTime();
			qint64 remaining = total - current;

			if (m_session.state() == Phonon::PlayingState && total)
			{
				// TODO
				if (remaining < 2000 && !m_aboutToFinishEmitted)
				{
					m_aboutToFinishEmitted = true;
					emit aboutToFinish();
				}

				if (m_prefinishMark > 0 && remaining < m_prefinishMark && !m_prefinishEmitted)
				{
					m_prefinishEmitted = true;
					emit prefinishMarkReached(remaining);
				}
			}

			emit tick(current);
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
				onTick();
			}
		}

		void MediaObject::paused()
		{
			m_ticker.stop();
		}

		void MediaObject::stopped()
		{
			m_ticker.stop();
		}

		void MediaObject::ended()
		{
			// TODO
			if (m_hasNextSource)
			{
				qDebug("Switching to next source: \"%s\"", m_nextSource.url().toString().toLocal8Bit().data());
				MediaSource temp = m_nextSource;
				m_nextSource = MediaSource();
				m_hasNextSource = false;
				setSource(temp);
				m_session.Play();
			}
			else
			{
				qDebug("No next source, ending");
				m_session.Stop();
				m_ticker.stop();

				if (!m_finishedEmitted)
				{
					m_finishedEmitted = true;
					emit finished();
				}
			}
		}
	}
}

QT_END_NAMESPACE