#include "fcfs.h"

#include <iostream>
#include <sstream>

using namespace std;

Event FCFS::switchToThread(Thread upNext) {
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
  ready.pop();

  return e;
}

FCFS::FCFS() {
  // do nothing
}

string FCFS::dispatchMessage() {
  stringstream ss;
  ss << "    Selected from " << ready.size() + 1
    << " threads; will run to completion of burst";
  return ss.str();
}

// get next thread to execute on CPU
Event FCFS::nextThread(bool verbose) {
  Thread upNext = ready.front();
  Event e = switchToThread(upNext);
  return e;
}

// add a new thread to the ready queue
void FCFS::scheduleThread(Thread t) {
  ready.push(t);
}

// change status of running thread to blocked, use pid/tid to check
Event FCFS::threadBlocked(int pid, int tid) {
  blocked.push_back(running);
  Event e;
  e.type = IO_BURST_COMPLETED;
  e.time = cpu.getCurrentBurst().ioTime;
  e.pid = pid;
  e.tid = tid;

  return e;
}

// change status of specified thread to ready
void FCFS::threadReady(int pid, int tid) {
  // TODO: better lookup for blocked threads
  for (vector<Thread>::iterator it = blocked.begin(); it != blocked.end(); ++it) {
    if (it->getParentPid() == pid && it->getTid() == tid) {
      ready.push(*it);
      ready.back().popBursts();
      blocked.erase(it);
      break;
    }
  }
}

// change status of running thread to complete, use pid/tid to check
Event FCFS::threadComplete(int pid, int tid) {
  // create an event to find next thread
  // will result in current time when returned and time is added
  return getNextDispatch();
}

// preempt currently running thread with specified thread
Event FCFS::threadPreempted(int pid, int tid) {
  // do nothing for now since FCFS is non-preemptive
  Event e;
  return e;
}

// start CPU burst after switch
Event FCFS::startBurst(int pid, int tid) {
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

Event FCFS::getNextDispatch() {
  Event e;
  e.type = DISPATCHER_INVOKED;
  // if ready queue isn't empty, dispatch immediately
  if (!ready.empty()) {
    e.time = 0;
  }
  // if both queues are empty, simulation is over
  else if (ready.empty() && blocked.empty()) {
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

