//# FilebufIO.cc: Class for IO on a  file descriptor
//# Copyright (C) 2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casacore/casa/aips.h>
#include <casacore/casa/IO/LargeIOFuncDef.h>
#include <casacore/casa/IO/FilebufIO.h>
#include <casacore/casa/Utilities/Assert.h>
#include <casacore/casa/Exceptions/Error.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>                     // needed for errno
#include <casacore/casa/string.h>               // needed for strerror


namespace casacore { //# NAMESPACE CASACORE - BEGIN

FilebufIO::FilebufIO()
: itsSeekable   (False),
  itsReadable   (False),
  itsWritable   (False),
  itsBuffer     (nullptr),
  itsBufSize    (0),
  itsFileDesc   (-1),
  itsDupFileDesc(-1),
  itsStream     (nullptr)
{}

FilebufIO::FilebufIO (int fd, uInt bufferSize)
: itsSeekable   (False),
  itsReadable   (False),
  itsWritable   (False),
  itsBuffer     (nullptr),
  itsBufSize    (0),
  itsFileDesc   (-1),
  itsDupFileDesc(-1),
  itsStream     (nullptr)
{
  attach (fd, bufferSize);
}

FilebufIO::~FilebufIO()
{
  detach();
}


void FilebufIO::attach (int fd, uInt bufferSize)
{
  //AlwaysAssert (itsFile == -1, AipsError);
  itsFileDesc = fd;
  int openFlags = fcntl(fd, F_GETFL, 0);
  std::string mode;
  switch(openFlags & O_ACCMODE)
  {
  case O_RDONLY:
    mode = "r";
    break;
  case O_WRONLY:
    mode = "w";
    break;
  default:
    mode = "r+";
  }
  itsDupFileDesc = dup(fd);
  itsStream = fdopen(itsDupFileDesc, mode.c_str());
  rewind(itsStream);
  if(bufferSize != 0)
  {
    itsBuffer.reset(new char[bufferSize]);
    itsBufSize = bufferSize;
    setvbuf(itsStream, itsBuffer.get(), _IOFBF, bufferSize);
  }
  fillRWFlags (fd);
  fillSeekable();
}

void FilebufIO::detach(Bool closeFile)
{
    if(itsStream != nullptr)
        fclose(itsStream);
    if(itsStream != nullptr && closeFile)
        close(itsFileDesc);
    itsFileDesc = -1;
    itsDupFileDesc = -1;
    itsStream   = nullptr;
}

void FilebufIO::fillRWFlags (int fd)
{
    itsReadable = False;
    itsWritable = False;
    int flags = fcntl (fd, F_GETFL);
    if ((flags & O_RDWR)  ==  O_RDWR) {
        itsReadable = True;
        itsWritable = True;
    } else if ((flags & O_WRONLY)  ==  O_WRONLY) {
        itsWritable = True;
    } else {
        itsReadable = True;
    }
}

void FilebufIO::fillSeekable()
{
    itsSeekable = (seek (0, ByteIO::Current)  >= 0);
}

void FilebufIO::flush()
{
    if(itsStream != nullptr)
        fflush(itsStream);
}

void FilebufIO::write (Int64 size, const void* buf)
{
    // Throw an exception if not writable.
    if (!itsWritable) {
        throw AipsError ("FilebufIO " + itsFileName
                         + "is not writable");
    }
    if(size > 0)
    {
        if (fwrite(buf, size, 1, itsStream) != 1) {
            int error = errno;
            throw AipsError ("FilebufIO: write error in "
                             + itsFileName + ": " + strerror(error));
        }
    }
}

Int64 FilebufIO::read (Int64 size, void* buf, Bool throwException)
{
  // Throw an exception if not readable.
  if (!itsReadable) {
    throw AipsError ("FilebufIO::read " + itsFileName
                     + " - is not readable");
  }
  size_t bytesRead = fread (buf, 1, size, itsStream);
  //int error = errno;
  if (ferror(itsStream) != 0 && throwException) { // Should never be executed
    throw AipsError ("FilebufIO::read " + itsFileName
                     + " - read returned a bad value");
  }
  else if (bytesRead < (size_t)size && throwException) {
      throw AipsError ("FilebufIO::read - incorrect number of bytes ("
                       + String::toString(bytesRead) + " out of "
                       + String::toString(size) + ") read for file "
                       + fileName());
  }

  return bytesRead;
}

Int64 FilebufIO::doSeek (Int64 offset, ByteIO::SeekOption dir)
{
    if(itsStream == nullptr)
        return 0;

    switch (dir) {
    case ByteIO::Begin:
        fseeko (itsStream, offset, SEEK_SET);
        break;
    case ByteIO::End:
        fseeko (itsStream, offset, SEEK_END);
        break;
    default:
        fseeko (itsStream, offset, SEEK_CUR);
	break;
    }
    return ftello (itsStream);
}

Int64 FilebufIO::length()
{
    // Get current position to be able to reposition.
    Int64 pos = seek (0, ByteIO::Current);
    // Seek to the end of the stream.
    // If it fails, we cannot seek and the current position is the length.
    Int64 len = seek (0, ByteIO::End);
    if (len < 0) {
	return pos;
    }
    // Reposition and return the length.
    seek (pos, ByteIO::Begin);
    return len;
}

   
Bool FilebufIO::isReadable() const
{
    return itsReadable;
}

Bool FilebufIO::isWritable() const
{
    return itsWritable;
}

Bool FilebufIO::isSeekable() const
{
    return itsSeekable;
}

String FilebufIO::fileName() const
{
    return "";
}

void FilebufIO::resync()
{
    rewind(itsStream);
}

} //# NAMESPACE CASACORE - END
