/*
** Copyright 2009-2013,2015,2018 Centreon
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

#include <cerrno>
#include <chrono>
#include <clocale>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <thread>

#include "com/centreon/broker/brokerrpc.hh"
#include "com/centreon/broker/config/applier/init.hh"
#include "com/centreon/broker/config/applier/logger.hh"
#include "com/centreon/broker/config/applier/state.hh"
#include "com/centreon/broker/config/logger.hh"
#include "com/centreon/broker/config/parser.hh"
#include "com/centreon/broker/config/state.hh"
#include "com/centreon/broker/log_v2.hh"
#include "com/centreon/broker/logging/logging.hh"
#include "com/centreon/broker/misc/diagnostic.hh"

using namespace com::centreon::broker;

/**************************************
 *                                     *
 *           Static Objects            *
 *                                     *
 **************************************/

// Main config file.
static std::vector<std::string> gl_mainconfigfiles;
static config::state gl_state;

/**
 *  Function called when updating configuration (when program receives
 *  SIGHUP).
 *
 *  @param[in] signum Signal number.
 */
static void hup_handler(int signum) {
  (void)signum;

  // Disable SIGHUP handling during handler execution.
  signal(SIGHUP, SIG_IGN);

  // Log message.
  log_v2::core()->info("main: configuration update requested");
  logging::config(logging::high) << "main: configuration update requested";

  try {
    // Parse configuration file.
    config::parser parsr;
    config::state conf{parsr.parse(gl_mainconfigfiles.front())};

    try {
      // Apply resulting configuration.
      config::applier::state::instance().apply(conf);

      gl_state = conf;
    }
    catch (std::exception const& e) {
      logging::error(logging::high)
          << "main: configuration update "
          << "could not succeed, reloading previous configuration: "
          << e.what();
      config::applier::state::instance().apply(gl_state);
    }
    catch (...) {
      logging::error(logging::high)
          << "main: configuration update "
          << "could not succeed, reloading previous configuration";
      config::applier::state::instance().apply(gl_state);
    }
  }
  catch (std::exception const& e) {
    logging::config(logging::high) << "main: configuration update failed: "
                                   << e.what();
  }
  catch (...) {
    logging::config(logging::high)
        << "main: configuration update failed: unknown exception";
  }

  // Reenable SIGHUP handler.
  signal(SIGHUP, &hup_handler);
}

/**
 *  Function called on termination request (when program receives
 *  SIGTERM).
 *
 *  @param[in] signum Unused.
 *  @param[in] info   Signal informations.
 *  @param[in] data   Unused.
 */
static void term_handler(int signum, siginfo_t* info, void* data) {
  (void)signum;
  (void)data;

  // Log message.
  logging::info(logging::high)
      << "main: termination request received by process id " << info->si_pid
      << " with real user id " << info->si_uid;

  // Unload endpoints.
  config::applier::deinit();

  exit(0);
}

/**************************************
 *                                     *
 *          Public Functions           *
 *                                     *
 **************************************/

/**
 *  @brief Program entry point.
 *
 *  main() is the first function called when the program starts.
 *
 *  @param[in] argc Number of arguments received on the command line.
 *  @param[in] argv Arguments received on the command line, stored in an
 *                  array.
 *
 *  @return 0 on normal termination, any other value on failure.
 */
