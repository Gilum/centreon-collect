/*
** Copyright 2011-2013 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef CCB_NOTIFICATION_OBJECTS_DATERANGE_HH
#  define CCB_NOTIFICATION_OBJECTS_DATERANGE_HH

#  include <list>
#  include <string>
#  include <ctime>
#  include <vector>
#  include "com/centreon/broker/notification/objects/timerange.hh"
#  include "com/centreon/broker/namespace.hh"
#  include "com/centreon/broker/notification/utilities/ptr_typedef.hh"

CCB_BEGIN()

namespace               notification {
  namespace             objects {
    /**
     *  @class daterange daterange.hh "com/centreon/broker/notification/objects/daterange.hh"
     *  @brief Daterange object.
     *
     *  The object containing a daterange.
     */
    class               daterange {
    public:
                        DECLARE_SHARED_PTR(daterange);

      static const int  daterange_types = 5;

      enum              type_range {
        none = -1,
        calendar_date = 0,
        month_date = 1,
        month_day = 2,
        month_week_day = 3,
        week_day = 4
      };

                        daterange(type_range type);
                        daterange(daterange const& right);
                        ~daterange();
      daterange&        operator=(daterange const& right);
      bool              operator==(daterange const& right) const throw ();
      bool              operator!=(daterange const& right) const throw ();
      bool              operator<(daterange const& right) const throw ();
      void              month_end(unsigned int value);
      unsigned int      month_end() const throw ();
      void              month_start(unsigned int value);
      unsigned int      month_start() const throw ();
      void              month_day_end(int value);
      int               month_day_end() const throw ();
      void              month_day_start(int value);
      int               month_day_start() const throw ();
      void              skip_interval(unsigned int value);
      unsigned int      skip_interval() const throw ();
      void              timeranges(std::list<timerange> const& value);
      std::list<timerange> const&
                        timeranges() const throw ();
      void              type(type_range value);
      type_range        type() const throw ();
      void              week_day_end(unsigned int value);
      unsigned int      week_day_end() const throw ();
      void              week_day_start(unsigned int value);
      unsigned int      week_day_start() const throw ();
      void              week_day_end_offset(int value);
      int               week_day_end_offset() const throw ();
      void              week_day_start_offset(int value);
      int               week_day_start_offset() const throw ();
      void              year_end(unsigned int value);
      unsigned int      year_end() const throw ();
      void              year_start(unsigned int value);
      unsigned int      year_start() const throw ();

      bool              to_time_t(
                          time_t const preferred_time,
                          time_t& start,
                          time_t& end) const;

      static bool       build_calendar_date(
                          std::string const& line,
                          std::vector<std::list<daterange> >& list);
      static bool       build_other_date(
                          std::string const& line,
                          std::vector<std::list<daterange> >& list);
      static bool       build_dateranges_from_string(
                          std::string const& value,
                          std::vector<std::list<daterange> >& list);

    private:
      unsigned int      _month_end;
      unsigned int      _month_start;
      int               _month_day_end;
      int               _month_day_start;
      unsigned int      _skip_interval;
      std::list<timerange>
                        _timeranges;
      type_range        _type;
      unsigned int      _week_day_end;
      unsigned int      _week_day_start;
      int               _week_day_end_offset;
      int               _week_day_start_offset;
      unsigned int      _year_end;
      unsigned int      _year_start;

      struct            time_info;

      bool              _calendar_date_to_time_t(
                          time_t& start,
                          time_t& end) const;
      bool              _month_date_to_time_t(
                          time_info const& ti,
                          time_t& start,
                          time_t& end) const;
      bool              _month_day_to_time_t(
                          time_info const& ti,
                          time_t& start,
                          time_t& end) const;
      bool              _month_week_day_to_time_t(
                          time_info const& ti,
                          time_t& start,
                          time_t& end) const;
      bool              _week_day_to_time_t(
                          time_info const& ti,
                          time_t& start,
                          time_t& end) const;
    };
  }
}

CCB_END()

#endif // !CCB_NOTIFICATION_OBJECTS_DATERANGE_HH
