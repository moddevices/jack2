 /*
Copyright (C) 2004-2008 Grame

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#include "JackPosixProcessSync.h"
#include "JackError.h"

namespace Jack
{

void JackPosixProcessSync::Signal()
{
    int res = pthread_cond_signal(&fCond);
    if (res != 0) {
        jack_error("JackPosixProcessSync::Signal error err = %s", strerror(res));
    }
}

// TO DO : check thread consistency?
void JackPosixProcessSync::LockedSignal()
{
    int res = pthread_mutex_lock(&fMutex);
    if (res != 0) {
        jack_error("JackPosixProcessSync::LockedSignal error err = %s", strerror(res));
    }
    res = pthread_cond_signal(&fCond);
    if (res != 0) {
        jack_error("JackPosixProcessSync::LockedSignal error err = %s", strerror(res));
    }
    res = pthread_mutex_unlock(&fMutex);
    if (res != 0) {
        jack_error("JackPosixProcessSync::LockedSignal error err = %s", strerror(res));
    }
}

void JackPosixProcessSync::SignalAll()
{
    int res = pthread_cond_broadcast(&fCond);
    if (res != 0) {
        jack_error("JackPosixProcessSync::SignalAll error err = %s", strerror(res));
    }
}

// TO DO : check thread consistency?
void JackPosixProcessSync::LockedSignalAll()
{
    int res = pthread_mutex_lock(&fMutex);
    if (res != 0) {
        jack_error("JackPosixProcessSync::LockedSignalAll error err = %s", strerror(res));
    }
    res = pthread_cond_broadcast(&fCond);
    if (res != 0) {
        jack_error("JackPosixProcessSync::LockedSignalAll error err = %s", strerror(res));
    }
    res = pthread_mutex_unlock(&fMutex);
    if (res != 0) {
        jack_error("JackPosixProcessSync::LockedSignalAll error err = %s", strerror(res));
    }
}

void JackPosixProcessSync::Wait()
{
    ThrowIf(!pthread_equal(pthread_self(), fOwner), JackException("JackPosixProcessSync::Wait: a thread has to have locked a mutex before it can wait"));
    fOwner = 0;

    int res = pthread_cond_wait(&fCond, &fMutex);
    if (res != 0) {
        jack_error("JackPosixProcessSync::Wait error err = %s", strerror(res));
    } else {
        fOwner = pthread_self();
    }
}

// TO DO : check thread consistency?
void JackPosixProcessSync::LockedWait()
{
    int res;
    res = pthread_mutex_lock(&fMutex);
    if (res != 0) {
        jack_error("JackPosixProcessSync::LockedWait error err = %s", strerror(res));
    }
    if ((res = pthread_cond_wait(&fCond, &fMutex)) != 0) {
        jack_error("JackPosixProcessSync::LockedWait error err = %s", strerror(res));
    }
    res = pthread_mutex_unlock(&fMutex);
    if (res != 0) {
        jack_error("JackPosixProcessSync::LockedWait error err = %s", strerror(res));
    }
}

bool JackPosixProcessSync::TimedWait(long usec)
{
    ThrowIf(!pthread_equal(pthread_self(), fOwner), JackException("JackPosixProcessSync::TimedWait: a thread has to have locked a mutex before it can wait"));
    fOwner = 0;

    int res;
    timespec now;
    clock_gettime(fClockType, &now);

    timespec delta = { usec / 1000000, (usec % 1000000) * 1000 };
    timespec end   = { now.tv_sec + delta.tv_sec, now.tv_nsec + delta.tv_nsec };
    if (end.tv_nsec >= 1000000000L) {
        ++end.tv_sec;
        end.tv_nsec -= 1000000000L;
    }

    res = pthread_cond_timedwait(&fCond, &fMutex, &end);
    if (res != 0) {
        jack_error("JackPosixProcessSync::TimedWait error usec = %ld err = %s", usec, strerror(res));
    } else {
        fOwner = pthread_self();
    }

    return (res == 0);
}

// TO DO : check thread consistency?
bool JackPosixProcessSync::LockedTimedWait(long usec)
{
    int res1 = pthread_mutex_lock(&fMutex);
    if (res1 != 0) {
        jack_error("JackPosixProcessSync::LockedTimedWait error err = %s", usec, strerror(res1));
    }

    timespec now;
    clock_gettime(fClockType, &now);

    timespec delta = { usec / 1000000, (usec % 1000000) * 1000 };
    timespec end   = { now.tv_sec + delta.tv_sec, now.tv_nsec + delta.tv_nsec };
    if (end.tv_nsec >= 1000000000L) {
        ++end.tv_sec;
        end.tv_nsec -= 1000000000L;
    }

    int res2 = pthread_cond_timedwait(&fCond, &fMutex, &end);
    if (res2 != 0) {
        jack_error("JackPosixProcessSync::LockedTimedWait error usec = %ld err = %s", usec, strerror(res2));
    }

    res1 = pthread_mutex_unlock(&fMutex);
    if (res1 != 0) {
        jack_error("JackPosixProcessSync::LockedTimedWait error err = %s", usec, strerror(res1));
    }

    return (res2 == 0);
}


} // end of namespace

