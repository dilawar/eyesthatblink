/***
 *       Filename:  ConfigManager.cpp
 *
 *    Description:  Config manager.
 *
 *        Version:  0.0.1
 *        Created:  2017-10-07
 *       Revision:  none
 *
 *         Author:  Dilawar Singh <dilawars@ncbs.res.in>
 *   Organization:  NCBS Bangalore
 *
 *        License:  GNU GPL2
 */

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

#include "ConfigManager.h"
#include "helpers.h"

#include "../config.h"
#include "plog/Log.h"


namespace bfs = boost::filesystem;
namespace po = boost::program_options;

ConfigManager::ConfigManager()
{
    config_file_ = bfs::path(expand_user(CONFIG_FILEPATH));
    initialize();

    configTree_.put("global.fraction_eyelid_closed_time", 0.1f);
    configTree_.put("global.icon_name", expand_user(ICON_FILENAME));
    configTree_.put("global.datadir", expand_user(APP_DATADIR));
    configTree_.put("global.blink_rate_per_minute", 10);
    configTree_.put("global.user_has_small_eyes", false);
    configTree_.put("global.user_wearing_glasses", false);
    configTree_.put("global.show_user_face", false);

    // Check if config file exists. If yes then populate the map.
    if (boost::filesystem::exists(config_file_)) {
        // If parsing fails then delete the file.
        try {
            readConfigFile();
        }
        catch (...) {
            // delete the file.
            LOG_WARNING << "Removing badly formatted config file "
                        << config_file_;
            bfs::remove(config_file_);
            writeConfigFile();
        }
    }
    else {
        LOG_DEBUG << "Writing config file to " << config_file_;
        writeConfigFile();
    }

    LOG_DEBUG << "ConfigManager is initialized" << endl;
}

void ConfigManager::initialize()
{
    if (!bfs::exists(config_file_)) {
        auto res = bfs::create_directories(config_file_.parent_path());
        LOG_INFO << "Created " << config_file_.parent_path() << "? " << res;
    }
}

ConfigManager::~ConfigManager() {}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns The fraction of time eyelid is closed.
 */
/* ----------------------------------------------------------------------------*/
double ConfigManager::getBlinkThreshold(void)
{
    return configTree_.get<double>("global.fraction_eyelid_closed_time");
}

double ConfigManager::getBlinkPerMinuteThreshold()
{
    return configTree_.get<double>("global.blink_rate_per_minute");
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Given blink rate per minute, write it to config file.
 *
 * @Param blinkratePerMinute
 */
/* ----------------------------------------------------------------------------*/
void ConfigManager::setBlinkThreshold(double blinkratePerMinute)
{
    LOG_INFO << "Setting blink threshold to " << blinkratePerMinute
             << " blinks/minute";
    configTree_.put("global.blink_rate_per_minute", blinkratePerMinute);

    // Blink threshold is fraction of time, eye lids are closed.
    configTree_.put("global.fraction_eyelid_closed_time",
                    blinkratePerMinute * AVG_BLINK_DURATION / 60000.0);
    writeConfigFile();
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Write to configuration file.
 */
/* ----------------------------------------------------------------------------*/
void ConfigManager::writeConfigFile()
{
    LOG_INFO << "Writing to config file " << config_file_;
    boost::property_tree::write_ini(config_file_.c_str(), configTree_);
}

void ConfigManager::readConfigFile()
{
    LOG_INFO << "Reading config file " << config_file_;
    boost::property_tree::read_ini(config_file_.c_str(), configTree_);
}

const bfs::path ConfigManager::getIconpath()
{
    string iconName = configTree_.get<string>("global.icon_name");

    if (iconName.size() < 1) {
        throw runtime_error("Name of icon is empty.");
    }
    auto iconPath = bfs::path(APP_DATADIR) / bfs::path(iconName);
    if (! boost::filesystem::exists(iconPath)) {

        // Try another path from config.h file. Installation path.
        if (boost::filesystem::exists(iconPath)) 
            return iconPath;

        throw runtime_error(iconPath.string() + " not found");
    }

    return iconPath;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Get the cascade file.
 *
 * First search in local ./cascades/ directory, if not found, search in
 * installation path CASCADE_INSTALL_DIR.
 *
 * @Param cascadeName
 *
 * @Returns
 */
/* ----------------------------------------------------------------------------*/
const bfs::path ConfigManager::getCascadeFile(const string& cascadeName)
{
    // If datadir is passed from commandline. Search the directory.
    LOG_INFO << "Searching for " << cascadeName;

    bfs::path path;

    if (cmdArgs_.count("datadir"))
       path = bfs::path(cmdArgs_["datadir"].as<string>())/ "cascades"/ bfs::path(cascadeName);
    else
       path = bfs::path(APP_DATADIR) / "cascades" / bfs::path(cascadeName);

    if (bfs::exists(path)) return path.string();

    throw runtime_error("Cascade file " + cascadeName +
                        " not found."
                        ". Searched path " +
                        path.string());
}

boost::program_options::variables_map& ConfigManager::getCmdArgs()
{
    return cmdArgs_;
}
