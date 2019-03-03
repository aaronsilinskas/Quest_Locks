#include <Arduino.h>
#include <unity.h>

#include "Quest_ComboLock.h"

#define KEY_MAX_LENGTH 10
uint16_t key[KEY_MAX_LENGTH];

#define EVENT_QUEUE_SIZE 20
Event eventQueue[EVENT_QUEUE_SIZE];

uint8_t teamID = randomBits(QE_TEAM_ID_BITS);
uint8_t playerID = randomBits(QE_PLAYER_ID_BITS);

void setupTests()
{
    randomSeed(analogRead(0));
    for (uint8_t i = 0; i < KEY_MAX_LENGTH; i++)
    {
        key[i] = random(0xFFFF);
    }
}
void test_locked_by_default()
{
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH, NULL);
    TEST_ASSERT_FALSE(cl.unlocked);
}

void test_valid_steps_progress_key()
{
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH, NULL);
    for (uint8_t step = 0; step < KEY_MAX_LENGTH; step++)
    {
        uint16_t validStep = key[step];
        TEST_ASSERT_TRUE(cl.tryStep(validStep));
        TEST_ASSERT_EQUAL(step + 1, cl.keyPosition);
    }
}

void test_invalid_step_resets_key()
{
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH, NULL);
    uint8_t stepToChooseInvalidValue = random(KEY_MAX_LENGTH);

    // choose valid values until an invalid value will be tests
    for (uint8_t step = 0; step < stepToChooseInvalidValue; step++)
    {
        uint16_t validStep = key[step];
        TEST_ASSERT_TRUE(cl.tryStep(validStep));
    }

    // choose an invalid value
    uint16_t invalidStep = !(key[stepToChooseInvalidValue]);
    TEST_ASSERT_FALSE(cl.tryStep(invalidStep));
    TEST_ASSERT_EQUAL(0, cl.keyPosition);
}

void test_all_valid_steps_unlock_key()
{
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH, NULL);
    // try valid values for all but the last step
    for (uint8_t step = 0; step < KEY_MAX_LENGTH - 1; step++)
    {
        cl.tryStep(key[step]);
        TEST_ASSERT_FALSE(cl.unlocked);
    }
    // try last step
    cl.tryStep(key[KEY_MAX_LENGTH - 1]);
    TEST_ASSERT_TRUE(cl.unlocked);
}

void test_trying_after_unlock_does_nothing()
{
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH, NULL);
    // enter values to unlock
    for (uint8_t step = 0; step < KEY_MAX_LENGTH; step++)
    {
        cl.tryStep(key[step]);
    }
    TEST_ASSERT_TRUE(cl.unlocked);

    // try a value after unlocked
    TEST_ASSERT_FALSE(cl.tryStep(123));
    TEST_ASSERT_EQUAL(cl.keyLength, cl.keyPosition);
    TEST_ASSERT_TRUE(cl.unlocked);
}

void test_force_unlock()
{
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH, NULL);
    cl.unlock();

    TEST_ASSERT_TRUE(cl.unlocked);
    TEST_ASSERT_EQUAL(cl.keyLength, cl.keyPosition);
}

void test_force_lock()
{
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH, NULL);
    // enter values to unlock
    for (uint8_t step = 0; step < KEY_MAX_LENGTH; step++)
    {
        cl.tryStep(key[step]);
    }

    cl.lock();
    TEST_ASSERT_FALSE(cl.unlocked);
    TEST_ASSERT_EQUAL(0, cl.keyPosition);
}

