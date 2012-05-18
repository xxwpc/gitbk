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

#include <set>
#include <string>

#include "HashId.h"


class HashSet
{
public:
   HashSet( const std::string & );
   ~HashSet( );

   const HashId * find( const HashId & ) const;
   void insert( const HashId & );

private:
   bool load( );
   bool store( ) const;

private:
   std::set< HashId >   _set;
   std::string          _filename;

   std::set<HashId>::size_type   _old_size;
};



extern HashSet *g_hash_set;

extern void load_hash_set( );
