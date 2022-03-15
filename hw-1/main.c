/**
 * @file main.c
 * @author Пак Леонард
 * @brief Вариант #13
  Создать структуру для хранения расписания
 занятий: дня недели, времени начала, вида
 (лекция, семинар) и продолжительности
 занятий, названия дисциплины, фамилии
 преподавателя, номера (шифра) аудитории,
 курса, группы. Составить с ее
  использованием программу для определения
 перемещений по учебному корпусу при
  посещении всех занятий.
 * @version 0.1
 * @date 2022-03-06
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "schedule.h"

int main() {
  FILE* file = stdin;
  static Lessons* schedule = NULL;
  if (CreateSchedule(file, &schedule) == 0) {
    PrintSchedule(file, &schedule);
  };
  DeleteSchedule(&schedule);
  return 0;
}
