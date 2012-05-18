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

#include "Bz2File.h"
#include "path.h"


Bz2File::Bz2File( const HashId &id )
   : _wp( nullptr )
   , _rp( nullptr )
{
   char buf[67];
   id.toObjPath( buf );
   auto path = path_get( PathType::OBJ, buf );

   _fp = fopen( path.c_str(), "rb" );
   if ( _fp != nullptr )
   {
      int be;
      _rp = BZ2_bzReadOpen( &be, _fp, 0, 0, NULL, 0 );
   }
}



Bz2File::~Bz2File( )
{
   if ( _wp != nullptr )
      BZ2_bzWriteClose( nullptr, _wp, 0, nullptr, nullptr );

   if ( _rp != nullptr )
      BZ2_bzReadClose( nullptr, _rp );

   if ( _fp != nullptr )
      fclose( _fp );
}
