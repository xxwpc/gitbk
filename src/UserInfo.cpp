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


struct UserInfo
{
   unsigned long     id;
   char              name[UT_NAMESIZE];
};



static UserInfo *s_userArr;
static int s_userCount;


static UserInfo *s_groupArr;
static int s_groupCount;


static int cmp_id( const void *a, const void *b )
{
   unsigned long id_a = static_cast< const UserInfo * >( a )->id;
   unsigned long id_b = static_cast< const UserInfo * >( b )->id;

   if ( id_a < id_b )
      return -1;

   if ( id_a > id_b )
      return 1;

   return 0;
}



static void parse_file( UserInfo **arr, int *count, const char *file )
{
   std::ifstream is( file );
   std::string line;

   boost::regex re( "([^:]+):[^:]+:(\\d+):.*" );
   boost::smatch match;

   UserInfo   *a = nullptr;
   int         c = 0;
   int         m = 0;

   while ( std::getline( is, line ) )
   {
      if ( !boost::regex_match( line, match, re ) )
         continue;

      if ( c == m )
      {
         m += 32;
         a = static_cast< UserInfo * >( realloc( a, sizeof( UserInfo ) * m ) );
      }

      a[c].id = atoi( match.str( 2 ).c_str( ) );
      strcpy( a[c].name, match.str( 1 ).c_str( ) );

      ++c;
   }

   ::qsort( a, c, sizeof( UserInfo ), cmp_id );

   *arr = a;
   *count = c;
}



static const char * get_name( const UserInfo *arr, int count, unsigned long id )
{
   UserInfo k;
   k.id = id;
   UserInfo *p = static_cast< UserInfo * >( bsearch( &k, arr, count, sizeof(UserInfo), cmp_id ) );
   return p->name;
}



static unsigned long get_id( const UserInfo *arr, int count, const char *name )
{
   for ( int i = 0; i < count; ++i )
      if ( strcmp( arr[i].name, name ) == 0 )
         return arr[i].id;

   return static_cast< unsigned long >( -1 );
}



void user_info_init( )
{
   parse_file( &s_userArr, &s_userCount, "/etc/passwd" );
   
   parse_file( &s_groupArr, &s_groupCount, "/etc/group" );
}



const char * user_name( uid_t id )
{
   return get_name( s_userArr, s_userCount, static_cast< unsigned long >( id ) );
}



uid_t user_id( const char *name )
{
   return static_cast< uid_t >( get_id( s_userArr, s_userCount, name ) );
}



const char * group_name( gid_t id )
{
   return get_name( s_groupArr, s_groupCount, static_cast< unsigned long >( id ) );
}



gid_t group_id( const char *name )
{
   return static_cast< gid_t >( get_id( s_groupArr, s_groupCount, name ) );
}

