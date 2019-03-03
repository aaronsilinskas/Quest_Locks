#ifndef quest_combolock_h
#define quest_combolock_h

#include <Arduino.h>
#include <Quest_Event.h>
#include <Quest_EventQueue.h>

class Quest_ComboLock
{
public:
  bool unlocked = false;
  uint8_t keyPosition;
  uint8_t keyLength;

  Quest_ComboLock(uint16_t *key, uint8_t keyLength, Quest_EventQueue *eventQueue);
  bool tryStep(uint16_t value);
  void unlock();
  void lock();

private:
  uint16_t *key;
  Quest_EventQueue *eventQueue;
  uint8_t tmpEventData[2];
};

#endif
