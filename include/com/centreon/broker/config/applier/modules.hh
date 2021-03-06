/*
** Copyright 2011-2012 Centreon
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

#ifndef CCB_CONFIG_APPLIER_MODULES_HH
#define CCB_CONFIG_APPLIER_MODULES_HH

#include <list>
#include <mutex>
#include <string>
#include "com/centreon/broker/modules/loader.hh"
#include "com/centreon/broker/namespace.hh"

CCB_BEGIN()

namespace config {
namespace applier {
/**
 *  @class modules modules.hh "com/centreon/broker/config/applier/modules.hh"
 *  @brief Load necessary modules.
 *
 *  Load modules as per the configuration.
 */
class modules {
  broker::modules::loader _loader;
  std::mutex _m_modules;

 public:
  typedef broker::modules::loader::iterator iterator;

  modules() = default;
  modules(modules const& m) = delete;
  modules& operator=(modules const& m) = delete;
  ~modules();
  void apply(std::list<std::string> const& module_list,
             std::string const& module_dir,
             void const* arg = NULL);
  iterator begin();
  iterator end();
  void discard();
  static modules& instance();
  static void load();
  static void unload();

  std::mutex& module_mutex();
};
}  // namespace applier
}  // namespace config

CCB_END()

#endif  // !CCB_CONFIG_APPLIER_MODULES_HH
