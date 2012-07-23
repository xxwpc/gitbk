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

#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <dirent.h>
#include <bzlib.h>
#include <openssl/sha.h>
#include <utime.h>
#include <utmp.h>
#include <errno.h>
#include <signal.h>

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <iostream>
#include <list>

#include <boost/assert.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/spirit/include/classic_file_iterator.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/regex.hpp>

extern void verbose( const char *fmt, ... );
extern void verbose_err( const char *fmt, ... );
