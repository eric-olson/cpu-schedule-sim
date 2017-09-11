#include "roundrobin.h"

#include <iostream>
#include <sstream>

using namespace std;

Event RoundRobin::switchToThread(Thread upNext) {
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

RoundRobin::RoundRobin() {
  // do nothing
}

string RoundRobin::dispatchMessage() {
  stringstream ss;
  ss << "    Selected from " << ready.size() + 1
    << " threads; will run for at most " << quantum << " ticks";
  return ss.str();
}

// get next thread to execute on CPU
Event RoundRobin::nextThread(bool verbose) {
  Thread upNext = ready.front();
  Event e = switchToThread(upNext);
  return e;
}

// add a new thread to the ready queue
void RoundRobin::scheduleThread(Thread t) {
  ready.push(t);
}

// change status of running thread to blocked, use pid/tid to check
Event RoundRobin::threadBlocked(int pid, int tid) {
  blocked.push_back(running);
  Event e;
  e.type = IO_BURST_COMPLETED;
  e.time = cpu.getCurrentBurst().ioTime;
  e.pid = pid;
  e.tid = tid;

  return e;
}

// change status of specified thread to ready
void RoundRobin::threadReady(int pid, int tid) {
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
Event RoundRobin::threadComplete(int pid, int tid) {
  // create an event to find next thread
  // will result in current time when returned and time is added
  return getNextDispatch();
}

// preempt currently running thread with specified thread
Event RoundRobin::threadPreempted(int pid, int tid) {
  // update remaining time in burst
  running.updateNextBurstTime(quantum);
  ready.push(running);
  return getNextDispatch();
}

// start CPU burst after switch
Event RoundRobin::startBurst(int pid, int tid) {
  int endTime = cpu.startBurst();
  Event e;
  // first case: burst time > quantum
  if (cpu.getCurrentBurst().cpuTime > quantum) {
    e.type = THREAD_PREEMPTED;
    e.time = quantum;
  }
  // second case: burst time <= quantum
  else {
    if (cpu.getCurrentBurst().ioTime != 0) {
      e.type = CPU_BURST_COMPLETED;
    }
    else {
      e.type = THREAD_COMPLETED;
    }
    e.time = endTime;
  }
  e.pid = pid;
  e.tid = tid;

  return e;
}

Event RoundRobin::getNextDispatch() {
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

