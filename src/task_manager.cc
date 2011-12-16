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

#include <assert.h>
#include <stdlib.h>
#include "com/centreon/task_manager.hh"

using namespace com::centreon;
using namespace com::centreon::concurrency;

/**
 *  Default constructor.
 *
 *  @param[in] max_thread_count  The number of threads into the thread
 *                               pool.
 */
task_manager::task_manager(unsigned int max_thread_count)
  : _current_id(0), _th_pool(max_thread_count) {

}

/**
 *  Default destructor.
 */
task_manager::~task_manager() throw () {
  // Wait the end of all running task.
  _th_pool.wait_for_done();

  // Delete all internal task.
  for (std::multimap<timestamp, internal_task*>::const_iterator
         it(_tasks.begin()), end(_tasks.end());
       it != end;
       ++it)
    delete it->second;
}

/**
 *  Add a new task into the task manager.
 *
 *  @param[in] t              The new task.
 *  @param[in] when           The time limit to execute the task.
 *  @param[in] is_runnable    If the task should run simultaneously.
 *  @param[in] should_delete  If the task should delete after running.
 *
 *  @return The id of the new task in the task manager.
 */
unsigned long task_manager::add(
                              task* t,
                              timestamp when,
                              bool is_runnable,
                              bool should_delete) {
  internal_task* itask(new internal_task(
                             ++_current_id,
                             t,
                             when,
                             0,
                             is_runnable,
                             should_delete));
  _tasks.insert(std::pair<timestamp, internal_task*>(when, itask));
  return (itask->id);
}

/**
 *  This method is an overload to add.
 *
 *  @param[in] t              The new task.
 *  @param[in] when           The time limit to execute the task.
 *  @param[in] interval       Define the recurrency of the task.
 *  @param[in] is_runnable    If the task should run simultaneously.
 *  @param[in] should_delete  If the task should delete after running.
 *
 *  @return The id of the new task in the task manager.
 */
unsigned long task_manager::add(
                              task* t,
                              timestamp when,
                              unsigned int interval,
                              bool is_runnable,
                              bool should_delete) {
  internal_task* itask(new internal_task(
                             ++_current_id,
                             t,
                             when,
                             interval,
                             is_runnable,
                             should_delete));
  _tasks.insert(std::pair<timestamp, internal_task*>(when, itask));
  return (itask->id);
}

/**
 *  Execute all the task to need run before the time limit.
 *
 *  @param[in] now  The time limit to execute tasks.
 *
 *  @return The number of task to be execute.
 */
unsigned int task_manager::execute(timestamp now) {
  // Stock the new recurring task into this list for inject all
  // task at the end of the execution.
  std::list<std::pair<timestamp, internal_task*> > recurring;

  unsigned int count_execute(0);
  for (std::multimap<timestamp, internal_task*>::iterator
         it(_tasks.begin()), next(it), end(_tasks.end());
       it != end;
       it = next) {
    // This task dosn't need to be run now.
    if (it->first > now)
      break;

    internal_task* itask(it->second);
    if (itask->interval) {
      // This task is recurring, push it into recurring list.
      timestamp new_time(now);
      new_time.add_usecond(itask->interval);
      recurring.push_back(std::pair<timestamp,
                                    internal_task*>(new_time,
                                                    itask));
    }

    if (itask->is_runnable) {
      // This task allow to run in the thread.
      _th_pool.start(itask);
    }
    else {
      // This task need to be run in the main thread without
      // any concurrence.
      _th_pool.wait_for_done();
      itask->t->run();
      if (itask->get_auto_delete())
        delete itask;
    }
    ++(next = it);
    _tasks.erase(it);
    ++count_execute;
  }

  // Update the task table with the recurring task.
  for (std::list<std::pair<timestamp, internal_task*> >::const_iterator
         it(recurring.begin()), end(recurring.end());
       it != end;
       ++it) {
    it->second->when = it->first;
    _tasks.insert(*it);
  }
  return (count_execute);
}

