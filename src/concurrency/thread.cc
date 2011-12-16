/*
** Copyright 2011 Merethis
**
** This file is part of Centreon Clib.
**
** Centreon Clib is free software: you can redistribute it
** and/or modify it under the terms of the GNU Affero General Public
** License as published by the Free Software Foundation, either version
** 3 of the License, or (at your option) any later version.
**
** Centreon Clib is distributed in the hope that it will be
** useful, but WITHOUT ANY WARRANTY; without even the implied warranty
** of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public
** License along with Centreon Clib. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include "com/centreon/exception/basic.hh"
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/concurrency/thread.hh"

using namespace com::centreon::concurrency;

/**
 *  Default constructor.
 */
thread::thread() {

}

/**
 *  Default destructor.
 */
thread::~thread() throw () {

}

/**
 *  Execute the running method in the new thread.
 */
void thread::exec() {
  int ret(pthread_create(&_th, NULL, &_execute, this));
  if (ret)
    throw (basic_error() << "failed to create thread:"
           << strerror(ret));
}

/**
 *  Get the current thread id.
 *
 *  @return The current thread id.
 */
thread_id thread::get_current_id() throw () {
  return (pthread_self());
}

/**
 *  Makes the calling thread sleep untils timeout.
 *  @remark This function is static.
 *
 *  @param[in] msecs  Time to sleep in milliseconds.
 */
void thread::msleep(unsigned long msecs) {
  // Get the current time.
  timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts))
    throw (basic_error() << "failed sleep thread:"
           << strerror(errno));

  // Add timeout.
  time_t sec(msecs / 1000);
  msecs -= sec * 1000;
  ts.tv_sec += sec;
  ts.tv_nsec += msecs * 1000 * 1000;

  // Sleep the calling thread.
  _sleep(&ts);
}

/**
 *  Makes the calling thread sleep untils timeout.
 *  @remark This function is static.
 *
 *  @param[in] nsecs  Time to sleep in nanoseconds.
 */
void thread::nsleep(unsigned long nsecs) {
  // Get the current time.
  timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts))
    throw (basic_error() << "failed sleep thread:"
           << strerror(errno));

  // Add timeout.
  ts.tv_nsec += nsecs;

  // Sleep the calling thread.
  _sleep(&ts);
}

/**
 *  Makes the calling thread sleep untils timeout.
 *  @remark This function is static.
 *
 *  @param[in] secs  Time to sleep in seconds.
 */
void thread::sleep(unsigned long secs) {
  // Get the current time.
  timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts))
    throw (basic_error() << "failed sleep thread:"
           << strerror(errno));

  // Add timeout.
  ts.tv_sec += secs;

  // Sleep the calling thread.
  _sleep(&ts);
}

/**
 *  Makes the calling thread sleep untils timeout.
 *  @remark This function is static.
 *
 *  @param[in] usecs  Time to sleep in micoseconds.
 */
void thread::usleep(unsigned long usecs) {
  // Get the current time.
  timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts))
    throw (basic_error() << "failed sleep thread:"
           << strerror(errno));

  // Add timeout.
  time_t sec(usecs / (1000 * 1000));
  usecs -= sec * (1000 * 1000);
  ts.tv_sec += sec;
  ts.tv_nsec += usecs * 1000;

  // Sleep the calling thread.
  _sleep(&ts);
}

/**
 *  Wait the end of the thread.
 */
void thread::wait() {
  locker lock(&_mtx);

  // Wait the end of the thread.
  int ret(pthread_join(_th, NULL));
  if (ret && ret != ESRCH)
    throw (basic_error() << "failed to wait thread:"
           << strerror(ret));
}

/**
 *  This is an overload of wait.
 *
 *  @param[in] timeout  Define the timeout to wait the end of
 *                      the thread.
 *
 *  @return True if the thread end before timeout, otherwise false.
 */
bool thread::wait(unsigned long timeout) {
  locker lock(&_mtx);

  // Get the current time.
  timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts))
    throw (basic_error() << "failed to wait thread:"
           << strerror(errno));

  // Add timeout.
  time_t sec(timeout / 1000);
  timeout -= sec * 1000;
  ts.tv_sec += sec;
  ts.tv_nsec += timeout * 1000 * 1000;

  // Wait the end of the thread or timeout.
  int ret(pthread_timedjoin_np(_th, NULL, &ts));
  if (!ret || ret == ESRCH)
    return (true);
  if (ret == ETIMEDOUT)
    return (false);
  throw (basic_error() << "failed to wait thread:" << strerror(ret));
}

/**
 *  Causes the calling thread to relinquish the CPU.
 *  @remark This function is static.
 */
void thread::yield() throw () {
  sched_yield();
}

/**
 *  Default copy constructor.
 *
 *  @param[in] right  The object to copy.
 */
thread::thread(thread const& right) {
  _internal_copy(right);
}

/**
 *  Default copy operator.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
thread& thread::operator=(thread const& right) {
  return (_internal_copy(right));
}

/**
 *  The thread start routine.
 *  @remark This function is static.
 *
 *  @param[in] data  This thread object.
 *
 *  @return always return zero.
 */
void* thread::_execute(void* data) {
  thread* self(static_cast<thread*>(data));
  if (self)
    self->_run();
  return (0);
}

/**
 *  Internal copy.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
thread& thread::_internal_copy(thread const& right) {
  (void)right;
  assert(!"impossible to copy thread");
  abort();
  return (*this);
}

/**
 *  Internal sleep, Makes the calling thread sleep untils timeout.
 *  @remark This function is static.
 *
 *  @param[in] ts  Time to sleep with timespec struct.
 */
void thread::_sleep(timespec* ts) {
  int ret(0);

  // Create mutex.
  pthread_mutex_t mtx;
  pthread_mutex_init(&mtx, NULL);

  // Create condition variable.
  pthread_cond_t cnd;
  pthread_cond_init(&cnd, NULL);

  // Lock the mutex.
  if ((ret = pthread_mutex_lock(&mtx)))
    throw (basic_error() << "impossible to sleep:" << strerror(ret));

  // Wait the timeout of the condition variable.
  if ((ret = pthread_cond_timedwait(&cnd, &mtx, ts))
      && ret != ETIMEDOUT)
    throw (basic_error() << "impossible to sleep:" << strerror(ret));

  // Release mutex.
  if ((ret = pthread_mutex_unlock(&mtx)))
    throw (basic_error() << "impossible in sleep:" << strerror(ret));

  // Cleanup.
  pthread_cond_destroy(&cnd);
  pthread_mutex_destroy(&mtx);
}
