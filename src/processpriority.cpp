#include "processpriority.h"
#include "event.h"

#include <iostream>
#include <sstream>

using namespace std;

queue<Thread> *ProcessPriority::threadQueue(Thread t) {
  Priority p = t.getPriority();
  switch(p) {
    case SYSTEM:
      return &sysReady;
    case INTERACTIVE:
      return &intReady;
    case NORMAL:
      return &norReady;
    case BATCH:
      return &batReady;
    default:
      // something has gone very wrong
      cerr << "No priority found for thread" << endl;
      exit(-1);
  };
}

queue<Thread> *ProcessPriority::nextQueue() {
  if (!sysReady.empty()) {
    mostRecentQueue = 0;
    return &sysReady;
  }
  else if (!intReady.empty()) {
    mostRecentQueue = 1;
    return &intReady;
  }
  else if (!norReady.empty()) {
    mostRecentQueue = 2;
    return &norReady;
  }
  else {
    mostRecentQueue = 3;
    return &batReady;
  }
}

Event ProcessPriority::switchToThread(Thread upNext) {
  Event e;
  e.pid = upNext.getParentPid();
  e.tid = upNext.getTid();

  // figure out switch time
  int switchTime = 0;
  if (running.getParentPid() == upNext.getParentPid()) {
    switchTime = upNext.getThreadOverhead();
    e.type = THREAD_DISPATCH_COMPLETED;
  }
  else {
    switchTime = upNext.getProcessOverhead();
    e.type = PROCESS_DISPATCH_COMPLETED;
  }

  e.time = switchTime;
  // switch running process
  running = upNext;
  cpu.startSwitch(running.getNextBurst(), switchTime);
  threadQueue(upNext)->pop();

  return e;
}

ProcessPriority::ProcessPriority() {
  // do nothing
}

string ProcessPriority::dispatchMessage() {
  stringstream ss;
  ss << "    Selected from queue " << mostRecentQueue
    << " [S:" << sysReady.size() << " I:" << intReady.size()
    << " N:" << norReady.size() << " B:" << batReady.size() << "]";
  return ss.str();
}

// get next thread to execute on CPU
Event ProcessPriority::nextThread(bool verbose) {
  Thread upNext = nextQueue()->front();
  Event e = switchToThread(upNext);
  return e;
}

// add a new thread to the ready queue
void ProcessPriority::scheduleThread(Thread t) {
  threadQueue(t)->push(t);
}

// change status of running thread to blocked, use pid/tid to check
Event ProcessPriority::threadBlocked(int pid, int tid) {
  blocked.push_back(running);
  Event e;
  e.type = IO_BURST_COMPLETED;
  e.time = cpu.getCurrentBurst().ioTime;
  e.pid = pid;
  e.tid = tid;

  return e;
}

// change status of specified thread to ready
void ProcessPriority::threadReady(int pid, int tid) {
  // TODO: better lookup for blocked threads
  for (vector<Thread>::iterator it = blocked.begin(); it != blocked.end(); ++it) {
    if (it->getParentPid() == pid && it->getTid() == tid) {
      // get queue for thread
      queue<Thread> *q = threadQueue(*it);
      // add thread to appropriate queue
      q->push(*it);
      // pop bursts from thread after adding to queue
      q->back().popBursts();
      // remove thread from blocked queue
      blocked.erase(it);
      break;
    }
  }
}

// change status of running thread to complete, use pid/tid to check
Event ProcessPriority::threadComplete(int pid, int tid) {
  // create an event to find next thread
  // will result in current time when returned and time is added
  return getNextDispatch();
}

// preempt currently running thread with specified thread
Event ProcessPriority::threadPreempted(int pid, int tid) {
  // do nothing for now since FCFS is non-preemptive
  Event e;
  return e;
}

// start CPU burst after switch
Event ProcessPriority::startBurst(int pid, int tid) {
  int endTime = cpu.startBurst();
  Event e;
  if (cpu.getCurrentBurst().ioTime != 0) {
    e.type = CPU_BURST_COMPLETED;
  }
  else {
    e.type = THREAD_COMPLETED;
  }
  e.time = endTime;
  e.pid = pid;
  e.tid = tid;

  return e;
}

Event ProcessPriority::getNextDispatch() {
  Event e;
  e.type = DISPATCHER_INVOKED;
  // if all queues aren't empty, dispatch immediately
  if (!nextQueue()->empty()) {
    e.time = 0;
  }
  // if all queues are empty, simulation is over
  else if (nextQueue()->empty() && blocked.empty()) {
    e.type = SIMULATION_COMPLETE;
    e.time = 0;
  }
  // otherwise, find out when next blocked process finishes
  else {
    int lowestTime = blocked[0].getNextBurst().ioTime;
    for (Thread i : blocked) {
      int t = i.getNextBurst().ioTime;
      if (t < lowestTime)
        lowestTime = t;
    }
    e.time = lowestTime;
  }
  return e;
}

