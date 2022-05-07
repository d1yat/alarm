#include <SPI.h>
#include <SD.h>
#include <RTClib.h>

#define PIN 10
#define COL_SIZE 20
#define ERR_PIN 8

// variable list ini utk menampung data yg ada di sd card
String *list;

// variable index fungsinya utk m'geser jam ke jam berikutnya, 
// sesuai dgn yg ada di variable list
int index = 0;

int pinBuzzer = 9;
int beep = 3;

RTC_DS1307 RTC;
Sd2Card sd;

char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
bool isreading = false;
char* scheduled_day;
//String alarm_set[3];

// Deprecated (karena fungsi strtok() m'ubah nilai aslinya)
void strsplit(char* result[], char str[], const char* delimiter)
{
  char* portion = strtok(str, delimiter);
  int index = 0;
  while (portion)
  {
    result[index] = portion;
    index++;
    portion = strtok(NULL, delimiter);
  }
  // result[index] = NULL;
}

void err_sign()
{
  int i = 0;
  while (i < 3)
  {
    digitalWrite(ERR_PIN, HIGH);
    delay(100);
    digitalWrite(ERR_PIN, LOW);
    delay(100);
    i++;
  }
}

void alert(int count, int frequency = 1000, int wait = 500) {
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
//  index++;
}

String* find_schedule(char* buffer, const char* day)
{
  // initialize variables...
  // Notes:
  // string lemparan maksudnya parameter buffer pada fungsi ini :-)
  
  // angka 3 di variable list, adalah jumlah kolom yg ada di file csv (hari tdk termasuk)
  static String list[COL_SIZE];
  
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

void check_sd() 
{
  if (!sd.init(SPI_HALF_SPEED, PIN))
  {
    err_sign();
//    alert(5, 3000, 300);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  while (!Serial)
  {
    ;
  }

Begin:  
  Serial.print("Initializing SD card... ");

  if (!SD.begin(PIN))
  {
    Serial.println("FAILED!!!");
    goto Begin;
    // for (;;);
  }

  Serial.println("DONE.");
  pinMode(pinBuzzer, OUTPUT);

  RTC.begin();

  if (!RTC.isrunning())
  {
    Serial.println("RTC is not running!");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

//  RTC.adjust(DateTime(__DATE__, __TIME__));
  
  list = NULL;
//  alarm_set[0] = "00:07";
//  alarm_set[1] = "00:08";
//  alarm_set[2] = "00:09";

// kode di bawah ini hanya utk kebutuhan development
// sengaja dikomentari
// jgn dihapus ya...

//  list = read_csv("schedule.csv", "Rabu");

//  while (list[index] != "")
//  {
//    Serial.println(list[index]);
//    index++;
//  }
}


void loop() {
  // put your main code here, to run repeatedly:

  DateTime now = RTC.now();

  char* day = daysOfTheWeek[now.dayOfTheWeek()];
  unsigned char hour = now.hour();
  unsigned char minute = now.minute();
  String jam = "0";
  String menit = "0";
  String separator = ":";
  if (hour < 10) jam.concat(hour); else jam = (String)hour;
  jam.concat(separator);
  if (minute < 10) menit.concat(minute); else menit = (String)minute;
  jam.concat(menit);

  // check if scheduled day is equal to current day
  // if not, initialize scheduled day as current day
  if (scheduled_day != day)
  {
    scheduled_day = day;
    isreading = true;
  }

  // read data from sd card only once per day
  if (isreading)
  {
    list = read_csv("schedule.csv", scheduled_day);
    isreading = false;
  }

  // get scheduled time 
  String jam_sd;
  String jml_bel;
  if (list != NULL && list[index] != "")
  {
    char* result[2];
    char str[8];
    (list[index]).toCharArray(str, 8);
    // split string
    // from:
    // (0) -> "08:00*3" 
    // into: 
    // (0) -> "08:00", (1) -> "3"
    strsplit(result, str, "*");
    
    jam_sd = result[0];
    jml_bel = result[1];
  }
  
//  if (alarm_set[index] == jam)
  if (jam_sd == jam)
  {
//    Serial.print("Alarm ke ");
//    Serial.print(index + 1);
//    Serial.print(" berbunyi pada pukul ");
//    Serial.print(jam);
//    Serial.println();
    
    alert(jml_bel.toInt());
    index++; 
  }
  
  check_sd();

//  while (index < 5)
//  {
//    Serial.print(day);
//    Serial.print(", ");
//    Serial.println(jam);
//    index++;
//  }
  
}
