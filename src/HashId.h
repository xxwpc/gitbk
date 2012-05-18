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

#pragma once


struct HashId
{
   bool operator < ( const HashId &o ) const
   {
      return memcmp( hash, o.hash, 32 ) < 0;
   }

   void set( const unsigned char h[32] )
   {
      memcpy( hash, h, sizeof(hash) );
   }

   void setString( const char * );

   size_t toObjPath( char * ) const;

   size_t toString( char * ) const;


   boost::filesystem::path getStorePath( ) const;

   unsigned char              hash[32];
   std::uint64_t              size;
   std::uint64_t              reserve;
};



std::basic_istream<char> & operator >> ( std::basic_istream<char> &, HashId & );
std::basic_ostream<char> & operator << ( std::basic_ostream<char> &, const HashId & );
