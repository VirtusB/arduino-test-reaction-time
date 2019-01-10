/**@file test.ino */

#include "../../../Arduino/hardware/arduino/avr/cores/arduino/USBAPI.h"
#include <EEPROM.h>
#include <SPI.h>
#include <Ethernet.h>

/*
 * HOW TO PLAY
 * Click the button to start the test, the LED will turn on at a random time between 5 and 30 seconds
 * The LED will turn on 5 times where you will have to react as fast as you can, every time
 * Once 5 the LED has turned on 5 times, you will get your average reaction time from those 5 runs
 */

#define MAX_INT_SIZE 4294967295

/*
 * The integer number of the pushbutton pin
 */
const int buttonPin = 2;

/*
 * The integer number of the LED pin
 */
const int ledPin =  9;

/*
 * Boolean that will be true when the LED is turned on
 */
boolean isLedOn = false;

/*
 * Boolean that will be true when the reaction test has been run once
 */
boolean testHasRun = false;

/*
 * Unsigned long number of the time when LED is turned on, in milliseconds
 */
unsigned long whenTurnedOn = NULL;

/*
 * Unsigned long number of the time when the pushbutton is clicked, in milliseconds
 */
unsigned long whenButtonClicked = NULL;

/*
 * Boolean that will only be true if the user has cheated
 */
boolean hasCheated = false;

/*
 * The count of tests the user has to complete to get his average time
 */
int countTestsToGetAverage = 5;

/*
 * Number that stores how many tests the user has currently completed
 */
int currentTries = 0;

/*
 * An array that contains the user's test times. This will be used to the the average run time.
 */
unsigned long testRunTimes[5];

/*
 * Integer for storing the pushbutton status
 */
int buttonState = 0;

/*
 * MAC address of the Ethernet shield
 */
byte mac[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

/*
 * IP address of the WebServer
 */
IPAddress ip(10, 233, 132, 24);

/*
 * The HTTP port for the WebServer
 */
const int WebServerPort = 5000;

/*
 * The WebServer
 */
EthernetServer server(WebServerPort);

//! Method that returns a random number between 5000 and 30000
//! \return unsigned long
unsigned long getRandomDelay() {
    return random(5, 30) * 1000;
}

/*
 * Method found on GitHub that formats a string. Works like sprintf()
 */
int aprintf(char *str, ...) {
    int i, j, count = 0;

    va_list argv;
    va_start(argv, str);
    for(i = 0, j = 0; str[i] != '\0'; i++) {
        if (str[i] == '%') {
            count++;

            Serial.write(reinterpret_cast<const uint8_t*>(str+j), i-j);

            switch (str[++i]) {
                case 'd': Serial.print(va_arg(argv, int));
                    break;
                case 'l': Serial.print(va_arg(argv, long));
                    break;
                case 'f': Serial.print(va_arg(argv, double));
                    break;
                case 'c': Serial.print((char) va_arg(argv, int));
                    break;
                case 's': Serial.print(va_arg(argv, char *));
                    break;
                case '%': Serial.print("%");
                    break;
                default:;
            };

            j = i+1;
        }
    };
    va_end(argv);

    if(i > j) {
        Serial.write(reinterpret_cast<const uint8_t*>(str+j), i-j);
    }

    return count;
}

/*
 * This is the first function that runs.
 * It's responsible for setting up the WebServer, setting pinModes and starting the serial connection.
 */
void setup() {
    Serial.begin(9600);

    Ethernet.begin(mac, ip);
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable is not connected.");
    }
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
    Serial.println("\n");

    pinMode(ledPin, OUTPUT);
    pinMode(buttonPin, INPUT);

    if (hasSavedScore()) {
        aprintf("Dit bedste gennemsnit: %l ms\n", getSavedScore());
    }

}


/*
 * Method that starts the reaction time test
 */
void startTest() {
    delay(getRandomDelay());

    digitalWrite(ledPin, HIGH);

    isLedOn = true;
    whenTurnedOn = millis();
}

/*
 * Method that resets the test and makes it ready to be started again
 */
void startOver() {
    isLedOn = false;
    whenTurnedOn = NULL;
    testHasRun = false;
    whenButtonClicked = NULL;
}

//! Returns true if the user has a high score, else false
//! \return boolean
boolean hasSavedScore() {
    unsigned long avg = readLong(5300);
    return avg != MAX_INT_SIZE;
}

