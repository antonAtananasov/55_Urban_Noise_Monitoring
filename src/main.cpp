/**
 *       Author: Anton Atanasov - 12621101
 *       Project: 55 Urban Noise Monitoring
 **/

// user configuration and wiring:
#define SERIAL_BAUD 115200                            // baud rate for serial communication
#define SD_SPI_MHZ 1                                  // the SPI clock speed (depends on the speed of the SD card)
#define MEASUREMENTS_FILE "measurements.csv"          // name of the file to store measurements (fft levels, time, location)
#define NUMBER_OF_SAMPLES 16                          // amount of samples to take for FFT (eats up SRAM). Has to be a power of 2.
#define PRESCALE_MULTIPLIER 64                        // lower prescale = higher sample rate. Supported values: 1,16,64,128. Recommended: 64
#define MEASUREMENT_DELAY 30                          // amount of seconds to wait between measurements. 0 = no automatic measurements
#define WIFI_CMD "AT+CWJAP=\"SSID\",\"PWD\""          // ESP-AT command to connect to wifi access point: AT+CWJAP=\"<SSID>\",\"<PWD>\""
#define SERVER_CMD "AT+CIPSTART=\"TCP\",\"IP\",12601" // ESP-AT command to a server running TCP socket "AT+CIPSTART=\"TCP\",\"<IPv4>\",<PORT>"
#define ENABLE_COMMAND_LOG_AT_SETUP false             // log output from ESP-AT commands to serial (AT command echos can reveal the wifi credentials)
/*
PRESCALE_MULTIPLIER 1 = Sample rate 125 kHz
PRESCALE_MULTIPLIER 16 = Sample rate 76800 Hz
PRESCALE_MULTIPLIER 64 = Sample rate 19200 Hz
PRESCALE_MULTIPLIER 128 = Sample rate 9600 Hz
*/

// Wiring
// #define MIC_DIGITAL_OUT_PIN 2  // the digital pin reading microphone loudness level
#define MIC_ANALOG_OUT_PIN A0  // the analog pin reading microphone audio (samples)
#define BATTERY_MONITOR_PIN A5 // the analog pin reading battery voltage for telemetry
#define SD_CS_PIN 10           // the io pin used for Chip Select in the SPI protocol to communicate with the SD Card
#define ESP01_WIFI_RX_PIN 6    // the pin that connects to RX of the ESP01 board
#define ESP01_WIFI_TX_PIN 7    // the pin that connects to TX of the ESP01 board

// libraries:
#include <arduinoFFT.h>
#include <RTClib.h>
#include <SdFat.h>
#include <avr/pgmspace.h>
#include <SoftwareSerial.h>

const uint8_t samples = NUMBER_OF_SAMPLES; // This value MUST ALWAYS be a power of 2
unsigned int samplingFrequency = 9600;
// const float simulatedSignalFrequencies[] = {440, 5600, 11500};
// const float simulatedSignalAmplitudes[] = {28, 100, 150};
float vReal[samples];
float vImag[samples];
String swSerBuffer = "";
String hwSerBuffer = "";
float latitude = 43.216667, longitude = 27.916667;
uint32_t fileIndex = 0;
unsigned long lastMeasurement = 0;
bool setUp = false;

ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, samples, samplingFrequency);
RTC_DS1307 RTC;
SdFat SD;
SdFile sdFile;
SoftwareSerial swSerial(ESP01_WIFI_TX_PIN, ESP01_WIFI_RX_PIN); // wireless serial
// put function declarations here:
// int myFunction(int, int);

// bool MIC_DIGITAL_OUT_LEVEL = 0;
// int MIC_ANALOG_OUT_LEVEL = 0;

#include "main.h" //function prototypes and string constants

void setup()
{
    // put your setup code here, to run once:

    // initialize serial and print header
    initSerial();

    // initialize pins
    initPins(PRESCALE_MULTIPLIER);

    // // initialize variables
    // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[9]));
    // Serial.println();

    // initialize modules
    // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[10]));
    Serial.println();

    while (!initRTC())
    {
        // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[18]));
        delay(5000);
    }
    // initialize SD card
    while (!initSD())
    {
        Serial.println(F("SD err"));
        delay(5000);
    }
    while (!initWlSer())
    {
        // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[18]));
        delay(5000);
    }
    // Serial.println((__FlashStringHelper *)pgm_read_word(&serialmsg[4]));
    seekSDLines(-1); // go to end of file
    sendToServer("[READY]");

    Serial.println();
    digitalWrite(LED_BUILTIN, HIGH);
    setUp = true;
}

