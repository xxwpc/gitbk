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

#include "gitbk.h"



static int initProc( const std::vector<std::string> &args )
{
   if ( args.size( ) <= 1 )
   {
      std::cout << "gitbk init dir" << std::endl;
      return 1;
   }

   boost::filesystem::path path( args[1] );

   boost::filesystem::current_path();
   boost::filesystem::create_directory( path );
   boost::filesystem::create_directory( path / "objects" );
   boost::filesystem::create_directory( path / "backups" );
   boost::filesystem::create_directory( path / "tmp" );

   return 0;
}


static SubCmd initCmd( &g_gitbkCmds, "init", initProc );
