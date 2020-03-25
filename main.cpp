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

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "plog/Log.h"

#include "config.h"
#include "core/globals.h"
#include "core/helpers.h"
#include "core/main_loop.h"
#include "ui/ui_unix.h"


using namespace std;

namespace po = boost::program_options;

extern boost::program_options::variables_map vm_;

/**
 * @function main
 */
int main(int argc, char* argv[])
{
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<string>(), "filepath of config file")
        ("datadir,d", po::value<string>(), "directory containing application data.")
        ;

    po::store(po::parse_command_line(argc, argv, desc), vm_);
    po::notify(vm_);

    plog::init(plog::debug);

    if(vm_.count("help")) {
        cout << desc << endl;
        return 1;
    }

    init_camera();
    unix_ui(argc, argv);
    return 0;
}