int main(int argc, char* argv[]) {
  // Initialization.
  config::applier::init();
  std::string broker_name = "unknown";
  uint16_t default_port{51000};

  // Return value.
  int retval(0);

  try {
    // Check the command line.
    bool check(false);
    bool debug(false);
    bool diagnose(false);
    bool help(false);
    bool version(false);
    if (argc >= 2) {
      for (int i(1); i < argc; ++i)
        if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--check"))
          check = true;
        else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug"))
          debug = true;
        else if (!strcmp(argv[i], "-D") || !strcmp(argv[i], "--diagnose"))
          diagnose = true;
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
          help = true;
        else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version"))
          version = true;
        else
          gl_mainconfigfiles.push_back(argv[i]);
    }

    // Apply default configuration.
    config::state default_state;
    {
      // Logging object.
      config::logger default_log;
      default_log.config(!help);
      default_log.debug(debug);
      default_log.error(!help);
      default_log.info(true);
      logging::level level;
      if (debug)
        level = logging::low;
      else if (check)
        level = logging::medium;
      else
        level = logging::high;
      default_log.level(level);
      default_log.name((check || version) ? "stdout" : "stderr");
      default_log.type(config::logger::standard);

      // Configuration object.
      default_state.loggers().push_back(default_log);

      // Apply configuration.
      config::applier::logger::instance().apply(default_state.loggers());
    }

    // Check parameters requirements.
    if (diagnose) {
      if (gl_mainconfigfiles.empty()) {
        logging::error(logging::high)
            << "diagnostic: no configuration file provided: "
            << "DIAGNOSTIC FILE MIGHT NOT BE USEFUL";
      }
      misc::diagnostic diag;
      diag.generate(gl_mainconfigfiles);
    } else if (help) {
      logging::info(logging::high)
          << "USAGE: " << argv[0] << " [-c] [-d] [-D] [-h] [-v] [<configfile>]";
      logging::info(logging::high) << "  -c  Check configuration file.";
      logging::info(logging::high) << "  -d  Enable debug mode.";
      logging::info(logging::high) << "  -D  Generate a diagnostic file.";
      logging::info(logging::high) << "  -h  Print this help.";
      logging::info(logging::high) << "  -v  Print Centreon Broker version.";
      logging::info(logging::high) << "Centreon Broker "
                                   << CENTREON_BROKER_VERSION;
      logging::info(logging::high) << "Copyright 2009-2018 Centreon";
      logging::info(logging::high)
          << "License ASL 2.0 "
             "<http://www.apache.org/licenses/LICENSE-2.0>";
      retval = 0;
    } else if (version) {
      logging::info(logging::high) << "Centreon Broker "
                                   << CENTREON_BROKER_VERSION;
      retval = 0;
    } else if (gl_mainconfigfiles.empty()) {
      logging::error(logging::high)
          << "USAGE: " << argv[0] << " [-c] [-d] [-D] [-h] [-v] [<configfile>]";
      retval = 1;
    } else {
      logging::info(logging::medium) << "Centreon Broker "
                                     << CENTREON_BROKER_VERSION;
      logging::info(logging::medium) << "Copyright 2009-2018 Centreon";
      logging::info(logging::medium)
          << "License ASL 2.0 "
             "<http://www.apache.org/licenses/LICENSE-2.0>";

      // Reset locale.
      setlocale(LC_NUMERIC, "C");

      {
        // Parse configuration file.
        config::parser parsr;
        config::state conf{parsr.parse(gl_mainconfigfiles.front())};

        // Verification modifications.
        if (check) {
          // Loggers.
          for (std::list<config::logger>::iterator it(conf.loggers().begin()),
               end(conf.loggers().end());
               it != end;
               ++it)
            it->types(0);
          conf.loggers().push_back(default_state.loggers().front());
        }

        // Add debug output if in debug mode.
        if (debug)
          conf.loggers().insert(conf.loggers().end(),
                                default_state.loggers().begin(),
                                default_state.loggers().end());

        // Apply resulting configuration totally or partially.
        config::applier::state::instance().apply(conf, !check);
        std::string err;
        broker_name = conf.broker_name();
        if (!log_v2::instance().load(
                 "/etc/centreon-broker/log-config.json", broker_name, err))
          logging::error(logging::low) << err;
        gl_state = conf;
      }

      // Set configuration update handler.
      if (signal(SIGHUP, hup_handler) == SIG_ERR) {
        char const* err(strerror(errno));
        logging::info(logging::high)
            << "main: could not register configuration update handler: " << err;
      }

      // Init signal handler.
      struct sigaction sigterm_act;
      memset(&sigterm_act, 0, sizeof(sigterm_act));
      sigterm_act.sa_sigaction = &term_handler;
      sigterm_act.sa_flags = SA_SIGINFO | SA_RESETHAND;

      // Set termination handler.
      if (sigaction(SIGTERM, &sigterm_act, nullptr) < 0)
        logging::info(logging::high)
            << "main: could not register termination handler";

      if (gl_state.rpc_port() == 0)
        default_port += gl_state.broker_id();
      else
        default_port = gl_state.rpc_port();
      std::unique_ptr<brokerrpc, std::function<void(brokerrpc*)> > rpc(
          new brokerrpc("0.0.0.0", default_port, broker_name),
          [](brokerrpc* rpc) {
            rpc->shutdown();
            delete rpc;
          });

      // Launch event loop.
      if (!check)
        for (;;) {
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
      else
        retval = EXIT_SUCCESS;
    }
  }
  // Standard exception.
  catch (std::exception const& e) {
    logging::error(logging::high) << e.what();
    std::cout << "Error: " << e.what() << "\n";
    retval = EXIT_FAILURE;
  }
  // Unknown exception.
  catch (...) {
    logging::error(logging::high) << "main: unknown error, aborting execution";
    std::cout << "Fatal error\n";
    retval = EXIT_FAILURE;
  }

  // Unload endpoints.
  config::applier::deinit();

  return retval;
}
