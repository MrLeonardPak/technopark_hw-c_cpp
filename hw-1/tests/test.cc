/**
 * @file test.cc
 * @author your name (you@domain.com)
 * @brief
 * lcov -t "tests/tests_schedule" -o coverage.info -c -d schedule/
 * genhtml -o report coverage.info
 * cd report && python3 -m http.server 8000
 * @version 0.1
 * @date 2022-03-16
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "gtest/gtest.h"

extern "C" {
#include "schedule.h"
#include "schedule_private.h"
}

#define GOOD 1
#define BAD 0

TEST(MAIN_TESTS, RIGHT_INPUT_TEST) {
  char input[] =
      "1\n12 00\n1\n3\nMath\nVasileva D.S.\n904l\n1\n2\n0\n1\n1\n2\n";
  FILE* stream = fmemopen(input, strlen(input), "r");
  Lessons* schedule = NULL;

  EXPECT_EQ(GOOD, CreateSchedule(stream, &schedule));
  EXPECT_EQ(GOOD, PrintSchedule(stream, schedule));
  ASSERT_NO_THROW(DeleteSchedule(&schedule));

  fclose(stream);
}

TEST(MAIN_TESTS, WRONG_INPUT_TEST) {
  char input[] = "1\n12 00\n1\n3\nMath\nVasileva D.S.\n904l\n1\n2\nq\n";
  FILE* stream = fmemopen(input, strlen(input), "r");
  Lessons* schedule = NULL;

  EXPECT_EQ(BAD, CreateSchedule(stream, &schedule));
  ASSERT_NO_THROW(DeleteSchedule(&schedule));

  fclose(stream);
}

TEST(schedule_private_TESTS, AddBeginTime_TEST) {
  char input[] = "12 00\n1200\n12:00\nqwerty\n";
  FILE* stream = fmemopen(input, strlen(input), "r");
  time_t timer = 0;

  EXPECT_EQ(GOOD, AddBeginTime(stream, &timer));
  EXPECT_EQ(BAD, AddBeginTime(stream, &timer));
  EXPECT_EQ(BAD, AddBeginTime(stream, &timer));
  EXPECT_EQ(BAD, AddBeginTime(stream, &timer));

  fclose(stream);
}

TEST(schedule_private_TESTS, AddLessonType_TEST) {
  char input[] = "2\n0\nqwerty\n";
  FILE* stream = fmemopen(input, strlen(input), "r");
  LessonType type = lecture;

  EXPECT_EQ(GOOD, AddLessonType(stream, &type));
  EXPECT_EQ(BAD, AddLessonType(stream, &type));
  EXPECT_EQ(BAD, AddLessonType(stream, &type));

  fclose(stream);
}

TEST(schedule_private_TESTS, AddDuration_TEST) {
  char input[] = "5\n30\nqwerty\n";
  FILE* stream = fmemopen(input, strlen(input), "r");
  time_t timer = 0;

  EXPECT_EQ(GOOD, AddDuration(stream, &timer));
  EXPECT_EQ(BAD, AddDuration(stream, &timer));
  EXPECT_EQ(BAD, AddDuration(stream, &timer));

  fclose(stream);
}

TEST(schedule_private_TESTS, AddSubject_TEST) {
  char input[] = "Ababab Ababab\nTurututu Turututu\n";
  FILE* stream = fmemopen(input, strlen(input), "r");
  char* str = NULL;

  EXPECT_EQ(GOOD, AddSubject(stream, &str));
  EXPECT_EQ(GOOD, AddSubject(stream, &str));
  EXPECT_EQ(BAD, AddSubject(stream, &str));

  free(str);
  fclose(stream);
}

TEST(schedule_private_TESTS, AddTeacher_TEST) {
  char input[] = "Ababab Ababab\nTurututu Turututu\n";
  FILE* stream = fmemopen(input, strlen(input), "r");
  char* str = NULL;

  EXPECT_EQ(GOOD, AddTeacher(stream, &str));
  EXPECT_EQ(GOOD, AddTeacher(stream, &str));
  EXPECT_EQ(BAD, AddTeacher(stream, &str));

  free(str);
  fclose(stream);
}

TEST(schedule_private_TESTS, AddClassroom_TEST) {
  char input[] = "385\n221m\n";
  FILE* stream = fmemopen(input, strlen(input), "r");
  char* str = NULL;

  EXPECT_EQ(GOOD, AddClassroom(stream, &str));
  EXPECT_EQ(GOOD, AddClassroom(stream, &str));
  EXPECT_EQ(BAD, AddClassroom(stream, &str));

  free(str);
  fclose(stream);
}

TEST(schedule_private_TESTS, GetDay_TEST) {
  char input[] = "2\n12\nqwerty\n";
  FILE* stream = fmemopen(input, strlen(input), "r");
  int num = 0;

  EXPECT_EQ(GOOD, GetDay(stream, &num));
  EXPECT_EQ(BAD, GetDay(stream, &num));
  EXPECT_EQ(BAD, GetDay(stream, &num));

  fclose(stream);
}

TEST(schedule_private_TESTS, GetYear_TEST) {
  char input[] = "2\n12\nqwerty\n";
  FILE* stream = fmemopen(input, strlen(input), "r");
  int num = 0;

  EXPECT_EQ(GOOD, GetYear(stream, &num));
  EXPECT_EQ(BAD, GetYear(stream, &num));
  EXPECT_EQ(BAD, GetYear(stream, &num));

  fclose(stream);
}

TEST(schedule_private_TESTS, GetGroup_TEST) {
  char input[] = "1\n12\nqwerty\n";
  FILE* stream = fmemopen(input, strlen(input), "r");
  int num = 0;

  EXPECT_EQ(GOOD, GetGroup(stream, &num));
  EXPECT_EQ(BAD, GetGroup(stream, &num));
  EXPECT_EQ(BAD, GetGroup(stream, &num));

  fclose(stream);
}