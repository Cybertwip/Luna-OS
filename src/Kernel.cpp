#include "Engine.h"

extern "C" void _start() {
    GameEngine engine;
    engine.run();
}