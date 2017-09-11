#include "custom.h"
#include "event.h"

#include <iostream>
#include <sstream>

using namespace std;

queue<Thread> *Custom::threadQueue(Thread t) {
  int p = t.currentQueue;
  switch(p) {
    case 0:
      return &rq0;
    case 1:
      return &rq1;
    case 2:
      return &rq2;
    case 3:
      return &rq3;
    default:
      // something has gone very wrong
      cerr << "No queue found for thread" << endl;
      exit(-1);
  };
}

queue<Thread> *Custom::getQueue(int p) {
  switch(p) {
    case 0:
      return &rq0;
    case 1:
      return &rq1;
    case 2:
      return &rq2;
    case 3:
      return &rq3;
    default:
      // something has gone very wrong
      cerr << "No queue found for thread" << endl;
      exit(-1);
  };
}

queue<Thread> *Custom::nextQueue() {
  if (!rq0.empty()) {
    mostRecentQueue = 0;
    currentQuantum = q0 / (rq0.front().getPriority() + 1);
    return &rq0;
  }
  else if (!rq1.empty()) {
    mostRecentQueue = 1;
    currentQuantum = q1 / (rq1.front().getPriority() + 1);
    return &rq1;
  }
  else if (!rq2.empty()) {
    mostRecentQueue = 2;
    currentQuantum = q2 / (rq2.front().getPriority() + 1);
    return &rq2;
  }
  else {
    mostRecentQueue = 3;
    currentQuantum = q3 / (rq3.front().getPriority() + 1);
    return &rq3;
  }
}

Event Custom::switchToThread(Thread upNext) {
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

Custom::Custom() {
  // do nothing
}

string Custom::dispatchMessage() {
  stringstream ss;
  ss << "    Selected from queue " << mostRecentQueue
    << " [0:" << rq0.size() << " 1:" << rq1.size()
    << " 2:" << rq2.size() << " 3:" << rq3.size()
    << "]. Will run for at most " << currentQuantum << " ticks";
  return ss.str();
}

// get next thread to execute on CPU
Event Custom::nextThread(bool verbose) {
  Thread upNext = nextQueue()->front();
  Event e = switchToThread(upNext);
  return e;
}

// add a new thread to the ready queue
void Custom::scheduleThread(Thread t) {
  rq0.push(t);
}

// change status of running thread to blocked, use pid/tid to check
Event Custom::threadBlocked(int pid, int tid) {
  blocked.push_back(running);
  Event e;
  e.type = IO_BURST_COMPLETED;
  e.time = cpu.getCurrentBurst().ioTime;
  e.pid = pid;
  e.tid = tid;

  return e;
}

// change status of specified thread to ready
void Custom::threadReady(int pid, int tid) {
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
Event Custom::threadComplete(int pid, int tid) {
  // create an event to find next thread
  // will result in current time when returned and time is added
  return getNextDispatch();
}

// preempt currently running thread with specified thread
Event Custom::threadPreempted(int pid, int tid) {
  // select next queue for thread
  int q = mostRecentQueue;
  if (q < 3)
    q++;
  // update queue in burst
  running.currentQueue = q;
  // update remaining time in burst
  running.updateNextBurstTime(currentQuantum);
  getQueue(q)->push(running);
  return getNextDispatch();
}

// start CPU burst after switch
Event Custom::startBurst(int pid, int tid) {
  int endTime = cpu.startBurst();
  Event e;
  // first case: burst time > quantum
  if (cpu.getCurrentBurst().cpuTime > currentQuantum) {
    e.type = THREAD_PREEMPTED;
    e.time = currentQuantum;
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

Event Custom::getNextDispatch() {
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

