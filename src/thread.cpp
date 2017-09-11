#include "thread.h"

Thread::Thread() {
}

Thread::Thread(int tid, int parentPid, int arrivalTime, int threadOverhead, int processOverhead, Priority priority) {
  this->tid = tid;
  this->parentPid = parentPid;
  this->arrivalTime = arrivalTime;
  this->threadOverhead = threadOverhead;
  this->processOverhead = processOverhead;
  this->priority = priority;
}

void Thread::addBurst(CPUBurst b) {
  bursts.push(b);
}

CPUBurst Thread::getNextBurst() {
  return bursts.front();
}

CPUBurst Thread::popBursts() {
  CPUBurst b = bursts.front();
  bursts.pop();
  return b;
}

void Thread::updateNextBurstTime(int elapsed) {
  bursts.front().cpuTime -= elapsed;
}

int Thread::getTid() {
  return this->tid;
}

int Thread::getParentPid() {
  return this->parentPid;
}

int Thread::getArrivalTime() {
  return this->arrivalTime;
}
int Thread::getThreadOverhead() {
  return this->threadOverhead;
}

int Thread::getProcessOverhead() {
  return this->processOverhead;
}

Priority Thread::getPriority() {
  return priority;
}

void Thread::setState(ThreadState s) {
  this->state = s;
  return;
}