//! Returns the user's highscore
//! \return unsigned long
unsigned long getSavedScore() {
    unsigned long avg = readLong(5300);
    return avg;
}

/*
 * This method runs continuously, as fast as the CPU allows.
 */
void loop() {
    EthernetClient client = server.available();
    if (client) {
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();

                if (c == '\n' && currentLineIsBlank && hasSavedScore()) {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println();
                    client.println("<!DOCTYPE HTML>");
                    client.println("<html>");
                    client.println("<style>body {background: #333; color: #fff; font-size: 2rem;}</style>");

                    unsigned long bestAvg = getSavedScore();
                    client.print("Highscore: ");
                    client.print(bestAvg);
                    client.print(" ms");

                    client.println("</html>");
                    break;
                }
                if (c == '\n') {
                    currentLineIsBlank = true;
                } else if (c != '\r') {
                    currentLineIsBlank = false;
                }
            }
        }
        delay(1);
        client.stop();
    }

    if (digitalRead(buttonPin) == HIGH && testHasRun == false && isLedOn == false && whenTurnedOn == NULL && whenButtonClicked == NULL) {
        startTest();
        testHasRun = true;
    }

    if (isLedOn == true && whenButtonClicked == NULL) {
        buttonState = digitalRead(buttonPin);

        if (buttonState == HIGH) {
            whenButtonClicked = millis();
            unsigned long timeItTook = whenButtonClicked - whenTurnedOn;

            if (timeItTook <= 10 || timeItTook == 0) {
                Serial.println("Du har snydt");
                hasCheated = true;
            } else {
                aprintf("Det tog dig %d ms at reagere\n", timeItTook);
                testRunTimes[currentTries] = timeItTook;
                currentTries++;
            }


            if (currentTries == countTestsToGetAverage) {
                unsigned long avg = getAverageTimeForRuns();
                aprintf("Dit gennemsnit: %l ms\n", avg);
                currentTries = 0;
                if (hasSavedScore()) {
                    unsigned long oldAvg = getSavedScore();
                    if (avg < oldAvg) {
                        unsigned long difference = oldAvg - avg;
                        aprintf("Du har slået din highscore med %l ms\n", difference);
                        writeLong(5300, avg);
                    }
                } else {
                    aprintf("Din score blev gemt: %l ms\n", avg);
                    writeLong(5300, avg);
                }
            } else if (hasCheated != true) {
                aprintf("Du mangler %d forsøg for at få dit gennemsnit\n", countTestsToGetAverage - currentTries);
            }


            digitalWrite(ledPin, LOW);

            if (currentTries < 5 && currentTries != 0 && hasCheated != true) {
                startOver();
                startTest();
            } else {
                Serial.println("Vent 2 sekunder for at starte igen\n");
                delay(2000);

                startOver();
            }


        }
    }

}



//! Get the average run time of 5 user tests in milliseconds
//! \return unsigned long
unsigned long getAverageTimeForRuns() {
    unsigned long avg = average(testRunTimes);
    return avg;
}

//! Get the average value of an unsigned long array
//! \param array - The array you want the average of
//! \return unsigned long
unsigned long average (unsigned long * array)
{
    int len = sizeof(array);

    long sum = 0L;

    for (int i = 0 ; i < len ; i++)
        sum += array [i] ;
    return  floor(((unsigned long) sum) / len);
}


/**
 * Writes a long to the EEPROM
 * @param address - The address where the value should be saved
 * @param valueToSave - The value to save
 */
void writeLong(int address, unsigned long valueToSave) { // save 4 bytes.
    byte value = 0;

    value = (valueToSave & 0xFF000000) >> 24; // MSB first
    EEPROM.write(address, value);

    value = (valueToSave & 0x00FF0000) >> 16;
    EEPROM.write(address + 1, value);

    value = (valueToSave & 0x0000FF00) >> 8;
    EEPROM.write(address + 2, value);

    value = (valueToSave & 0x000000FF) >> 0; // LSB last
    EEPROM.write(address + 3, value);
}

/**
 * Read a long value from EEPROM
 * @param address - The address of the value that should be read
 * @return - unsigned long that was read from the address
 */
unsigned long readLong(int address) {
    unsigned long value = 0;

    value = (unsigned long)EEPROM.read(address)<<24;

    value = value | (unsigned long)EEPROM.read(address + 1)<<16;

    value = value | (unsigned long)EEPROM.read(address + 2)<<8;

    value = value | (unsigned long)EEPROM.read(address + 3)<<0;


    return value;
}