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



class InputFile;
class OutputFile;



struct HashId
{
   bool operator < ( const HashId &o ) const
   {
      return memcmp( hash, o.hash, 64 ) < 0;
   }

   void setSha256( const unsigned char [32] );
   void getSha256( unsigned char [32] ) const;

   void setString( const char *str )
   {
      memcpy( hash, str, 64 );
   }

   size_t toString( char *buf ) const
   {
      memcpy( buf, hash, 64 );
      return 64;
   }

   // size_t toObjPath( char * ) const;


   boost::filesystem::path getStorePath( ) const;

   bool load( InputFile & );
   void store( OutputFile & ) const;

   char                       hash[64];
   std::uint64_t              size;
   std::uint64_t              reserve;
};



std::basic_istream<char> & operator >> ( std::basic_istream<char> &, HashId & );
std::basic_ostream<char> & operator << ( std::basic_ostream<char> &, const HashId & );

