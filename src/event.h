#pragma once

#include "scheduler.h"
#include "fcfs.h"
#include "roundrobin.h"
#include "processpriority.h"
#include "custom.h"
#include "cpu.h"
#include "process.h"

#include <map>
#include <queue>

struct ThreadStats {
  int pid;
  int tid;
  int arr;
  int cpu;
  int io;
  int end;
  int trt;
  int response = -1;
};

struct SimStats {
  int count;
  int totalResponse;
  int totalTurnaround;
};

// overload operator< for priority queue
bool operator<(const Event& lhs, const Event& rhs);

class EventHandler {
  private:
    int time = 0;
    int serviceTime = 0;
    int ioTime = 0;
    int dispatchTime = 0;
    int idleTime = 0;
    bool verbose = false;
    bool perThread = false;

    std::map<int, Process> processes;

    std::priority_queue<Event> events;

    // scheduler
    Scheduler *scheduler;
    FCFS fcfs;
    RoundRobin rr;
    ProcessPriority pp;
    Custom custom;

    // simulation stats
    std::map<Priority, SimStats> simStatsMap;

    // handler for generic event
    void handleEvent(Event e);

    // handlers for each event type
    void handleThreadArrived(Event e);
    void handleThreadDispatchCompleted(Event e);
    void handleProcessDispatchCompleted(Event e);
    void handleCPUBurstCompleted(Event e);
    void handleIOBurstCompleted(Event e);
    void handleThreadCompleted(Event e);
    void handleThreadPreempted(Event e);
    void handleDispatcherInvoked(Event e);
    void handleSimulationComplete(Event e);

  public:
    // constructor
    EventHandler();
    EventHandler(bool verbose, bool perThread, int schedType);

    // thread stats for per-thread output
    std::map<int, std::map<int, ThreadStats>> stats;

    // update map of processes
    void updateProcessMap(std::map<int, Process> p);

    // addEvent
    void addEvent(Event e);

    // runSimulation runs through all events
    void runSimulation();

    // getEvents
    std::priority_queue<Event> getEvents();

    // getProcess
    Process getProcess(int pid);
    Priority getProcessPriority(int pid);
    string getPriorityString(int pid);

    // printStats
    void printStats();

    // printSummary
    void printSummary();
};

