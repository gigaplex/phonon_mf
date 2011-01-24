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
        MediaObject::MediaObject(QObject *parent) : //m_file(0), m_stream(0),
                                                    //m_hWaveOut(0), m_nextBufferIndex(1), 
                                                    //m_mediaSize(-1), m_bufferingFinished(0),
                                                    //m_paused(0), m_tickInterval(0),
                                                    //m_hasNextSource(0), m_hasSource(0),
                                                    //m_sourceIsValid(0),
													//m_state(Phonon::LoadingState),
													m_hasVideo(false),
													m_errorType(Phonon::NoError)//,
                                                    //m_currentTime(0), m_transitionTime(0),
                                                    //m_tick(0), m_volume(100), m_prefinishMark(0),
                                                    //m_tickIntervalResolution(0), m_bufferPrepared(0),
                                                    //m_stopped(0)
        {
            setParent(parent);
            //setState(Phonon::LoadingState);            

			connect(&m_session, SIGNAL(hasVideo(bool)), this, SLOT(setHasVideo(bool)));
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
			// TODO
			return false;
		}

		bool MediaObject::isSeekable() const
		{
			// TODO
			return false; 
		}

		qint64 MediaObject::totalTime() const
		{
			// TODO
			return 0;//m_totalTime;
		}

		qint64 MediaObject::currentTime() const
		{
			// TODO
			return 0;//m_currentTime;
		}

		qint32 MediaObject::tickInterval() const
		{
			// TODO
			return 0;//m_tickInterval * m_tickIntervalResolution;
		}

		void MediaObject::setTickInterval(qint32 /*newTickInterval*/)
		{
			// TODO
			/*if ((m_tickIntervalResolution == 0) || (newTickInterval == 0))
				return;
			m_tickInterval = newTickInterval / m_tickIntervalResolution;
			if ((newTickInterval > 0) && (m_tickInterval == 0))
				m_tickInterval = 1;*/
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
			// TODO
			return 0;//m_totalTime - m_currentTime;
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
			m_source = source;
			m_session.LoadURL(source.url().toString().utf16());
			emit currentSourceChanged(source);
			// TODO
			/*if (m_state == Phonon::PlayingState)
			{
				setError(Phonon::NormalError, QLatin1String("source changed while playing"));
				stop();
			}

			m_source = source;
			m_hasSource = true;
			m_sourceIsValid = false;

			emit currentSourceChanged(source);

			if (source.type() == Phonon::MediaSource::LocalFile) {
				if (!openWaveFile(source.fileName())) {
					setError(Phonon::FatalError, QLatin1String("cannot open media file"));
					return ;
				}
			} else if (source.type() == Phonon::MediaSource::Stream) {
				if (m_stream)
					delete m_stream;
				m_stream = new IOWrapper(this, source);
				m_mediaSize = m_stream->size();
			} else if (source.type() == Phonon::MediaSource::Url) {
				if (!openWaveFile(source.url().toLocalFile())) {
					setError(Phonon::FatalError, QLatin1String("cannot open media file"));
					return ;
				}
			} else {
				setError(Phonon::FatalError, QLatin1String("type of source not supported"));
				return ;
			}
			setState(Phonon::LoadingState);

			if (!readHeader())
				setError(Phonon::FatalError, QLatin1String("invalid header"));
			else if (!getWaveOutDevice())
				setError(Phonon::FatalError, QLatin1String("No waveOut device available"));
			else if (!fillBuffers())
				setError(Phonon::FatalError, QLatin1String("no data for buffering"));
			else if (!prepareBuffers())
				setError(Phonon::FatalError, QLatin1String("cannot prepare buffers"));
			else
				m_sourceIsValid = true;

			if (m_sourceIsValid)
				setState(Phonon::StoppedState);*/
		}

		void MediaObject::seek(qint64 /*time*/)
		{
			// TODO
			/*if (!m_sourceIsValid) {
				setError(Phonon::NormalError, QLatin1String("source is not valid"));
				return;
			}
			if ((time >= 0) && (time < m_totalTime)) {
				int counter = 0;
				while (!m_bufferingFinished && (counter < 200)) {
					Sleep(20);
					counter ++;
				}
				if (counter >= 200) {
					setError(Phonon::NormalError, QLatin1String("buffering timed out"));
					return;
				}

				m_stream->seek(WAVEHEADER_SIZE + time * m_waveFormatEx.nSamplesPerSec * m_waveFormatEx.wBitsPerSample * m_waveFormatEx.nChannels / 8 / 1000);
				m_currentTime = time;
				if (m_state == Phonon::PlayingState)
					play();
			} else {
				setError(Phonon::NormalError, QLatin1String("seeking out of range"));
			}*/
		}

		//void MediaObject::setState(Phonon::State newState)
		//{
		//	if (m_state == newState)
		//		return;
		//	emit stateChanged(newState, m_state);
		//	m_state = newState;
		//}

		//void MediaObject::setError(ErrorType errorType, QString errorMessage)
		//{
		//	m_errorType = errorType;
		//	setState(Phonon::ErrorState);
		//	m_errorString = errorMessage;
		//}

		//void MediaObject::setAudioOutput(QObject* /*audioOutput*/)
		//{
		//	// TODO
		//	/*m_audioOutput = qobject_cast<AudioOutput*>(audioOutput);

		//	if (m_audioOutput) {
		//		m_volume = m_audioOutput->volume();
		//		connect(m_audioOutput, SIGNAL(volumeChanged(qreal)), this, SLOT(setVolume(qreal)));
		//	}*/
		//}

		void MediaObject::setVolume(qreal /*newVolume*/)
		{
			// TODO
			//m_volume = newVolume;
		}

		void MediaObject::setVideoWidget(VideoWidget* videoWidget)
		{
			m_session.setVideoWidget(videoWidget);
		}

		void MediaObject::setHasVideo(bool hasVideo)
		{
			if (m_hasVideo != hasVideo)
			{
				m_hasVideo = hasVideo;
				emit hasVideoChanged(m_hasVideo);
			}
		}
	}
}

QT_END_NAMESPACE