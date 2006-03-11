/***************************************************************************
                          FProtScanner.h  -  description
                             -------------------
    begin                : Mit Jun 29 2005
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

#ifndef FPROTSCANNER_H
#define FPROTSCANNER_H

#include "scannerfilehandler.h"
#include "sockethandler.h"

using namespace std;

class FProtScanner : public ScannerFileHandler  {

private:

SocketHandler FProtSocket;

public:

bool InitDatabase();

bool ReloadDatabase();

void FreeDatabase();

int Scanning();


	FProtScanner();
	~FProtScanner();
};

#endif
