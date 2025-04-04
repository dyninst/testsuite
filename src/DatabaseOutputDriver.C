/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include <boost/date_time.hpp>


#ifdef os_windows_test
#define MAX_USER_NAME	256
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

//#include "test_lib.h"

#include "DatabaseOutputDriver.h"
#include "UsageMonitor.h"

extern "C" {
	TestOutputDriver *outputDriver_factory(void * data) 
	{ 
		return new DatabaseOutputDriver(data); 
	}
}

DatabaseOutputDriver::DatabaseOutputDriver(void * data)
  : attributes(NULL),
    submittedResults(false),
    wroteLogHeader(false),
    currTest(NULL),
    result(UNKNOWN)
{
   sqlLogFilename = std::string((char *)data);

    FILE * fp = fopen(sqlLogFilename.c_str(), "r");
    if (fp != NULL) {
        // log file already exists, so don't rewrite header
        wroteLogHeader = true;
        fclose(fp);
    }
}

DatabaseOutputDriver::~DatabaseOutputDriver() {
  if (attributes != NULL) {
    delete attributes;
    attributes = NULL;
  }
}

void
DatabaseOutputDriver::startNewTest(std::map<std::string, std::string> &attrs,
                                   TestInfo *test, RunGroup *group)
{
  currTest = test;

  // This needs to set up a log file to store output from all streams
  if (attributes != NULL) {
    delete attributes;
  }
  attributes = new std::map<std::string, std::string>(attrs);

  // I want to log to the file "dblog.<testname>"
  std::stringstream fnstream;
  fnstream << "dblog." << (*attributes)[std::string("test")];
  dblogFilename = fnstream.str();

  // Write anything in pretestLog to the dblog file
  std::ofstream out(dblogFilename.c_str(), std::ios::app);
  out << pretestLog.str();
  out.close();
  pretestLog.str(std::string()); // Clear the buffer

  // We haven't submitted results for this test yet
  submittedResults = false;
  // Don't know what the test's result is
  result = UNKNOWN;
}

void DatabaseOutputDriver::redirectStream(TestOutputStream stream, const char * filename) {
  // This is a no-op for database output
}

void DatabaseOutputDriver::logResult(test_results_t res, int stage) {
  // What does this do, exactly?  Store the result code for database check-in
  // I guess..
  // Do I want to submit the results to the database in this method, or use
  // finalizeOutput for that?
  result = res;
}

void DatabaseOutputDriver::logCrash(std::string crashedTest) {
  // New idea: we've already called startNewTest for this test, so
  // dblogFilename is set to recover the old log data for the test.
  // All we need to do is register a crashed result for the test.
  result = CRASHED;
}

void DatabaseOutputDriver::log(TestOutputStream stream, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(stream, fmt, args);
  va_end(args);
}

void DatabaseOutputDriver::vlog(TestOutputStream stream, const char *fmt,
				va_list args)
{
  FILE *dbout = NULL;

  if (dblogFilename.empty()) {
#if defined(os_windows_test)
     char *tfile = _tempnam(".", "dts");
     if (tfile) {
        dbout = fopen(tfile, "w+b");
     }
#else
    dbout = tmpfile();
#endif
    if (NULL == dbout) {
      fprintf(stderr, "[%s:%u] - Error opening temp log file\n", __FILE__, __LINE__);
    } else {
      int count = vfprintf(dbout, fmt, args);
      fflush(dbout);
      fseek(dbout, 0, SEEK_SET);
      std::vector<char> buffer(count);
      auto const nbytes = fread(buffer.data(), sizeof (char), count, dbout);
      if(nbytes != buffer.size()) {
        fprintf(stderr, "[%s:%u] - Read less than expected\n", __FILE__, __LINE__);
      }
      pretestLog.write(buffer.data(), count);
      fclose(dbout);
    }
  } else {
    dbout = fopen(dblogFilename.c_str(), "a");
    if (NULL == dbout) {
      fprintf(stderr, "[%s:%u] - Error opening log file\n", __FILE__, __LINE__);
      return;
    }
    vfprintf(dbout, fmt, args);
    fclose(dbout);
  }
}

