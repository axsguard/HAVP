/***************************************************************************
                          connectiontobrowser.cpp  -  description
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

#include "connectiontobrowser.h"

//Prepare Header for Server
string ConnectionToBrowser::PrepareHeaderForServer()
{

string header ="";

  vector<string>::iterator it;

  for (it = tokens.begin(); it != tokens.end(); ++it)
  {
    //Change GET line
    if( ( it->find( "GET", 0) == 0) || ( it->find( "POST", 0 ) == 0 ) || ( it->find( "HEAD", 0 ) == 0 ) )
    {
      string temp = *it;
      int location = temp.find(" HTTP",0);
      int length = temp.length();
      Request = temp.replace( location, length, "" );

      
      temp = *it;
      transform(temp.begin(), temp.end(), temp.begin(), (int(*)(int))std::tolower );
      HostwithPort = "http://" + HostwithPort;
      location = temp.find(HostwithPort,0);
      length = HostwithPort.length();
      temp = it->replace( location, length, "" );

      //Use HTTP 1.0
      location = temp.find("HTTP",0);
      length = temp.length();
      temp = temp.replace( location, length, "HTTP/1.0" );
      header += temp + "\r\n";
      continue;
      //Change POST line
    } else if( it->find( "Proxy", 0 ) == 0 )
    {
     continue;
    } else if ( it->find( "Keep-Alive", 0 ) == 0 )
    {
     continue;
    } else if (( it->find( "Accept-Encoding", 0 ) && NOENCODING ) == 0 )
    {
     continue;
    } else if( it->find( "Connection", 0) == 0 )
     {
     continue;
     }

     header += *it;

   }//for


   header += "Connection: close\r\n";
   
   header += "Via: ";
   header += VERSION;
   header += "Havp\r\n";
   
   header += "\r\n";


return header;

}


//Get host and port
bool ConnectionToBrowser::GetHostAndPort( string *HostT, int *PortT )
{
unsigned int PositionPort;
string PortConvert;

  *HostT = Host = HostwithPort = "";

  vector<string>::iterator it;

  for (it = tokens.begin(); it != tokens.end(); ++it)
  {

    if( it->find( "Host:", 0 ) == 0 )
    {
      //Hostname to lowercase
      HostwithPort = it->substr(6, it->length()-8);
      transform(HostwithPort.begin(), HostwithPort.end(), HostwithPort.begin(), (int(*)(int))std::tolower );

       if( ( PositionPort = HostwithPort.rfind( ":", string::npos )) != string::npos )
        {
          *HostT = HostwithPort.substr(0, PositionPort );
           PortConvert = HostwithPort.substr( PositionPort+1, HostwithPort.length()-PositionPort );
           *PortT = atoi( PortConvert.c_str() );
           Host = *HostT;
          return true;
        }
      *PortT = 80;
      *HostT = Host = HostwithPort;
      return true;
    }
  }
  *HostT="";
  *PortT = 0;
  return false;
}


//Constructor
ConnectionToBrowser::ConnectionToBrowser(){
}

//Destructor
ConnectionToBrowser::~ConnectionToBrowser(){
}
