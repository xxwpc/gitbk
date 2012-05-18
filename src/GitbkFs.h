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


#include "HashId.h"
#include "NodeAttr.h"



class GitbkFs
{
public:
   GitbkFs( const NodeAttr &attr, const boost::filesystem::path & );

   virtual ~GitbkFs( );

public:
   size_t toString( char *buf ) const
   {
      return _attr.toString( buf );
   }

   virtual void run( );

   GitbkFs         *next;

   void setParent( GitbkFs *p )
   {
      _parent = p;
   }

   unsigned char success( ) const
   {
      return _success;
   }

   const boost::filesystem::path & getPath( ) const
   {
	   return _path;
   }

protected:
   virtual void onFile( ) = 0;
   virtual void onSymlink( ) = 0;
   virtual void onDirBegin( ) = 0;
   virtual void onDirEnd( ) = 0;
   virtual void onComplete( );

   static bool cmp( GitbkFs *, GitbkFs * );

   void subPush( GitbkFs * );
   void subSort( );
   void onSubComplete( );

protected:
   boost::filesystem::path       _path;
   NodeAttr                      _attr;
   GitbkFs                      *_parent;
   boost::detail::atomic_count   _cnt;
   std::vector<GitbkFs*>         _sub;
   bool                          _first;
   bool                          _success;
};




class RunRoot : public GitbkFs
{
public:
   RunRoot( int i, GitbkFs *rf )
      : GitbkFs( NodeAttr(),"" )
      , _count( i )
   {
      rf->setParent(this);
      ++_cnt;
   }

   void onFile( ) override;
   void onSymlink( ) override;
   void onDirBegin( ) override;
   void onDirEnd( ) override;

public:
   void run( ) override;

private:
   int          _count;
};

