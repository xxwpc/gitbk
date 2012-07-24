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

#include "GitbkFs.h"
#include "mthread.h"




GitbkFs::GitbkFs( const NodeAttr &attr, const boost::filesystem::path &path )
   : _path( path )
   , _attr( attr )
   , _cnt( 0 )
   , _first( true )
   , _success( false )
{
}



GitbkFs::GitbkFs( const boost::filesystem::path &path )
   : _path( path )
   , _cnt( 0 )
   , _first( true )
   , _success( false )
{
   _attr.stat( path.c_str( ) );
}



GitbkFs::~GitbkFs( )
{
}



void GitbkFs::onSubComplete( )
{
   assert( _cnt > 0 );

   if ( --_cnt > 0 )
      return;

   GitbkFs *th = this;
   RunListAdd( &th, 1 );
}




void GitbkFs::subPush( GitbkFs *rf )
{
   rf->setParent( this );
   _sub.push_back( rf );

   if ( !rf->_success )
      ++_cnt;
}



void GitbkFs::run( )
{
   if ( S_ISDIR(_attr.mode) )
   {
      if ( _first )
      {
         _first = false;
         onDirBegin( );

         if ( _cnt > 0 )
         {
            RunListAdd( (GitbkFs**)&_sub[0], _sub.size( ) );
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
