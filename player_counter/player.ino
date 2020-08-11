#include <SPI.h>
#include <MFRC522.h> // библиотека "RFID".

#include <GyverTM1637.h>
#include <EEPROM.h>
#include <Wire.h>

// #include <IonDB.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);
unsigned long uidDecCurrent, uidDecProcessed, uidDecCleanedBy, uidDec, uidDecTemp; // для храниения номера метки в десятичном формате
unsigned long uidDecCleanedOn = 0;

const byte PLAYER_COUNT_ADDR_1 = 0;

unsigned long TIME_LIMIT = 5 * 60000; // 5 min ban time

unsigned long CLEAR_INTERVAL = 30000; // update ban list each 30 sec
unsigned long intervalClean = 0;

const byte PLAYERS_COUNT = 100;

byte WAVE_PLAYERS_COUNT = 5;

byte currentPlayersCount = WAVE_PLAYERS_COUNT;

unsigned long playersIds[PLAYERS_COUNT];
unsigned long playersBanned[PLAYERS_COUNT];
unsigned long playersBanTime[PLAYERS_COUNT];

const unsigned long INC_ID = 2845298873;
const unsigned long DEC_ID = 3559090202;

bool isReading = false;

const byte LED_CLK = 7;
const byte LED_DIO = 6;

GyverTM1637 tm1637(LED_CLK, LED_DIO);

// Dictionary<unsigned long, unsigned long> *playersDictionary = new SkipList<unsigned long, unsigned long>(-1, key_type_numeric_signed, sizeof(unsigned long), sizeof(unsigned long), 100);

void clearBanList() {
    // remove from ban if time expired
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        if (playersBanned[i] != 0 && millis() - playersBanTime[i] > TIME_LIMIT)
        {
            Serial.println("Unban " + String(playersBanned[i]) + " at " + String(playersBanTime[i]));
            playersBanned[i] = 0;
            playersBanTime[i] = 0;
        }
    }
}

void clear()
{
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        ban(playersIds[i]);
        playersIds[i] = 0;
    }
    
    // remove from ban if time expired
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        if (playersBanned[i] != 0 && millis() - playersBanTime[i] > TIME_LIMIT)
        {
            Serial.println("Unban " + String(playersBanned[i]) + " at " + String(playersBanTime[i]));
            playersBanned[i] = 0;
            playersBanTime[i] = 0;
        }
    }

    // Cursor < unsigned long, unsigned long > *my_cursor = playersDictionary->allRecords();

    // int cnt = 0;
	// while (my_cursor->next()) {
    //     unsigned long key   = my_cursor->getKey();
    //     playersDictionary->deleteRecord(key);
	// }

    uidDecCleanedOn = millis();
}

bool isBanned(unsigned long ID)
{
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        if (playersBanned[i] == ID)
        {
            bool banned = millis() - playersBanTime[i] < TIME_LIMIT;
            Serial.println(String(ID) + " banned: " + String(banned));
            return banned;
        }
    }
    return false;
}

void ban(unsigned long ID)
{
    if (ID == 0) {
        return;
    }
    // for (int i = 0; i < PLAYERS_COUNT; i++)
    // {
    //     if (playersBanned[i] == ID)
    //     {
    //         return;
    //     }
    // }
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        if (playersBanned[i] == 0)
        {
            playersBanned[i] = ID;
            playersBanTime[i] = millis();
            
            Serial.println("Ban " + String(ID) + " at " + String(playersBanTime[i]));
            break;
        }
    }
}

void add(unsigned long ID)
{
    if (ID == 0) {
        return;
    }
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        // don't add to waiting list if player came early
        if (isBanned(ID))
        {
            return;
        }
    }
    // don't add the same player twice
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        if (playersIds[i] == ID)
        {
            return;
        }
    }
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        if (playersIds[i] == 0)
        {
            playersIds[i] = ID;
            Serial.println(String(ID) + " added ");
            break;
        }
    }
    // Serial.println("add:" + String(ID));
    // const byte cnt = count();
    // Serial.println("CURRENT:" + String(cnt));  
}

unsigned long count()
{
    byte count = 0;
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        if (playersIds[i] != 0)
        {
            Serial.println(String(i) + ":" + String(playersIds[i]));
            count++;
        }
    }

    return count;

    // Cursor < unsigned long, unsigned long > *my_cursor = playersDictionary->allRecords();

    // int cnt = 0;
	// while (my_cursor->next()) {
    //     cnt++;
	// }

	// /* Remember to clean up the cursor after you're done */
	// delete my_cursor;

    // return cnt;
}

void savePlayersCount(byte count)
{
    EEPROM.write(PLAYER_COUNT_ADDR_1, count);
}

byte loadPlayersCount()
{
    return EEPROM.read(PLAYER_COUNT_ADDR_1);
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Waiting for card…");
    SPI.begin();        // инициализация SPI / Init SPI bus.
    mfrc522.PCD_Init(); // инициализация MFRC522 / Init MFRC522 card.
    clear();
    uidDecCurrent = 0;

    WAVE_PLAYERS_COUNT = loadPlayersCount();
    tm1637.brightness(7);
    tm1637.displayInt(WAVE_PLAYERS_COUNT);

    intervalClean = 0;
}

void loop()
{    
    if ((millis() - intervalClean) > CLEAR_INTERVAL) {
        Serial.println("clearBanList");
        clearBanList();
        intervalClean = millis();
    }

    // Serial.println("loop");
    // Поиск метки
    uidDec = 0;
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return;
    }
    // Считывание метки
    if (!mfrc522.PICC_ReadCardSerial())
    {
        Serial.println("Reading");
        uidDecProcessed = 0;
        return;
    }
    // Выдача серийного номера метки.
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        uidDecTemp = mfrc522.uid.uidByte[i];
        uidDec = uidDec * 256 + uidDecTemp;
    }
    if (uidDec)
    {
        // Serial.println("Card UID: ");
        // Serial.println(uidDec); // Выводим UID метки в консоль.
        uidDecCurrent = uidDec; // запомнили последний
    }

    // обрабатываем последний

    if (uidDecCurrent == INC_ID)
    {
        WAVE_PLAYERS_COUNT++;
        savePlayersCount(WAVE_PLAYERS_COUNT);
        Serial.println("PLAYERS LIMIT:" + String(WAVE_PLAYERS_COUNT));
        tm1637.displayInt(WAVE_PLAYERS_COUNT);
        clear();
        uidDecProcessed = uidDecCurrent;
    }
    if (uidDecCurrent == DEC_ID)
    {
        WAVE_PLAYERS_COUNT--;
        savePlayersCount(WAVE_PLAYERS_COUNT);
        Serial.println("PLAYERS LIMIT:" + String(WAVE_PLAYERS_COUNT));
        tm1637.displayInt(WAVE_PLAYERS_COUNT);
        clear();
        uidDecProcessed = uidDecCurrent;
    }

    if (uidDecCurrent == DEC_ID || uidDecCurrent == INC_ID)
    {
        delay(2000);
        return;
    }

    // не добавляем игрока, если он был последним
    if (uidDecCleanedBy != uidDecCurrent)
    {
        add(uidDecCurrent);
    }

    if (millis() - uidDecCleanedOn > 30000)
    {
        uidDecCleanedBy = 0;
    }

    const byte cnt = count();
    tm1637.displayInt(WAVE_PLAYERS_COUNT - cnt);
    if (cnt == WAVE_PLAYERS_COUNT)
    {
        uidDecCleanedBy = uidDecCurrent;
        clear();
        // TODO ADD TONE
    }
}