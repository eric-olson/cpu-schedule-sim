#include "scheduler.h"

#include <iostream>
#include <sstream>

using namespace std;

Scheduler::Scheduler() {
  Thread t(-1, -1, 0, 0, 0, SYSTEM);
  running = t;
}

