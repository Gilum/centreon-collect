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

#include <string>
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/deleter.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros.hh"
#include "test/unittest.hh"
#include "xodtemplate.hh"

using namespace com::centreon::engine;

struct               global {
  command*           commands;
  contact*           contacts;
  contactgroup*      contactgroups;
  host*              hosts;
  hostdependency*    hostdependencies;
  hostescalation*    hostescalations;
  hostgroup*         hostgroups;
  service*           services;
  servicedependency* servicedependencies;
  serviceescalation* serviceescalations;
  servicegroup*      servicegroups;
  timeperiod*        timeperiods;

  bool               accept_passive_host_checks;
  bool               accept_passive_service_checks;
  int                additional_freshness_latency;
  std::string        admin_email;
  std::string        admin_pager;
  bool               allow_empty_hostgroup_assignment;
  bool               auto_reschedule_checks;
  unsigned int       auto_rescheduling_interval;
  unsigned int       auto_rescheduling_window;
  unsigned long      cached_host_check_horizon;
  unsigned long      cached_service_check_horizon;
  bool               check_external_commands;
  bool               check_host_freshness;
  bool               check_orphaned_hosts;
  bool               check_orphaned_services;
  unsigned int       check_reaper_interval;
  std::string        check_result_path;
  bool               check_service_freshness;
  int                command_check_interval;
  std::string        command_file;
  int                date_format;
  std::string        debug_file;
  unsigned long      debug_level;
  unsigned int       debug_verbosity;
  bool               enable_environment_macros;
  bool               enable_event_handlers;
  bool               enable_failure_prediction;
  bool               enable_flap_detection;
  bool               enable_notifications;
  bool               enable_predictive_host_dependency_checks;
  bool               enable_predictive_service_dependency_checks;
  unsigned long      event_broker_options;
  unsigned int       event_handler_timeout;
  bool               execute_host_checks;
  bool               execute_service_checks;
  int                external_command_buffer_slots;
  std::string        global_host_event_handler;
  std::string        global_service_event_handler;
  float              high_host_flap_threshold;
  float              high_service_flap_threshold;
  unsigned int       host_check_timeout;
  unsigned int       host_freshness_check_interval;
  int                host_inter_check_delay_method;
  std::string        illegal_object_chars;
  std::string        illegal_output_chars;
  unsigned int       interval_length;
  bool               log_event_handlers;
  bool               log_external_commands;
  std::string        log_file;
  bool               log_host_retries;
  bool               log_initial_states;
  bool               log_notifications;
  bool               log_passive_checks;
  bool               log_service_retries;
  float              low_host_flap_threshold;
  float              low_service_flap_threshold;
  unsigned int       max_check_reaper_time;
  unsigned long      max_check_result_file_age;
  unsigned long      max_debug_file_size;
  unsigned int       max_host_check_spread;
  unsigned int       max_parallel_service_checks;
  unsigned int       max_service_check_spread;
  unsigned int       notification_timeout;
  bool               obsess_over_hosts;
  bool               obsess_over_services;
  std::string        ochp_command;
  unsigned int       ochp_timeout;
  std::string        ocsp_command;
  unsigned int       ocsp_timeout;
  bool               passive_host_checks_are_soft;
  bool               process_performance_data;
  unsigned long      retained_contact_host_attribute_mask;
  unsigned long      retained_contact_service_attribute_mask;
  unsigned long      retained_host_attribute_mask;
  unsigned long      retained_process_host_attribute_mask;
  bool               retain_state_information;
  unsigned int       retention_scheduling_horizon;
  unsigned int       retention_update_interval;
  unsigned int       service_check_timeout;
  unsigned int       service_freshness_check_interval;
  int                service_inter_check_delay_method;
  int                service_interleave_factor_method;
  float              sleep_time;
  bool               soft_state_dependencies;
  unsigned int       status_update_interval;
  unsigned int       time_change_threshold;
  bool               translate_passive_host_checks;
  bool               use_aggressive_host_checking;
  bool               use_large_installation_tweaks;
  bool               use_regexp_matches;
  bool               use_retained_program_state;
  bool               use_retained_scheduling_info;
  bool               use_syslog;
  std::string        use_timezone;
  bool               use_true_regexp_matching;
};

