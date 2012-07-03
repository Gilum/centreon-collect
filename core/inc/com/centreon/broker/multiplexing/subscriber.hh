/*
** Copyright 2009-2012 Merethis
**
** This file is part of Centreon Broker.
**
** Centreon Broker is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Broker is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Broker. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef CCB_MULTIPLEXING_SUBSCRIBER_HH
#  define CCB_MULTIPLEXING_SUBSCRIBER_HH

#  include <ctime>
#  include <QMutex>
#  include <QQueue>
#  include <QWaitCondition>
#  include "com/centreon/broker/io/stream.hh"
#  include "com/centreon/broker/namespace.hh"

CCB_BEGIN()

namespace          multiplexing {
  /**
   *  @class subscriber subscriber.hh "com/centreon/broker/multiplexing/subscriber.hh"
   *  @brief Receive events from publishers and make them
   *         available through the interface::source interface.
   *
   *  This class is used as a cornerstone in event multiplexing.
   *  Each output willing to receive events will request a
   *  subscriber object. All publisher objects broadcast events
   *  they receive to every subscriber objects.
   *
   *  @see publisher
   */
  class            subscriber : public io::stream {
  public:
                   subscriber();
                   ~subscriber();
    void           process(bool in = false, bool out = true);
    void           read(misc::shared_ptr<io::data>& d);
    void           read(
                     misc::shared_ptr<io::data>& d,
                     time_t timeout,
                     bool* timed_out = NULL);
    void           write(misc::shared_ptr<io::data> const& d);

  private:
                   subscriber(subscriber const& s);
    subscriber&    operator=(subscriber const& s);
    void           clean();

    QWaitCondition _cv;
    QQueue<misc::shared_ptr<io::data> >
                   _events;
    QMutex         _mutex;
    bool           _process_in;
    bool           _process_out;
  };
}

CCB_END()

#endif // !CCB_MULTIPLEXING_SUBSCRIBER_HH
