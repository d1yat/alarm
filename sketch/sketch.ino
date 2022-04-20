#include <SPI.h>
#include <SD.h>

#define PIN 4
#define LIST_SIZE 4

String* find_schedule(char* buffer, const char* day)
{
  static String list[LIST_SIZE];
  int counter = 0;

  char* value = strtok(buffer, ",");

  while (value)
  {
    if (strcasecmp(value, day) == 0)
    {
      while (value)
      {
        list[counter] = value;
        counter++;
        value = strtok(NULL, ",");
      }
    }
    value = strtok(NULL, ",");
  }

  return list;
}

String* read_csv(const char* path, const char* day)
{
  Serial.print("Initializing SD card... ");

  if (!SD.begin(PIN))
  {
    Serial.println("FAILED!!!");
    for (;;);
  }

  Serial.println("DONE.");

  if (!SD.exists(path))
  {
    Serial.println("File does not exists.");
    for (;;);
  }

  File file = SD.open(path, FILE_READ);
  char buffer[255];
  int index = 0;
  int row = 0;
  int column = 0;

  String* list;
  int counter = 0;

  while (file.available())
  {
    buffer[index] = file.read();

    if (buffer[index] == '\n' || buffer[index] == '\r')
    {
      buffer[index] = '\0';
      index = 0;

      if (strlen(buffer) > 0)
      {
        column = 0;
        row++;

        if (row == 1)
        {
          continue;
        }
        
        list = find_schedule(buffer, day);
      }
    }
    else
    {
      index++;
    }
  }

  file.close();

  return list;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  while (!Serial)
  {
    ;
  }

  String* list = read_csv("schedule.csv", "KAMIS");

  while (*list != "")
  {
    Serial.println(*list);
    list++;
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
