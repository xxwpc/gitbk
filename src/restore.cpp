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
#include "path.h"
#include "GitbkFs.h"
#include "NodeAttr.h"
#include "mthread.h"
#include "Bz2File.h"
#include "IGbStream.h"



class RestoreFs
   : public GitbkFs
{
public:
   RestoreFs( const NodeAttr &attr, const boost::filesystem::path &path )
      : GitbkFs( attr, path / attr.name )
   { }

   void onFile( ) override;
   void onSymlink( ) override;
   void onDirBegin( ) override;
   void onDirEnd( ) override;
   void onComplete( ) override;
};



void RestoreFs::onFile( )
{
   Bz2File bf( _attr.hash );

   int fo = creat( _path.c_str(), S_IRWXU );
   if ( fo == -1 )
      return;

   char buf[4096];

   while ( true )
   {
      auto n = bf.read( buf, sizeof(buf) );
      if ( n == 0 )
         break;

      write( fo, buf, n );
   }

   close( fo );
}



void RestoreFs::onSymlink( )
{
   IGbStream igs( _attr.hash );
   std::string sym;
   std::getline( igs, sym );
   symlink( sym.c_str( ), _path.c_str( ) );
}



void RestoreFs::onDirBegin( )
{
   Bz2File bf( _attr.hash );

   std::string s;

   while ( true )
   {
      char buf[512];
      int n = bf.read( buf, sizeof(buf) );
      if ( n == 0 )
    	  break;

      s.append( buf, n );
   }

   NodeAttr attr;
   const char *p = s.c_str( );
   const char *e = p + s.length( );

   while ( p < e )
   {
      attr.parse( p );
      subPush( new RestoreFs( attr, _path ) );

      const char *pn = strchr( p, '\n' );
      if ( pn == nullptr )
    	  break;

      p = pn + 1;
   }

   boost::filesystem::create_directories( _path );
}



void RestoreFs::onDirEnd( )
{
}



void RestoreFs::onComplete( )
{
   chmod( _path.c_str( ), _attr.mode );

   utimbuf utb = { _attr.mtime, _attr.mtime };
   utime( _path.c_str( ), &utb );

   GitbkFs::onComplete( );
}



static GitbkFs * fromItem( const char *item, const std::string &path )
{
   BOOST_ASSERT( item != nullptr );

   NodeAttr node;
   node.parse( item );

   return new RestoreFs( node, path );
}



static int restoreProc( const SubCmd::Args &args )
{
   if ( args.size() != 3 )
   {
      printf( "gitfs restore tag dest\n" );
      return 1;
   }

   path_init( args[1] );

   boost::spirit::classic::file_iterator<char> fi( args[1] );

   if ( !fi )
      std::cout << "error" << std::endl;

   else
   {
      std::auto_ptr<GitbkFs> rfp( fromItem( &*fi, args[2] ) );
      RunMt( rfp.get( ) );
   }

   return 0;
}


static SubCmd restoreCmd( &g_gitbkCmds, "restore", restoreProc );
