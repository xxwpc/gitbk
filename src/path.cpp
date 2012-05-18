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

#include "path.h"



static const char path_tmp[] = "tmp";
static const char path_obj[] = "objects";
static const char path_bak[] = "backups";


static boost::filesystem::path path_repo;



char *src_pos;
char *obj_pos;



void path_init( const std::string &repo )
{
   path_repo = repo;

   while ( true )
   {
      if ( boost::filesystem::exists( path_repo / path_bak ) )
         break;

      if ( path_repo.has_parent_path( ) )
         path_repo = path_repo.parent_path( );
      else
         break;
   }
}



boost::filesystem::path path_get( PathType type, const char *f )
{
   auto path = path_repo;
   const char *s;
   switch ( type )
   {
   case PathType::TMP:
      s = "tmp";
      break;

   case PathType::OBJ:
      s = "objects";
      break;

   case PathType::BAK:
      s = "backups";
      break;

   default:
      BOOST_ASSERT( false );
   }

   path /= s;

   if ( f != nullptr )
	   path /= f;

   return path;
}
