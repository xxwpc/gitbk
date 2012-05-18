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



class Bz2File
{
public:
   Bz2File( const HashId & );
   ~Bz2File( );

   size_t read( void *buf, size_t size )
   {
      return ::BZ2_bzRead( nullptr, _rp, buf, size );
   }

   size_t write( const void *, size_t );

private:
   BZFILE           *_wp;
   BZFILE           *_rp;
   FILE             *_fp;
};
