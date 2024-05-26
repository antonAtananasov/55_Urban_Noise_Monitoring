// #include <Arduino.h>
// #include <SoftwareSerial.h>

// #define sw_serial_rx_pin 6 //  Connect this pin to RX on the esp8266
// #define sw_serial_tx_pin 7 //  Connect this pin to TX on the esp8266

// SoftwareSerial swSerial(sw_serial_tx_pin, sw_serial_rx_pin);
// const char ERR[] = "ERROR";
// const char BUSYP[] = "busy p...";
// String swSerBuffer = "";
// String hwSerBuffer = "";

bool wlCommand(const __FlashStringHelper *cmd, String resp)
{
    bool successful;
    if (strcmp_P("", (const char *)cmd))
        swSerial.println(cmd);
    String respBuffer = "";
    while (!swSerial.available()) // wait for any response
        ;
    while (true)
    {
        if (swSerial.available())
        {

            char b = swSerial.read();
            if (b != '\n')
            {
                respBuffer += b;
                if (respBuffer.endsWith(resp))
                {
                    successful = true;
                    break;
                }
                else if (respBuffer.endsWith(ERR) || respBuffer.endsWith(FAIL))
                {
                    successful = false;
                    break;
                }
                else if (respBuffer.endsWith(BUSYP))
                {
                    if (strcmp_P("", (const char *)cmd))
                        successful = wlCommand(cmd, resp);
                    else
                        successful = false;
                    break;
                }
            }
            else
            {
                Serial.print(F("E "));
                Serial.println(respBuffer);
                respBuffer = "";
            }
        }
    }
    Serial.println();
    return successful;
}
/*
bool wlCommandEmpty(String resp = "OK")
{
    bool successful;
    while (!swSerial.available()) // wait for any response
        ;
    String respBuffer = "";
    while (true)
    {
        if (swSerial.available())
        {

            char b = swSerial.read();
            if (b != '\n')
            {
                respBuffer += b;
                if (respBuffer.endsWith(resp))
                {
                    successful = true;
                    break;
                }
                else if (respBuffer.endsWith(ERR) || respBuffer.endsWith(FAIL))
                {
                    successful = false;
                    break;
                }
                else if (respBuffer.endsWith(BUSYP))
                {
                    successful = false;
                    break;
                }
            }
            else
            {
                Serial.print(F("E "));
                Serial.println(respBuffer);
                respBuffer = "";
            }
        }
    }
    Serial.println();
    return successful;
}
*/
bool sendToServer(String str, bool newLine)
{
    do
    {
        swSerial.print(F("AT+CIPSEND="));
        swSerial.println(str.length() + (newLine ? 1 : 0));
    } while (!wlCommand(F("")));
    // } while (!wlCommandEmpty());

    swSerial.println(newLine ? str + '\n' : str);
    // return wlCommandEmpty();
    return wlCommand(F(""));
}
bool initWlSer()
{
    Serial.begin(115200);
    swSerial.begin(9600);
    wlCommand(F("AT+CIPCLOSE"));
    while (!wlCommand(F("AT+RST"), "ready"))
    {
    }
    bool b = true;
    b &= wlCommand(F(WIFI_CMD));
    b &= wlCommand(F(SERVER_CMD));

    Serial.println();
    // if (b)
    //     sendToServer("[READY]");
    // else
    //     Serial.println(F("[ERROR]"));
    return b;
}