void DatabaseOutputDriver::finalizeOutput() {
	// This is where we'll send all the info we've received so far to the
	// database
	if (submittedResults) {
		return; // Only submit results for a test once
	}

	//write the header if necessary
	if (!wroteLogHeader) {
		// get hostname and username for log header
		// FIXME This needs to be platform-independent
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX (255)
#endif
		char hostname[HOST_NAME_MAX];
		if (gethostname(hostname, HOST_NAME_MAX)) {
			// TODO Handle error
		}
		
		std::string userName;

#ifdef os_windows_test
		char * szUserName = new char[MAX_USER_NAME + 1];
		LPDWORD lpnSize = (LPDWORD)malloc(sizeof(DWORD));
		*lpnSize = MAX_USER_NAME + 1;
		if (lpnSize == NULL) {
			fprintf(stderr, "[%s:%u] - Out of memory!\n", __FILE__, __LINE__);
			// TODO Handle error;
		}
		memset(szUserName, 0, MAX_USER_NAME + 1);
		if (!GetUserName(szUserName, lpnSize)) {
			fprintf(stderr, "[%s:%u] - Failed to get username: %s\n",
					__FILE__, __LINE__, GetLastError());
			//TODO Handle error
		}
		userName = std::string(szUserName);

		free(lpnSize);
		delete [] szUserName;
#else
		//note: geteuid() is always successful
		//FIXME: use getpwuid_r
		struct passwd * pw = getpwuid(geteuid());
		if (NULL == pw) {
			//TODO unknown user
			userName = "unknown";
		} else {
			userName = pw->pw_name;
		}
#endif

		std::string logHeader = userName + "@" + hostname;
                if (getenv("PLATFORM") != 0) {
                    logHeader += "\nPLATFORM=";
                    logHeader += getenv("PLATFORM");
                }
                logHeader += "\n\n";

		FILE * sqlLog = fopen(sqlLogFilename.c_str(), "wb");
		if (NULL == sqlLog) {
			fprintf(stderr, "[%s:%u] - Error opening log file: %s\n",
					__FILE__, __LINE__, sqlLogFilename.c_str());
			//TODO handle error
		}
		int size = strlen(logHeader.c_str());
		if (fwrite(logHeader.c_str(), sizeof(char), size, sqlLog) != size) {
			fprintf(stderr, "[%s:%u] - Error writing to log file.\n", __FILE__, __LINE__);
			//TODO handle error
		}
		fclose(sqlLog);

		wroteLogHeader = true;
	}
	
	writeSQLLog();
	return;

	//TODO: what about this stuff?
	submittedResults = true;
}

// This is called if there is an error submitting results to the database.
// It writes the test results to a log file that can be read or later
// resubmitted to the database
void DatabaseOutputDriver::writeSQLLog() {
  static volatile int recursion_guard = 0;
  assert(!recursion_guard);
  recursion_guard = 1;

  std::ofstream sql_log(sqlLogFilename, std::ofstream::app);
  if(!sql_log) {
   std::cerr << "Failed to open SQL log file '" << sqlLogFilename << "'\n";
   return;
  }

  auto datetime = []() {
   namespace bg = boost::gregorian;
   namespace bp = boost::posix_time;
   auto date = bg::day_clock::universal_day();
   auto time = bp::second_clock::universal_time();
   return to_iso_extended_string(date) + " " + to_iso_extended_string(time);
  }();

  sql_log << "BEGIN TEST\n" << datetime << '\n';

  sql_log << "{";
  if(attributes) {
    bool first = true;
    for(auto const &attr : *attributes) {
      if(!first) sql_log << ", ";
      sql_log << std::get<0>(attr) << ": " << std::get<1>(attr);
      first = false;
    }
  }
  sql_log << "}\n";

  std::ifstream db_log(dblogFilename);
  if(!db_log) {
   std::cerr << "Failed to open database log file '" << dblogFilename << "'\n';";
   return;
  }

  auto data = std::string(std::istream_iterator<char>(db_log),{});

  //remove trailing whitespace
  size_t found = data.find_last_not_of(" \t\f\v\n\r");
  if(found != std::string::npos)
    data.erase(found + 1);
  else
    data.clear();

  sql_log << data;

  if(data.rfind("RESULT:") == std::string::npos) {
    sql_log << "\nRESULT: " << result;
    if(currTest && currTest->usage.has_data()) {
      // %ld.%06ld
      auto const& usage = currTest->usage;
      sql_log << "\nCPU: " << usage.cpuUsage().tv_sec
              << "." << std::setw(6) << std::setfill('0') << usage.cpuUsage().tv_usec
              << "MEMORY: " << currTest->usage.memUsage();
    }
  }
  sql_log << "\n\n";

  unlink(dblogFilename.c_str());
  dblogFilename.clear();
  recursion_guard = 0;
}

void DatabaseOutputDriver::getMutateeArgs(std::vector<std::string> &args) {
	args.clear();
	args.push_back(std::string("-dboutput"));
}

