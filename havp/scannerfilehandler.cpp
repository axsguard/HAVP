/***************************************************************************
                          scannerfilehandler.cpp  -  description
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

#include "scannerfilehandler.h"

//Open and lock file which will be scanned
bool ScannerFileHandler::OpenAndLockFile( )
{

struct flock lock;
struct stat fstatpuff;

FileLength = 0;

   lock.l_type   = F_WRLCK; 
   lock.l_start  = 0;    // Byte-Offset
   lock.l_whence = SEEK_SET;      // SEEK_SET, SEEK_CUR oder SEEK_END
   lock.l_len    = MAXFILELOCKSIZE;    // number of bytes; 0 = EOF

  FileName = tempnam( SCANDIRECTORY , "havp-");
          
   if ( (fd_scan = open(FileName.c_str(), O_RDWR|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
   {
    LogFile::ErrorMessage ("Could not open Scannerfile: %s\n", FileName.c_str() );
    return false;
   }

  write(fd_scan, "1", 1);

   // set-group-ID and group-execute
   if (fstat(fd_scan, &fstatpuff) < 0)
      LogFile::ErrorMessage ("Fstat Error\n");
   if (fchmod(fd_scan, (fstatpuff.st_mode & ~S_IXGRP) | S_ISGID) < 0)
      LogFile::ErrorMessage ("fchmod-Fehler\n");

  if ( fcntl(fd_scan, F_SETLK, &lock) < 0)
  {
    LogFile::ErrorMessage ("Could not lock Scannerfile: %s\n", FileName.c_str() );
    return -1;
  }

if (lseek(fd_scan, 0, SEEK_SET) == -1)
 LogFile::ErrorMessage ("Could not lseek Scannerfile: %s\n", FileName.c_str() );

return fd_scan;
}


//Unlock file
bool ScannerFileHandler::UnlockFile()
{
struct flock    lock;
   lock.l_type   = F_UNLCK; 
   lock.l_start  = 0;    // byte-offset (abhaengig von wie)
   lock.l_whence = SEEK_SET;
   lock.l_len    = 0;     // number of bytes; 0 = EOF

  //Partial unlock file
  if ( fcntl(fd_scan, F_SETLK, &lock) < 0)
  {
    LogFile::ErrorMessage ("Could not partial unlock Scannerfile: %s\n", FileName.c_str() );
    close (fd_scan);
    return false;
  }
 close (fd_scan);
 return true;
}


bool ScannerFileHandler::DeleteFile() {

 if ( unlink(FileName.c_str()) < 0 ){
    LogFile::ErrorMessage ("Could not unlink: %s\n", FileName.c_str() );
   return false;
   }

return true;
}

bool ScannerFileHandler::SetFileSize( unsigned long ContentLengthT )
{

 if ( lseek (fd_scan, ContentLengthT-1 , SEEK_SET ) < 0 )
 { return false; }
 if (  write ( fd_scan, "1", 1 ) < 0 )
 { return false; }

 if (lseek(fd_scan, 0, SEEK_SET) < 0)
 LogFile::ErrorMessage ("Could not lseek Scannerfile: %s\n", FileName.c_str() );

 
return true;

}

bool ScannerFileHandler::ExpandFile( char *dataT, int lengthT , bool unlockT)
{

struct flock lock;

FileLength = FileLength + lengthT;

   if ( write(fd_scan, dataT, lengthT) < 0 ) {
    LogFile::ErrorMessage ("Could not write: %s\n", FileName.c_str() );
      return false;
     }

if(unlockT == true )
{
   
   lock.l_type   = F_UNLCK; 
   lock.l_start  = 0;    // byte-offset
   lock.l_whence = SEEK_SET;
   lock.l_len    = FileLength;  // number of bytes; 0 = EOF

  //partly unlock
  if ( fcntl(fd_scan, F_SETLK, &lock) < 0)
  {
    LogFile::ErrorMessage ("Could not lock: %s\n", FileName.c_str() );
    return false;
  }
}
  
return true;
}


//Constructor
ScannerFileHandler::ScannerFileHandler(){
}

//Destructor
ScannerFileHandler::~ScannerFileHandler(){
}