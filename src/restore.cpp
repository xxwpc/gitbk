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

#include "gitbk.h"
#include "SubCmd.h"
#include "path.h"
#include "GitbkFs.h"
#include "NodeAttr.h"
#include "mthread.h"
#include "GbFile.h"



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
   InBz2File bf( _attr.hash );

   OutputFile of( _path.string() );

   of.write( bf );
}



void RestoreFs::onSymlink( )
{
   char buf[PATH_MAX];
   InBz2File bf( _attr.hash );

   auto n = bf.read( buf, sizeof(buf)-1 );
   buf[n] = 0;

   symlink( buf, _path.c_str( ) );
}



void RestoreFs::onDirBegin( )
{
   InBz2File bf( _attr.hash );
   NodeAttr attr;

   while ( bf.parseNodeAttr(&attr) )
      subPush( new RestoreFs( attr, _path ) );

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
