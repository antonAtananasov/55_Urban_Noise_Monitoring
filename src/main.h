// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// setup global variables:
// #define SCL_INDEX 0x00
// #define SCL_TIME 0x01
// #define SCL_FREQUENCY 0x02
// #define SCL_PLOT 0x03

// float log2f(float number);
// int testPrescaler();
// void simulateSamples();
void PrintVector(float *vData);
void computeFFT();
void readSamples();
// void readSerial();
void printFFTResult();
void printTime();
bool initSerial();
void initPins(uint8_t prescaleMultiplier);
bool initRTC();
bool initSD();
void writeToSD();
// void writeTelemetryToSD();
float vbatt();
bool wlCommand(const __FlashStringHelper *cmd, String resp = "OK"); //?,unsigned long timeout=6000);
// bool wlCommandEmpty(String resp = "OK");                            //?,unsigned long timeout=6000);
bool sendToServer(String str = "OK", bool newLine = true);
bool initWlSer();
void processCommandFromStr(String str);
void readSwSerial();
void readHwSerial();
void readFromSD(int lines,bool seekOnly=false);
void geoloc(String str, float &coord);

// boolean wlSerCommand(String cmd, String ack);
// boolean echoFind(String keyword);

const char ERR[] = "ERROR";
const char BUSYP[] = "busy p...";
const char BUSYS[] = "busy s...";
const char FAIL[] = "FAIL";
const char  BIGMSG[] PROGMEM = "A msg>20B";
const char M_FILE[] PROGMEM=MEASUREMENTS_FILE;


const char setupmsg0[] PROGMEM = "Pjoject: Urban Noise Monitoring";
const char setupmsg1[] PROGMEM = "Author: Anton Atanasov";
const char setupmsg2[] PROGMEM = ""; //"Last Modified: ";
const char setupmsg3[] PROGMEM = ""; //"Initializing...";
const char setupmsg4[] PROGMEM = ""; //"Testing prescaler...";
const char setupmsg5[] PROGMEM = "Prescaler set to ";
const char setupmsg6[] PROGMEM = "Sampling rate: ";
const char setupmsg7[] PROGMEM = ""; //" kHz";
const char setupmsg8[] PROGMEM = "Samples: ";
const char setupmsg9[] PROGMEM = "";         //"Init variables...";
const char setupmsg10[] PROGMEM = "Loading"; //"Init modules...";
// const char setupmsg11[] PROGMEM = "ERROR";//"RTC error.";
const char setupmsg12[] PROGMEM = "RTC OK"; //"RTC loaded.";
const char setupmsg13[] PROGMEM = "ADCTEST: ";
const char setupmsg14[] PROGMEM = " ms"; //" msec (1000 calls)";
const char setupmsg15[] PROGMEM = "";    //"Init SD Card...";
// const char setupmsg16[] PROGMEM = "ERROR";//"SD error!";
const char setupmsg17[] PROGMEM = "SD OK"; //"SD loaded.";
const char setupmsg18[] PROGMEM = "Retry...";
const char setupmsg19[] PROGMEM = __DATE__;
const char setupmsg20[] PROGMEM = __TIME__;
const char setupmsg21[] PROGMEM = "";

// const char *const setupmsg[] PROGMEM = {
//     setupmsg0,
//     setupmsg1,
//     setupmsg2,
//     setupmsg3,
//     setupmsg4,
//     setupmsg5,
//     setupmsg6,
//     setupmsg7,
//     setupmsg8,
//     setupmsg9,
//     setupmsg10,
//     ERR,
//     setupmsg12,
//     setupmsg13,
//     setupmsg14,
//     setupmsg15,
//     ERR,
//     setupmsg17,
//     setupmsg18,
//     setupmsg19,
//     setupmsg20,
//     setupmsg21

// };
const char helpmsg0[] PROGMEM = "? - Help";
const char helpmsg1[] PROGMEM = "tt - Get current time";
const char helpmsg2[] PROGMEM = "ss - Record samples";
const char helpmsg3[] PROGMEM = ""; //"simulate - Simulate samples";
const char helpmsg4[] PROGMEM = ""; //"ssamp - Show samples";
const char helpmsg5[] PROGMEM = "ff - Compute FFT from samples";
const char helpmsg6[] PROGMEM = ""; //"pfft - Show FFT result";
const char helpmsg7[] PROGMEM = "ff - sample+ssamp+fft+pfft";
const char helpmsg8[] PROGMEM = "wr - Write fft results to SD";
// const char helpmsg9[] PROGMEM = "tel - Write telemetry data to SD";
const char helpmsg9[] PROGMEM = "bt - Get battery voltage";
const char helpmsg10[] PROGMEM = "lat - Set latitude";
const char helpmsg11[] PROGMEM = "lon - Set longitude";
const char helpmsg12[] PROGMEM = "rdN - Read N lines from SD";
const char helpmsg13[] PROGMEM = "sdN - Set SD file index to line N";
const char helpmsg14[] PROGMEM = "marco - Test server conn";

// const char *const helpmsg[] PROGMEM = {
//     helpmsg0,
//     helpmsg1,
//     helpmsg2,
//     helpmsg3,
//     helpmsg4,
//     helpmsg5,
//     helpmsg6,
//     helpmsg7,
//     helpmsg8,
//     helpmsg9,
//     helpmsg10,
//     helpmsg11,
//     helpmsg12

// };
const char serialmsg0[] PROGMEM = "A "; //"Recv: ";
const char serialmsg1[] PROGMEM = "";   //"Simulated frequencies:";
const char serialmsg2[] PROGMEM = "ERR";
const char serialmsg3[] PROGMEM = ""; //"Retry...";
const char serialmsg4[] PROGMEM = "OK";
const char serialmsg5[] PROGMEM = ""; //"Hz";
const char serialmsg6[] PROGMEM = ""; //"DateTime: ";
const char serialmsg7[] PROGMEM = ""; //"SD type: ";
const char serialmsg8[] PROGMEM = ""; //" amplitude ";
const char serialmsg9[] PROGMEM = "";

// const char *const serialmsg[] PROGMEM = {
//     serialmsg0,
//     serialmsg1,
//     serialmsg2,
//     serialmsg3,
//     serialmsg4,
//     serialmsg5,
//     serialmsg6,
//     serialmsg7,
//     serialmsg8,
//     serialmsg9

// };

// const char telemetry_file_header_columns[] PROGMEM = "VBatt,Time,Loc";
// const char measurements_file_header_columns[] PROGMEM = "Amplitudes,Sample f,Time,Loc";