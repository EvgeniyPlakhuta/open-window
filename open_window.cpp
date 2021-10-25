#include <Wire.h>
#include <DS3231.h>

#define STEPS_TO_OPEN 150
#define DIR_PIN 2
#define STEP_PIN 3

DS3231 clock;
RTClib rtc;

struct Peremena {
  DateTime* startTime;
  DateTime* endTime;
};

DateTime* newTime(const char* time) {
  return new DateTime("Jan 01 1970", "");
}

Peremena END = Peremena();

Peremena MON[] = {
  Peremena { .startTime = newTime("08:40:00"), .endTime = newTime("08:45:00") },
  Peremena { .startTime = newTime("09:25:00"), .endTime = newTime("09:35:00") },
  Peremena { .startTime = newTime("10:15:00"), .endTime = newTime("10:30:00") },
  Peremena { .startTime = newTime("11:10:00"), .endTime = newTime("11:15:00") },
  Peremena { .startTime = newTime("11:55:00"), .endTime = newTime("12:00:00") },
  END
};
Peremena TUE[] = {
  Peremena { .startTime = newTime("08:40:00"), .endTime = newTime("08:45:00") },
  Peremena { .startTime = newTime("09:25:00"), .endTime = newTime("09:35:00") },
  Peremena { .startTime = newTime("10:15:00"), .endTime = newTime("10:30:00") },
  Peremena { .startTime = newTime("11:10:00"), .endTime = newTime("11:15:00") },
  Peremena { .startTime = newTime("11:55:00"), .endTime = newTime("12:00:00") },
  END
};
Peremena WED[] = {
  Peremena { .startTime = newTime("08:40:00"), .endTime = newTime("08:45:00") },
  Peremena { .startTime = newTime("09:25:00"), .endTime = newTime("09:35:00") },
  Peremena { .startTime = newTime("10:15:00"), .endTime = newTime("10:30:00") },
  Peremena { .startTime = newTime("11:10:00"), .endTime = newTime("11:15:00") },
  Peremena { .startTime = newTime("11:55:00"), .endTime = newTime("12:00:00") },
  END
};
Peremena THU[] = {
  Peremena { .startTime = newTime("08:40:00"), .endTime = newTime("08:45:00") },
  Peremena { .startTime = newTime("09:25:00"), .endTime = newTime("09:35:00") },
  Peremena { .startTime = newTime("10:15:00"), .endTime = newTime("10:30:00") },
  Peremena { .startTime = newTime("11:10:00"), .endTime = newTime("11:15:00") },
  Peremena { .startTime = newTime("11:55:00"), .endTime = newTime("12:00:00") },
  END
};
Peremena FRI[] = {
  Peremena { .startTime = newTime("08:40:00"), .endTime = newTime("08:45:00") },
  Peremena { .startTime = newTime("09:25:00"), .endTime = newTime("09:35:00") },
  Peremena { .startTime = newTime("10:15:00"), .endTime = newTime("10:30:00") },
  Peremena { .startTime = newTime("11:10:00"), .endTime = newTime("11:15:00") },
  Peremena { .startTime = newTime("11:55:00"), .endTime = newTime("12:00:00") },
  END
};
Peremena SAT[] = {
  Peremena { .startTime = newTime("08:40:00"), .endTime = newTime("08:45:00") },
  Peremena { .startTime = newTime("09:25:00"), .endTime = newTime("09:35:00") },
  Peremena { .startTime = newTime("10:15:00"), .endTime = newTime("10:30:00") },
  Peremena { .startTime = newTime("11:10:00"), .endTime = newTime("11:15:00") },
  Peremena { .startTime = newTime("11:55:00"), .endTime = newTime("12:00:00") },
  END
};
Peremena SUN[] = {
  END
};
Peremena WEEK_END[] = {};

Peremena* weekSchedule[] = {MON, TUE, WED, THU, FRI, SAT, SUN, WEEK_END};

byte currentDayIndex = 0;

void setup()
{
  Wire.begin();
  clock.setClockMode(false); // 24h
  currentDayIndex = clock.getDoW();

  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
}

byte compare(DateTime l, DateTime r) {
  if (l.hour() < r.hour()) return -1;
  else if (l.hour() > r.hour()) return 1;

  if (l.minute() < r.minute()) return -1;
  else if (l.minute() > r.minute()) return 1;

  if (l.second() < r.second()) return -1;
  else if (l.second() > r.second()) return 1;

  return 0;
}

void open_window() {
  digitalWrite(DIR_PIN, HIGH);

  for (int x = 0; x < STEPS_TO_OPEN; x++)
  {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(2000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(2000);
  }
}

void close_window() {
  digitalWrite(DIR_PIN, LOW);

  for (int x = 0; x < STEPS_TO_OPEN; x++)
  {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(2000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(2000);
  }
}

void wait_for_monday() {
  while (clock.getDoW() != 0) delay(7200000);
}

int is_end(Peremena peremena) {
  if (compare(*peremena.startTime, *END.startTime) == 0
      && compare(*peremena.endTime, *END.endTime) == 0) return 1;

  return 0;
}

void loop()
{
  Peremena* daySchedule = weekSchedule[currentDayIndex];
  while (daySchedule != WEEK_END) {
    for (int i = 0; is_end(daySchedule[i]) == 0; i++) {
      DateTime currentTime = rtc.now();
      Peremena peremena = daySchedule[i];
      while (compare(currentTime, *peremena.startTime) == -1) {
        delay(2000);
        currentTime = rtc.now();
      }
      open_window();
      currentTime = rtc.now();
      while (compare(currentTime, *peremena.endTime) == -1) {
        delay(2000);
        currentTime = rtc.now();
      }
      close_window();
    }
    daySchedule = weekSchedule[++currentDayIndex];
  }

  wait_for_monday();
  currentDayIndex = 0;
}