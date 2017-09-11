#pragma once

#include "cpu.h"

#include <queue>

enum ThreadState { READY, BLOCKED, COMPLETE };
enum Priority { SYSTEM, INTERACTIVE, NORMAL, BATCH };

class Thread {
  private:
    int tid;
    int parentPid;
    int arrivalTime;
    int threadOverhead;
    int processOverhead;
    Priority priority;

    ThreadState state = READY;
    std::queue<CPUBurst> bursts;

  public:
    // constructors
    Thread();
    Thread(int tid, int parentPid, int arrivalTime, int threadOverhead, int processOverhead, Priority priority);

    // currentQueue holds info about which queue thread belongs in
    int currentQueue = 0;
    // addBurst is used in creation of threads
    void addBurst(CPUBurst b);

    // getNextBurst will return the next burst of a thread
    CPUBurst getNextBurst();
    CPUBurst popBursts();

    // updateNextBurstTime will subtract time from the next burst's CPU time
    void updateNextBurstTime(int elapsed);

    // getters
    int getTid();
    int getParentPid();
    int getArrivalTime();
    int getThreadOverhead();
    int getProcessOverhead();
    Priority getPriority();

    // setter for state
    void setState(ThreadState s);
};
