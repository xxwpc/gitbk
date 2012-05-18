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
#include "IGbStream.h"



static boost::shared_mutex        *hash_mutex;



static void sha256( const void *buf, size_t size, HashId *id )
{
   unsigned char     r[32];

   SHA256( reinterpret_cast<const unsigned char *>(buf), size, r );

   id->set( r );
}



static boost::filesystem::path store_object( const void *buf, size_t size )
{
   static boost::detail::atomic_count count( 0 );

   auto idx = ++count;

   auto s = boost::lexical_cast<std::string>( idx );

   auto tmp_path = path_get( PathType::TMP, s.c_str( ) );
   auto tmp_name = tmp_path.c_str( );

   FILE *fp = fopen( tmp_name, "wb" );
   if ( fp == NULL )
      tmp_path.clear( );

   else
   {
      int be;
      BZFILE *bf = BZ2_bzWriteOpen( &be, fp, 9, 0, 0 );
      BZ2_bzWrite( &be, bf, const_cast< void * >( buf ), (int)size );
      BZ2_bzWriteClose( &be, bf, 0,  NULL, NULL );
      fclose( fp );
   }

   return tmp_path;
}



static int rename_tmp_file( const boost::filesystem::path &tmp, const HashId &id )
{
   auto path = id.getStorePath( );

   auto p2 = path.parent_path( );
   auto p1 = p2.parent_path( );

   boost::filesystem::create_directory( p1 );
   boost::filesystem::create_directory( p2 );

   boost::system::error_code ec;
   boost::filesystem::rename( tmp, path );

   return ec == 0;
}



static int backup_mem( const void *buf, size_t size, HashId &id )
{
   int succ;

   sha256( buf, size, &id );

   {
      boost::shared_lock<boost::shared_mutex> lock( *hash_mutex );
      succ = g_hash_set->find( id ) != nullptr;
   }

   if ( !succ )
   {
      auto tmp_bz2 = store_object( buf, size );

      succ = !tmp_bz2.empty( );

      if ( succ )
      {
         boost::unique_lock<boost::shared_mutex> lock( *hash_mutex );

         succ = g_hash_set->find( id ) != nullptr;

         if ( !succ && (succ=rename_tmp_file(tmp_bz2,id)) )
         {
            id.size = size;
            g_hash_set->insert( id );
         }

         else
            boost::filesystem::remove( tmp_bz2 );
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

   const NodeAttr * findOldAttr( const NodeAttr & ) const;

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
      IGbStream igs( _oldAttr->hash );
      std::string l;
      while ( std::getline(igs, l) )
      {
         attr.parse( l.c_str() );
         _subAttrList.push_back( attr );
      }
   }

   // 2.
   boost::filesystem::directory_iterator dir_itr( _path );
   boost::filesystem::directory_iterator dir_end;

   NodeAttr na;

   for ( ; dir_itr != dir_end; ++dir_itr )
   {
      auto &entry = *dir_itr;
      auto &path = entry.path( );

      if ( na.stat( path ) )
      {
         const NodeAttr *attr = findOldAttr( na );
         GitbkFs *node = new BackupFs( na, path, attr );

         subPush( node );
      }
   }
}



void BackupFs::onDirEnd( )
{
   subSort( );

   char buf[512];
   std::string str;

   auto itr( _sub.begin( ) );
   auto end( _sub.end( ) );

   for ( ; itr != end; ++itr )
   {
      auto ptr = *itr;

      if ( ptr->success( ) )
      {
         ptr->toString( buf );
         str += buf;
      }

      delete ptr;
   }

   _success = backup_mem( str.c_str(), str.length(), _attr.hash );
}



const NodeAttr * BackupFs::findOldAttr( const NodeAttr &attr ) const
{
   for ( const auto &at : _subAttrList )
   {
      if ( strcmp(at.name, attr.name) == 0 )
         return &at;
   }

   return nullptr;
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
      std::cout << "bkfs backup src dest" << std::endl;
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
   if ( na.stat(s) )
   {
      NodeAttr oldAttr;
      auto succ = getLastStore( s, &oldAttr );
      BackupFs bd( na, s, succ?&oldAttr:nullptr );
      RunMt( &bd );
      save_result( bd );
   }
   else
   {
      std::cerr << "Can't stat " << s;
   }

   return 0;
}



static SubCmd initCmd( &g_gitbkCmds, "backup", backupProc );
