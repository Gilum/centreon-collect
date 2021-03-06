/*
** Copyright 2013 Centreon
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** For more information : contact@centreon.com
*/

#include "com/centreon/broker/neb/statistics/passive_service_latency.hh"
#include <iomanip>
#include <sstream>
#include "com/centreon/broker/config/applier/state.hh"
#include "com/centreon/broker/neb/internal.hh"
#include "com/centreon/broker/neb/statistics/compute_value.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::broker;
using namespace com::centreon::broker::neb;
using namespace com::centreon::broker::neb::statistics;
using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
passive_service_latency::passive_service_latency()
    : plugin("passive_service_latency") {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
passive_service_latency::passive_service_latency(
    passive_service_latency const& right)
    : plugin(right) {}

/**
 *  Destructor.
 */
passive_service_latency::~passive_service_latency() {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
passive_service_latency& passive_service_latency::operator=(
    passive_service_latency const& right) {
  plugin::operator=(right);
  return (*this);
}

/**
 *  Get statistics.
 *
 *  @param[out] output   The output return by the plugin.
 *  @param[out] perfdata The perf data return by the plugin.
 */
void passive_service_latency::run(std::string& output, std::string& perfdata) {
  compute_value<double> cv;
  for (service_map::const_iterator it{service::services.begin()},
       end{service::services.end()};
       it != end; ++it)
    if (it->second->get_check_type() == checkable::check_passive)
      cv << it->second->get_latency();

  if (cv.size()) {
    // Output.
    std::ostringstream oss;
    oss << "Engine " << config::applier::state::instance().poller_name()
        << " has an average passive service latency of " << std::fixed
        << std::setprecision(2) << cv.avg() << "s";
    output = oss.str();

    // Perfdata.
    oss.str("");
    oss << "avg=" << cv.avg() << "s min=" << cv.min() << "s max=" << cv.max()
        << "s";
    perfdata = oss.str();
  } else {
    // Output.
    output =
        "No passive service to compute passive service "
        "latency on " +
        config::applier::state::instance().poller_name();
  }
  return;
}
