#pragma once

#include "scheduler.h"

// Priority schedules threads by priority
class Custom : public Scheduler {
  private:
    static const int q0 = 4;
    static const int q1 = 8;
    static const int q2 = 16;
    static const int q3 = 32;
    std::queue<Thread> rq0;
    std::queue<Thread> rq1;
    std::queue<Thread> rq2;
    std::queue<Thread> rq3;
    std::vector<Thread> blocked;

    // get appropriate queue for a thread
    std::queue<Thread> *threadQueue(Thread t);
    std::queue<Thread> *nextQueue();
    std::queue<Thread> *getQueue(int p);

    int mostRecentQueue;
    int currentQuantum;

  public:
    Event switchToThread(Thread upNext);

    Custom();

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
