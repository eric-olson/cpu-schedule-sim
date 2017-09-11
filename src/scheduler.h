#pragma once

#include "thread.h"

#include <queue>
#include <string>

enum EventType { THREAD_ARRIVED, THREAD_DISPATCH_COMPLETED,
  PROCESS_DISPATCH_COMPLETED, CPU_BURST_COMPLETED,
  IO_BURST_COMPLETED, THREAD_COMPLETED, THREAD_PREEMPTED,
  DISPATCHER_INVOKED, SIMULATION_COMPLETE};

struct Event {
  EventType type;
  int time;
  int pid;
  int tid;
};

// scheduler is an interface for specific scheduling algorithms to implement
class Scheduler {
  protected:
    Thread running;
    CPU cpu;

  public:
    virtual Event switchToThread(Thread upNext) = 0;

    Scheduler();

    // get message for dispatcher verbose output
    virtual std::string dispatchMessage() = 0;

    // get next thread to execute on CPU
    virtual Event nextThread(bool verbose) = 0;

    // add a new thread to the ready queue
    virtual void scheduleThread(Thread t) = 0;

    // change status of running thread to blocked, use pid/tid to check
    virtual Event threadBlocked(int pid, int tid) = 0;

    // change status of specified thread to ready
    virtual void threadReady(int pid, int tid) = 0;

    // change status of running thread to complete, use pid/tid to check
    virtual Event threadComplete(int pid, int tid) = 0;

    // preempt currently running thread with specified thread
    virtual Event threadPreempted(int pid, int tid) = 0;

    // start CPU burst after switch
    virtual Event startBurst(int pid, int tid) = 0;

    // get next dispatch
    virtual Event getNextDispatch() = 0;
};
