#include <stdlib.h>
#include <unistd.h>

#include "delune-application.h"
#include "ui.h"
#include "sound.h"

int main(int argc, char **argv) {
  int status;
  status = g_application_run(G_APPLICATION(delune_application_new()), argc, argv);

  /* close_soundio(); */

  return status;
}
