#include "pch.h"

#include "DirItr.h"



DirectoryIterator::DirectoryIterator( const std::string &dir )
   : idx( -1 )
{
   const char *cdir = dir.c_str( );
   DIR *dirp = opendir( cdir );
   if ( dirp == nullptr )
      return;

   struct dirent entry;
   struct dirent *pe;

   char pathBuf[PATH_MAX];
   size_t len = dir.length( );
   memcpy( pathBuf, cdir, len );
   char *pend = pathBuf + len;
   *pend++ = '/';

   size_t count = 0;

   while ( true )
   {
      readdir_r( dirp, &entry, &pe );
      if ( pe == nullptr )
         break;

      if ( entry.d_name[0] == '.' )
      {
         if ( entry.d_name[1] == 0 )
            continue;

         if ( ( entry.d_name[1] == '.' ) && ( entry.d_name[2] == 0 ) )
            continue;
      }

      strcpy( pend, entry.d_name );

      entrys.resize( ++count );
      NodeAttr &attr = entrys.back( );

      if ( !attr.stat( pathBuf ) )
         entrys.resize( --count );
   }

   closedir( dirp );

   if ( count > 1 )
      ::qsort( &entrys[0], count, sizeof(NodeAttr), []( const void *a, const void *b)
      {
         const NodeAttr *pa = static_cast< const NodeAttr * >( a );
         const NodeAttr *pb = static_cast< const NodeAttr * >( b );
         return strcmp( pa->name, pb->name );
      });
}



bool DirectoryIterator::next( )
{
   if ( ( idx + 1 ) >= entrys.size( ) )
      return false;

   ++idx;
   return true;
}



RDirectoryIterator::RDirectoryIterator( const std::string &dir )
{
   push( dir.c_str( ) );
}



bool RDirectoryIterator::next( )
{
   if ( stack.size( ) == 0 )
      return false;

   if ( S_ISDIR( _lastMode ) )
      push( attr( ).name );

   if ( !top->next( ) )
      do
      {
         stack.pop_back( );
         if ( stack.size( ) == 0 )
            return false;

         _path = _path.parent_path( );
         top = &stack.back( );
      } while ( !top->next( ) );

   _lastMode = attr( ).mode;
   return true;
}



void RDirectoryIterator::push( const char *sub )
{
   _path /= sub;
   
   stack.push_back( DirectoryIterator(_path.string()) );

   top = &stack.back( );
}

