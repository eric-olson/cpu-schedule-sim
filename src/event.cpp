#include "event.h"

#include <iostream>

using namespace std;

bool operator<(const Event& lhs, const Event& rhs) {
  //if (lhs.time == rhs.time) {
    //return lhs.type > rhs.type;
  //}
  return lhs.time > rhs.time;
}

// handler for generic event
void EventHandler::handleEvent(Event e) {
  time = e.time;
  switch (e.type) {
    case THREAD_ARRIVED:
      handleThreadArrived(e);
      break;
    case THREAD_DISPATCH_COMPLETED:
      handleThreadDispatchCompleted(e);
      break;
    case PROCESS_DISPATCH_COMPLETED:
      handleProcessDispatchCompleted(e);
      break;
    case CPU_BURST_COMPLETED:
      handleCPUBurstCompleted(e);
      break;
    case IO_BURST_COMPLETED:
      handleIOBurstCompleted(e);
      break;
    case THREAD_COMPLETED:
      handleThreadCompleted(e);
      break;
    case THREAD_PREEMPTED:
      handleThreadPreempted(e);
      break;
    case DISPATCHER_INVOKED:
      handleDispatcherInvoked(e);
      break;
    case SIMULATION_COMPLETE:
      handleSimulationComplete(e);
      break;
  }
}

// thread has been created in system, needs to be scheduled
void EventHandler::handleThreadArrived(Event e) {
  // get next thread from process
  Thread t = processes[e.pid].popThread();
  // send thread to scheduler
  scheduler->scheduleThread(t);
  if (verbose) {
    cout << "At time " << e.time << ":" << endl
         << "    THREAD_ARRIVED" << endl
         << "    Thread " << e.tid << " in process " << e.pid
         << " [" << getPriorityString(e.pid) << "]" << endl
         << "    Transitioned from NEW to READY" << endl << endl;
  }

  // update SimStats- increment counter
  simStatsMap[getProcessPriority(e.pid)].count++;
}

// thread switch done, time to execute new thread on CPU
void EventHandler::handleThreadDispatchCompleted(Event e) {
  // tell the CPU to start the next burst
  Event burstComplete = scheduler->startBurst(e.pid, e.tid);
  // update total service time
  serviceTime += burstComplete.time;
  // add current time to offset
  burstComplete.time += time;
  // add event to simulation
  addEvent(burstComplete);

  // print message (if verbose)
  if (verbose) {
    cout << "At time " << e.time << ":" << endl
         << "    THREAD_DISPATCH_COMPLETED" << endl
         << "    Thread " << e.tid << " in process " << e.pid
         << " [" << getPriorityString(e.pid) << "]" << endl
         << "    Transitioned from READY to RUNNING" << endl << endl;
  }
  // update response time
  ThreadStats *ts = &stats[e.pid][e.tid];
  if (ts->response == -1)
    ts->response = e.time - ts->arr;
}

// process switch done, time to execute new thread from different process
void EventHandler::handleProcessDispatchCompleted(Event e) {
  // tell the CPU to start the next burst
  Event completed = scheduler->startBurst(e.pid, e.tid);
  // update total service time
  serviceTime += completed.time;
  // add current time to offset
  completed.time += time;
  // add event to simulator
  addEvent(completed);
  if (verbose) {
    cout << "At time " << e.time << ":" << endl
         << "    PROCESS_DISPATCH_COMPLETED" << endl
         << "    Thread " << e.tid << " in process " << e.pid
         << " [" << getPriorityString(e.pid) << "]" << endl
         << "    Transitioned from READY to RUNNING" << endl << endl;
  }
  // update response time
  ThreadStats *ts = &stats[e.pid][e.tid];
  if (ts->response == -1)
    ts->response = e.time - ts->arr;
}

