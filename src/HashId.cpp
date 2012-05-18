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
#include "HashId.h"
#include "path.h"


static const char hex[16] =
{
   '0', '1', '2', '3',   '4', '5', '6', '7',
   '8', '9', 'a', 'b',   'c', 'd', 'e', 'f'
};



static unsigned char hexToValue( char c )
{
   if ( ( c >= '0' ) && ( c <= '9' ) )
      return c - '0';

   if ( ( c >= 'a' ) && ( c <= 'f' ) )
      return c - 'a' + 10;

   BOOST_ASSERT( ( c >= 'A' ) && ( c <= 'F') );

   return c - 'A' + 10;
}


static unsigned char toCharValue( const char *s )
{
   unsigned char v = hexToValue( s[0] ) << 4;
   v |= hexToValue( s[1] );
   return v;
}



void HashId::setString( const char *s )
{
   for ( int i = 0; i < 32; s += 2, ++i )
      hash[i] = toCharValue( s );
}



size_t HashId::toString( char *buf ) const
{
   for ( int i = 0; i < 32; ++i )
   {
      unsigned char c = hash[i];
      *buf++ = hex[c>>4];
      *buf++ = hex[c&15];
   }

   *buf = 0;

   return sizeof( hash ) * 2;
}



size_t HashId::toObjPath( char *path ) const
{
   unsigned char c = hash[0];
   *path++ = hex[c>>4];
   *path++ = hex[c&15];

   *path++ = '/';

   c = hash[1];
   *path++ = hex[c>>4];
   *path++ = hex[c&15];

   *path++ = '/';

   for ( int i = 2; i < 32; ++i )
   {
      c = hash[i];
      *path++ = hex[c>>4];
      *path++ = hex[c&15];
   }

   *path = 0;

   return sizeof( hash ) * 2 + 2;
}



boost::filesystem::path HashId::getStorePath( ) const
{
	boost::filesystem::path path = path_get( PathType::OBJ, nullptr );
	char buf[80];
	toObjPath( buf );
	path /= buf;
	return path;
}



std::basic_istream<char> & operator >> ( std::basic_istream<char> &is, HashId &id )
{
   is.read( reinterpret_cast<char*>(&id), sizeof(id) );
   return is;
}



std::basic_ostream<char> & operator << ( std::basic_ostream<char> &os, const HashId &id )
{
   os.write( reinterpret_cast<const char *>( &id ), sizeof(id) );
   return os;
}
