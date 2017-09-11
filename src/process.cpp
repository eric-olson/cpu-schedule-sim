#include "process.h"

#include <iostream>

using namespace std;

string priorityToString(Priority p) {
  return PriorityNames[p];
}

Process::Process() {
};

Process::Process(int pid, int switchOverhead, Priority priority) {
  this->pid = pid;
  this->switchOverhead = switchOverhead;
  this->priority = priority;
};

void Process::addThread(Thread t) {
  threads.push(t);
};

int Process::getPid() {
  return pid;
};

int Process::getSwitchOverhead() {
  return switchOverhead;
};

Priority Process::getPriority() {
  return priority;
};

Thread Process::getNextThread() {
  return threads.front();
};

Thread Process::popThread() {
  if (!threads.empty()) {
    Thread t = threads.front();
    threads.pop();
    return t;
  }
  else
    return Thread(-1, -1, -1, -1, -1, SYSTEM);
};
