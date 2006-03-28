/***************************************************************************
                          clamdscanner.cpp  -  description
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

#include "clamdscanner.h"


bool ClamdScanner::InitDatabase()
{
    return true;
}


bool ClamdScanner::ReloadDatabase()
{
    return false;
}


string ClamdScanner::Scan( const char *FileName )
{
    int fd = open(FileName, O_RDONLY);

    if ( fd < 0 )
    {
        LogFile::ErrorMessage("Clamd: Could not open tempfile: %s\n", strerror(errno));
        ScannerAnswer = "2Could not open file to scan";
        return ScannerAnswer;
    }

    //Wait till file is set up for scanning
    while (read(fd, Ready, 1) < 0 && errno == EINTR);
    while (close(fd) < 0 && errno == EINTR);

    if ( CLAMDSocket.ConnectToSocket( Params::GetConfigString("CLAMDSOCKET"), 1 ) == false )
    {
        //Prevent log flooding, show error only once per minute
        if ( (LastError == 0) || (LastError + 60 < time(NULL)) )
        {
            LogFile::ErrorMessage("Clamd: Could not connect to scanner! Scanner down?\n");
            LastError = time(NULL);
        }

        ScannerAnswer = "2Could not connect to scanner socket";
        return ScannerAnswer;
    }

    //Construct command for scanner
    ScannerCmd = "SCAN ";
    ScannerCmd += FileName;
    ScannerCmd += "\n";

    //Send command
    if ( CLAMDSocket.Send( &ScannerCmd ) == false )
    {
        CLAMDSocket.Close();

        //Try to reconnect
        if ( CLAMDSocket.ConnectToSocket( Params::GetConfigString("CLAMDSOCKET"), 1 ) == false )
        {
            //Prevent log flooding, show error only once per minute
            if ( (LastError == 0) || (LastError + 60 < time(NULL)) )
            {
                LogFile::ErrorMessage("Clamd: Could not connect to scanner! Scanner down?\n");
                LastError = time(NULL);
            }

            ScannerAnswer = "2Could not connect to scanner socket";
            return ScannerAnswer;
        }

        if ( CLAMDSocket.Send( &ScannerCmd ) == false )
        {
            CLAMDSocket.Close();

            LogFile::ErrorMessage("Clamd: Could not write command to scanner\n");
            ScannerAnswer = "2Scanner connection failed";
            return ScannerAnswer;
        }
    }

    string Response;

    //Get response
    if ( CLAMDSocket.GetLine( &Response, "\n", 600 ) == false )
    {
        CLAMDSocket.Close();

        ScannerAnswer = "2Could not read from scanner socket";
        return ScannerAnswer;
    }
    
    //Connection will be closed
    CLAMDSocket.Close();

    //Clean?
    if ( Response.find(" OK") != string::npos )
    {
        ScannerAnswer = "0Clean";
        return ScannerAnswer;
    }

    string::size_type Position;

    //Virus?
    if ( (Position = Response.find(" FOUND")) != string::npos )
    {
        string::size_type PositionStart = Response.find(": ") + 2;

        ScannerAnswer = "1" + Response.substr( PositionStart, Position - PositionStart );
        return ScannerAnswer;
    }

    //Error?
    if ( (Position = Response.find(" ERROR")) != string::npos )
    {
        string::size_type PositionStart = Response.find(": ") + 2;

        ScannerAnswer = "2" + Response.substr( PositionStart, Position - PositionStart );
        return ScannerAnswer;
    }

    //Unknown answer..
    LogFile::ErrorMessage("Clamd: Unknown response from scanner: %s\n", Response.c_str());
    ScannerAnswer = "2Unknown response from scanner";
    return ScannerAnswer;
}


void ClamdScanner::FreeDatabase()
{
}


//Constructor
ClamdScanner::ClamdScanner()
{
    ScannerName = "Clamd Socket Scanner";

    LastError = 0;

    ScannerAnswer.reserve(100);
}


//Destructor
ClamdScanner::~ClamdScanner()
{
}