void loop()
{
    readHwSerial();
    readSwSerial();

    if (MEASUREMENT_DELAY != 0 && (millis() - lastMeasurement > ((unsigned long)MEASUREMENT_DELAY) * 1000 || lastMeasurement == 0))
    {
        lastMeasurement = millis();
        // Serial.println(RTC.now().timestamp());
        readSamples();
        computeFFT();
        writeToSD();
        // processCommandFromStr("tt");
        processCommandFromStr("rd");
        // writeTelemetryToSD();
        // delay(((unsigned long)5) * 60 * 1000);
    }
}

// void simulateSamples()
// {
//     /*
//     for (uint16_t i = 0; i < samples; i++)
//     {
//       vReal[i] = 0;   // reset data
//       vImag[i] = 0.0; // Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
//     }
//     for (int j = 0; j < (int)(sizeof(simulatedSignalFrequencies) / sizeof(simulatedSignalFrequencies[0])); j++)
//     {
//       double simulatedSignalFrequency = simulatedSignalFrequencies[j];
//       double simulatedSignalAmplitude = simulatedSignalAmplitudes[j];
//       double ratio = twoPi * simulatedSignalFrequency / samplingFrequency; // Fraction of a complete cycle stored at each sample (in radians)

//       for (uint16_t i = 0; i < samples; i++)
//       {
//         vReal[i] += (float)(simulatedSignalAmplitude * sin(i * ratio) / 2.0); // Build data with positive and negative values
//         // vReal[i] = uint8_t((amplitude * (sin(i * ratio) + 1.0)) / 2.0);// Build data displaced on the Y axis to include only positive values
//       }
//     }
//     */
// }

int testPrescaler()
{
    // Serial.print((__FlashStringHelper *)pgm_read_word(&setupmsg[13]));
    int start = millis();
    for (int i = 0; i < 1000; i++)
        analogRead(0);
    int time = millis() - start;
    Serial.print(time);
    // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[14]));
    return time;
}

void computeFFT()
{
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward); /* Weigh data */
    FFT.compute(FFTDirection::Forward);                       /* Compute FFT */
    FFT.complexToMagnitude();                                 /* Compute magnitudes */
}
void readSamples()
{
    for (uint8_t i = 0; i < samples; i++)
    {
        vReal[i] = analogRead(MIC_ANALOG_OUT_PIN); /* Build data with positive and negative values*/
    }
    for (uint8_t i = 0; i < samples; i++)
    {
        vImag[i] = 0; /* Build data with positive and negative values*/
    }
    FFT = ArduinoFFT<float>(vReal, vImag, samples, samplingFrequency);
}
void PrintVector(float *vData)
{
    for (uint8_t i = 0; i < samples / 2; i++)
    {
        /*
        double abscissa;
        // Print abscissa value
        switch (scaleType)
        {
        case SCL_INDEX:
          abscissa = (i * 1.0);
          break;
        case SCL_TIME:
          abscissa = ((i * 1.0) / samplingFrequency);
          break;
        case SCL_FREQUENCY:
          abscissa = ((i * 1.0 * samplingFrequency) / samples);
          break;
        }
        if (scaleType != SCL_PLOT)
        {
          Serial.print(abscissa, 6);
          if (scaleType == SCL_FREQUENCY)
            // Serial.print((__FlashStringHelper *)pgm_read_word(&serialmsg[5]));
          Serial.print(" ");
        }
        */
        Serial.println(vData[i]);
    }
    Serial.println();
}

// float log2f(float number)
// {
//     return float(log10f(number) / log10f(2));
// }

void printTime()
{
    // DateTime dateTimeNow = RTC.now();
    // Serial.print((__FlashStringHelper *)pgm_read_word(&serialmsg[6]));
    Serial.print(RTC.now().timestamp());
    // return dateTimeNow;
}

