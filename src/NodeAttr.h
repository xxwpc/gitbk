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

#pragma once

#include "HashId.h"



struct NodeAttr
{
   HashId          hash;
   char            name[256];
   mode_t          mode;
   uid_t           uid;
   gid_t           gid;
   time_t          mtime;
   std::uint64_t   size;

   NodeAttr( )
   {
      memset( this, 0, sizeof(*this) );
   }

   void clear( )
   {
      memset( this, 0, sizeof(*this) );
   }

   bool operator < ( const NodeAttr &o ) const
   {
      return strcmp( name, o.name ) < 0;
   }

   size_t toString( char *buf ) const;

   bool parse( const char * );
   bool stat( const boost::filesystem::path & );
};
