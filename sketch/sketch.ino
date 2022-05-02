#include <SPI.h>
#include <SD.h>
#include <RTClib.h>

#define PIN 4

// variable list ini utk menampung data yg ada di sd card
String *list;

// variable index fungsinya utk m'geser jam ke jam berikutnya, 
// sesuai dgn yg ada di variable list
int index = 0;

int pinBuzzer = 9;
int beep = 3;

RTC_DS1307 RTC;

char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};

void alert(int count, int frequency, int wait) {
  int i = 0;
  // yg ini mah udh ngerti pasti
  // ga usah dijelasin lagi :-)
  while (i < count) {
    tone(pinBuzzer, frequency);
    delay(wait);
    noTone(pinBuzzer);
    delay(wait);
    i++;
  }
  index++;
}

String* find_schedule(char* buffer, const char* day)
{
  // initialize variables...
  // Notes:
  // string lemparan maksudnya parameter buffer pada fungsi ini :-)
  
  // angka 3 di variable list, adalah jumlah kolom yg ada di file csv (hari tdk termasuk)
  static String list[3];
  
  // variable counter dipake utk menyimpan data di kolom selanjutnya
  int counter = 0;

  // ambil index pertama dari string lemparan, index dipisahkan dgn koma (,)
  // misal satu,dua,tiga
  // index ke 0 bernilai satu
  // index ke 1 bernilai dua
  // index ke 2 bernilai tiga
  char* value = strtok(buffer, ",");

  // jika nilai variable 'value' sama dgn nilai variable 'day', 
  // maka jalankan fungsi berikut
  if (strcasecmp(value, day) == 0)
  {
    // lanjut ke index berikutnya dari string lemparan
    value = strtok(NULL, ",");
    
    while (value)
    {
      list[counter] = value;
      counter++;
      
      // lanjut ke index berikutnya dari sisa string yg disimpan di variable 'value'
      value = strtok(NULL, ",");
    }
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
  char buffer[128];
  int index = 0;
  int row = 0;
  int column = 0;

  String* list;
  int counter = 0;

  while (file.available())
  {
    // baca satu karakter dari file csv
    buffer[index] = file.read();

    // bila sudah ada di akhir karakter, maka jalankan fungsi di bawah ini, 
    // bila tidak, masuk ke else di bawah
    if (buffer[index] == '\n' || buffer[index] == '\r')
    {
      // akhiri di bagian akhir karakter dgn '\0' atau NULL termination
      buffer[index] = '\0';
      
      // mulai lagi dari awal karakter baris selanjutnya
      index = 0;

      if (strlen(buffer) > 0)
      {
        // variable column ini udh ga dipake, 
        // dipindah fungsinya ke find_schedule()
        column = 0;
        
        row++;

        // lompati baris pertama (header) file csv
        if (row == 1)
        {
          continue;
        }
        
        list = find_schedule(buffer, day);
      }
    }
    else
    {
      // lanjut ke karakter selanjutnya
      index++;
    }
  }

  file.close();

  return list;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  pinMode(pinBuzzer, OUTPUT);

  while (!Serial)
  {
    ;
  }

  RTC.begin();

  if (!RTC.isrunning())
  {
    Serial.println("RTC is not running!");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  list = NULL;

  // 10:00->13:00->16:00->EMPTY
//  list = read_csv("schedule.csv", "KAMIS");

// kode ini utk kebutuhan development
// sengaja dikomentari
// jgn dihapus ya...
//  while (list[index] != "")
//  {
//    Serial.print(list[index] + "->");
//    index++;
//  }
//  Serial.println("EMPTY");
}

bool isreading = false;
char* scheduled_day;

void loop() {
  // put your main code here, to run repeatedly:
//  Serial.println(list[index]);

  DateTime now = RTC.now();

  char* day = daysOfTheWeek[now.dayOfTheWeek()];
  unsigned char hour = now.hour();
  unsigned char minute = now.minute();
  String separator = ":";
  String jam = (String)hour;
  jam.concat(separator);
  jam.concat((String)minute);

  if (scheduled_day != day)
  {
    scheduled_day = day;
    isreading = true;
  }

  if (isreading)
  {
    list = read_csv("schedule.csv", "Minggu");
    isreading = false;
  }

  String jam_sd;
  if (list != NULL && list[index] != "") jam_sd = list[index];
  
  Serial.println(jam_sd);
  if (jam_sd == jam)
  {
    alert(3, 1000, 500);
//    index++; // udah ada di atas update indexnya setelah m'ambil fungsi alert
  }

//  while (index < 5)
//  {
//    Serial.println(jam);
//    index++;
//  }
  
//  if (list[index] == "10:00")
//  {
//    alert(3, 1000, 500);
//  }
}
