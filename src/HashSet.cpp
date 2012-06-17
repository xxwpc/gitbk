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

#include "HashSet.h"
#include "path.h"
#include "GbFile.h"



HashSet *g_hash_set = NULL;



HashSet::HashSet( )
{
}



HashSet::~HashSet( )
{
}



const HashId * HashSet::find( const HashId &id ) const
{
   auto itr = _set.find( id );
   if ( itr == _set.end() )
      return nullptr;

   return &*itr;
}



void HashSet::insert( const HashId &id )
{
   _set.insert( id );
}



bool HashSet::load( )
{
   InputFile   in( path_get( PathType::OBJ, "index" ).string( ) );
   HashId      id;

   while ( id.load( in ) )
      _set.insert( id );

   _old_size = _set.size( );

   return true;
}



bool HashSet::store( ) const
{
   OutputFile out;

   for ( const auto &id : _set )
      id.store( out );

   return out.rename( path_get( PathType::OBJ, "index" ).string( ) );
}



void load_hash_set( bool loadIndex )
{
   BOOST_ASSERT( g_hash_set == nullptr );

   g_hash_set = new HashSet( );
   if ( loadIndex )
      g_hash_set->load( );
}



bool store_hash_set( bool del )
{
   BOOST_ASSERT( g_hash_set != nullptr );

   bool rst = g_hash_set->store( );

   if ( del )
      delete g_hash_set;

   return rst;
}



void free_hash_set( )
{
   BOOST_ASSERT( g_hash_set != nullptr );
   delete g_hash_set;
}

