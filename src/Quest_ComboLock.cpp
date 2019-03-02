#include "Quest_ComboLock.h"

Quest_ComboLock::Quest_ComboLock(uint16_t *key, uint8_t keyLength)
{
    this->key = key;
    this->keyLength = keyLength;
    unlocked = false;
    keyPosition = 0;
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
        if (keyPosition == keyLength)
        {
            unlocked = true;
        }
        return true;
    }

    keyPosition = 0;
    return false;
}

void Quest_ComboLock::unlock()
{
    unlocked = true;
    keyPosition = keyLength;
}

void Quest_ComboLock::lock()
{
    unlocked = false;
    keyPosition = 0;
}
