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
#include "SubCmd.h"
#include "UserInfo.h"



SubCmd *g_gitbkCmds;


static void usage( )
{
   std::cout << "gitbk 2012-05-18" << std::endl;
   std::cout << "bug report <xxw_pc@163.com>" << std::endl;
   std::cout << "LICENSE GPLv2" << std::endl;
}



int main( int argc, char *argv[] )
{
   umask( 077 );

   std::vector<std::string>   args;

#ifdef _DEBUG
   if ( argc == 1 )
   {
      std::cout << "input: " << std::flush;
      std::string cmdline;
      getline( std::cin, cmdline );
      args = boost::program_options::split_unix( cmdline );
   }
   else
#endif
   {
	   for ( int i = 1; i < argc; ++i )
		   args.push_back( argv[i] );
   }

   if ( args.size( ) == 0 )
   {
      usage( );
      return 0;
   }

   try
   {
      user_info_init( );
      return SubCmd::procCmd( g_gitbkCmds, args );
   }
   catch ( std::exception &e )
   {
      std::cerr << std::endl << "catch exception:" << std::endl << e.what() << std::endl;
      return -1;
   }
}



void verbose( const char *fmt, ... )
{
   static boost::mutex mt;

   boost::mutex::scoped_lock lock( mt );

   va_list ap;
   va_start( ap, fmt );
   vfprintf( stdout, fmt, ap );
   va_end( ap );
}



void verbose_err( const char *fmt, ... )
{
   static boost::mutex mt;

   boost::mutex::scoped_lock lock( mt );

   va_list ap;
   va_start( ap, fmt );
   vfprintf( stderr, fmt, ap );
   va_end( ap );
}
