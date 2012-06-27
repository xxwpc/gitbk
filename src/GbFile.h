/*
 * COPYRIGHT (C) 2012 肖选文
 *
 * This file is part of gitbk.
 *
 * gitbk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gitbk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gitbk.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "HashId.h"
#include "NodeAttr.h"



class GbFile
{
protected:
   enum { BUF_SIZE = 128*1024 };

public:
   const std::string & getFilename( )
   {
      return _filename;
   }

protected:
   GbFile( const std::string &fn )
      : _rawBufSize( 0 )
      , _fd( -1 )
      , _filename( fn )
   { }

   virtual ~GbFile( )
   { }

   char           _rawBuf[BUF_SIZE];
   unsigned       _rawBufSize;

   int            _fd;
   std::string    _filename;
};


class OutputFile;


class InputFile
   : public GbFile
{
public:
   InputFile( const std::string & );
   ~InputFile( );

   size_t read( void *, size_t );
   std::string readString( );

   bool parseNodeAttr( NodeAttr * );

protected:
   virtual unsigned readBuf( void *, unsigned );

   unsigned       _rawBufPos;

   friend class OutputFile;
};




class InBz2File
   : public InputFile
{
public:
   InBz2File( const HashId & );
   ~InBz2File( );

protected:
   unsigned readBuf( void *, unsigned ) override;

   bz_stream      _bzStream;
   char           _bzBuf[BUF_SIZE];
};




class OutputFile
   : public GbFile
{
public:
   OutputFile( );
   OutputFile( const std::string & );

   ~OutputFile( );

   void write( const void *, unsigned );
   void write( InputFile & );

   void close( );

   bool rename( const std::string & );
   bool rename( const HashId & );

protected:
   virtual size_t writeBuf( const void *, size_t );
   virtual void onClose( );

   bool           _tmpFile;
};




class OutBz2File
   : public OutputFile
{
public:
   OutBz2File( );

protected:
   size_t writeBuf( const void *, size_t ) override;

   void onClose( ) override;

   bz_stream      _bzStream;
   char           _bzBuf[BUF_SIZE];
};

