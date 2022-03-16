#ifndef HW1_SCHEDULE_PRIVATE_H
#define HW1_SCHEDULE_PRIVATE_H

#include <stdio.h>
#include <time.h>

int AddBeginTime(FILE* file, time_t* startTime);
int AddLessonType(FILE* file, LessonType* type);
int AddDuration(FILE* file, time_t* duration);
int AddSubject(FILE* file, char** subject);
int AddTeacher(FILE* file, char** teacher);
int AddClassroom(FILE* file, char** classroom);
int AddYear(FILE* file, int* year);
int AddGroup(FILE* file, int* group);
int GetDay(FILE* file, int* day);
int GetYear(FILE* file, int* year);
int GetGroup(FILE* file, int* group);
int AddLesson(FILE* file, Lessons* lesson);
void PrintLesson(Lesson const* lesson);
inline void copy_string(char src[], size_t len, char** dst);
inline size_t calculate_group_year_index(int const group, int const year);

#endif  // HW1_SCHEDULE_PRIVATE_H