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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "logfile.h"
#include "default.h"
#include "sockethandler.h"
#include "genericscanner.h"
#include "clamlibscanner.h"
#include "proxyhandler.h"

#include <sys/wait.h>
//#include <sys/types.h>
//#include <unistd.h>


int MakeDeamon()
{
 pid_t deamon;
 if (( deamon = fork() ) < 0)
 { return (-1); //Parent error
 } else if ( deamon != 0) {
  exit (0); //Parent exit
 }

 //Child
 setsid();
 chdir("/tmp/");
 umask(0);
 return (0);
   
}
                         
int main(int argc, char *argv[])
{


SocketHandler ProxyServer;
ProxyHandler Proxy;
GenericScanner *VirusScanner = new (ClamLibScanner);


if (LogFile::InitLogFiles( ACCESSLOG, ERRORLOG ) == false){
  cout << "Could not create logfiles" << endl;
  exit (-1);
  }
  
cout << "Starting Havp Version:" << VERSION << endl;
LogFile::ErrorMessage("Starting Havp Version: %s\n", VERSION);


if( ProxyServer.CreateServer( PORT ) == false) {
  cout << "Could not create Server" << endl;
  LogFile::ErrorMessage("Could not create Server\n");
  exit (-1);
  }


if ( VirusScanner->InitDatabase( ) == false ) {
  cout << "Could not init scanner database" << endl;
  LogFile::ErrorMessage("Could init scanner database\n");
  exit (-1);
  }

#ifdef DEAMON
MakeDeamon();
#endif
  
#ifdef SERVERNUMBER
pid_t pid;
//Start Server
for( int i = 0; i < SERVERNUMBER; i++ )
{
 pid=fork();

 if (pid == 0)
 {
   //Child
   VirusScanner->PrepareScanning ( VirusScanner );
   Proxy.Proxy ( &ProxyServer, VirusScanner );
   exit (1);
 }
}
#endif

while(1){

VirusScanner->ReloadDatabase();

#ifdef SERVERNUMBER
int status;
pid = wait( &status);

pid=fork();

 if (pid == 0)
 {
   //Child
   VirusScanner->PrepareScanning ( VirusScanner );
   Proxy.Proxy ( &ProxyServer, VirusScanner );
   exit (1);
 }
#else 
VirusScanner->PrepareScanning ( VirusScanner );
Proxy.Proxy ( &ProxyServer, VirusScanner );
#endif
}
return 0;
}