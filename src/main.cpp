#include "Options.h"
#include "SimpleSynth.h"
#include <glibmm.h>
#include <iostream>

static Glib::RefPtr<Glib::MainLoop> theMainLoop;

static void quit(int)
{
  if(theMainLoop)
    theMainLoop->quit();
}

void connectSignals()
{
  signal(SIGINT, quit);
  signal(SIGQUIT, quit);
  signal(SIGTERM, quit);
}

void runMainLoop()
{
  theMainLoop = Glib::MainLoop::create();
  theMainLoop->run();
}

int main(int args, char *argv[])
{
  Glib::init();

  connectSignals();

  Options options(args, argv);

  auto synth = std::make_unique<SimpleSynth>(options);
  synth->start();
  runMainLoop();
  synth->stop();

  return EXIT_SUCCESS;
}
