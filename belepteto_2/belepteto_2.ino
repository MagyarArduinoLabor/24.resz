/* Beléptető rendszer készítése 2.

   Készítette: Kotán Tamás Balázs - Magyar Arduino Labor©

   Dátum: Győr, 2016.01.25.
*/


#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Keypad.h>

Servo daServo;
#define szervomotor 5

#define RST_PIN         49
#define SS_1_PIN        53
#define NR_OF_READERS  1

byte ssPins[] = {SS_1_PIN};

MFRC522 mfrc522[NR_OF_READERS];

int a = 0;

int pirosLED = 38;
int zoldLED = 40;
int kekLED = 39;

char password1 = '9';        // A jelszó első számjegye
char password2 = '8';        // A jelszó második számjegye
char password3 = '7';        // A jelszó harmadik számjegye
char password4 = '6';        // A jelszó negyedik számjegye

int jo = 0;                  // Egy jó számjegy lenyomásakor nő eggyel az értéke
int bekeres = 0;             // A bekeres változó deklarálása és értékadása (a beütött számok darabszámának értéke)
char mentett;                // A mentett számjegy, ami aktuálisan beírunk

const byte ROWS = 4;         // A billentyűzet sorainak száma
const byte COLS = 4;         // A billentyűzet oszlopainak száma
char keys[ROWS][COLS] = {    // A billentyűzet elemei tömbben megadva
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {A7, A6, A5, A4};    // A billentyűzet sorainak bekötése R1 = A7, R2 = A6, R3 = A5, R4 = A4
byte colPins[COLS] = {A3, A2, A1, A0};    // A billentyűzet oszlopainak bekötése C1 = A3, C2 = A2, C3 = A1. C4 = A0
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);    // A keypad hivatkozásának létrehozása


void setup() {

  Serial.begin(115200);

  pinMode(pirosLED, OUTPUT);
  pinMode(zoldLED, OUTPUT);
  pinMode(kekLED, OUTPUT);

  digitalWrite(pirosLED, LOW);
  digitalWrite(zoldLED, LOW);
  digitalWrite(kekLED, HIGH);

  daServo.attach(szervomotor);
  servo(50);

  SPI.begin();

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)
  {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN);
  }
}

void loop()
{
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)

  {

    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial())
    {
      Serial.print("Reader ");
      Serial.print(reader);
      Serial.println(":");

      Serial.print("Card UID:");
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();
      Serial.print("PICC type: ");
      MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));
      Serial.println();

      if (mfrc522[reader].uid.uidByte[0] == 222 &&
          mfrc522[reader].uid.uidByte[1] == 225 &&
          mfrc522[reader].uid.uidByte[2] == 15 &&
          mfrc522[reader].uid.uidByte[3] == 135 &&
          reader == 0 )
      {
        Serial.println("0-s kartya oke");
        a++;
      }
      if (mfrc522[reader].uid.uidByte[0] != 222 ||
          mfrc522[reader].uid.uidByte[1] != 225 ||
          mfrc522[reader].uid.uidByte[2] != 15 ||
          mfrc522[reader].uid.uidByte[3] != 135 ||
          reader != 0)
      {
        Serial.println("nem 0-s kartya vagy nem 0-s olvaso");

        kitiltas();
      }


      Serial.println();

      mfrc522[reader].PICC_HaltA();
      mfrc522[reader].PCD_StopCrypto1();
    }
  }

  if (a > 0)
  {
    digitalWrite(zoldLED, HIGH);
    delay(500);
    digitalWrite(zoldLED, LOW);
    while (bekeres != 4)
    {
      kodbeiras();
    }
    if (jo == 4)
    {
      beengedes();
      bekeres = 0;
    }
    else
    {
      kitiltas();
      bekeres = 0;
    }
  }

}


////////////////////////////////BEENGEDÉS//////////////////////////////////
void beengedes()
{
  Serial.println("Bemehet!");
  digitalWrite(pirosLED, LOW);
  digitalWrite(zoldLED, HIGH);
  digitalWrite(kekLED, LOW);
  servo(150);
  Serial.println("NYITVA");
  delay(3000);
  servo(50);
  Serial.println("ZARVA");
  digitalWrite(pirosLED, LOW);
  digitalWrite(zoldLED, LOW);
  digitalWrite(kekLED, HIGH);

  jo = 0;
  a = 0;
}


