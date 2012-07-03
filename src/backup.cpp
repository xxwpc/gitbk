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
#include "mthread.h"
#include "GitbkFs.h"
#include "HashId.h"
#include "HashSet.h"
#include "path.h"
#include "GbFile.h"
#include "DirItr.h"



static boost::shared_mutex        *hash_mutex;


static int backup_mem( const void *buf, size_t size, HashId &id )
{
   int succ;

   {
      unsigned char r[32];
      SHA256( reinterpret_cast<const unsigned char *>(buf), size, r );
      id.setSha256( r );
   }

   {
      boost::shared_lock<boost::shared_mutex> lock( *hash_mutex );
      succ = g_hash_set->find( id ) != nullptr;
   }

   if ( !succ )
   {
      OutBz2File tmpFile;

      tmpFile.write( buf, size );
      tmpFile.close( );

      {
         boost::unique_lock<boost::shared_mutex> lock( *hash_mutex );

         succ = g_hash_set->find( id ) != nullptr;

         if ( !succ && (succ=tmpFile.rename( id ) ) )
         {
            id.size = size;
            g_hash_set->insert( id );
         }
      }
   }

   return succ;
}



class BackupFs : public GitbkFs
{
public:
	BackupFs( const NodeAttr &na, const boost::filesystem::path &path, const NodeAttr *oldAttr )
      : GitbkFs( na, path )
	  , _oldAttr( oldAttr )
   { }

private:
   void onFile( ) override;
   void onSymlink( ) override;

   void onDirBegin( ) override;
   void onDirEnd( ) override;

   void setHash( const HashId &hash )
   {
      _attr.hash = hash;
      _success = true;
   }

   void setOldAttr( const NodeAttr *attr )
   {
      _oldAttr = attr;
   }

private:
   const NodeAttr         *_oldAttr;
   std::list<NodeAttr>     _subAttrList;
};



void BackupFs::onSymlink( )
{
   char buf[PATH_MAX];
   auto n = readlink( _path.c_str( ), buf, sizeof(buf)-1 );
   if ( n < 0 )
      return;

   auto succ = backup_mem( buf, n, _attr.hash );
   if ( succ == 0 )
      return;

   _success = true;
}



void BackupFs::onFile( )
{
   // 1.
   if ( ( _oldAttr != nullptr ) && S_ISREG(_oldAttr->mode) )
   {
      if ( _attr.size == g_hash_set->find(_oldAttr->hash)->size )
   	     if ( _attr.mtime == _oldAttr->mtime )
   	     {
            _attr.hash = _oldAttr->hash;
            _success = true;
            return;
   	     }
   }

   // 2.
   boost::spirit::classic::file_iterator<char> fi( _path.string() );

   if ( !fi )
      return;

   const char *pb = &*fi;
   const char *pe = &*fi.make_end( );

   auto succ = backup_mem( pb, pe-pb, _attr.hash );

   if ( succ == 0 )
      return;

   _success = true;
}



void BackupFs::onDirBegin( )
{
   // 1.
   if ( ( _oldAttr != nullptr ) && ( S_ISDIR(_oldAttr->mode)) )
   {
      NodeAttr  attr;
      InBz2File in( _oldAttr->hash );

      while ( in.parseNodeAttr(&attr) )
         _subAttrList.push_back( attr );
   }


   auto old_itr = _subAttrList.begin( );
   auto old_end = _subAttrList.end( );
   const NodeAttr *old_ptr = nullptr;
   if ( old_itr != old_end )
      old_ptr = &*old_itr;

   // 2.
   DirectoryIterator dir_itr( _path.string( ) );

   while ( dir_itr.next( ) )
   {
      const NodeAttr &attr = dir_itr.attr( );

      BackupFs *node = new BackupFs( attr, _path/attr.name, nullptr );

      int cmpRst = -1;
      while ( old_ptr != nullptr )
      {
         cmpRst = strcmp( attr.name, old_ptr->name );
         if ( cmpRst > 0 )
         {
            if ( ++old_itr != old_end )
               old_ptr = &*old_itr;
            else
               old_ptr = nullptr;
            continue;
         }

         if ( cmpRst == 0 )
         {
            if ( S_ISREG( attr.mode ) )
            {
               if ( S_ISREG( old_ptr->mode )
                  && ( attr.size == g_hash_set->find(old_ptr->hash)->size )
                  && ( attr.mtime == old_ptr->mtime ) )
               {
                     node->setHash( old_ptr->hash );
               }
            }
            else
               node->setOldAttr( old_ptr );
         }

         break;
      }

      subPush( node );
   }
}



void BackupFs::onDirEnd( )
{
   char buf[512];
   std::string str;

   for ( auto ptr : _sub )
   {
      if ( ptr->success( ) )
      {
         ptr->toString( buf );
         str += buf;
      }

      delete ptr;
   }

   _success = backup_mem( str.c_str(), str.length(), _attr.hash );
}



static void save_result( const GitbkFs &rf )
{
   boost::gregorian::date today = boost::gregorian::day_clock::local_day( );

   auto path = path_get( PathType::BAK, rf.getPath().filename().c_str() );

   boost::filesystem::create_directory( path );

   path /= boost::gregorian::to_iso_string( today );

   auto fp = fopen( path.c_str(), "w" );
   if ( fp != NULL )
   {
      char buf[512];

      rf.toString( buf );
      fputs( buf, fp );
      fclose( fp );
   }
}



static bool getLastStore( std::string p, NodeAttr *attr )
{
   std::string path;

   auto dir = path_get( PathType::BAK, boost::filesystem::path(p).filename().c_str() );

   boost::system::error_code ec;

   bool succ = false;

   boost::filesystem::directory_iterator dir_itr( dir, ec );
   if ( ec == 0 )
   {
      boost::filesystem::directory_iterator dir_end;

      for ( ; dir_itr != dir_end; ++dir_itr )
      {
         auto &filepath = *dir_itr;
         if ( filepath.path( ).string( ) > path )
            path = filepath.path( ).string( );
      }

      attr->clear( );
      if ( !path.empty( ) )
      {
         boost::spirit::classic::file_iterator<char> fi( path );
         if ( fi )
            succ = attr->parse( &*fi );
      }
   }

   return succ;
}



int backupProc( const std::vector<std::string> &args )
{
   if ( args.size() < 3 )
   {
      std::cout << "gitbk backup src dest" << std::endl;
      return 1;
   }

   if ( !boost::filesystem::is_directory( args[1] ) )
   {
      std::cout << '\'' << args[1] << "' is not directory!" << std::endl;
      return 1;
   }

   path_init( args[2] );

   load_hash_set( );

   hash_mutex = new boost::shared_mutex;
   std::auto_ptr<boost::shared_mutex> mp( hash_mutex );

   std::string s = args[1];
   if ( s[s.size()-1] == '/' )
      s.resize( s.size() - 1 );

   NodeAttr na;
   if ( na.stat( s.c_str( ) ) )
   {
      NodeAttr oldAttr;
      auto succ = getLastStore( s, &oldAttr );
      BackupFs bd( na, s, succ?&oldAttr:nullptr );
      RunMt( &bd );
      if ( store_hash_set( false ) )
         save_result( bd );
   }
   else
   {
      std::cerr << "Can't stat " << s;
   }

   free_hash_set( );

   return 0;
}



static SubCmd initCmd( &g_gitbkCmds, "backup", backupProc );