#define check_value(id) \
  if (g1.id != g2.id) { \
    std::cerr << #id << " is different (" \
      << g1.id << ", " << g2.id << ")" << std::endl; \
    ret = false; \
  }

static std::string to_str(char const* str) { return (str ? str : ""); }

/**
 *  Check difference between to list of object.
 *
 *  @param[in] l1 The first list.
 *  @param[in] l2 The second list.
 *
 *  @return True if all list is equal, otherwise false.
 */
template<typename T>
static bool chkdiff(T const* l1, T const* l2) {
  T const* obj1(l1);
  T const* obj2(l2);
  while (obj1 && obj2) {
    if (*obj1 != *obj2) {
      std::cerr << "difference detected" << std::endl;
      std::cerr << "old " << *obj1 << std::endl;
      std::cerr << "new " << *obj2 << std::endl;
      return (false);
    }
    obj1 = obj1->next;
    obj2 = obj2->next;
  }
  if (!obj1) {
    std::cerr << "missing object" << std::endl;
    std::cerr << "new " << *obj2 << std::endl;
    return (false);
  }
  if (!obj2) {
    std::cerr << "missing object" << std::endl;
    std::cerr << "old " << *obj1 << std::endl;
    return (false);
  }
  return (true);
}

/**
 *  Check difference between global object.
 *
 *  @param[in] l1 The first struct.
 *  @param[in] l2 The second struct.
 *
 *  @return True if globals are equal, otherwise false.
 */
