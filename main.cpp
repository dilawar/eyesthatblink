/**
 *       Filename:  main.cpp
 *
 *    Description:  Entry point of the application.
 *
 *        Version:  0.0.1
 *        Created:  2017-04-16
 *       Revision:  none
 *
 *         Author:  Dilawar Singh <dilawars@ncbs.res.in>
 *   Organization:  NCBS Bangalore
 *
 *        License:  GNU GPL2
 */

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>

#include "config.h"
#include "core/globals.h"
#include "core/helpers.h"
#include "core/main_loop.h"
#include "ui/ui_unix.h"

#include "plog/Log.h"

using namespace std;

namespace po = boost::program_options;

extern unique_ptr<ConfigManager> pConfigManager_;

/**
 * @function main
 */
int main(int argc, char* argv[])
{
    plog::init(plog::debug);

    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")(
        "configfile,c", po::value<string>(), "filepath of config file")(
        "datadir,d", po::value<string>(),
        "directory containing application data.");

    po::store(po::parse_command_line(argc, argv, desc),
              pConfigManager_->getCmdArgs());
    po::notify(pConfigManager_->getCmdArgs());

    if (pConfigManager_->getCmdArgs().count("help")) {
        cout << desc << endl;
        return 0;
    }

    LOG_INFO << "Starting application ... " << endl;

    init_camera();
    unix_ui();
    return 0;
}

