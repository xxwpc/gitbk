/*
 * COPYRIGHT (C) 2012 \u8096\u9009\u6587
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

#include "GitbkFs.h"
#include "mthread.h"


/*
static size_t timeToStr( char *buf, time_t t )
{
   struct tm tm;
   localtime_r( &t, &tm );
   return strftime( buf, 24, "%Y-%m-%d %H:%M:%S ", &tm );
}
*/



GitbkFs::GitbkFs( const NodeAttr &attr, const boost::filesystem::path &path )
   : _path( path )
   , _attr( attr )
   , _cnt( 0 )
   , _first( true )
   , _success( false )
{
}



GitbkFs::~GitbkFs( )
{
}



/*
size_t RunFs::toString( char *buf ) const
{
   char *b = buf;

   buf += _attr.hash.toString( buf );
   buf += sprintf( buf, " %06o ", _mode );

   auto p1 = getpwuid( _uid );
   if ( p1 == NULL )
   {
      switch ( errno )
      {
      case EINTR:
         BOOST_ASSERT( false );
         break;

      case EMFILE:
         BOOST_ASSERT( false );
         break;

      case ENFILE:
         BOOST_ASSERT( false );
         break;

      case ENOMEM:
         BOOST_ASSERT( false );
         break;

      case ERANGE:
         BOOST_ASSERT( false );
         break;
      }
   }

   buf += sprintf( buf, "%s ", p1->pw_name );
   auto p2 = getgrgid( _gid );
   buf += sprintf( buf, "%s ", p2->gr_name );
   buf += timeToStr( buf, _mtime );
   buf += sprintf( buf, "%s\n", _path.filename().c_str() );

   return buf - b;
}
*/



bool GitbkFs::cmp( GitbkFs *pa, GitbkFs *pb )
{
   if ( pa->_success != pb->_success )
      return pa->_success > pb->_success;

   return pa->_path < pb->_path;
}



void GitbkFs::onSubComplete( )
{
   assert( _cnt > 0 );

   if ( --_cnt > 0 )
      return;

   GitbkFs *th = this;
   RunListAdd( &th, 1 );
}


/*
RunFsDir::RunFsDir( const boost::filesystem::path &path )
   : ScanFs( path )
   , _firstRun( true )
{
}



RunFsDir::~RunFsDir( )
{
}



void RunFsDir::run( )
{
   if ( _firstRun )
   {
      _firstRun = false;
      before( );
   }
   else
   {
      after( );
      complete( );
   }
}
*/


void GitbkFs::subPush( GitbkFs *rf )
{
   rf->setParent( this );
   _sub.push_back( rf );
   ++_cnt;
}



void GitbkFs::subSort( )
{
   std::sort( _sub.begin(), _sub.end(), cmp );
}



void GitbkFs::run( )
{
   if ( S_ISDIR(_attr.mode) )
   {
      if ( _first )
      {
         _first = false;
         onDirBegin( );
         auto siz = _sub.size( );
         if ( siz > 0 )
         {
            RunListAdd( (GitbkFs**)&_sub[0], siz );
            return;
         }
      }

      onDirEnd( );
   }
   else if ( S_ISREG(_attr.mode) )
      onFile( );

   else if ( S_ISLNK( _attr.mode ) )
      onSymlink( );

   onComplete( );

   _parent->onSubComplete( );
}



void GitbkFs::onComplete( )
{
   verbose( "%s\n", _path.c_str( ) );
}




void RunRoot::onFile( )
{
}



void RunRoot::onSymlink( )
{
}



void RunRoot::onDirBegin( )
{
}



void RunRoot::onDirEnd( )
{
}
