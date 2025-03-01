#include "Robota.h"
#include "Arduino.h"

Robota::Robota() {}

void Robota::init() {
  for (int i = 0; i < MAX_MODULE_AMOUNT; i++) {
    if (modules[i] == (Module *)0) {
      continue;
    } else {
      modules[i]->robota = this;
      modules[i]->init();
    }
  }
}

void Robota::tick() {
  for (uint16_t i = 0; i < MAX_MODULE_AMOUNT; i++) {
    if (modules[i] == (Module *)0) {
      continue;
    } else {
      modules[i]->tick();
    }
  }
  ticks++;
}

uint32_t Robota::getTicks() {
  return ticks;
}

int16_t Robota::addModule(Module *module) {
  for (uint16_t i = 0; i < MAX_MODULE_AMOUNT; i++) {
    if (modules[i] == (Module *)0) {
      modules[i] = module;
      moduleTypes[i] = module->getType();
      return i;
    }
  }
  return 0;
}

Module *Robota::getModule(int16_t index) {
  return modules[index];
}

Module *Robota::getModule(int16_t type, int16_t index) {
  uint16_t previous = 0, i;
  for (i = 0; i < MAX_MODULE_AMOUNT; i++) {
    if (moduleTypes[i] == type) {
      if (previous == index) {
        return modules[i];
      } else {
        previous++;
      }
    } else {
      // do nothing
    }
  }
  return (Module *)0;
}

Module *Robota::getFirstModule(int16_t type) {
  for (uint16_t i = 0; i < MAX_MODULE_AMOUNT; i++) {
    if (moduleTypes[i] == type)
      return modules[i];
  }
  return (Module *)0;
}