void test_valid_step_progress_event()
{
    uint8_t teamID = randomBits(QE_TEAM_ID_BITS);
    uint8_t playerID = randomBits(QE_PLAYER_ID_BITS);

    Quest_EventQueue eq = Quest_EventQueue(eventQueue, EVENT_QUEUE_SIZE, teamID, playerID);
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH, &eq);

    for (uint8_t i = 0; i < KEY_MAX_LENGTH; i++)
    {
        cl.tryStep(key[i]);
    }

    Event next;
    for (uint8_t i = 0; i < KEY_MAX_LENGTH; i++)
    {
        TEST_ASSERT_TRUE(eq.poll(&next));
        TEST_ASSERT_EQUAL(teamID, next.teamID);
        TEST_ASSERT_EQUAL(playerID, next.playerID);
        TEST_ASSERT_EQUAL(QE_ID_PROGRESS, next.eventID);
        TEST_ASSERT_EQUAL(16, next.dataLengthInBits);
        TEST_ASSERT_EQUAL(i + 1, next.data[0]);
        TEST_ASSERT_EQUAL(KEY_MAX_LENGTH, next.data[1]);
    }
}

void test_invalid_step_progress_event()
{
    Quest_EventQueue eq = Quest_EventQueue(eventQueue, EVENT_QUEUE_SIZE, teamID, playerID);
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH, &eq);

    uint16_t invalidStep = !(key[0]);
    TEST_ASSERT_FALSE(cl.tryStep(invalidStep));

    Event next;
    TEST_ASSERT_TRUE(eq.poll(&next));
    TEST_ASSERT_EQUAL(teamID, next.teamID);
    TEST_ASSERT_EQUAL(playerID, next.playerID);
    TEST_ASSERT_EQUAL(QE_ID_PROGRESS, next.eventID);
    TEST_ASSERT_EQUAL(16, next.dataLengthInBits);
    TEST_ASSERT_EQUAL(0, next.data[0]);
    TEST_ASSERT_EQUAL(KEY_MAX_LENGTH, next.data[1]);
}

void test_unlock_event()
{
    Quest_EventQueue eq = Quest_EventQueue(eventQueue, EVENT_QUEUE_SIZE, teamID, playerID);
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH, &eq);

    // enter values to unlock
    for (uint8_t step = 0; step < KEY_MAX_LENGTH; step++)
    {
        cl.tryStep(key[step]);
    }

    // ignore the progress events
    Event next;
    for (uint8_t step = 0; step < KEY_MAX_LENGTH; step++)
    {
        TEST_ASSERT_TRUE(eq.poll(&next));
        TEST_ASSERT_EQUAL(QE_ID_PROGRESS, next.eventID);
    }

    TEST_ASSERT_TRUE(eq.poll(&next));
    TEST_ASSERT_EQUAL(teamID, next.teamID);
    TEST_ASSERT_EQUAL(playerID, next.playerID);
    TEST_ASSERT_EQUAL(QE_ID_UNLOCKED, next.eventID);
    TEST_ASSERT_EQUAL(0, next.dataLengthInBits);
}

void test_force_unlock_event()
{
    Quest_EventQueue eq = Quest_EventQueue(eventQueue, EVENT_QUEUE_SIZE, teamID, playerID);
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH, &eq);

    cl.unlock();

    Event next;
    TEST_ASSERT_TRUE(eq.poll(&next));
    TEST_ASSERT_EQUAL(teamID, next.teamID);
    TEST_ASSERT_EQUAL(playerID, next.playerID);
    TEST_ASSERT_EQUAL(QE_ID_UNLOCKED, next.eventID);
    TEST_ASSERT_EQUAL(0, next.dataLengthInBits);
}

// test locked event

void setup()
{
    delay(4000);
    setupTests();

    UNITY_BEGIN();

    RUN_TEST(test_locked_by_default);
    RUN_TEST(test_valid_steps_progress_key);
    RUN_TEST(test_invalid_step_resets_key);
    RUN_TEST(test_all_valid_steps_unlock_key);
    RUN_TEST(test_trying_after_unlock_does_nothing);
    RUN_TEST(test_force_unlock);
    RUN_TEST(test_force_lock);
    RUN_TEST(test_valid_step_progress_event);
    RUN_TEST(test_invalid_step_progress_event);
    RUN_TEST(test_unlock_event);
    RUN_TEST(test_force_unlock_event);

    UNITY_END();
}

void loop()
{
}
