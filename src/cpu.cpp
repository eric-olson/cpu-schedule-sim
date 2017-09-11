#include "cpu.h"

using namespace std;

CPU::CPU() {
  currentTime = 0;
  startedTime = 0;
  currentStatus = IDLE;
}

void CPU::setTime(int time) {
  currentTime = time;
}

CPUBurst CPU::startSwitch(CPUBurst b, int switchTime) {
  // update info on burst to return
  CPUBurst returnBurst = currentBurst;
  int delta = currentTime - startedTime;
  returnBurst.cpuTime -= delta;

  // update CPU with new burst info
  currentBurst = b;
  startedTime = currentTime + switchTime;

  // update CPU status
  currentStatus = SWITCHING;

  return returnBurst;
}

int CPU::startBurst() {
  // TODO: do shtuff
  currentStatus = RUNNING;

  return currentBurst.cpuTime;
  // return length of current burst
}

Status CPU::getStatus() {
  return currentStatus;
}

CPUBurst CPU::getCurrentBurst() {
  return currentBurst;
}
