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



static char cur_path[PATH_MAX];

static const char *filename;


void findDir( const NodeAttr &attr )
{
   char *pend = cur_path + strlen(cur_path);
   *pend = '/';
   ++pend;

   InBz2File in( attr.hash );

   NodeAttr na;

   while ( in.parseNodeAttr( &na ) )
   {
      if ( strcmp( na.name, filename ) == 0 )
      {
         strcpy( pend, na.name );
         std::cout << cur_path << std::endl;
      }

      if ( S_ISDIR(na.mode) )
      {
         strcpy( pend, na.name );
         findDir( na );
      }
   }
}



static void findEntry( const std::string &path )
{
   InputFile in( path );

   NodeAttr na;

   if ( in.parseNodeAttr( &na ) )
   {
      strcpy( cur_path, path.c_str() );

      if ( strcmp( na.name, filename ) == 0 )
         std::cout << cur_path << std::endl;

      if ( S_ISDIR( na.mode ) )
         findDir( na );
   }
}



static void findAll( const std::string &path )
{
   if ( boost::filesystem::is_regular_file( path ) )
   {
      findEntry( path );
      return;
   }

   boost::filesystem::recursive_directory_iterator itr( path );
   boost::filesystem::recursive_directory_iterator end;

   NodeAttr na;

   for ( ; itr != end; ++itr )
   {
      if ( !boost::filesystem::is_regular_file( itr->status( ) ) )
         continue;

      findEntry( itr->path( ).string( ) );
   }
}



int findProc( const std::vector<std::string> &args )
{
   if ( args.size() < 3 )
   {
      std::cout << "gitbk find <repo path> filename" << std::endl;
      return 1;
   }

   filename = args[2].c_str( );

   path_init( args[1] );

   findAll( args[1] );

   return 0;
}



static SubCmd initCmd( &g_gitbkCmds, "find", findProc );

