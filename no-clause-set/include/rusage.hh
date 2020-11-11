/******************************************************************************\
 *    This file is part of packup.                                            *
 *                                                                            *
 *    packup is free software: you can redistribute it and/or modify          *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation, either version 3 of the License, or       *
 *    (at your option) any later version.                                     *
 *                                                                            *
 *    packup is distributed in the hope that it will be useful,               *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *    GNU General Public License for more details.                            *
 *                                                                            *
 *    You should have received a copy of the GNU General Public License       *
 *    along with packup.  If not, see <http://www.gnu.org/licenses/>.         *            
\******************************************************************************/           
/*----------------------------------------------------------------------------*\
 * Version: $Id: rusage.hh 73 2007-07-26 15:16:48Z jpms $
 *
 * Author: jpms
 * 
 * Description: Obtain time and memory resource usage stats.
 *              Adapted from minisat and from MCSAT.
 *
 *                                    Copyright (c) 2006, Joao Marques-Silva
\*----------------------------------------------------------------------------*/

#ifndef _RUSAGE_HH_
#define _RUSAGE_HH_ 1

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>

#include <iostream>

namespace RUSAGE {
  static inline double read_cpu_time_self();
  static inline double read_cpu_time_children();
  static inline long read_mem_stats(int fields);
  static inline double read_mem_used();
  static inline void print_cpu_time(const char* msg, std::ostream& outs=std::cout);
  static inline void print_mem_used(const char* msg, std::ostream& outs=std::cout);
}


static inline double RUSAGE::read_cpu_time_self()
{
  struct rusage ru; getrusage(RUSAGE_SELF, &ru);
  return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000;
}

static inline double RUSAGE::read_cpu_time_children()
{
  struct rusage ru; getrusage(RUSAGE_CHILDREN, &ru);
  return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000;
}

#define NSTRSZ 256
static inline long RUSAGE::read_mem_stats(int fields)
{
  char    name[NSTRSZ];
  pid_t pid = getpid();
  sprintf(name, "/proc/%d/statm", pid);
  FILE*   finp = fopen(name, "rb");
  if (finp == NULL) { /* assert(0); */ return 0; }
  int value;
  for (; fields >= 0; fields--) { fscanf(finp, "%d", &value); }
  fclose(finp);
  return value;
}

#define __1MBYTE__ 1048576

static inline double RUSAGE::read_mem_used()
{
  return ((long)read_mem_stats(0) * (long)getpagesize() * 1.0) / __1MBYTE__;
}

static inline void RUSAGE::print_cpu_time(const char* msg, std::ostream& outs)
{
  outs << msg << ": "<< RUSAGE::read_cpu_time_self() << endl;
}

static inline void RUSAGE::print_mem_used(const char* msg, std::ostream& outs)
{
  outs << msg << ": " << RUSAGE::read_mem_used() << endl;
}

#endif /* _RUSAGE_HH_ */

/*----------------------------------------------------------------------------*/
