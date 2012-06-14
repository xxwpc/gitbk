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

#include "mthread.h"


static pthread_mutex_t list_mutex;


static sem_t    list_count;



static GitbkFs   *list_head;



static void mt_run( )
{
   while ( true )
   {
      sem_wait( &list_count );

      pthread_mutex_lock( &list_mutex );
      GitbkFs *r = list_head;
      if ( r != NULL )
	 list_head = list_head->next;
      pthread_mutex_unlock( &list_mutex );

      if ( r == NULL )
         break;

      r->next = NULL;
      r->run( );
   }
}




void RunListAdd( GitbkFs **rl, int i )
{
   pthread_mutex_lock( &list_mutex );

   for ( ; i > 0; ++rl, --i )
   {
      (*rl)->next = list_head;
      list_head = *rl;
      sem_post( &list_count );
   }

   pthread_mutex_unlock( &list_mutex );
}



void RunMt( GitbkFs *r )
{
   pthread_mutex_init( &list_mutex, NULL );
   sem_init( &list_count, 0, 1 );

   list_head = r;
   list_head->next = NULL;

   int n = sysconf( _SC_NPROCESSORS_ONLN );

   if ( n <= 4 )
      ++n;
   else
	   n = n * 5 / 4;

   std::cout << "Use " << n << " work threads." << std::endl;

   RunRoot rr( n, r );

   boost::thread_group tgroup;

   for ( int i = 0; i < n; ++i )
      tgroup.create_thread( &mt_run );

   tgroup.join_all( );

   sem_destroy( &list_count );
   pthread_mutex_destroy( &list_mutex );
}



void RunRoot::run( )
{
   for ( int i = 0; i < _count; ++i )
      sem_post( &list_count );
}

