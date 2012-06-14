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

#include "pch.h"

#include "path.h"
#include "GbFile.h"



InputFile::InputFile( const std::string &filename )
   : GbFile( filename )
   , _rawBufPos( 0 )
{
   _fd = open( _filename.c_str( ), 0 );
}



InputFile::~InputFile( )
{
   ::close( _fd );
}



size_t InputFile::read( void *buf, size_t size )
{
   unsigned remind = _rawBufSize - _rawBufPos;

   if ( remind >= size )
   {
      memcpy( buf, _rawBuf + _rawBufPos, size );
      _rawBufPos += size;
      return size;
   }

   if ( remind > 0 )
   {
      memcpy( buf, _rawBuf + _rawBufPos, remind );
      buf = static_cast< void * >( (char*)buf + remind );
      size -= remind;
   }

   _rawBufSize = 0;
   _rawBufPos  = 0;

   if ( size > sizeof(_rawBuf) )
      return readBuf( buf, size ) + remind;

   _rawBufSize = readBuf( _rawBuf, sizeof(_rawBuf) );
   unsigned n = std::min<size_t>( size, _rawBufSize );
   memcpy( buf, _rawBuf, n );
   _rawBufPos = n;

   return n + remind;
}



unsigned InputFile::readBuf( void *buf, unsigned size )
{
   auto s = ::read( _fd, buf, size );

   if ( s == static_cast< ssize_t >( -1 ) )
      return 0;

   return static_cast< unsigned >( s );
}



bool InputFile::parseNodeAttr( NodeAttr *na )
{
   unsigned size = _rawBufSize - _rawBufPos;
   char *p = static_cast<char *>( memchr(_rawBuf+_rawBufPos, '\n', size) );
   if ( p == nullptr )
   {
      memmove( _rawBuf, _rawBuf+_rawBufPos, size );
      _rawBufPos  = 0;
      _rawBufSize = size;
      size = readBuf( _rawBuf+size, sizeof(_rawBuf) - size );
      _rawBufSize += size;

      p = static_cast<char*>( memchr(_rawBuf+_rawBufPos, '\n', size) );
      if ( p == nullptr )
   	     return false;
   }

   auto ret = na->parse( _rawBuf + _rawBufPos );
   if ( ret )
      _rawBufPos = p - _rawBuf + 1;

   return ret;
}




InBz2File::InBz2File( const HashId &id )
   : InputFile( id.getStorePath( ).string( ) )
{
   memset( &_bzStream, 0, sizeof(_bzStream) );
   BZ2_bzDecompressInit( &_bzStream, 0, 0 );
}



InBz2File::~InBz2File( )
{
   BZ2_bzDecompressEnd( &_bzStream );
}



unsigned InBz2File::readBuf( void *buf, unsigned size )
{
   _bzStream.next_out  = static_cast< char * >( buf );
   _bzStream.avail_out = size;

   while ( _bzStream.avail_out > 0 )
   {
      if ( _bzStream.avail_in == 0 )
      {
         auto n = ::read( _fd, _bzBuf, sizeof(_bzBuf) );

         _bzStream.avail_in = n;
         _bzStream.next_in  = _bzBuf;
      }

      auto ret = BZ2_bzDecompress( &_bzStream );
      if ( ret == BZ_STREAM_END )
         break;

      if ( ret != BZ_OK )
         break;
   }

   return size - _bzStream.avail_out;
}



static std::string getTmpFile( )
{
   static boost::detail::atomic_count count( 0 );

   auto idx = ++count;

   auto s = boost::lexical_cast<std::string>( idx );

   auto tmp_path = path_get( PathType::TMP, s.c_str( ) );

   return tmp_path.string( );
}



OutputFile::OutputFile( const std::string &filename )
   : GbFile( filename )
   , _tmpFile( false )
{
   _fd = ::creat( _filename.c_str( ), 0 );
}



OutputFile::OutputFile( )
   : GbFile( getTmpFile( ) )
   , _tmpFile( true )
{
   _fd = ::creat( _filename.c_str(), S_IRUSR|S_IWUSR );
}



