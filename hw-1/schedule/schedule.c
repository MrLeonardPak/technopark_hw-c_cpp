#include "schedule.h"
#include "schedule_private.h"

// + 1 для '\0'
#define MAX_CHARS_IN_SUBJECT 20 + 1
#define MAX_CHARS_IN_TEACHER 20 + 1
#define MAX_CHARS_IN_CLASSROOM 5 + 1
#define MAX_CHARS_IN_BUFFER 20

static const unsigned int kAcademicHourInMinutes = 40;
static const unsigned int kMinutesInHour = 60;
static const unsigned int kSecondsInMinute = 60;
static const unsigned int kHoursInDay = 24;
static const int kMaxYear = 4;
static const int kMaxGroups = 2;
static const int kDaysInWeek = 7;
static const unsigned int kMinArrSize = 2;

extern inline void free_string(char* str);
extern inline void free_lesson(Lesson* const lesson);
extern inline void copy_string(char src[], size_t len, char** dst);
extern inline size_t calculate_group_year_index(int const group,
                                                int const year);

int AddBeginTime(FILE* file, time_t* begin_time) {
  printf("Start time (format:hh mm):\n");
  char* check_gets = NULL;
  char buf[MAX_CHARS_IN_BUFFER] = {0};
  check_gets = fgets(buf, MAX_CHARS_IN_BUFFER, file);
  if (check_gets) {
    int check = -1;
    unsigned int hh = 0;
    unsigned int mm = 0;
    check = sscanf(buf, "%u %u", &hh, &mm);
    if ((check == 2) && (hh <= kHoursInDay) && (mm <= kMinutesInHour)) {
      *begin_time = (hh * kMinutesInHour + mm) * kSecondsInMinute;
      return 0;
    }
  }
  return -1;
}

int AddLessonType(FILE* file, LessonType* type) {
  printf("Lesson type (lecture:1, seminar:2):\n");
  char* check_gets = NULL;
  char buf[MAX_CHARS_IN_BUFFER] = {0};
  check_gets = fgets(buf, MAX_CHARS_IN_BUFFER, file);
  if (check_gets) {
    int check = -1;
    int type_buf = 0;
    check = sscanf(buf, "%d", &type_buf);
    if (check == 1) {
      switch (type_buf) {
        case lecture:
          *type = lecture;
          break;
        case seminar:
          *type = seminar;
          break;
        default:
          return -1;
      }
      return 0;
    }
  }
  return -1;
}

int AddDuration(FILE* file, time_t* duration) {
  printf("Duration n academic hour (max: %u):\n", kHoursInDay);
  char* check_gets = NULL;
  char buf[MAX_CHARS_IN_BUFFER] = {0};
  check_gets = fgets(buf, MAX_CHARS_IN_BUFFER, file);
  if (check_gets) {
    int check = -1;
    unsigned int time_buf = 0;
    check = sscanf(buf, "%u", &time_buf);
    if ((check == 1) && (time_buf <= kHoursInDay)) {
      *duration = time_buf * kAcademicHourInMinutes * kSecondsInMinute;
      return 0;
    }
  }
  return -1;
}

int AddSubject(FILE* file, char** subject) {
  printf("Subject (%d symbols):\n", MAX_CHARS_IN_SUBJECT - 1);  // - 1 для '\0'
  char sub_buf[MAX_CHARS_IN_SUBJECT] = {0};
  char* check = NULL;
  check = fgets(sub_buf, MAX_CHARS_IN_SUBJECT, file);
  if (check) {
    copy_string(sub_buf, strlen(sub_buf), subject);
    return 0;
  }
  return -1;
}

int AddTeacher(FILE* file, char** teacher) {
  printf("Teacher (%d symbols):\n", MAX_CHARS_IN_SUBJECT - 1);  // - 1 для '\0'
  char teacher_buf[MAX_CHARS_IN_TEACHER] = {0};
  char* check = NULL;
  check = fgets(teacher_buf, MAX_CHARS_IN_SUBJECT, file);
  if (check) {
    copy_string(teacher_buf, strlen(teacher_buf), teacher);
    return 0;
  }
  return -1;
}

