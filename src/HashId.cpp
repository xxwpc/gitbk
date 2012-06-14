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
#include "HashId.h"
#include "path.h"
#include "GbFile.h"



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



void HashId::setSha256( const unsigned char sha256[32] )
{
   for ( int i = 0; i < 32; ++i )
   {
      unsigned char c = sha256[i];

      int idx = i * 2;
      hash[idx]   = hex[c>>4];
      hash[idx+1] = hex[c&15];
   }
}



void HashId::getSha256( unsigned char sha256[32] ) const
{
   for ( int i = 0; i < 32; ++i )
      sha256[i] = toCharValue( hash + i*2 );
}



boost::filesystem::path HashId::getStorePath( ) const
{
	boost::filesystem::path path = path_get( PathType::OBJ, nullptr );
	char buf[67];
   buf[0] = hash[0];
   buf[1] = hash[1];
   buf[2] = '/';
   buf[3] = hash[2];
   buf[4] = hash[3];
   buf[5] = '/';
   memcpy( buf+6, hash+4, 60 );
   buf[66] = 0;

	path /= buf;
	return path;
}



struct IdStroe
{
   unsigned char  sha256[32];
   std::uint64_t  size;
   std::uint64_t  reserve;
};



bool HashId::load ( InputFile &in )
{
   IdStroe store;

   if ( in.read( static_cast<void*>(&store), sizeof(store) ) != sizeof(store) )
      return false;

   setSha256( store.sha256 );
   size    = store.size;
   reserve = store.reserve;

   return true;
}



void HashId::store( OutputFile &out ) const
{
   IdStroe store;
   getSha256( store.sha256 );
   store.size    = size;
   store.reserve = reserve;

   out.write( static_cast<void *>( &store ), sizeof(store) );
}

