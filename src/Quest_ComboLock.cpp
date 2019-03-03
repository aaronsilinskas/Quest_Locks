#include "Quest_ComboLock.h"

Quest_ComboLock::Quest_ComboLock(uint16_t *key, uint8_t keyLength, Quest_EventQueue *eventQueue)
{
    this->key = key;
    this->keyLength = keyLength;
    this->eventQueue = eventQueue;
    this->unlocked = false;
    this->keyPosition = 0;
}

bool Quest_ComboLock::tryStep(uint16_t value)
{
    if (unlocked)
    {
        return false;
    }

    if (value == key[keyPosition])
    {
        keyPosition++;
    }
    else
    {
        keyPosition = 0;
    }

    if (eventQueue != NULL)
    {
        tmpEventData[0] = keyPosition;
        tmpEventData[1] = keyLength;
        eventQueue->offer(QE_ID_PROGRESS, tmpEventData, 16);
    }

    if (keyPosition == keyLength)
    {
        unlock();
    }

    return keyPosition != 0;
}

void Quest_ComboLock::unlock()
{
    unlocked = true;
    keyPosition = keyLength;

    if (eventQueue != NULL)
    {
        eventQueue->offer(QE_ID_UNLOCKED);
    }
}

void Quest_ComboLock::lock()
{
    unlocked = false;
    keyPosition = 0;

    if (eventQueue != NULL)
    {
        eventQueue->offer(QE_ID_LOCKED);
    }
}
