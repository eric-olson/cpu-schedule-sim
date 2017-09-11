#include "event.h"
#include "filehandler.h"

int main(int argc, char *argv[]) {
  // update flags
  update_flags(argc, argv);

  // create event handler from file
  EventHandler eh = loadFile(get_filename());

  // run simulation
  eh.runSimulation();

  return 0;
}