void printFFTResult()
{
    PrintVector(vReal);
}
bool initSD()
{
    pinMode(SD_CS_PIN, OUTPUT);
    bool successful = SD.begin(SD_CS_PIN, SD_SCK_MHZ(4)); // attempt to start SPI with SD card
    // initializations of file and cardType before goto statements
    // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[15]));
    // uint8_t cardType;
    if (!successful)
    {
        // debug("Card Mount Failed");
        // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[16]));
        return false;
    }
    /*
    cardType = SD.fatType(); // check type of SD card
    // Serial.print((__FlashStringHelper *)pgm_read_word(&serialmsg[7]));
    Serial.println(cardType);

     //create a debug log file on SD
    if (!SD.exists(TELEMETRY_FILE))
    {
      sdFile = SdFile(TELEMETRY_FILE, FILE_WRITE);
      // strncpy_P(buffer, (const char*)(__FlashStringHelper *)pgm_read_word(&telemetry_file_header_columns), (size_t)16);
      sdFile.write("VBatt,Time,Loc");
      sdFile.sync();
      sdFile.close();
    }
    */
    if (!SD.exists(MEASUREMENTS_FILE))
    {
        sdFile = SdFile(MEASUREMENTS_FILE, FILE_WRITE);
        // strncpy_P(buffer, (const char*)(__FlashStringHelper *)pgm_read_word(&measurements_file_header_columns), (size_t)30);
        // sdFile.write("Amplitudes,Sample f,Time,Loc,VBatt");
        sdFile.sync();
        sdFile.close();
    }
    /*
    if (!SD.exists(LOG_FILE))
    {
      sdFile = SdFile(LOG_FILE, FILE_WRITE);
      // test
      char buffer[30];
      ltoa(RTC.now().unixtime(), buffer, 10);
      sdFile.write(buffer);
      sdFile.write('\n');
      sdFile.sync();
      sdFile.close();
    }
    */
    // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[17]));
    // Serial.println();
    return successful;
}
bool initRTC()
{
    bool successful = true;
    if (!RTC.begin())
    {
        // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[11]));
        successful = false;
    }
    else
    {
        // // DateTime compileTime = DateTime((__FlashStringHelper *)pgm_read_word(&setupmsg[19]), (__FlashStringHelper *)pgm_read_word(&setupmsg[20]));
        DateTime compileTime = DateTime(__DATE__, __TIME__);
        if (compileTime > RTC.now())
            RTC.adjust(compileTime);
        // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[12]));
        // Serial.println();

        // printTime();
    }
    Serial.println(RTC.now().unixtime());
    return successful;
}
bool initSerial()
{
    // bool successful = true;
    Serial.begin(SERIAL_BAUD);
    while (!Serial)
    { // wait for serial to initialize
      // successful = false;
    }
    // successful = true;
    // Serial.println();
    // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[0]));
    // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[1]));
    // Serial.print((__FlashStringHelper *)pgm_read_word(&setupmsg[2]));
    // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[19]));
    // Serial.println();
    // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[3]));
    // Serial.println();
    return true;
}
void initPins(uint8_t prescaleMultiplier)
{
    // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[4]));
#if defined(ADCSRA)
    switch (prescaleMultiplier)
    {
    // case 128:
    //     // set prescale to 128 (faster than default=128, at 16MHZ/128 => max 9600 sample rate)
    //     sbi(ADCSRA, ADPS2);
    //     sbi(ADCSRA, ADPS1);
    //     sbi(ADCSRA, ADPS0);
    //     break;
    case 64:
        // set prescale to 64
        sbi(ADCSRA, ADPS2);
        sbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS0);
        break;
        // case 16:
        //     // prescale to 16
        //     sbi(ADCSRA, ADPS2);
        //     cbi(ADCSRA, ADPS1);
        //     cbi(ADCSRA, ADPS0);
        //     break;
        // case 1:
        //     // no prescale
        //     cbi(ADCSRA, ADPS2);
        //     cbi(ADCSRA, ADPS1);
        //     cbi(ADCSRA, ADPS0);
        //     break;

    default:
        break;
    }