bool chkdiff(global const& g1, global const& g2) {
  bool ret(true);

  check_value(accept_passive_host_checks);
  check_value(accept_passive_service_checks);
  check_value(additional_freshness_latency);
  check_value(admin_email);
  check_value(admin_pager);
  check_value(allow_empty_hostgroup_assignment);
  check_value(auto_reschedule_checks);
  check_value(auto_rescheduling_interval);
  check_value(auto_rescheduling_window);
  check_value(cached_host_check_horizon);
  check_value(cached_service_check_horizon);
  check_value(check_external_commands);
  check_value(check_host_freshness);
  check_value(check_orphaned_hosts);
  check_value(check_orphaned_services);
  check_value(check_reaper_interval);
  check_value(check_result_path);
  check_value(check_service_freshness);
  check_value(command_check_interval);
  check_value(command_file);
  check_value(date_format);
  check_value(debug_file);
  check_value(debug_level);
  check_value(debug_verbosity);
  check_value(enable_environment_macros);
  check_value(enable_event_handlers);
  check_value(enable_failure_prediction);
  check_value(enable_flap_detection);
  check_value(enable_notifications);
  check_value(enable_predictive_host_dependency_checks);
  check_value(enable_predictive_service_dependency_checks);
  check_value(event_broker_options);
  check_value(event_handler_timeout);
  check_value(execute_host_checks);
  check_value(execute_service_checks);
  check_value(external_command_buffer_slots);
  check_value(global_host_event_handler);
  check_value(global_service_event_handler);
  check_value(high_host_flap_threshold);
  check_value(high_service_flap_threshold);
  check_value(host_check_timeout);
  check_value(host_freshness_check_interval);
  check_value(host_inter_check_delay_method);
  check_value(illegal_object_chars);
  check_value(illegal_output_chars);
  check_value(interval_length);
  check_value(log_event_handlers);
  check_value(log_external_commands);
  check_value(log_file);
  check_value(log_host_retries);
  check_value(log_initial_states);
  check_value(log_notifications);
  check_value(log_passive_checks);
  check_value(log_service_retries);
  check_value(low_host_flap_threshold);
  check_value(low_service_flap_threshold);
  check_value(max_check_reaper_time);
  check_value(max_check_result_file_age);
  check_value(max_debug_file_size);
  check_value(max_host_check_spread);
  check_value(max_parallel_service_checks);
  check_value(max_service_check_spread);
  check_value(notification_timeout);
  check_value(obsess_over_hosts);
  check_value(obsess_over_services);
  check_value(ochp_command);
  check_value(ochp_timeout);
  check_value(ocsp_command);
  check_value(ocsp_timeout);
  check_value(passive_host_checks_are_soft);
  check_value(process_performance_data);
  check_value(retained_contact_host_attribute_mask);
  check_value(retained_contact_service_attribute_mask);
  check_value(retained_host_attribute_mask);
  check_value(retained_process_host_attribute_mask);
  check_value(retain_state_information);
  check_value(retention_scheduling_horizon);
  check_value(retention_update_interval);
  check_value(service_check_timeout);
  check_value(service_freshness_check_interval);
  check_value(service_inter_check_delay_method);
  check_value(service_interleave_factor_method);
  check_value(sleep_time);
  check_value(soft_state_dependencies);
  check_value(status_update_interval);
  check_value(time_change_threshold);
  check_value(translate_passive_host_checks);
  check_value(use_aggressive_host_checking);
  check_value(use_large_installation_tweaks);
  check_value(use_regexp_matches);
  check_value(use_retained_program_state);
  check_value(use_retained_scheduling_info);
  check_value(use_syslog);
  check_value(use_timezone);
  check_value(use_true_regexp_matching);

  if (!chkdiff(g1.commands, g2.commands))
    ret = false;
  if (!chkdiff(g1.contacts, g2.contacts))
    ret = false;
  if (!chkdiff(g1.contactgroups, g2.contactgroups))
    ret = false;
  if (!chkdiff(g1.hosts, g2.hosts))
    ret = false;
  if (!chkdiff(g1.hostdependencies, g2.hostdependencies))
    ret = false;
  if (!chkdiff(g1.hostescalations, g2.hostescalations))
    ret = false;
  if (!chkdiff(g1.hostgroups, g2.hostgroups))
    ret = false;
  if (!chkdiff(g1.services, g2.services))
    ret = false;
  if (!chkdiff(g1.servicedependencies, g2.servicedependencies))
    ret = false;
  if (!chkdiff(g1.serviceescalations, g2.serviceescalations))
    ret = false;
  if (!chkdiff(g1.servicegroups, g2.servicegroups))
    ret = false;
  if (!chkdiff(g1.timeperiods, g2.timeperiods))
    ret = false;
  return (ret);
}

/**
 *  Get all global configuration.
 *
 *  @retrun All global.
 */