// thread done with CPU burst, moving to I/O
void EventHandler::handleCPUBurstCompleted(Event e) {
  // tell scheduler that thread has blocked
  Event burstComplete = scheduler->threadBlocked(e.pid, e.tid);
  // update total I/O time
  ioTime += burstComplete.time;
  // add current time to offset
  burstComplete.time += time;
  // add event to simulator
  addEvent(burstComplete);
  // invoke dispatcher
  Event dispatch = scheduler->getNextDispatch();
  dispatch.time += time;
  addEvent(dispatch);
  if (verbose) {
    cout << "At time " << e.time << ":" << endl
         << "    CPU_BURST_COMPLETED" << endl
         << "    Thread " << e.tid << " in process " << e.pid
         << " [" << getPriorityString(e.pid) << "]" << endl
         << "    Transitioned from RUNNING to BLOCKED" << endl << endl;
  }

}
// thread done with I/O burst, moving to CPU
void EventHandler::handleIOBurstCompleted(Event e) {
  // tell scheduler that thread is ready
  scheduler->threadReady(e.pid, e.tid);
  if (verbose) {
    cout << "At time " << e.time << ":" << endl
         << "    IO_BURST_COMPLETED" << endl
         << "    Thread " << e.tid << " in process " << e.pid
         << " [" << getPriorityString(e.pid) << "]" << endl
         << "    Transitioned from BLOCKED to READY" << endl << endl;
  }
}

// thread done with final CPU burst, complete
void EventHandler::handleThreadCompleted(Event e) {
  // tell scheduler that thread is complete
  Event dispatcherInvoked = scheduler->threadComplete(e.pid, e.tid);
  dispatcherInvoked.time += time;
  addEvent(dispatcherInvoked);
  if (verbose) {
    cout << "At time " << e.time << ":" << endl
         << "    THREAD_COMPLETED" << endl
         << "    Thread " << e.tid << " in process " << e.pid
         << " [" << getPriorityString(e.pid) << "]" << endl
         << "    Transitioned from RUNNING to EXIT" << endl << endl;
  }
  // update ThreadStats with completion time
  ThreadStats* ts = &stats[e.pid][e.tid];
  ts->end = e.time;
  ts->trt = e.time - ts->arr;
}

// thread preempted during execution
void EventHandler::handleThreadPreempted(Event e) {
  // tell scheduler that thread has been preempted
  Event dispatcherInvoked = scheduler->threadPreempted(e.pid, e.tid);
  dispatcherInvoked.time += time;
  addEvent(dispatcherInvoked);
  if (verbose) {
    cout << "At time " << e.time << ":" << endl
         << "    THREAD_PREEMPTED" << endl
         << "    Thread " << e.tid << " in process " << e.pid
         << " [" << getPriorityString(e.pid) << "]" << endl
         << "    Transitioned from RUNNING to READY" << endl << endl;
  }
}

// time to determine next thread- start CPU switch
void EventHandler::handleDispatcherInvoked(Event e) {
  // tell scheduler to send next burst to CPU
  Event dispatchComplete = scheduler->nextThread(verbose);
  // update total dispatch time
  dispatchTime += dispatchComplete.time;
  // add current time to offset
  dispatchComplete.time += time;
  // add event to simulation
  addEvent(dispatchComplete);
  if (verbose) {
    cout << "At time " << e.time << ":" << endl
         << "    DISPATCHER_INVOKED" << endl
         << "    Thread " << dispatchComplete.tid
         << " in process " << dispatchComplete.pid
         << " [" << getPriorityString(dispatchComplete.pid) << "]" << endl
         << scheduler->dispatchMessage() << endl << endl;
  }
}

void EventHandler::handleSimulationComplete(Event e) {
  // do nothing
}

EventHandler::EventHandler() {
  scheduler = &fcfs;
}

EventHandler::EventHandler(bool verbose, bool perThread, int schedType) {
  this->verbose = verbose;
  this->perThread = perThread;
  switch(schedType) {
    case 0:
      scheduler = &fcfs;
      break;
    case 1:
      scheduler = &rr;
      break;
    case 2:
      scheduler = &pp;
      break;
    case 3:
      scheduler = &custom;
      break;
    default:
      scheduler = &fcfs;
      break;
  }
}

