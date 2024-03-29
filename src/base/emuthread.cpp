/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "emuthread.h"
#include "emu.h"
#include "hostaudio.h"
#include "statelistmodel.h"
#include <QMutex>
#include <QWaitCondition>
#include <QDateTime>
#include <qmath.h>

/*!
	\class EmuThread
	EmuThread class manages the execution of the emulation.
 */

/*! Creates a new object with the given \a emu. */
EmuThread::EmuThread(Emu *emu) :
	m_emu(emu),
	m_running(false),
	m_inFrameGenerated(false),
	m_frameSkip(1)
{
}

/*! Destroys the thread. */
EmuThread::~EmuThread()
{
}

/*! Resumes the execution of the emulation. */
void EmuThread::resume()
{
	m_running = true;
	start();
}

/*! Pauses the execution of the emulation. */
void EmuThread::pause()
{
	m_running = false;
}

static void sleepMs(uint msecs)
{
	QMutex mutex;
	mutex.lock();
	QWaitCondition waitCondition;
	waitCondition.wait(&mutex, msecs);
	mutex.unlock();
}

/*! \internal */
void EmuThread::run()
{
	// resume the emulation
	m_emu->setRunning(true);
#if defined(MEEGO_EDITION_HARMATTAN)
	// prevent screen from locking
	int blankinkgPauseCounter = 0;
	m_displayState.setBlankingPause();
#endif
	qreal frameTime = 1000.0 / m_emu->frameRate();
	QTime time;
	time.start();
	qreal currentFrameTime = 0;
	int frameCounter = 0;
	while (m_running) {
		qreal currentTime = time.elapsed();
		currentFrameTime += frameTime;
		if (currentTime < currentFrameTime && frameCounter == 0) {
			m_emu->emulateFrame(true);
			m_inFrameGenerated = true;
			emit frameGenerated(true);
			m_inFrameGenerated = false;

			qreal currentTime = time.addMSecs(5).elapsed();
			if (currentTime < currentFrameTime)
				sleepMs(qFloor(currentFrameTime - currentTime));
		} else {
			m_emu->emulateFrame(false);
			emit frameGenerated(false);

			if (frameCounter != 0) {
				qreal currentTime = time.addMSecs(5).elapsed();
				if (currentTime < currentFrameTime)
					sleepMs(qFloor(currentFrameTime - currentTime));
			} else {
				currentFrameTime = 0;
				time.restart();
			}
		}
		if (++frameCounter > m_frameSkip)
			frameCounter = 0;
#if defined(MEEGO_EDITION_HARMATTAN)
		// preventing screen from locking must occur at least one time in 60s
		if (++blankinkgPauseCounter > 1000) {
			m_displayState.setBlankingPause();
			blankinkgPauseCounter = 0;
		}
#endif
	}
#if defined(MEEGO_EDITION_HARMATTAN)
	// stop preventing screen from locking
	m_displayState.cancelBlankingPause();
#endif
	// pause the emulation
	m_emu->setRunning(false);
}

/*! Sets frameskip to \a n. */
void EmuThread::setFrameSkip(int n)
{
	m_frameSkip = n;
}

/*!
	\fn int EmuThread::frameSkip() const
	Returns current frameskip.
 */
