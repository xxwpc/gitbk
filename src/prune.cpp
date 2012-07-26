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



static boost::detail::atomic_count count_found( 0 );
static boost::detail::atomic_count count_removed( 0 );
static boost::detail::atomic_count count_threads( 0 );



static void delete_file( int first, int last )
{
   auto obj_path = path_get( PathType::OBJ, nullptr );

   char path[PATH_MAX];

   HashId   id;

   for ( int i = first; i < last; ++i )
   {
      char *dir_end1 = path + sprintf( path, "%s/%02x/", obj_path.c_str( ), i );

      auto dir1 = ::opendir( path );
      if ( dir1 == nullptr )
         continue;

      int dir_fd1 = dirfd( dir1 );

      struct dirent  entry1, *pe1;
      int            un_rm1 = 0;

      while ( ( ::readdir_r( dir1, &entry1, &pe1 ) == 0 ) && ( pe1 != nullptr ) )
      {
         if ( !::isxdigit( entry1.d_name[0] ) || !::isxdigit( entry1.d_name[1] ) || ( entry1.d_name[2] != 0 ) )
         {
            ++un_rm1;
            continue;
         }

         strcpy( dir_end1, entry1.d_name );
         auto dir2 = ::opendir( path );
         if ( dir2 == nullptr )
         {
            ++un_rm1;
            continue;
         }

         int dir_fd2 = dirfd( dir2 );

         struct dirent  entry2, *pe2;
         int            un_rm2 = 0;

         while ( ( ::readdir_r( dir2, &entry2, &pe2 ) == 0 ) && ( pe2 != nullptr ) )
         {
            if ( !( entry2.d_type & DT_REG ) && ( strlen( entry2.d_name ) != 60 ) )
            {
               ++un_rm2;
               continue;
            }

            ++count_found;

            sprintf( id.hash, "%02x%s", i, entry1.d_name );
            memcpy( id.hash + 4, entry2.d_name, 60 );

            if ( g_hash_set->find( id ) != nullptr )
            {
               ++un_rm2;
               continue;
            }

            int rm = unlinkat( dir_fd2, entry2.d_name, 0 );
            int err = errno;


            if ( rm == 0 )
               ++count_removed;
            else
               ++un_rm2;

            if ( rm == 0 )
               continue;

            std::stringstream oss;
            oss << "\rrm ";
            oss.write( id.hash, 64 );
            if ( rm != 0 )
            {
               oss << ' ';
               oss << strerror( err );
            }

            oss << std::endl;

            verbose( oss.str().c_str() );
         }

         ::closedir( dir2 );

         if ( ( un_rm2 > 2 ) || unlinkat( dir_fd1, entry1.d_name, AT_REMOVEDIR ) )
            ++un_rm1;
      }

      ::closedir( dir1 );

      if ( un_rm1 <= 2 )
      {
         *dir_end1 = 0;
         ::rmdir( path );
      }
   }

   --count_threads;
}



static unsigned long long s_object_count;


static void recursive_record_dir( const HashId & );


static void record_tree_file( InputFile &in )
{
   NodeAttr na;

   while ( in.parseNodeAttr( &na ) )
   {
      if ( g_hash_set->find( na.hash ) != nullptr )
         continue;

      g_hash_set->insert( na.hash );
      ++s_object_count;

      if ( S_ISDIR(na.mode) )
         recursive_record_dir( na.hash );
   }
}


static void recursive_record_dir( const HashId &id )
{
   InBz2File in( id );

   NodeAttr na;

   record_tree_file( in );
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

      record_tree_file( in );
   }
}



static void sigroutine( int signo )
{
   if ( signo != SIGALRM )
      return;

   std::cout << "\rCounting objects " << s_object_count << std::flush;
   signal( SIGALRM, sigroutine );
}



static void remove_process( )
{
   long threads;

   do
   {
      threads = count_threads;
      verbose( "\rfound %ld, removed %ld", (long)count_found, (long)count_removed );
      fflush( stdout );
      ::usleep( 50000 );
   } while ( threads );
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

   {
      signal( SIGALRM, sigroutine );
      static const struct itimerval v =
      {
         { 0, 50000 },
         { 0, 50000 },
      };
      setitimer( ITIMER_REAL, &v, nullptr );
   }

   recordAllHash( );

   signal( SIGALRM, SIG_IGN );
   std::cout << "\rCounting objects " << s_object_count << std::endl;

   if ( store_hash_set( false ) )
   {
      const int n = sysconf( _SC_NPROCESSORS_ONLN );

      for ( int i = 0; i < n; ++i )
         ++count_threads;

      boost::thread_group tgroup;
      for ( int i = 0; i < n; ++i )
         tgroup.create_thread( boost::bind(&delete_file, 256*i/n, (256*(i+1)/n) ) );

      tgroup.create_thread( &remove_process );

      tgroup.join_all( );

      printf( "\n" );
   }

   free_hash_set( );

   return 0;
}


static SubCmd initCmd( &g_gitbkCmds, "prune", pruneProc );

