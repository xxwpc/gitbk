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

#include "NodeAttr.h"



static size_t timeToStr( char *buf, time_t t )
{
   struct tm tm;
   localtime_r( &t, &tm );
   return strftime( buf, 24, "%Y-%m-%d %H:%M:%S ", &tm );
}



static time_t timeFromStr( const char *s )
{
   struct tm tm;
   strptime( s, "%Y-%m-%d %H:%M:%S ", &tm );
   return mktime( &tm );
}



size_t NodeAttr::toString( char *buf ) const
{
   char *b = buf;

   char buffer[512];

   buf += hash.toString( buf );
   buf += sprintf( buf, " %06o ",  mode );

   {
      struct passwd pwd;
      struct passwd *result;
      getpwuid_r( uid, &pwd, buffer, 512, &result );
      if ( result == nullptr )
         return 0;
      buf += sprintf( buf, "%s ", pwd.pw_name );
   }

   {
      struct group grp;
      struct group *result;
      getgrgid_r( gid, &grp, buffer, 512, &result );
      if ( result == nullptr )
         return 0;

      buf += sprintf( buf, "%s ", grp.gr_name );
   }

   buf += timeToStr( buf, mtime );
   buf += sprintf( buf, "%s\n", name );

   return buf - b;
}



//
bool NodeAttr::parse( const char *attr )
{
   char buf[256];

   hash.setString( attr );
   attr += 65;

   sscanf( attr, "%o ", &mode );
   attr += 7;

   sscanf( attr, "%s ", buf );
   struct passwd *pw = getpwnam( buf );
   uid = pw->pw_uid;
   attr += strlen( buf ) + 1;

   sscanf( attr, "%s ", buf );
   struct group *gp = getgrnam( buf );
   gid = gp->gr_gid;
   attr += strlen( buf ) + 1;

   mtime = timeFromStr( attr );
   attr += 20;

   const char *e = strchr( attr, '\n' );
   if ( e == NULL )
      strcpy( name, attr );
   else
   {
      memcpy( name, attr, e - attr );
      name[e-attr] = 0;
   }

   return true;
}



bool NodeAttr::stat( const boost::filesystem::path &path )
{
   struct stat st;
   if ( ::lstat( path.c_str(), &st ) != 0 )
   {
      return false;
   }

   strcpy( name, path.filename().c_str() );

   mode   = st.st_mode;
   uid    = st.st_uid;
   gid    = st.st_gid;
   mtime  = st.st_mtime;
   size   = st.st_size;

   return true;
}
