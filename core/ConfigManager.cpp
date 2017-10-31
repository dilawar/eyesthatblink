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
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/filesystem.hpp>

#include "ConfigManager.h"
#include "helpers.h"
#include "plog/Log.h"

#include "../config.h"

namespace boostfs = boost::filesystem;


ConfigManager::ConfigManager( )
{
    configFile_ = expand_user( CONFIG_FILE_PATH );

    configTree_.put( "global.fraction_eyelid_closed_time", 0.1f );
    configTree_.put( "global.icon_path", expand_user( ICONFILE_PATH ) );
    configTree_.put( "global.blink_rate_per_minute", 10 );
    configTree_.put( "global.user_has_small_eyes", false );
    configTree_.put( "global.user_wearing_glasses", false );
    configTree_.put( "global.show_user_face", false );

    // Check if config file exists. If yes then populate the map.
    if( boost::filesystem::exists( configFile_ ) )
    {
        // If parsing fails then delete the file.
        try
        {
            readConfigFile( );
        }
        catch( ... )
        {
            // delete the file.
            LOG_DEBUG << "Removing badly formatted config file " << configFile_;
            boost::filesystem::remove( configFile_ );
            writeConfigFile( );
        }
    }
    else
    {
        LOG_DEBUG << "Writing config file to " << configFile_;
        writeConfigFile( );
    }
}


ConfigManager::~ConfigManager( )
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns The fraction of time eyelid is closed.
 */
/* ----------------------------------------------------------------------------*/
double ConfigManager::getBlinkThreshold( void )
{
    return configTree_.get<double>( "global.fraction_eyelid_closed_time" );
}

double ConfigManager::getBlinkPerMinuteThreshold( )
{
    return configTree_.get<double>( "global.blink_rate_per_minute" );
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Given blink rate per minute, write it to config file.
 *
 * @Param blinkratePerMinute
 */
/* ----------------------------------------------------------------------------*/
void ConfigManager::setBlinkThreshold( double blinkratePerMinute )
{
    LOG_DEBUG << "Setting blink threshold to " << blinkratePerMinute << " blinks/minute";
    configTree_.put( "global.blink_rate_per_minute", blinkratePerMinute );

    // Blink threshold is fraction of time, eye lids are closed.
    configTree_.put( "global.fraction_eyelid_closed_time"
                     , blinkratePerMinute * AVG_BLINK_DURATION / 60000.0
                   );
    writeConfigFile( );
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Write to configuration file.
 */
/* ----------------------------------------------------------------------------*/
void ConfigManager::writeConfigFile( )
{
    LOG_DEBUG << "Writing to config file " << configFile_;
    boost::property_tree::write_ini( configFile_, configTree_ );
}

void ConfigManager::readConfigFile( )
{
    LOG_DEBUG << "Reading config file " << configFile_;
    boost::property_tree::read_ini( configFile_, configTree_ );
}

const string ConfigManager::getIconpath( )
{
    string iconPath = configTree_.get<string>( "global.icon_path" );

    if( iconPath.size( ) < 1 )
    {
        LOG_ERROR << "Empty icon file path";
        throw runtime_error( "Iconpath is empty" );
    }
    if( ! boost::filesystem::exists( iconPath ) )
    {
        // Try another path from config.h file. Installation path.
        iconPath = ICONFILE_PATH;
        if( boost::filesystem::exists( iconPath ) )
            return iconPath;

        throw runtime_error( "Icon file " + iconPath + " not found" );
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
const string ConfigManager::getCascadeFile( const string& cascadeName )
{
    LOG_DEBUG << "Searching for " << cascadeName;
    boostfs::path path = boostfs::path( CASCADE_INSTALL_DIR ) / boostfs::path( cascadeName );
    if( boostfs::exists( path ) )
        return path.string( );
    else
        LOG_ERROR << "Could not find cascade " << cascadeName << endl;

    throw runtime_error( "Cascade file " + cascadeName + " not found" );
}

