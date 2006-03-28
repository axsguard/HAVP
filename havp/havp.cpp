/***************************************************************************
                          havp.cpp  -  description
                             -------------------
    begin                : Sa Feb 12 2005
    copyright            : (C) 2005 by Christian Hilgers
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
#include "logfile.h"
#include "helper.h"
#include "scannerhandler.h"
#include "sockethandler.h"
#include "proxyhandler.h"
#include "params.h"
#include "whitelist.h"

#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <iostream>

#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long) -1)
#endif

URLList Whitelist;
URLList Blacklist;
bool rereadall = false;
bool childrestart = false;
int LL = 0; //LogLevel

//Needs to be here so we can delete it from signal handler etc..
char TempFileName[MAXSCANTEMPFILELENGTH+1];
int fd_tempfile = -1;

int main(int argc, char *argv[])
{
    if ( Params::SetParams(argc,argv) == false ) exit(1);

    LL = Params::GetConfigInt("LOGLEVEL");

    if ( Params::GetConfigBool("DISPLAYINITIALMESSAGES") )
    {
        cout << "Starting HAVP Version: " << VERSION << endl;
    }

    //Test that some options are sane
    if ( Params::GetConfigInt("SERVERNUMBER") < 1 )
    {
        cout << "Invalid Config: SERVERNUMBER needs to be greater than 0" << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }
    if ( Params::GetConfigString("ACCESSLOG").substr(0,1) != "/" || Params::GetConfigString("ERRORLOG").substr(0,1) != "/" )
    {
        cout << "Invalid Config: Log paths need to be abolute" << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }
    if ( Params::GetConfigString("SCANTEMPFILE").find("XXXXXX") == string::npos )
    {
        cout << "Invalid Config: SCANTEMPFILE must contain string \"XXXXXX\"" << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }
    if ( Params::GetConfigInt("MAXSERVERS") > 500 )
    {
        cout << "Note: MAXSERVERS is unusually high! You are sure you want this?" << endl;
    }
    if ( Params::GetConfigString("BIND_ADDRESS") == "NULL" ) Params::SetConfig("BIND_ADDRESS","");
    if ( Params::GetConfigString("BIND_ADDRESS") != "" )
    {
        if ( inet_addr( Params::GetConfigString("BIND_ADDRESS").c_str() ) == INADDR_NONE )
        {
            cout << "Invalid Config: Invalid BIND_ADDRESS" << endl;
            cout << "Exiting.." << endl;
            exit(1);
        }
    }
    if ( Params::GetConfigString("SOURCE_ADDRESS") == "NULL" ) Params::SetConfig("SOURCE_ADDRESS","");
    if ( Params::GetConfigString("SOURCE_ADDRESS") != "" )
    {
        if ( inet_addr( Params::GetConfigString("SOURCE_ADDRESS").c_str() ) == INADDR_NONE )
        {
            cout << "Invalid Config: Invalid SOURCE_ADDRESS" << endl;
            cout << "Exiting.." << endl;
            exit(1);
        }
    }
    if ( Params::GetConfigString("PARENTPROXY") != "" && Params::GetConfigInt("PARENTPORT") < 1 )
    {
        cout << "Invalid Config: Invalid PARENTPROXY/PARENTPORT" << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }

    //Install signal handlers
    if ( InstallSignal(0) < 0 )
    {
        cout << "Could not install signal handlers" << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }

    //Change user/group ID
    if ( ChangeUserAndGroup(Params::GetConfigString("USER"), Params::GetConfigString("GROUP")) == false )
    {
        cout << "Exiting.." << endl;
        exit(1);
    }

    if ( LogFile::InitLogFiles(Params::GetConfigString("ACCESSLOG").c_str(), Params::GetConfigString("ERRORLOG").c_str()) == false )
    {
        cout << "Could not open logfiles!" << endl;
        cout << "Invalid permissions? Maybe you need: chown " << Params::GetConfigString("USER") << " " << Params::GetConfigString("ACCESSLOG").substr(0, Params::GetConfigString("ACCESSLOG").rfind("/")) << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }

    LogFile::ErrorMessage("=== Starting HAVP Version: %s\n", VERSION);
    LogFile::ErrorMessage("Change to user %s\n", Params::GetConfigString("USER").c_str());
    LogFile::ErrorMessage("Change to group %s\n", Params::GetConfigString("GROUP").c_str());

    if ( Whitelist.CreateURLList( Params::GetConfigString("WHITELIST") ) == false )
    {
        cout << "Could not read whitelist!" << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }

    if ( Blacklist.CreateURLList( Params::GetConfigString("BLACKLIST") ) == false )
    {
        cout << "Could not read blacklist!" << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }

    if ( Params::GetConfigString("PARENTPROXY") != "" )
    {
        LogFile::ErrorMessage("Use parent proxy: %s %d\n", Params::GetConfigString("PARENTPROXY").c_str(), Params::GetConfigInt("PARENTPORT"));
    }

    if ( Params::GetConfigBool("TRANSPARENT") )
    {
        LogFile::ErrorMessage("Use transparent proxy mode\n");
    }

    SocketHandler ProxyServer;

    if ( ProxyServer.CreateServer( Params::GetConfigInt("PORT"), Params::GetConfigString("BIND_ADDRESS") ) == false )
    {
        cout << "Could not create server (already running?)" << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }

    //Test that mandatory locking works (this leaves file with EICAR data on disk)
    if ( HardLockTest() == false )
    {
        if (fd_tempfile > -1) while (close(fd_tempfile) < 0 && errno == EINTR);
        while (unlink(TempFileName) < 0 && (errno == EINTR || errno == EBUSY));
        cout << "Exiting.." << endl;
        exit(1);
    }

    //Tempfile descriptor not needed anymore
    while (close(fd_tempfile) < 0 && errno == EINTR);
    fd_tempfile = -1;

    ScannerHandler *Scanners = new ScannerHandler;

    //Initialize scanners
    if ( Scanners->InitScanners() == false )
    {
        cout << "One or more scanners failed to initialize!" << endl;
        cout << "Check errorlog for errors." << endl;
        cout << "Exiting.." << endl;
        while (close(fd_tempfile) < 0 && errno == EINTR);
        while (unlink(TempFileName) < 0 && (errno == EINTR || errno == EBUSY));
        exit(1);
    }

    //We can remove the hardlock/eicar testfile now
    while (unlink(TempFileName) < 0 && (errno == EINTR || errno == EBUSY));

    if ( Params::GetConfigBool("DAEMON") )
    {
        if ( MakeDaemon() < 0 )
        {
            cout << "Could not fork daemon" << endl;
            cout << "Exiting.." << endl;
            exit(1);
        }
    }

    pid_t pid = getpid();

    if ( WritePidFile( pid ) == false )
    {
        LogFile::ErrorMessage("Can not write to PIDFILE!\n");
    }

    LogFile::ErrorMessage("Process ID: %d\n", pid);

    int maxservers = Params::GetConfigInt("MAXSERVERS");
    int servernumber = Params::GetConfigInt("SERVERNUMBER");
    int dbreload = Params::GetConfigInt("DBRELOAD");

    int Instances = 0;
    int startchild = 0;
    int status;

    time_t LastRefresh = time(NULL);
    bool restartchilds = false;

    ProxyHandler Proxy;

    //Infinite Server Loop
    for(;;)
    {
        if ( rereadall ) //Signal Refresh
        {
            rereadall = false;

            LogFile::ErrorMessage("Signal HUP received, reloading scanners and lists\n");

            if ( Scanners->ReloadDatabases() == true )
            {
                restartchilds = true;
            }

            Whitelist.ReloadURLList( Params::GetConfigString("WHITELIST") );
            Blacklist.ReloadURLList( Params::GetConfigString("BLACKLIST") );

            LastRefresh = time(NULL);
        }
        else if ( time(NULL) > (LastRefresh + dbreload*60) ) //Time Refresh
        {
            if ( Scanners->ReloadDatabases() == true )
            {
                restartchilds = true;
            }

            LastRefresh = time(NULL);
        }

        //Send restart signal to childs if needed
        if ( restartchilds )
        {
            restartchilds = false;

            killpg(getpgid(0), SIGUSR1);
        }

        //Clean proxyhandler zombies
        while (waitpid(-1, &status, WNOHANG) > 0)
        {
            Instances--;
        }

        while ((startchild > 0) || (Instances < servernumber))
        {
            if ( (pid = fork()) < 0 ) //Fork Error
            {
                //Too many processes or out of memory?
                LogFile::ErrorMessage("Could not fork proxychild: %s\n", strerror(errno));

                //Lets hope the the causing error goes away soon
                sleep(10);
            }
            else if ( pid == 0 ) //Child
            {
                //Install ProxyHandler Signals
                if ( InstallSignal(1) < 0 )
                {
                    LogFile::ErrorMessage("Error installing ProxyHandler signals\n");
                    sleep(10);
                    exit(1);
                }

                //Create tempfile for scanning
                if ( Scanners->InitTempFile() == false )
                {
                    sleep(10);
                    exit(1);
                }

                //Fork scanner handler process
                if ( Scanners->CreateScanners( &ProxyServer ) == false )
                {
                    sleep(10);
                    exit(1);
                }

                //Start processing requests
                Proxy.Proxy( &ProxyServer, Scanners );

                exit(1);
            }
            else //Parent
            {
                if ( startchild > 0 ) startchild--;
                Instances++;
            }
        }

        //Do we need more proxy-processes?
        if ( Instances >= servernumber )
        {
            bool hangup = ProxyServer.CheckForData(0);
            sleep(1);

            if ( (hangup == true) && (Instances < maxservers) )
            {
                //Did a old process take care or is there still data? Create two if needed
                if ( ProxyServer.CheckForData(0) )
                {
                    if (LL>0) LogFile::ErrorMessage("All childs busy, spawning new (now: %d) - SERVERNUMBER might be too low\n", Instances+2);
                    startchild += 2;
                }
            }
        }
    }

    return 0;
}
