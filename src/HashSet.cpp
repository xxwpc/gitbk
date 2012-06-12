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

#include "HashSet.h"
#include "path.h"
#include "GbFile.h"



HashSet *g_hash_set = NULL;



HashSet::HashSet( const std::string &fn )
   : _filename( fn )
{
   load( );
   _old_size = _set.size( );
}



HashSet::~HashSet( )
{
   if ( _old_size != _set.size( ) )
      store( );
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
   InputFile   in( _filename );
   HashId      id;

   while ( id.load( in ) )
      _set.insert( id );


/*
   FILE *fp = fopen( _filename.c_str( ), "rb" );
   if ( fp == NULL )
      return false;

   HashId id;

   while ( fread( &id, sizeof(id), 1, fp ) == 1 )
      _set.insert( id );

   fclose( fp );
*/
   return true;
}



bool HashSet::store( ) const
{
   OutputFile out( _filename );

   for ( const auto &id : _set )
      id.store( out );
/*
   std::ofstream os( _filename );

   if ( !os )
      return false;

   for ( const auto &id : _set )
      os << id;
*/
   return true;
}



void load_hash_set( )
{
   static bool load = false;

   if ( !load )
   {
      load = true;

      boost::filesystem::path path = path_get( PathType::OBJ, "index" );

      static std::auto_ptr<HashSet> phs( new HashSet( path.string() ) );
      g_hash_set = phs.get( );
   }
}