int AddClassroom(FILE* file, char** classroom) {
  printf("Classroom (%d symbols):\n",
         MAX_CHARS_IN_CLASSROOM - 1);  // - 1 для '\0'
  char classroom_buf[6] = {0};
  char* check = NULL;
  check = fgets(classroom_buf, MAX_CHARS_IN_CLASSROOM, file);
  if (check) {
    copy_string(classroom_buf, strlen(classroom_buf), classroom);
    return 0;
  }
  return -1;
}

int AddYear(FILE* file, int* year) {
  printf("Year (max: %u):\n", kMaxYear);
  return GetYear(file, year);
}

int AddGroup(FILE* file, int* group) {
  printf("Group (max: %u):\n", kMaxGroups);
  return GetGroup(file, group);
}

void PrintLesson(Lesson const* lesson) {
  struct tm* begin_time_buf = gmtime(&(lesson->begin_time));
  printf("Begin time:\t%d:%d\n", begin_time_buf->tm_hour,
         begin_time_buf->tm_min);
  switch (lesson->type) {
    case lecture:
      printf("Type:\t\tlecture\n");
      break;
    case seminar:
      printf("Type:\t\tseminar\n");
      break;
    default:
      printf("Type:\t\tnone\n");
      break;
  }
  struct tm* duration_buf = gmtime(&(lesson->duration));
  printf("Duration:\t%d:%d\n", duration_buf->tm_hour, duration_buf->tm_min);
  printf("Subject:\t%s\n", lesson->subject);
  printf("Teacher:\t%s\n", lesson->teacher);
  printf("Classroom:\t%s\n", lesson->classroom);
  printf("Year:\t%d\n", lesson->year);
  printf("Group:\t%d\n", lesson->group);
}

int GetDay(FILE* file, int* day) {
  printf("Choose day in week (monday-1 ... sunday-7):\n");
  char* check_gets = NULL;
  char buf[MAX_CHARS_IN_BUFFER] = {0};
  check_gets = fgets(buf, MAX_CHARS_IN_BUFFER, file);
  if (check_gets) {
    int day_buf = 0;
    int check = -1;
    check = sscanf(buf, "%d", &day_buf);
    if ((check == 1) && (day_buf <= kDaysInWeek) && (day_buf > 0)) {
      *day = day_buf;
      return 0;
    }
  }
  return -1;
}

int CreateSchedule(FILE* file, Lessons** schedule) {
  // Выделяем память для хранения расписания на каждый день недели для каждой
  // группы + курс
  *schedule =
      (Lessons*)calloc(kDaysInWeek * kMaxGroups * kMaxYear, sizeof(Lessons));
  int more = 1;
  int day = 0;
  while (more) {
    if (GetDay(file, &day))
      return -1;
    AddLesson(file, schedule[day - 1]);
    printf("More? (0 - no, 1 - yes)\n");
    char* check_gets = NULL;
    char buf[MAX_CHARS_IN_BUFFER] = {0};
    check_gets = fgets(buf, MAX_CHARS_IN_BUFFER, file);
    if (check_gets) {
      int check = -1;
      check = sscanf(buf, "%d", &more);
      if (check != 1)
        return -1;
    } else
      return -1;
  }
  return 0;
}

