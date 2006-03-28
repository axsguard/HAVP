/***************************************************************************
                          params.cpp  -  description
                             -------------------
    begin                : So Feb 20 2005
    copyright            : (C) 2005 by Peter Sebald / Christian Hilgers
    email                : christian@hilgers.ag
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "default.h"
#include "params.h"
#include "utils.h"

#include <iostream>
#include <fstream>

map <string,string> Params::params;

void Params::SetDefaults()
{
    SetConfig("DISPLAYINITIALMESSAGES", "true");
    SetConfig("USER",		"havp");
    SetConfig("GROUP",		"havp");
    SetConfig("DAEMON",		"true");
    SetConfig("SERVERNUMBER",	"8");
    SetConfig("MAXSERVERS",	"150");
    SetConfig("PORT",		"8080");
    SetConfig("BIND_ADDRESS",	"");
    SetConfig("SOURCE_ADDRESS",	"");
    SetConfig("PARENTPROXY",	"");
    SetConfig("PARENTPORT",	"0");
    SetConfig("ACCESSLOG",	"/var/log/havp/access.log");
    SetConfig("ERRORLOG",	"/var/log/havp/havp.log");
    SetConfig("LOG_OKS",	"true");
    SetConfig("LOGLEVEL",	"1");
    SetConfig("MAXSCANSIZE",	"0");
    SetConfig("KEEPBACKBUFFER",	"200000");
    SetConfig("KEEPBACKTIME",	"5");
    SetConfig("TRICKLING",	"30");
    SetConfig("WHITELISTFIRST",	"true");
    SetConfig("WHITELIST",	"/usr/local/etc/havp/whitelist");
    SetConfig("BLACKLIST",	"/usr/local/etc/havp/blacklist");
    SetConfig("TEMPLATEPATH",	"/usr/local/etc/havp/templates/en");
    SetConfig("TEMPDIR",	"/var/tmp");
    SetConfig("SCANTEMPFILE",	"/var/tmp/havp/havp-XXXXXX");
    SetConfig("PIDFILE",	"/var/run/havp/havp.pid");
    SetConfig("TRANSPARENT",	"false");
    SetConfig("RANGE",		"false");
    SetConfig("FORWARDED_IP",	"false");
    SetConfig("STREAMUSERAGENT","");
    SetConfig("STREAMSCANSIZE",	"20000");
    SetConfig("DBRELOAD",	"60");
    SetConfig("FAILSCANERROR",	"true");
    SetConfig("SCANNERTIMEOUT",	"10");
//SCANNERS
    SetConfig("ENABLECLAMLIB","false");
        SetConfig("CLAMDBDIR","");
        SetConfig("CLAMBLOCKMAX","true");
        SetConfig("CLAMBLOCKENCRYPTED","false");
        SetConfig("CLAMMAXFILES","1000");
        SetConfig("CLAMMAXFILESIZE","10");
        SetConfig("CLAMMAXRECURSION","8");
        SetConfig("CLAMMAXRATIO","250");
    SetConfig("ENABLECLAMD","false");
	SetConfig("CLAMDSOCKET", "/tmp/clamd");
    SetConfig("ENABLEAVG","false");
        SetConfig("AVGSERVER","127.0.0.1");
        SetConfig("AVGPORT","55555");
    SetConfig("ENABLEAVESERVER","false");
        SetConfig("AVESOCKET","/var/run/aveserver");
    SetConfig("ENABLEFPROT","false");
        SetConfig("FPROTPORT","10200");
        SetConfig("FPROTSERVER","127.0.0.1");
    SetConfig("ENABLENOD32","false");
        SetConfig("NOD32SOCKET","/tmp/nod32d.sock");
    SetConfig("ENABLETROPHIE","false");
    SetConfig("ENABLESOPHIE","false");
	SetConfig("SOPHIESOCKET", "/var/run/sophie");
}

void Params::ReadConfig( string file )
{
    ifstream input( file.c_str() );

    if ( !input )
    {
        cerr << "Could not open config file: " << file << endl;
        cerr << "Exiting.." << endl;
        exit(1);
    }

    string::size_type Position;
    string line, key, val;

    while ( input )
    {
        getline( input, line );

        //Strip whitespace from beginning and end
        if ( (Position = line.find_first_not_of(" \t")) != string::npos )
        {
            line = line.substr(Position, (line.find_last_not_of(" \t", string::npos) - Position) + 1);
        }

        //Read next if nothing found
        if ( (Position == string::npos) || (line.size() == 0) ) continue;

        //Read next if commented
        if ( line.substr(0, 1) == "#" ) continue;

        //Find key and value
        if ( (Position = line.find_first_of(" \t")) != string::npos )
        {
            key = line.substr(0, Position);

            if ( key == "REMOVETHISLINE" )
            {
                cout << "Configuration is not edited!" << endl;
                cout << "You must delete REMOVETHISLINE option." << endl;
                cout << "Review the configuration carefully. :)" << endl;
                cout << "Exiting.." << endl;
                exit(1);
            }

            if ( (Position = line.find_first_not_of(" \t", Position + 1)) == string::npos )
            {
                cout << "Invalid Config Line: " << line << endl;
                cout << "Exiting.." << endl;
                exit(1);
            }

            val = line.substr( Position );

            Params::SetConfig( key, val );
        }
        else
        {
            cout << "Invalid Config Line: " << line << endl;
            cout << "Exiting.." << endl;
            exit(1);
        }
    }

    input.close();
}

void Params::SetConfig( string param, string value )
{
    string TempParams[] = {CONFIGPARAMS};
    bool ParamFound = false;

    param = UpperCase(param);

    for ( unsigned int i = 0; i < sizeof(TempParams)/sizeof(string); i++ )
    {
        if ( param == TempParams[i] )
        {
            ParamFound = true;
        }
    }

    if ( ParamFound )
    {
        if ( UpperCase(value) == "TRUE" || UpperCase(value) == "FALSE" )
        {
            value = UpperCase(value);
        }

        params[param] = value;
    }
    else
    {
        cout << "Unknown Config Parameter: " << param << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }
}

int Params::GetConfigInt( string param )
{
    return atoi( params[param].c_str() );
}

bool Params::GetConfigBool( string param )
{
    if ( params[param] == "TRUE" )
    {
        return true;
    }
    else
    {
        return false;
    }
}

string Params::GetConfigString( string param )
{
    return params[param];
}

void Params::ShowConfig( string cfgfile )
{
    cout << endl << "# Using HAVP config: " << cfgfile << endl << endl;
    typedef map<string,string>::const_iterator CI;
    for(CI p = params.begin(); p != params.end(); ++p)
    {
        cout << p->first << "=" << p->second << '\n';
    }
    cout << endl;
}

void Params::Usage()
{
    cout << endl << "Usage: havp [Options]" << endl << endl;
    cout << "HAVP Version " << VERSION << endl << endl;
    cout << "Possible options are:" << endl;
    cout << "--help | -h                         This pamphlet" << endl;
    cout << "--conf-file=FileName | -c Filename  Use this Config-File" << endl;
    cout << "--show-config | -s                  Show configuration HAVP is using" << endl << endl;
}

bool Params::SetParams( int argvT, char* argcT[] )
{
    string option, value;
    string::size_type i1, i2;

    string cfgfile = CONFIGFILE;
    bool showconf = false;

    SetDefaults();

    while ( --argvT )
    {
        value = *++argcT;
        i1 = value.find_first_not_of("-");

        //No GNU options
        if ( i1 == 1 )
        {
            option = value.substr(i1, 1);

            if ( option == "c" )
            {
                --argvT;

                if ( argvT == 0 )
                {
                    Usage();
                    return false;
                }
                value = *++argcT;
            }
            else if ( option == "s" )
            {
                showconf = true;
            }
            else
            {
                Usage();
                return false;
            }
        }
        //GNU options
        else if ( i1 == 2 )
        {
            if ( (i2 = value.find("=")) != string::npos )
            {
                option = value.substr(i1, i2 - i1);

                if ( value.size() > i2 + 1 )
                {
                    value = value.substr(i2 + 1);
                }
                else
                {
                    Usage();
                    return false;
                }
            }
            else
            {
                option = value.substr(i1);
                value = "";
            }
        }
        else
        {
            Usage();
            return false;
        }

        if ( option == "help" )
        {
            Usage();
            return false;
        }
        else if ( option == "show-config" )
        {
            showconf = true;
        }
        else if ( option == "conf-file" || option == "c" )
        {
            if (value == "")
            {
                Usage();
                return false;
            }

            cfgfile = value;
        }
        else if ( showconf == true )
        {
            //Nothing: prevent Usage
        } 
        else
        {
            Usage();
            return false;
        }
    }

    ReadConfig( cfgfile );

    if ( showconf == true )
    {
       ShowConfig( cfgfile );
       return false;
    }

    return true;
}