////////////////////////////////KITILTÁS///////////////////////////////////
void kitiltas()
{
  Serial.println("Nem mehet be!");
  digitalWrite(pirosLED, HIGH);
  digitalWrite(zoldLED, LOW);
  digitalWrite(kekLED, LOW);
  delay(1500);
  digitalWrite(pirosLED, LOW);
  digitalWrite(zoldLED, LOW);
  digitalWrite(kekLED, HIGH);

  jo = 0;
  a = 0;
}

////////////////////////////////KÓDBEÍRÁS//////////////////////////////////
void kodbeiras()
{

  char key = keypad.getKey();         // A key deklarálása és értékadása (az lesz az értéke, amilyen gombot éppen lenyomunk)
  if (key != NO_KEY)                  // Ha megnyomunk egy gombot,
  {
    bekeres++;                        // A bekeres változó értéke eggyel nő
    if (bekeres == 1)                 // Ha a bekeres értéke 1,
    {
      mentett = key;                  // A key értékét beírjuk a mentett nevű változóba
      if (mentett == password1)       // Ha a mentett aktuális értéke, tehát az első lenyomott gomb értéke megegyezik a jelszó első számjegyével,
      {
        Serial.print(" jo");          // A soros monitorra kiírjuk: "jo"
        jo++;                         // A jo változó értékét eggyel növeljük, ami azt jelenti, hogy az első számjegy egyezik
      }
      else                    // Ha nem egyezik a két számjegy,
      {
        Serial.print(" rossz");       // A soros monitorra kiírjuk: "rossz"
      }
    }
    if (bekeres == 2)                 // Ha a bekeres értéke 2,
    {
      mentett = key;                  // A key értékét beírjuk a mentett nevű változóba
      if (mentett == password2)       // Ha a mentett aktuális értéke, tehát a második lenyomott gomb értéke megegyezik a jelszó második számjegyével,
      {
        Serial.print(" jo");          // A soros monitorra kiírjuk: "jo"
        jo++;                         // A jo változó értékét eggyel növeljük, ami azt jelenti, hogy a második számjegy egyezik
      }
      else                           // Ha nem egyezik a két számjegy,
      {
        Serial.print(" rossz");      // A soros monitorra kiírjuk: "rossz"
      }
    }
    if (bekeres == 3)                // Ha a bekeres értéke 3,
    {
      mentett = key;                 // A key értékét beírjuk a mentett nevű változóba
      if (mentett == password3)      // Ha a mentett aktuális értéke, tehát a harmadik lenyomott gomb értéke megegyezik a jelszó harmadik számjegyével,
      {
        Serial.print(" jo");         // A soros monitorra kiírjuk: "jo"
        jo++;                        // A jo változó értékét eggyel növeljük, ami azt jelenti, hogy a harmadik számjegy egyezik
      }
      else                           // Ha nem egyezik a két számjegy,
      {
        Serial.print(" rossz");      // A soros monitorra kiírjuk: "rossz"
      }
    }
    if (bekeres == 4)               // Ha a bekeres értéke 4,
    {
      mentett = key;                // A key értékét beírjuk a mentett nevű változóba
      if (mentett == password4)     // Ha a mentett aktuális értéke, tehát a negyedik lenyomott gomb értéke megegyezik a jelszó negyedik számjegyével,
      {
        Serial.print(" jo");        // A soros monitorra kiírjuk: "jo"
        jo++;                       // A jo változó értékét eggyel növeljük, ami azt jelenti, hogy a negyedik számjegy egyezik
        Serial.println("\n");       // A soros monitorra kiírjuk egy üres sort
      }
      else
      {
        Serial.print(" rossz");    // A soros monitorra kiírjuk: "rossz"
        Serial.println("\n");      // A soros monitorra kiírjuk egy üres sort
      }
    }

  }

}

///////////////////////////////SZERVÓ//////////////////////////////////////
void servo(int datPos)
{
  daServo.write(datPos);
  delay(15);
}


////////////////Az UID kiiratása DECimális számokkal///////////////////////

void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
