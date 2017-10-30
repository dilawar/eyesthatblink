/***
 *       Filename:  ConfigManager.h
 *
 *    Description:  Configuration parameters.
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

#ifndef CONFIGMANAGER_H

#define CONFIGMANAGER_H

#include <string>
#include <map>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>

#include "../config.h"
#include "plog/Log.h"

using namespace std;

namespace boostfs = boost::filesystem;

class ConfigManager
{
    public:
        ConfigManager();
        ~ConfigManager();

        double getBlinkThreshold( void );

        double getBlinkPerMinuteThreshold( );

        void setBlinkThreshold( double thres );

        void writeConfigFile( );

        void readConfigFile( );

        const string getIconpath( );

        const string getCascadeFile( const string& cascadeName );


        template<typename T>
        void setValue( const string& key, const T value )
        {
            configTree_.put<T>( key, value );
        }

        template<typename T>
        const T getValue( const string& key )
        {
            return configTree_.get<T>( key );
        }

    private:

        // Config.
        string configFile_;
        boost::property_tree::ptree configTree_;
};


#endif /* end of include guard: CONFIGMANAGER_H */
