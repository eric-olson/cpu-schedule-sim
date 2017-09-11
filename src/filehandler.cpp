#include "cpu.h"
#include "process.h"
#include "thread.h"
#include "event.h"

#include <fstream>
#include <getopt.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

// variables for flag values
static bool perThreadFlag = false;
static bool verboseFlag = false;
// TODO: algorithm
static bool helpFlag = false;
static string filename = "";
static int algorithmChoice = 0;

EventHandler loadFile(string filePath) {
  map<int, Process> processList;
  // open file
  ifstream fin;
  fin.open(filePath);

  if (!fin) {
    cerr << "Error opening file: " << filePath << endl;
    exit(1);
  }
// create EventHandler
  EventHandler eh(verboseFlag, perThreadFlag, algorithmChoice);

  // get cpu info
  int numProcesses, threadOverhead, processOverhead;
  fin >> numProcesses >> threadOverhead >> processOverhead;

  // loop over all processes
  for (int i = 0; i < numProcesses; i++) {
    // init variables
    int pid, nThreads;
    int pri;

    // read in pid, priority, and number of threads
    fin >> pid >> pri >> nThreads;
    // create process
    Process process(pid, processOverhead, static_cast<Priority>(pri));

    // create map of thread stats
    map<int, ThreadStats> tsMap;

    // populate threads in process
    for (int tid = 0; tid < nThreads; tid++) {
      // read in arrival time and number of bursts
      int arrivalTime, bursts;
      fin >> arrivalTime >> bursts;
      // create thread
      Thread thread(tid, pid, arrivalTime, threadOverhead, processOverhead,
          static_cast<Priority>(pri));

      // create ThreadStats for thread
      ThreadStats ts;
      ts.pid = pid;
      ts.tid = tid;
      ts.arr = arrivalTime;
      ts.cpu = 0;
      ts.io = 0;

      // create event for thread arrival
      Event e;
      e.type = THREAD_ARRIVED;
      e.time = arrivalTime;
      e.pid = pid;
      e.tid = tid;
      eh.addEvent(e);

      // loop over all bursts
      for (int b = 0; b < bursts - 1; b++) {
        CPUBurst cb;
        fin >> cb.cpuTime >> cb.ioTime;
        thread.addBurst(cb);
        ts.cpu += cb.cpuTime;
        ts.io += cb.ioTime;
      }
      // handle last burst with 0 I/O time
      CPUBurst cb;
      fin >> cb.cpuTime;
      cb.ioTime = 0;
      thread.addBurst(cb);
      ts.cpu += cb.cpuTime;

      // add thread to process
      process.addThread(thread);

      // add threadStats to map
      tsMap.insert(pair<int, ThreadStats>(tid, ts));
    }


    // push process to map
    processList.insert(pair<int, Process>(pid, process));
    // push ThreadStats map to EventHandler stats
    eh.stats.insert(pair<int, map<int, ThreadStats>>(pid, tsMap));
  }

  fin.close();
  // update event handler with process map
  eh.updateProcessMap(processList);
  return eh;
}


void display_help() {
  cout << "Usage: simulator [-dvh] filename" << endl << endl
    << "Options:" << endl
    << "  -h, --help:" << endl
    << "      Print this help message and exit." << endl
    << "  -t, --per_thread:" << endl
    << "      If set, outputs per-thread metrics at the end of the simulation." << endl
    << "  -v, --verbose:" << endl
    << "      If set, outputs all state transitions and scheduling choices." << endl
    << "  -a, --algorithm:" << endl
    << "      The scheduling algorithm to use. Valid values are:" << endl
    << "        FCFS: first-come, first-served (default)" << endl
    << "        RR: round-robin scheduling" << endl
    << "        PRIORITY: process-priority scheduling" << endl
    << "        CUSTOM: custom algorithm, described in report" << endl;
}

void update_flags(int argc, char **argv) {
  int c;
  while (1) {
    // define options
    static struct option long_options[] = {
      {"per-thread",    no_argument,       0, 't'},
      {"verbose",       no_argument,       0, 'v'},
      {"algorithm",     required_argument, 0, 'a'},
      {"help",          no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    c = getopt_long (argc, argv, "tva:h", long_options, &option_index);

    // stop at end of options
    if (c == -1)
      break;

    // convert optarg to string
    string alg;
    // interpret options
    switch(c) {
      case 0:
        // do nothing
        break;
      case 't':
        perThreadFlag = true;
        break;
      case 'v':
        verboseFlag = true;
        break;
      case 'a':
        alg.assign(optarg);
        if (alg == "FCFS") {
          algorithmChoice = 0;
        }
        else if (alg == "RR") {
          algorithmChoice = 1;
        }
        else if (alg == "PRIORITY") {
          algorithmChoice = 2;
        }
        else if (alg == "CUSTOM") {
          algorithmChoice = 3;
        }
        else {
          display_help();
          exit(1);
        }
        break;
      case 'h':
        display_help();
        exit(0);
        break;
      case '?':
        display_help();
        exit(1);
        break;
      default:
        abort();
    }
  };
  if (optind >= argc) {
    display_help();
    exit(1);
  }
  while (optind < argc) {
    filename += argv[optind++];
  }
}


bool get_per_thread() {
  return perThreadFlag;
}
bool get_verbose() {
  return verboseFlag;
}
// TODO: algorithm
bool get_help() {
  return helpFlag;
}
string get_filename() {
  return filename;
}
