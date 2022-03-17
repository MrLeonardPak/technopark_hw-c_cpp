#ifndef HW1_SCHEDULE_H
#define HW1_SCHEDULE_H

#include <stdio.h>
#include <time.h>

typedef enum { lecture = 1, seminar } LessonType;

typedef struct {
  time_t begin_time;
  LessonType type;
  time_t duration;
  char* subject;
  char* teacher;
  char* classroom;
  int year;
  int group;
} Lesson;

typedef struct {
  size_t use_size;
  size_t real_size;
  Lesson* lessons;  // Массив предметов для одной группы + курс
} Lessons;

int CreateSchedule(FILE* file, Lessons** schedule);
void DeleteSchedule(Lessons** schedule);
int PrintSchedule(FILE* file, Lessons const* schedule);

#endif  // HW1_SCHEDULE_H