void readSwSerial()
{
    while (swSerial.available())
    {
        char b = swSerial.read();
        if (b == '\n')
        {
            // process line
            if (swSerBuffer.startsWith("+IPD"))
            {
                swSerBuffer.remove(0, swSerBuffer.indexOf(':') + 1);
                Serial.print(F("T "));
                Serial.println(swSerBuffer);
                processCommandFromStr(swSerBuffer);
            }
            else
            {
                if (swSerBuffer.length() > 1)
                {
                    Serial.print(F("E "));
                    Serial.println(swSerBuffer);
                }
            }
            swSerBuffer = "";
        }
        else
        {
            swSerBuffer += b;
            if (swSerBuffer.length() > 20)
            {
                Serial.println(BIGMSG);
                swSerBuffer.remove(20, 1);
            }
        }
    }
}
void readHwSerial()
{
    while (Serial.available())
    {
        if (hwSerBuffer.length() == 0)
            Serial.print(F("A "));
        char b = Serial.read();
        Serial.print(b);
        if (b == '\n')
        {
            // process line
            processCommandFromStr(hwSerBuffer);
            hwSerBuffer = "";
        }
        else
        {
            hwSerBuffer += b;
            if (hwSerBuffer.length() > 20)
            {
                Serial.println(BIGMSG);
                hwSerBuffer.remove(20, 1);
            }
        }
    }
}
void processCommandFromStr(String str)
{
    if (str.startsWith("<")) // send to esp
    {
        str.remove(0, 1);
        swSerial.println(str);
    }
    else if (str.startsWith(">")) // send over tcp
    {
        str.remove(0, 1);
        sendToServer(str);
    }
    /*
    else if (str.startsWith("?"))
    {
        Serial.println(';');
        sendToServer();
        // // str.remove(0, 1);
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[0]));
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[1]));
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[2]));
        // // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[3]));
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[4]));
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[5]));
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[6]));
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[7]));
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[8]));
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[9]));
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[10]));
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[11]));
        // Serial.println((__FlashStringHelper *)pgm_read_word(&helpmsg[12]));
        // Serial.println();
        // if (str.length() == 8)
        // {
        // }
    }
    */
    else if (str.startsWith("marco"))
    {
        sendToServer("polo");
    }
    else if (str.startsWith("tt"))
    {
        // str.remove(0, 5);
        // printTime();
        Serial.println();
        sendToServer(String(RTC.now().unixtime()));
    }
    /*
    else if (str.startsWith("sa"))
    {
        // str.remove(0, 7);
        readSamples();
        // Serial.println((__FlashStringHelper *)pgm_read_word(&serialmsg[4]));
        for (int i = 0; i < (int)samples; i++)
        {
            Serial.println(vReal[i]);
            sendToServer(String(vReal[i]));
        }
    }
    else if (str.startsWith("ss"))
    {
        // str.remove(0, 6);
        for (int i = 0; i < (int)samples; i++)
            Serial.println(vReal[i]);
        Serial.println();
    }
    else if (str.startsWith("fft"))
    {
        // str.remove(0, 4);
        computeFFT();
        // Serial.println((__FlashStringHelper *)pgm_read_word(&serialmsg[4]));
        sendToServer();
        Serial.println();
    }
    else if (str.startsWith("pfft"))
    {
        // str.remove(0, 5);
        printFFTResult();
        Serial.println();
    }
    */
    else if (str.startsWith("ff"))
    {
        // str.remove(0, 8);
        readSamples();
        for (uint8_t i = 0; i < samples; i++)
        {
            Serial.println(vReal[i]);
            sendToServer(String(vReal[i]));
        }
        computeFFT();
        writeToSD();
        Serial.println();
        // printFFTResult();
        for (uint8_t i = 0; i < samples / 2; i++)
        {
            sendToServer(String(vReal[i]));
        }
        Serial.println();
    }
    /*
    else if (str.startsWith("wr"))
    {
        // str.remove(0, 6);
        writeToSD();
        // Serial.println((__FlashStringHelper *)pgm_read_word(&serialmsg[4]));
        sendToServer();
        Serial.println();
    }
    */
    else if (str.startsWith("bt"))
    {
        // str.remove(0, 6);
        Serial.println(vbatt());
        sendToServer(String(vbatt()));
        Serial.println();
    }
    else if (str.startsWith("rd"))
    {
        str.remove(0, 2);
        // str.remove(0, 6);
        char buff[5];
        str.toCharArray(buff, 5);
        readFromSD(max(atoi(buff), 1), false);
        // sendToServer();
        Serial.println();
    }
    else if (str.startsWith("sd"))
    {
        str.remove(0, 2);
        // str.remove(0, 6);
        char buff[5];
        str.toCharArray(buff, 5);
        int lines = atoi(buff);
        if (lines < 0)
        {
            fileIndex = SdFile(MEASUREMENTS_FILE, FILE_READ).fileSize();
        }
        else
        {
            fileIndex = 0;
            readFromSD(max(lines, 1), true);
        }
        sendToServer();
        Serial.println();
    }
    else if (str.startsWith("lat"))
    {
        geoloc(str, latitude);
    }
    else if (str.startsWith("lon"))
    {
        geoloc(str, longitude);
    }

    /*
    if (str.startsWith("simulate"))
    {
        // str.remove(0, 9);
        // Serial.println((__FlashStringHelper *)pgm_read_word(&serialmsg[1]));
        for (int j = 0; j < (int)(sizeof(simulatedSignalFrequencies) / sizeof(simulatedSignalFrequencies[0])); j++)
        {
        Serial.print(simulatedSignalFrequencies[j]);
        // Serial.print((__FlashStringHelper *)pgm_read_word(&serialmsg[8]));
        Serial.println(simulatedSignalAmplitudes[j]);
        }
        simulateSamples();
        // Serial.println((__FlashStringHelper *)pgm_read_word(&serialmsg[4]));
        Serial.println();
    }
    */
    /*
    else if (str.startsWith("tel"))
    {
        // str.remove(0, 6);
        writeTelemetryToSD();
        // Serial.println((__FlashStringHelper *)pgm_read_word(&serialmsg[4]));
        Serial.println();
    }
    */
}
void geoloc(String str, float &coord)
{
    str.remove(0, 3);
    // str.remove(0, 6);
    char buff[11];
    str.toCharArray(buff, 11);
    coord = atof(buff);
    sendToServer();
    Serial.println();
}