OutputFile::~OutputFile( )
{
   if ( _fd >= 0)
      close( );

   if ( _tmpFile )
      ::unlink( _filename.c_str() );
}



void OutputFile::write( const void *buf, unsigned size )
{
   if ( ( size + _rawBufSize ) <= sizeof(_rawBuf) )
   {
      memcpy( _rawBuf + _rawBufSize, buf, size );
      _rawBufSize += size;
      return;
   }

   if ( _rawBufSize > 0 )
   {
      unsigned siz = sizeof(_rawBuf) - _rawBufSize;
      memcpy( _rawBuf + _rawBufSize, buf, siz );
      buf = static_cast< void * >( (char*)buf + siz );
      size -= siz;

      writeBuf( _rawBuf, sizeof(_rawBuf) );
      _rawBufSize = 0;
   }

   if ( size > sizeof(_rawBuf) )
      writeBuf( buf, size );

   else
   {
      memcpy( _rawBuf, buf, size );
      _rawBufSize = size;
   }
}



void OutputFile::write( InputFile &in )
{
   if ( _rawBufSize > 0 )
   {
      writeBuf( _rawBuf, _rawBufSize );
      _rawBufSize = 0;
   }

   if ( in._rawBufSize > in._rawBufPos )
   {
      writeBuf( in._rawBuf, in._rawBufSize - in._rawBufPos );
      in._rawBufPos = in._rawBufSize;
   }

   while ( true )
   {
      auto n = in.readBuf( _rawBuf, sizeof(_rawBuf) );
      if ( n == 0 )
   	     break;

      writeBuf( _rawBuf, n );
   }
}



size_t OutputFile::writeBuf( const void *buf, size_t size )
{
   return ::write( _fd, buf, size );
}



bool OutputFile::rename( const std::string &path )
{
   close( );

   auto ret = ::rename( _filename.c_str(), path.c_str() ) == 0;

   if ( ret )
      _tmpFile = false;

   return ret;
}



bool OutputFile::rename( const HashId &id )
{
   auto path = id.getStorePath( );

   boost::filesystem::create_directories( path.parent_path( ) );

   return rename( path.string( ) );
}



void OutputFile::close( )
{
   if ( _fd < 0 )
      return;

   if ( _rawBufSize > 0 )
      writeBuf( _rawBuf, _rawBufSize );

   onClose( );
}



void OutputFile::onClose( )
{
   BOOST_ASSERT( _fd >= 0 );
   ::close( _fd );
   _fd = -1;
}



OutBz2File::OutBz2File( )
   : OutputFile( )
{
   memset( &_bzStream, 0, sizeof(_bzStream) );
   BZ2_bzCompressInit( &_bzStream, 9, 0, 0 );
   _bzStream.avail_out = sizeof( _bzBuf );
   _bzStream.next_out  = _bzBuf;
}



size_t OutBz2File::writeBuf( const void *buf, size_t size )
{
   _bzStream.next_in  = static_cast< char * >( const_cast<void*>(buf) );
   _bzStream.avail_in = size;

   while ( _bzStream.avail_in > 0 )
   {
      auto ret = BZ2_bzCompress( &_bzStream, BZ_RUN );
      if ( ret != BZ_RUN_OK )
         break;

      if ( _bzStream.avail_out == 0 )
      {
   	     ::write( _fd, _bzBuf, sizeof(_bzBuf) );
   	     _bzStream.avail_out = sizeof(_bzBuf);
   	     _bzStream.next_out  = _bzBuf;
      }
   }

   return size - _bzStream.avail_in;
}



void OutBz2File::onClose( )
{
   int ret;
   do
   {
      ret = BZ2_bzCompress( &_bzStream, BZ_FINISH );

      if ( _bzStream.avail_out < sizeof(_bzBuf) )
      {
         ::write( _fd, _bzBuf, sizeof(_bzBuf) - _bzStream.avail_out );
         _bzStream.avail_out = sizeof( _bzBuf );
         _bzStream.next_out  = _bzBuf;
      }
   } while ( ret == BZ_FINISH_OK );

   BZ2_bzCompressEnd( &_bzStream );

   OutputFile::onClose( );
}
