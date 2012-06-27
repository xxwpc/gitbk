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

#include "NodeAttr.h"



size_t NodeAttr::toString( char *buf ) const
{
   char *b = buf;

   buf += hash.toString( buf );
   buf += sprintf( buf, " %06o %s %s %ld %s\n", mode, user, group, mtime, name );

   return buf - b;
}



bool NodeAttr::parse( const char *attr )
{
   char *p;
   char c;

   hash.setString( attr );
   attr += 64;

   mode = 0;
   while ( (c=*++attr) != ' ' )
      mode = mode * 8 + c - '0';

   p = user;
   while ( (c=*++attr) != ' ' )
      *p++ = c;
   *p = 0;

   p = group;
   while ( (c=*++attr) != ' ' )
      *p++ = c;
   *p = 0;

   mtime = 0;
   while ( (c=*++attr) != ' ' )
      mtime = mtime * 10 + c - '0';

   p = name;
   while ( (c=*++attr) != '\n' )
      *p++ = c;
   *p = 0;

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

   char buf[512];

   mode   = st.st_mode;

   {
      struct passwd pwd;
      struct passwd *result;
      getpwuid_r( st.st_uid, &pwd, buf, 512, &result );
      if ( result == nullptr )
         return false;

      strcpy( user, pwd.pw_name );
   }

   {
      struct group grp;
      struct group *result;
      getgrgid_r( st.st_gid, &grp, buf, 512, &result );
      if ( result == nullptr )
         return false;

      strcpy( group, grp.gr_name );
   }

   mtime  = st.st_mtime;
   size   = st.st_size;

   return true;
}