/**
 *  Get the next execution time.
 *
 *  @return The next time to execute task or null timestamp if no
 *          task need to be run.
 */
timestamp task_manager::next_execution_time() const {
  std::multimap<timestamp, internal_task*>::const_iterator
    lower(_tasks.begin());
  if (lower == _tasks.end())
    return (timestamp());
  return (lower->first);
}

/**
 *  Remove task.
 *
 *  @param[in] t  The specific task.
 *
 *  @return The number of remove task.
 */
unsigned int task_manager::remove(task* t) {
  if (!t)
    return (0);

  unsigned int count_erase(0);
  for (std::multimap<timestamp, internal_task*>::iterator
         it(_tasks.begin()), next(it), end(_tasks.end());
       it != end;
       it = next)
    if (it->second->t == t) {
      if (it->second->get_auto_delete())
        delete it->second;
      ++(next = it);
      _tasks.erase(it);
      ++count_erase;
    }
  return (count_erase);
}

/**
 *  This method is an overload of remove.
 *
 *  @param[in] id  The task id to remove.
 *
 *  @return True if the task was remove, otherwise false.
 */
bool task_manager::remove(unsigned long id) {
  for (std::multimap<timestamp, internal_task*>::iterator
         it(_tasks.begin()), next(it), end(_tasks.end());
       it != end;
       it = next)
    if (it->second->id == id) {
      if (it->second->get_auto_delete())
        delete it->second;
      ++(next = it);
      _tasks.erase(it);
      return (true);
    }
  return (false);
}

/**
 *  Default copy constructor.
 *
 *  @param[in] right  The object to copy.
 */
task_manager::task_manager(task_manager const& right) {
  _internal_copy(right);
}

/**
 *  Default copy operator.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
task_manager& task_manager::operator=(task_manager const& right) {
  return (_internal_copy(right));
}

/**
 *  Internal copy.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
task_manager& task_manager::_internal_copy(task_manager const& right) {
  (void)right;
  assert(!"impossible to copy task_manager");
  abort();
  return (*this);
}

/**
 *  Default constructor.
 *
 *  @param[in] _id             The task id.
 *  @param[in] _t              The task.
 *  @param[in] _when           The time limit to execute the task.
 *  @param[in] _interval       Define the recurrency of the task.
 *  @param[in] _is_runnable    If the task should run simultaneously.
 *  @param[in] _should_delete  If the task should delete after running.
 */
task_manager::internal_task::internal_task(
                           unsigned long _id,
                           task* _t,
                           timestamp _when,
                           unsigned int _interval,
                           bool _is_runnable,
                           bool _should_delete)
  : runnable(),
    id(_id),
    interval(_interval),
    is_runnable(_is_runnable),
    should_delete(_should_delete),
    t(_t),
    when(_when) {
  // If the task is recurring we need to set auto delete at false, to
  // reuse this task.
  set_auto_delete(!interval);
}

/**
 *  Default copy constructor.
 *
 *  @param[in] right  The object to copy.
 */
task_manager::internal_task::internal_task(internal_task const& right)
  : runnable() {
  _internal_copy(right);
}

/**
 *  Default destructor.
 */
task_manager::internal_task::~internal_task() throw () {
  if (should_delete)
    delete t;
}

/**
 *  Default copy operator.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
task_manager::internal_task& task_manager::internal_task::operator=(internal_task const& right) {
  return (_internal_copy(right));
}

/**
 *  Run a task.
 */
void task_manager::internal_task::run() {
  t->run();
}

/**
 *  Internal copy.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
task_manager::internal_task& task_manager::internal_task::_internal_copy(internal_task const& right) {
  if (this != &right) {
    runnable::operator=(right);
    id = right.id;
    interval = right.interval;
    is_runnable = right.is_runnable;
    t = right.t;
    when = right.when;
  }
  return (*this);
}
