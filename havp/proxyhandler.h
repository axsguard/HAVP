/***************************************************************************
                          proxyhandler.h  -  description
                             -------------------
    begin                : So Feb 20 2005
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

#ifndef PROXYHANDLER_H
#define PROXYHANDLER_H

#include "connectiontobrowser.h"
#include "connectiontoserver.h"
#include "genericscanner.h"
#include "logfile.h"

#include <iostream>

using namespace std;


class ProxyHandler {

private:

bool HeaderSend;

string Host;
int Port;

ConnectionToBrowser ToBrowser;
ConnectionToServer ToServer;

bool ProxyMessage( int CommunicationAnswerT, GenericScanner *VirusScannerT );

int Communication( SocketHandler *ProxyServerT, GenericScanner *VirusScannerT);
 
public:

 bool Proxy ( SocketHandler *ProxyServerT, GenericScanner *VirusScannerT);
 
 
	ProxyHandler();
	~ProxyHandler();
};

#endif