#endif
    // int adcTestTime = testPrescaler();
    uint8_t prescaler = 64; // roundf(pow(2, round(log2f(adcTestTime))));
    switch (prescaler)
    {
    // case 128:
    //     samplingFrequency = 9600;
    //     break;
    case 64:
        samplingFrequency = 19200;
        break;
        // case 16:
        //     samplingFrequency = 76800; // out of 16bit int
        //     break;
        // case 1:
        //     samplingFrequency = 125000; // out of 16bit int
        //     break;

    default:
        break;
    }
    // Serial.print((__FlashStringHelper *)pgm_read_word(&setupmsg[5]));
    Serial.println(prescaler);
    // Serial.print((__FlashStringHelper *)pgm_read_word(&setupmsg[6]));
    Serial.println(samplingFrequency);
    // Serial.println((__FlashStringHelper *)pgm_read_word(&setupmsg[7]));
    // Serial.print((__FlashStringHelper *)pgm_read_word(&setupmsg[8]));
    Serial.println(samples);
    Serial.println();

    // pinMode(MIC_DIGITAL_OUT_PIN, INPUT);
    // analogReference(EXTERNAL);
    pinMode(LED_BUILTIN, INPUT);
    pinMode(MIC_ANALOG_OUT_PIN, INPUT);
    pinMode(BATTERY_MONITOR_PIN, INPUT);
    pinMode(SD_CS_PIN, OUTPUT);
    pinMode(ESP01_WIFI_RX_PIN, OUTPUT);
    pinMode(ESP01_WIFI_TX_PIN, INPUT);
}
void writeToSD()
{
    sdFile = SdFile(MEASUREMENTS_FILE, FILE_WRITE);
    // fft data
    for (uint8_t i = 0; i < samples / 2; i++)
    {
        char buffer[10];
        int intReal = (int)vReal[i];
        sprintf(buffer, "%d.%d", intReal, (int)((vReal[i] - intReal) * 100));
        // Serial.println(buffer);
        sdFile.write(buffer);
        sdFile.write(' ');
    }
    sdFile.write(',');
    // sampling frequency
    { // scope to dispose of buffer
        char buffer[6];
        ltoa(samplingFrequency, buffer, 10);
        sdFile.write(buffer);
    }
    // timestamp
    sdFile.write(',');
    { // scope to dispose of buffer
        char buffer[10];
        ltoa(RTC.now().unixtime(), buffer, 10);
        sdFile.write(buffer);
    }
    sdFile.write(',');
    // location
    {
        char buffer[9];
        // int intLat = (int)latitude;
        // sprintf(buffer, "%d.%d", intLat, (int)((latitude - intLat) * 10000));
        dtostrf(latitude, 8, 4, buffer);
        // Serial.println(buffer);
        sdFile.write(buffer);
    }
    sdFile.write(' ');
    {
        char buffer[9];
        // int intLon = (int)longitude;
        // sprintf(buffer, "%d.%d", intLon, (int)((longitude - intLon) * 10000));
        dtostrf(longitude, 8, 4, buffer);

        // Serial.println(buffer);
        sdFile.write(buffer);
    }
    sdFile.write(',');
    // battery voltage
    {
        char buffer[5];
        // float voltage = vbatt();
        // int intvoltage = (int)voltage;
        // sprintf(buffer, "%d.%2d", intvoltage, (int)((voltage - intvoltage) * 100));
        dtostrf(vbatt(), 4, 2, buffer);
        // Serial.println(buffer);
        sdFile.write(buffer);
    }
    sdFile.write("  \n");

    sdFile.sync();
    sdFile.close();
}
/*
void writeTelemetryToSD()
{
  sdFile = SdFile(TELEMETRY_FILE, FILE_WRITE);
  sdFile.write('\n');
  // battery voltage
  {
    char buffer[11];
    float voltage = vbatt();
    int intvoltage = (int)voltage;
    sprintf(buffer, "%d.%d", intvoltage, (int)((voltage - intvoltage) * 100));
    // Serial.println(buffer);
    sdFile.write(buffer);
  }
  sdFile.write(',');

  // timestamp
  { // scope to dispose of buffer
    char buffer[10];
    ltoa(RTC.now().unixtime(), buffer, 10);
    sdFile.write(buffer);
  }
  sdFile.write(',');
  // location
  {
    char buffer[11];
    int intLat = (int)latitude;
    sprintf(buffer, "%d.%d", intLat, (int)((latitude - intLat) * 10000));
    // Serial.println(buffer);
    sdFile.write(buffer);
  }
  sdFile.write(' ');
  {
    char buffer[11];
    int intLon = (int)longitude;
    sprintf(buffer, "%d.%d", intLon, (int)((longitude - intLon) * 10000));
    // Serial.println(buffer);
    sdFile.write(buffer);
  }

  sdFile.sync();
  sdFile.close();
}
*/
float vbatt()
{
    return analogRead(BATTERY_MONITOR_PIN) * (5.0 / 1023.0) - .37;
}
#include <wifi_and_serial.h>

void readFromSD(int lines, bool seekOnly)
{
    sdFile = SdFile(MEASUREMENTS_FILE, FILE_READ);
    if (!seekOnly)
        sdFile.seekSet(fileIndex); // go to last read end of line
    else
        sdFile.seekSet(0); // go to last read end of line
    uint32_t len = sdFile.fileSize();
    String readBuffer = "";
    while (sdFile.curPosition() < len && fileIndex < len && lines > 0)
    {
        if (sdFile.available() < 0)
            return;
        char b = sdFile.read();
        if (b != '\n')
        {
            if (!seekOnly)
            {
                if (readBuffer.length() < 10)
                {
                    readBuffer += b;
                }
                else
                {
                    // Serial.print(readBuffer);
                    sendToServer(readBuffer, false);
                    readBuffer = "";
                    readBuffer += b;
                }
            }
        }
        else
        {
            // Serial.println(readBuffer);
            if (!seekOnly)
            {
                sendToServer(readBuffer);
                readBuffer = "";
            }
            lines--;
            fileIndex = sdFile.curPosition();
        }
    }
}
