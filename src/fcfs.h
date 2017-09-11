#pragma once

#include "scheduler.h"

// FCFS schedules in a first-come, first-served order
class FCFS : public Scheduler {
  private:
    std::queue<Thread> ready;
    std::vector<Thread> blocked;

  public:
    Event switchToThread(Thread upNext);

    FCFS();

    // get message for dispatcher verbose output
    std::string dispatchMessage();

    // get next thread to execute on CPU
    Event nextThread(bool verbose);

    // add a new thread to the ready queue
    void scheduleThread(Thread t);

    // change status of running thread to blocked, use pid/tid to check
    Event threadBlocked(int pid, int tid);

    // change status of specified thread to ready
    void threadReady(int pid, int tid);

    // change status of running thread to complete, use pid/tid to check
    Event threadComplete(int pid, int tid);

    // preempt currently running thread with specified thread
    Event threadPreempted(int pid, int tid);

    // start CPU burst after switch
    Event startBurst(int pid, int tid);

    // get next dispatch
    Event getNextDispatch();
};
