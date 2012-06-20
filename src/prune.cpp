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
#include "SubCmd.h"
#include "gitbk.h"
#include "path.h"
#include "HashSet.h"
#include "NodeAttr.h"
#include "GbFile.h"



static void delete_file( )
{
   auto obj_path = path_get( PathType::OBJ, nullptr );
   unsigned l = obj_path.string( ).length( );

   boost::filesystem::recursive_directory_iterator itr( obj_path );
   boost::filesystem::recursive_directory_iterator end;

   NodeAttr na;

   for ( ; itr != end; ++itr )
   {
      auto &path = itr->path( ).string( );
      if ( ( path.length( ) - l ) != 67 )
         continue;

      const char *cstr = path.c_str( );
      na.hash.hash[0] = cstr[l+1];
      na.hash.hash[1] = cstr[l+2];
      na.hash.hash[2] = cstr[l+4];
      na.hash.hash[3] = cstr[l+5];
      memcpy( na.hash.hash + 4, cstr+l+7, 60 );

      if ( g_hash_set->find( na.hash ) == nullptr )
      {
         unlink( path.c_str( ) );
         std::cout << "delete " << *itr << std::endl;
      }
   }
}



static void recursive_dir( const HashId &id )
{
   InBz2File in( id );

   NodeAttr na;

   while ( in.parseNodeAttr( &na ) )
   {
      g_hash_set->insert( na.hash );
      if ( S_ISDIR(na.mode) )
         recursive_dir( na.hash );
   }
}



static void recordAllHash( )
{
   boost::filesystem::recursive_directory_iterator itr( path_get( PathType::BAK, nullptr ) );
   boost::filesystem::recursive_directory_iterator end;

   NodeAttr na;

   for ( ; itr != end; ++itr )
   {
      if ( !boost::filesystem::is_regular_file( itr->status( ) ) )
         continue;

      InputFile in( itr->path( ).string( ) );

      if ( in.parseNodeAttr( &na ) )
      {
         g_hash_set->insert( na.hash );
         if ( S_ISDIR(na.mode) )
            recursive_dir( na.hash );
      }
   }
}



int pruneProc( const std::vector<std::string> &args )
{
   if ( args.size() < 2 )
   {
      std::cout << "gitbk prune <repo path>" << std::endl;
      return 1;
   }

   path_init( args[1] );


   load_hash_set( false );

   recordAllHash( );

   if ( store_hash_set( false ) )
      delete_file( );

   free_hash_set( );

   return 0;
}


static SubCmd initCmd( &g_gitbkCmds, "prune", pruneProc );