static global get_globals() {
  global g;
  g.commands = command_list;
  command_list = NULL;
  g.contacts = contact_list;
  contact_list = NULL;
  g.contactgroups = contactgroup_list;
  contactgroup_list = NULL;
  g.hosts = host_list;
  host_list = NULL;
  g.hostdependencies = hostdependency_list;
  hostdependency_list = NULL;
  g.hostescalations = hostescalation_list;
  hostescalation_list = NULL;
  g.hostgroups = hostgroup_list;
  hostgroup_list = NULL;
  g.services = service_list;
  service_list = NULL;
  g.servicedependencies = servicedependency_list;
  servicedependency_list = NULL;
  g.serviceescalations = serviceescalation_list;
  serviceescalation_list = NULL;
  g.servicegroups = servicegroup_list;
  servicegroup_list = NULL;
  g.timeperiods = timeperiod_list;
  timeperiod_list = NULL;

  g.accept_passive_host_checks = accept_passive_host_checks;
  g.accept_passive_service_checks = accept_passive_service_checks;
  g.additional_freshness_latency = additional_freshness_latency;
  g.admin_email = to_str(macro_x[MACRO_ADMINEMAIL]);
  g.admin_pager = to_str(macro_x[MACRO_ADMINPAGER]);
  g.allow_empty_hostgroup_assignment = allow_empty_hostgroup_assignment;
  g.auto_reschedule_checks = auto_reschedule_checks;
  g.auto_rescheduling_interval = auto_rescheduling_interval;
  g.auto_rescheduling_window = auto_rescheduling_window;
  g.cached_host_check_horizon = cached_host_check_horizon;
  g.cached_service_check_horizon = cached_service_check_horizon;
  g.check_external_commands = check_external_commands;
  g.check_host_freshness = check_host_freshness;
  g.check_orphaned_hosts = check_orphaned_hosts;
  g.check_orphaned_services = check_orphaned_services;
  g.check_reaper_interval = check_reaper_interval;
  g.check_result_path = to_str(check_result_path);
  g.check_service_freshness = check_service_freshness;
  g.command_check_interval = command_check_interval;
  g.command_file = to_str(command_file);
  g.date_format = date_format;
  g.debug_file = to_str(debug_file);
  g.debug_level = debug_level;
  g.debug_verbosity = debug_verbosity;
  g.enable_environment_macros = enable_environment_macros;
  g.enable_event_handlers = enable_event_handlers;
  g.enable_failure_prediction = enable_failure_prediction;
  g.enable_flap_detection = enable_flap_detection;
  g.enable_notifications = enable_notifications;
  g.enable_predictive_host_dependency_checks = enable_predictive_host_dependency_checks;
  g.enable_predictive_service_dependency_checks = enable_predictive_service_dependency_checks;
  g.event_broker_options = event_broker_options;
  g.event_handler_timeout = event_handler_timeout;
  g.execute_host_checks = execute_host_checks;
  g.execute_service_checks = execute_service_checks;
  g.external_command_buffer_slots = external_command_buffer_slots;
  g.global_host_event_handler = to_str(global_host_event_handler);
  g.global_service_event_handler = to_str(global_service_event_handler);
  g.high_host_flap_threshold = high_host_flap_threshold;
  g.high_service_flap_threshold = high_service_flap_threshold;
  g.host_check_timeout = host_check_timeout;
  g.host_freshness_check_interval = host_freshness_check_interval;
  g.host_inter_check_delay_method = host_inter_check_delay_method;
  g.illegal_object_chars = to_str(illegal_object_chars);
  g.illegal_output_chars = to_str(illegal_output_chars);
  g.interval_length = interval_length;
  g.log_event_handlers = log_event_handlers;
  g.log_external_commands = log_external_commands;
  g.log_file = to_str(log_file);
  g.log_host_retries = log_host_retries;
  g.log_initial_states = log_initial_states;
  g.log_notifications = log_notifications;
  g.log_passive_checks = log_passive_checks;
  g.log_service_retries = log_service_retries;
  g.low_host_flap_threshold = low_host_flap_threshold;
  g.low_service_flap_threshold = low_service_flap_threshold;
  g.max_check_reaper_time = max_check_reaper_time;
  g.max_check_result_file_age = max_check_result_file_age;
  g.max_debug_file_size = max_debug_file_size;
  g.max_host_check_spread = max_host_check_spread;
  g.max_parallel_service_checks = max_parallel_service_checks;
  g.max_service_check_spread = max_service_check_spread;
  g.notification_timeout = notification_timeout;
  g.obsess_over_hosts = obsess_over_hosts;
  g.obsess_over_services = obsess_over_services;
  g.ochp_command = to_str(ochp_command);
  g.ochp_timeout = ochp_timeout;
  g.ocsp_command = to_str(ocsp_command);
  g.ocsp_timeout = ocsp_timeout;
  g.passive_host_checks_are_soft = passive_host_checks_are_soft;
  g.process_performance_data = process_performance_data;
  g.retained_contact_host_attribute_mask = retained_contact_host_attribute_mask;
  g.retained_contact_service_attribute_mask = retained_contact_service_attribute_mask;
  g.retained_host_attribute_mask = retained_host_attribute_mask;
  g.retained_process_host_attribute_mask = retained_process_host_attribute_mask;
  g.retain_state_information = retain_state_information;
  g.retention_scheduling_horizon = retention_scheduling_horizon;
  g.retention_update_interval = retention_update_interval;
  g.service_check_timeout = service_check_timeout;
  g.service_freshness_check_interval = service_freshness_check_interval;
  g.service_inter_check_delay_method = service_inter_check_delay_method;
  g.service_interleave_factor_method = service_interleave_factor_method;
  g.sleep_time = sleep_time;
  g.soft_state_dependencies = soft_state_dependencies;
  g.status_update_interval = status_update_interval;
  g.time_change_threshold = time_change_threshold;
  g.translate_passive_host_checks = translate_passive_host_checks;
  g.use_aggressive_host_checking = use_aggressive_host_checking;
  g.use_large_installation_tweaks = use_large_installation_tweaks;
  g.use_regexp_matches = use_regexp_matches;
  g.use_retained_program_state = use_retained_program_state;
  g.use_retained_scheduling_info = use_retained_scheduling_info;
  g.use_syslog = use_syslog;
  g.use_timezone = to_str(use_timezone);
  g.use_true_regexp_matching = use_true_regexp_matching;
  return (g);
}

