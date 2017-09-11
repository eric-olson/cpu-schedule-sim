#pragma once

#include "thread.h"

#include <string>

using namespace std;

static string PriorityNames[4] = {"SYSTEM", "INTERACTIVE", "NORMAL", "BATCH"};
string priorityToString(Priority p);

class Process {
  private:
    int pid;
    int switchOverhead;
    Priority priority;
    queue<Thread> threads;

  public:
    // constructors
    Process();
    Process(int pid, int switchOverhead, Priority priority);

    // addThread will add a thread to pid queue
    void addThread(Thread t);

    // getters
    int getPid();
    int getSwitchOverhead();
    Priority getPriority();
    Thread getNextThread();
    Thread popThread();
};

