/***************************************************************************
                          sockethandler.h  -  description
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

#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include "default.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <string>

using namespace std;

class SocketHandler {

private:

struct sockaddr_un my_u_addr;
struct sockaddr_in l_addr;
struct sockaddr_in peer_addr;
struct in_addr ip_addr;
struct hostent *server;
struct timeval Timeout;

socklen_t addr_len;

fd_set checkfd, wset;

string source_address;

protected:

struct sockaddr_in my_s_addr;

public:

int sock_fd;

bool CreateServer( int portT, in_addr_t bind_addrT = INADDR_ANY );
bool CreateServer( int portT, string bind_addrT );
bool AcceptClient( SocketHandler *accept_socketT );
bool ConnectToServer();
bool ConnectToSocket( string SocketPath, int retry );
bool Send( string *sock_outT );
ssize_t Recv( string *sock_inT, bool sock_delT, int timeout );
bool RecvLength( string *sock_inT, ssize_t sock_lengthT );
bool GetLine( string *lineT, string separator, int timeout );
bool SetDomainAndPort( const string domainT, int portT );
bool CheckForData( int timeout );
void Close();

#ifdef SSLTUNNEL
int CheckForSSLData( int sockBrowser, int sockServer );
#endif

SocketHandler();
~SocketHandler();

};

#endif
