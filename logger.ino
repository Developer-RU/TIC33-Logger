#include <TimerOne.h>
#include <SPI.h>
#include <SD.h>
#include <iarduino_RTC.h>
#include <EEPROM.h>

File file;
iarduino_RTC time(RTC_DS1307);

long full_counter = 0; // Общий счетчик
long day_counter = 0;  // Дневной счетчик

#define   Load    2   //  --------
#define   Din     3   //  Пины
#define   Dclk    4   //  Дисплея
#define   Lclk    5   //  --------

#define   Button  6   // Кнопка безопасное извлечение
#define   Sensor  7   // Кнопка для счетчика (датчик)
#define   Power   8   // Питание SD карты

#define   Led_SD A1   // Индикатор о готовности извлечения SD карты
#define   Led_ER A2   // Индикатор ошибки SD карты


bool flag_button = false;
bool flag_sensor = false;
bool isset_sd = true;

unsigned long timeproverka = 0;

typedef unsigned char u8;
                    
// Вывод на TIC33
const u8 DigitTIC33[]  = 
{
    2+8+16+32+64+128,
    2+8,
    128+2+4+32+16,
    2+8+128+4+16,
    64+4+2+8,
    128+64+4+8+16,
    128+64+4+8+16+32,
    128+2+8,
    2+4+8+16+32+64+128,
    2+4+8+16+64+128,
    4,
    2+4+64+128,
    0
};

void WriteCharTIC33(u8 ch,u8 point)
{
  u8 i;
  ch = DigitTIC33[ch];
  
  if(point) ch |= 1;
  // Вывод символа
  for(i = 8; i > 0; i--)
  {
      if(ch & 128) digitalWrite(Din, HIGH); else digitalWrite(Din, LOW);
      
      digitalWrite(Dclk, HIGH); 
      digitalWrite(Dclk, LOW);
      ch = ch<<1;
  }
}

void ClearTIC33 (void)
{
    u8 i;
    for(i = 0; i < 72; i++)
    {
        digitalWrite(Din, LOW);
        digitalWrite(Dclk, HIGH);
        digitalWrite(Dclk, LOW);
    }
    
    digitalWrite(Load, HIGH); 
    digitalWrite(Load, LOW);
}

void blinkLED(void)
{
    if(digitalRead(Lclk) == LOW) digitalWrite(Lclk, HIGH); else digitalWrite(Lclk, LOW);
}

/*
void readFile(String date)
{   
    String filename1 = date;
    filename1 += ".CSV";
    char filename[filename1.length()+1];
    filename1.toCharArray(filename,sizeof(filename));
    
    if (SD.exists(filename))
    {
        Serial.println("SEND");
        Serial.println("");
        
        File dataFile = SD.open(filename);
        
        while (dataFile.available()) {
            Serial.write(dataFile.read());
        }
        dataFile.close();

        Serial.println();
    }
    else {
        Serial.println("FILE NOT EXISTS");
    }    
}
*/

void writeData()
{
    String filename1 = String(time.gettime("m")) + "_" + String(time.gettime("Y")) + ".CSV";
    char filename[filename1.length()+1];
    filename1.toCharArray(filename,sizeof(filename));
    
    file = SD.open(filename, FILE_WRITE);     
    if (file) {
        file.print(String(time.gettime("d")) + ";" + String(time.gettime("m")) + ";" + String(time.gettime("Y")) + ";");
        file.println(day_counter);
        
        Serial.print(String(time.gettime("d")) + ";" + String(time.gettime("m")) + ";" + String(time.gettime("Y")) + ";");
        Serial.println(day_counter);
        
        file.close();
        delay(1000);
    } else {
        Serial.println("ERROR CREAT FILE"); return;
    }
}

String getTime()
{
    return String(time.gettime("H:i:s,d.m.Y"));
}

void setTime(int hours, int minuts, int seconds, int days, int mesyac, int years)
{
    time.settime(hours, minuts, seconds, days, mesyac, years);  // сек, мин, час, день, месяц, год
}

// чтение
long EEPROM_long_read(int addr)
{    
    byte raw[4];
    for(byte i = 0; i < 4; i++) raw[i] = EEPROM.read(addr+i);
    long &num = (long&)raw;
    return num;
}

// запись
void EEPROM_long_write(int addr, long num) 
{
    byte raw[4];
    (long&)raw = num;
    for(byte i = 0; i < 4; i++) EEPROM.write(addr+i, raw[i]);
}
  