int AddLesson(FILE* file, Lessons* lesson) {
  static Lesson lesson_buf;
  if (AddBeginTime(file, &lesson_buf.begin_time))
    return -1;
  if (AddLessonType(file, &lesson_buf.type))
    return -1;
  if (AddDuration(file, &lesson_buf.duration))
    return -1;
  if (AddSubject(file, &lesson_buf.subject))
    return -1;
  if (AddTeacher(file, &lesson_buf.teacher))
    return -1;
  if (AddClassroom(file, &lesson_buf.classroom))
    return -1;
  if (AddYear(file, &lesson_buf.year))
    return -1;
  if (AddGroup(file, &lesson_buf.group))
    return -1;
  // В начале хранятся все кусры 1 группы, затем все курсы 2 группы ...
  size_t group_year =
      calculate_group_year_index(lesson_buf.group, lesson_buf.year);
  if ((lesson[group_year].real_size == 0) &&
      (lesson[group_year].use_size == 0)) {
    lesson[group_year].real_size = kMinArrSize;
    lesson[group_year].lessons =
        (Lesson*)malloc(lesson[group_year].real_size * sizeof(Lesson));
  } else if (lesson[group_year].use_size == lesson[group_year].real_size) {
    lesson[group_year].real_size *= 2;
    lesson[group_year].lessons = (Lesson*)realloc(lesson[group_year].lessons,
                                                  lesson[group_year].real_size);
  } else
    return -1;

  ++lesson[group_year].use_size;
  size_t i = 0;
  if (lesson[group_year].use_size > 1) {
    for (; (i <= lesson[group_year].use_size - 2) &&
           (lesson[group_year]
                .lessons[lesson[group_year].use_size - 2 - i]
                .begin_time > lesson_buf.begin_time);
         ++i) {
      memcpy(&lesson[group_year].lessons[lesson[group_year].use_size - 1 - i],
             &lesson[group_year].lessons[lesson[group_year].use_size - 2 - i],
             sizeof(Lesson));
    }
  }
  memcpy(&lesson[group_year].lessons[lesson[group_year].use_size - 1 - i],
         &lesson_buf, sizeof(Lesson));
  return 0;
}

int PrintSchedule(FILE* file, Lessons** schedule) {
  int day = 0;
  if (GetDay(file, &day))
    return -1;

  printf("Choose year (max: %u):\n", kMaxYear);
  int year = 0;
  if (GetYear(file, &year))
    return -1;

  printf("Choose group (max: %u):\n", kMaxGroups);
  int group = 0;
  if (GetGroup(file, &group))
    return -1;

  size_t group_year = calculate_group_year_index(group, year);
  for (size_t i = 0; i < schedule[day - 1][group_year].use_size; ++i) {
    PrintLesson(&schedule[day - 1][group_year].lessons[i]);
  }
  return 0;
}

int GetYear(FILE* file, int* year) {
  char* check_gets = NULL;
  char buf[MAX_CHARS_IN_BUFFER] = {0};
  check_gets = fgets(buf, MAX_CHARS_IN_BUFFER, file);
  if (check_gets) {
    int check = -1;
    int year_buf = 0;
    check = sscanf(buf, "%d", &year_buf);
    if ((check == 1) && (year_buf <= kMaxYear) && (year_buf > 0)) {
      *year = year_buf;
      return 0;
    }
  }
  return -1;
}

int GetGroup(FILE* file, int* group) {
  char* check_gets = NULL;
  char buf[MAX_CHARS_IN_BUFFER] = {0};
  check_gets = fgets(buf, MAX_CHARS_IN_BUFFER, file);
  if (check_gets) {
    int check = -1;
    int group_buf = 0;
    check = sscanf(buf, "%d", &group_buf);
    if ((check == 1) && (group_buf <= kMaxGroups) && (group_buf > 0)) {
      *group = group_buf;
      return 0;
    }
  }
  return -1;
}

void DeleteSchedule(Lessons** schedule) {
  if (*schedule) {
    size_t size_i = kDaysInWeek * kMaxGroups * kMaxYear;
    for (size_t i = 0; i < size_i; ++i) {
      size_t size_j = (*schedule)[i].use_size;
      for (size_t j = 0; j < size_j; ++j) {
        free_lesson(&((*schedule)[i].lessons[j]));
      }
      if ((*schedule)[i].lessons) {
        free((*schedule)[i].lessons);
        (*schedule)[i].lessons = NULL;
      }
    }
    free(*schedule);
    *schedule = NULL;
  }
}
