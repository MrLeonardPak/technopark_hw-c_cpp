#ifndef HW1_SCHEDULE_H
#define HW1_SCHEDULE_H

#include <stdio.h>
#include <time.h>

#define DAYS_IN_WEEK 7

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
  Lesson* lessons;  // Массив распределения по группа + курс
} Lessons;

int CreateSchedule(FILE* file, Lessons** schedule);
void DeleteSchedule(Lessons** schedule);
int PrintSchedule(FILE* file, Lessons** schedule);
int AddBeginTime(FILE* file, time_t* startTime);
int AddLessonType(FILE* file, LessonType* type);
int AddDuration(FILE* file, time_t* duration);
int AddSubject(FILE* file, char** subject);
int AddTeacher(FILE* file, char** teacher);
int AddClassroom(FILE* file, char** classroom);
int AddYear(FILE* file, int* year);
int AddGroup(FILE* file, int* group);
void PrintLesson(Lesson const* lesson);
int AddLesson(FILE* file, Lessons* lesson);
int GetDay(FILE* file, int* day);
int GetYear(FILE* file, int* year);
int GetGroup(FILE* file, int* group);

#endif  // HW1_SCHEDULE_H