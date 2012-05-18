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

#include "SubCmd.h"

SubCmd::SubCmd( SubCmd **cmdList, const char *name, int (*proc)( const std::vector<std::string> &) )
{
   while ( (*cmdList != nullptr ) && ( std::strcmp( (*cmdList)->_name, name) < 0 ) )
	   cmdList = &(*cmdList)->_next;

   _next = (*cmdList);
   *cmdList = this;

   _name = name;
   _proc = proc;
}



SubCmd::~SubCmd()
{
}



int SubCmd::procCmd( const SubCmd *cmds, const std::vector<std::string> &args )
{
   BOOST_ASSERT( cmds != nullptr );
   BOOST_ASSERT( args.size( ) > 0 );

   auto &cmd = args[0];

   for ( ; cmds != nullptr; cmds = cmds->_next )
	   if ( cmd == cmds->_name )
		   return cmds->_proc( args );

   std::cout << '\'' << args[0] << "' is't a gitbk command!" << std::endl;
   return -1;
}
