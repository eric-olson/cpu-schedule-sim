#pragma once

// status gives info on CPU status
enum Status { RUNNING, SWITCHING, IDLE };

// CPUBurst holds info about burst
struct CPUBurst {
  int cpuTime;
  int ioTime;
};

class CPU {
  private:
    CPUBurst currentBurst;
    Status currentStatus;
    int startedTime;
    int currentTime;

  public:
    // constructor functions
    CPU();

    // setTime() will cause the CPU to update to the given time
    void setTime(int time);

    // startSwitch() is used to give the CPU a burst
    // returns last executed burst with updated CPU time
    CPUBurst startSwitch(CPUBurst b, int switchTime);

    // startBurst() starts the burst given in startSwitch()
    // this is necessary to allow the scheduler to determine next thread before
    // actually starting a CPU switch in order to give correct switch time
    int startBurst();

    // status returns current CPU status
    Status getStatus();
    CPUBurst getCurrentBurst();
};