/**
 *  Read configuration with new parser.
 *
 *  @parser[out] g        Fill global variable.
 *  @param[in]  filename  The file path to parse.
 *  @parse[in]  options   The options to use.
 *
 *  @return True on succes, otherwise false.
 */
static bool newparser_read_config(
              global& g,
              std::string const& filename,
              unsigned int options) {
  bool ret(false);
  try {
    init_macros();
    configuration::state config;
    configuration::parser p(options);
    p.parse(filename, config);
    configuration::applier::state::instance().apply(config);
    g = get_globals();
    clear_volatile_macros_r(get_global_macros());
    free_macrox_names();
    ret = true;
  }
  catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
  }
  return (ret);
}

/**
 *  Read configuration with old parser.
 *
 *  @parser[out] g        Fill global variable.
 *  @param[in]   filename The file path to parse.
 *  @parse[in]   options  The options to use.
 *
 *  @return True on succes, otherwise false.
 */
static bool oldparser_read_config(
              global& g,
              std::string const& filename,
              unsigned int options) {
  init_object_skiplists();
  init_macros();
  int ret(read_main_config_file(filename.c_str()));
  if (ret == OK) {
    ret = xodtemplate_read_config_data(
            filename.c_str(),
            options,
            false,
            false);
    if (ret == OK)
      g = get_globals();
  }
  clear_volatile_macros_r(get_global_macros());
  free_macrox_names();
  free_object_skiplists();
  return (ret == OK);
}

/**
 *  Release a specific object list.
 *
 *  @param[in] lst     The list to release.
 *  @param[in] deleter The deleter to use.
 */
template<typename T>
static void release(T* lst, void (*deleter)(void*)) {
  for (T* obj(lst); lst; lst = lst->next)
    deleter(obj);
}

/**
 *  Release all object list.
 *
 *  @param[in] g The object list.
 */
static void release_globals(global& g) {
  release(g.commands, &deleter::command);
  release(g.contacts, &deleter::contact);
  release(g.contactgroups, &deleter::contactgroup);
  release(g.hosts, &deleter::host);
  release(g.hostdependencies, &deleter::hostdependency);
  release(g.hostescalations, &deleter::hostescalation);
  release(g.hostgroups, &deleter::hostgroup);
  release(g.services, &deleter::service);
  release(g.servicedependencies, &deleter::servicedependency);
  release(g.serviceescalations, &deleter::serviceescalation);
  release(g.servicegroups, &deleter::servicegroup);
  release(g.timeperiods, &deleter::timeperiod);
}

/**
 *  Check if the configuration parser works properly.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  if (argc != 2)
    throw (engine_error() << "usage: " << argv[0] << " file.cfg");

  unsigned int options(configuration::parser::read_all);

  global oldcfg;
  if (!oldparser_read_config(oldcfg, argv[1], options))
    throw (engine_error() << "old parser can't parse " << argv[1]);

  global newcfg;
  if (!newparser_read_config(newcfg, argv[1], options))
    throw (engine_error() << "new parser can't parse " << argv[1]);

  bool ret(chkdiff(oldcfg, newcfg));
  release_globals(oldcfg);

  return (!ret);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