void setup()
{
    Serial.begin(9600);
    time.begin();

    pinMode(Load, OUTPUT); digitalWrite(Load, LOW);
    pinMode(Din, OUTPUT); digitalWrite(Din, LOW);
    pinMode(Dclk, OUTPUT); digitalWrite(Dclk, LOW);
    pinMode(Lclk, OUTPUT); digitalWrite(Lclk, LOW);

    pinMode(Button, INPUT);  digitalWrite(Button, HIGH);
    pinMode(Sensor, INPUT);  digitalWrite(Sensor, HIGH);
    pinMode(Power, INPUT);   digitalWrite(Power, HIGH);
    
    pinMode(Led_SD, OUTPUT);  digitalWrite(Led_SD, LOW);
    pinMode(Led_ER, OUTPUT);  digitalWrite(Led_ER, LOW);

    Timer1.initialize(5000);
    Timer1.attachInterrupt(blinkLED);
    
    delay(100);


    // Вытаскиваем общий счетчик из EEPROM --- Заносим в общий счетчик и выводим на экран
    full_counter = EEPROM_long_read(0);


    String cifra = (String)full_counter;            
    ClearTIC33();
    if(cifra.length() > 1)
        for(int i = 0; i < cifra.length(); i++) symbol(cifra.substring(i, i + 1).toInt());
    else
        symbol(cifra.toInt());
    digitalWrite(Load, HIGH);
    digitalWrite(Load, LOW);

    delay(100);
    
    timeproverka = millis();

    digitalWrite(Led_SD, LOW);           

    // Заново делаем инициализацию SD
    if(!SD.begin(10)) 
    { 
        digitalWrite(Led_ER, HIGH); 
        digitalWrite(Led_SD, HIGH);
        digitalWrite(Power, LOW);

        isset_sd = false;

        Serial.println("Error init SD");
    }

    pinMode(Power, INPUT); digitalWrite(Power, LOW);

}

void symbol(int i)
{
    switch(i)
    {
        case 0: WriteCharTIC33(0, 0); break;
        case 1: WriteCharTIC33(1, 0); break;
        case 2: WriteCharTIC33(2, 0); break;
        case 3: WriteCharTIC33(3, 0); break;
        case 4: WriteCharTIC33(4, 0); break;
        case 5: WriteCharTIC33(5, 0); break;
        case 6: WriteCharTIC33(6, 0); break;
        case 7: WriteCharTIC33(7, 0); break;
        case 8: WriteCharTIC33(8, 0); break;
        case 9: WriteCharTIC33(9, 0); break;
    }
}

void loop(void)
{  
    String val = ""; uint8_t ch = 0;
    
    if (Serial.available()) 
    {
        delay(200);
        while (Serial.available()) 
        {  
            ch = Serial.read();
            val += char(ch);
            delay(10);
        }
        /*
        if (val.indexOf("READFILE") > -1) 
        {
            int len = val.length() - 8;
            readFile(val.substring(len));  
        }
        */
        if (val.indexOf("GETTIME") > -1) Serial.println(getTime());  
           
        if (val.indexOf("SETTIME") > -1) 
        {
            // пример      SETTIME 23:58:50,02.12.2016          // часы, минуты, секунды, день, месяц, год
            
            int hours = val.substring(14, 16).toInt(); 
            int minuts = val.substring(11, 13).toInt(); 
            int seconds = val.substring(8, 10).toInt(); 

            int days = val.substring(17, 19).toInt(); 
            int mesyac = val.substring(20, 22).toInt(); 
            int years = val.substring(25, 27).toInt(); 
            
            setTime(hours, minuts, seconds, days, mesyac, years);
        }
    }
    else 
    {
        if(digitalRead(Sensor) == LOW && flag_sensor == false)
        {
            flag_sensor = true;
        }
        
        if(digitalRead(Sensor) == HIGH && flag_sensor == true)
        {
            flag_sensor = false;    //writeData();  
            
            EEPROM_long_write(0, full_counter);

            day_counter++;
            full_counter++;

            String cifra = (String)full_counter;            
            ClearTIC33();
            if(cifra.length() > 1)
                for(int i = 0; i < cifra.length(); i++) symbol(cifra.substring(i, i + 1).toInt());
            else
                symbol(cifra.toInt());
            digitalWrite(Load, HIGH);
            digitalWrite(Load, LOW);

            delay(100);
        } 

        // Если нажата кнопка безопасное извлечение
        if(digitalRead(Button) == LOW && flag_button == false)
        {
            flag_button = true;
        }
        
        if(digitalRead(Button) == HIGH && flag_button == true)
        {           
            if(isset_sd == true)
            {
                digitalWrite(Led_ER, LOW); 
                digitalWrite(Power, LOW);
                delay(500);
                digitalWrite(Led_SD, HIGH);
            }
            else
            {
                digitalWrite(Power, HIGH);
                
                delay(500);
                digitalWrite(Led_SD, LOW);           

                // Заново делаем инициализацию SD
                if(!SD.begin(10)) 
                { 
                    digitalWrite(Led_ER, HIGH); 
                    digitalWrite(Led_SD, HIGH);
                    digitalWrite(Power, LOW);

                    isset_sd = false;
                    Serial.println("Error init SD");
                } 
                else
                {
                    digitalWrite(Led_ER, LOW); 
                    digitalWrite(Led_SD, LOW);
                    isset_sd = true;
                }
            }

            pinMode(Power, INPUT); digitalWrite(Power, LOW);

            flag_button = false;
        }        
    }
    
    // Если пришло время сверить часы (для записи по суткам)
    if(millis() - timeproverka > 59500)
    {
        String thour = String(time.gettime("H"));
        String tminute = String(time.gettime("i"));

        if(thour == "23" && tminute == "59")
        {
            if(isset_sd = true) 
            {
                pinMode(Power, INPUT); digitalWrite(Power, HIGH);
                writeData(); 
                delay(200);
                pinMode(Power, INPUT); digitalWrite(Power, LOW);
            }
            delay(1000);
        }

        timeproverka = millis();
    }
}
