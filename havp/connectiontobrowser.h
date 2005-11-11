/***************************************************************************
                          connectiontobrowser.h  -  description
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

#ifndef CONNECTIONTOBROWSER_H
#define CONNECTIONTOBROWSER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "httphandler.h"
#include "logfile.h"
#include "params.h"
#include "default.h"

#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <stdarg.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std; 

class ConnectionToBrowser : public HTTPHandler  {

private:

map <string,string> URLRewrite;

string Request;

string Host;

string IP;

int Port;

vector <string> Methods;

string RequestType;

bool AnalyseHeaderLine( string *RequestT );

bool GetHostAndPortOfRequest(string *RequestT);

bool GetHostAndPortOfHostLine( string *HostLineT );

public:

string PrepareHeaderForServer();

string GetIP ();

const char *GetHost();

const string GetRequest();

const char *GetCompleteRequest();

const string GetRequestType();

int GetPort();

bool RewriteHost();

void ClearVars();

	ConnectionToBrowser();
	~ConnectionToBrowser();
};

#endif
