#include <Arduino.h>
#include "Module.h"

#ifndef Robota_h
#define Robota_h

#define MAX_MODULE_AMOUNT 64

// I chose this name to avoid confusion with Arduino's built in Robot class
class Robota : Module {
  // Global tick counter for modules that do tasks every n ticks
  uint32_t ticks;
  // Robot dimensions (in µm, along the x/y/z axis)
  uint16_t width, length, height;
  // Stored modules & types for quick type checking
  uint16_t moduleTypes[MAX_MODULE_AMOUNT];
  Module* modules[MAX_MODULE_AMOUNT];

public:
  Robota(); //TODO constructors with size
  void init();
  void tick();
  uint32_t getTicks();
  void setDimensions(uint16_t width, uint16_t length, uint16_t height);

  // returns the module index if successful, or -1 if failed
  int16_t addModule(Module* module);

  Module* getModule(int16_t index);
  Module* getModule(int16_t type, int16_t index);
  Module* getFirstModule(int16_t type);
};

#endif