void EventHandler::updateProcessMap(std::map<int, Process> p) {
  processes = p;
}

void EventHandler::addEvent(Event e) {
  events.push(e);
}

void EventHandler::runSimulation() {
  // populate simStatsMap
  SimStats ss;
  ss.count = 0;
  ss.totalResponse = 0;
  ss.totalTurnaround = 0;
  simStatsMap.insert(pair<Priority, SimStats>(SYSTEM, ss));
  simStatsMap.insert(pair<Priority, SimStats>(INTERACTIVE, ss));
  simStatsMap.insert(pair<Priority, SimStats>(NORMAL, ss));
  simStatsMap.insert(pair<Priority, SimStats>(BATCH, ss));

  // create first dispatcher event to start simulation
  Event startDispatcher;
  startDispatcher.type = DISPATCHER_INVOKED;
  startDispatcher.time = 0;
  startDispatcher.pid = 0;
  startDispatcher.tid = 0;
  addEvent(startDispatcher);

  // loop over events until empty
  while (!events.empty()) {
    Event e = events.top();
    handleEvent(e);
    events.pop();
  }
  printStats();
  cout << "SIMULATION COMPLETED!" << endl << endl;
  printSummary();
};

priority_queue<Event> EventHandler::getEvents() {
  return events;
}

Process EventHandler::getProcess(int pid) {
  return processes[pid];
}

Priority EventHandler::getProcessPriority(int pid) {
  return processes[pid].getPriority();
}

string EventHandler::getPriorityString(int pid) {
  Priority p = getProcessPriority(pid);
  return priorityToString(p);
}

void EventHandler::printStats() {
  // loop over all stats
  for(auto const &processStats : stats) {
    // print process ID and type
    if(perThread) {
      printf("Process %i [%s]:\n",
          processStats.first, getPriorityString(processStats.first).c_str());
    }
    // get SimStats for current process priority
    SimStats *ss = &simStatsMap[getProcessPriority(processStats.first)];
    for(auto const &threadStats : processStats.second) {
      ThreadStats ts = threadStats.second;
      if(perThread) {
        printf("    Thread %i:  ARR: %-7iCPU: %-7iI/O: %-7iTRT: %-7iEND: %-6iRES: %-6i\n",
            threadStats.first, ts.arr, ts.cpu, ts.io, ts.trt, ts.end, ts.response);
      }
      // update SimStats
      ss->totalResponse += ts.response;
      ss->totalTurnaround += ts.trt;
    }
    if(perThread)
      cout << endl;
  }
}

void EventHandler::printSummary() {
  // loop over simStatsMap
  for(auto const &simStats : simStatsMap) {
    // copy simStats
    SimStats ss = simStats.second;
    // calculate stat values
    double avgResponse = 0;
    double avgTurnaround = 0;
    if (ss.count > 0) {
      avgResponse = (double)ss.totalResponse / ss.count;
      avgTurnaround = (double)ss.totalTurnaround / ss.count;
    }
    // print stats for each thread type
    printf("%s THREADS:\n", priorityToString(simStats.first).c_str());
    printf("    Total count: %16i\n", ss.count);
    printf("    Avg response time: %10.2f\n", avgResponse);
    printf("    Avg turnaround time: %8.2f\n\n", avgTurnaround);
  }

  // derive idle time, utilization, efficiency
  idleTime = time - serviceTime - dispatchTime;
  double utilization = double(serviceTime + dispatchTime) / time * 100;
  double efficiency = double(serviceTime) / time * 100;
  // print global stats
  printf("Total elapsed time: %13i\n", time);
  printf("Total service time: %13i\n", serviceTime);
  printf("Total I/O time: %17i\n", ioTime);
  printf("Total dispatch time: %12i\n", dispatchTime);
  printf("Total idle time: %16i\n\n", idleTime);
  printf("CPU utilization: %15.2f%%\n", utilization);
  printf("CPU efficiency: %16.2f%%\n", efficiency);

}
