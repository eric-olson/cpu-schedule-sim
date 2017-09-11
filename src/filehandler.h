#pragma once

#include "event.h"
#include "process.h"

#include <map>

EventHandler loadFile(string filePath);

void update_flags(int argc, char **argv);

bool get_per_thread();
bool get_verbose();
// TODO: algorithm
bool get_help();
string get_filename();
