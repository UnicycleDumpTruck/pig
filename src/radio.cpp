#include <Arduino.h>
#include <radio.h>

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);

struct EvenDataPacket
{
    uint32_t counter;
    float batteryVoltage;
    uint8_t cubeID;
    uint8_t side;
} eventData;

uint8_t eventBuffer[sizeof(eventData)];
uint8_t from;
uint8_t len = sizeof(eventData);
int16_t packetnum = 0; // packet counter, we increment per xmission
uint32_t localCounter = 0;

void sendGoEvent(uint8_t s)
{
    eventData.side = s;
    eventData.cubeID = MY_ADDRESS;
    eventData.batteryVoltage = 0;
    eventData.counter++;
    Serial.print("About to send transmission number: ");
    Serial.println(eventData.counter);
    digitalWrite(RFM69_CS, LOW);
    delay(10);
    bool t_result = rf69.send((uint8_t *)&eventData, sizeof(eventData));
    rf69.waitPacketSent();
    if (t_result)
    {
        Serial.println("Send true");
    }
    else
    {
        Serial.println("Send false");
    }
    // delay(50);
    digitalWrite(RFM69_CS, HIGH);
}

void radioSetup()
{
    pinMode(RFM69_CS, OUTPUT); // This line caused the audio shield to not be found?
    // pinMode(RFM69_EN, OUTPUT);
    pinMode(RFM69_RST, OUTPUT);

    digitalWrite(RFM69_CS, LOW);
    //digitalWrite(RFM69_EN, HIGH);
    digitalWrite(RFM69_RST, LOW);
    delay(50);
    // manual reset
    digitalWrite(RFM69_RST, HIGH);
    delay(50);
    digitalWrite(RFM69_RST, LOW);
    delay(50);

    if (!rf69_manager.init())
    {
        delay(2000);
        Serial.println("RFM69 radio init failed");
        //while (1);
    }
    Serial.println("RFM69 radio init OK!");
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
    // No encryption
    if (!rf69.setFrequency(RF69_FREQ))
    {
        Serial.println("setFrequency failed");
    }

    // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
    // ishighpowermodule flag set like this:
    rf69.setTxPower(20, true); // range from 14-20 for power, 2nd arg must be true for 69HCW

    // The encryption key has to be the same as the one in the server
    uint8_t key[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                     0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    rf69.setEncryptionKey(key);

    eventData.counter = 0;

    sendGoEvent(0);
    digitalWrite(RFM69_CS, HIGH);
    Serial.println("Radio Setup Complete");
}

bool receiveFromCube()
{
  digitalWrite(RFM69_CS, LOW);
	if (rf69.recv((uint8_t*)&eventData, &len) && len == sizeof(eventData))
	{    
		char buf[16];
		String stringBuf = "-00.000";
		
		Serial.print("Received: ");
		Serial.print("c=");
		Serial.print(itoa(eventData.cubeID, buf, 10));
		Serial.print(" t=");
		Serial.print(itoa(eventData.counter, buf, 10));
		Serial.print(" s=");
		Serial.print(itoa(eventData.side, buf, 10));
		Serial.print(" g=");
		Serial.print(rf69.lastRssi(), DEC);
		Serial.print(" b=");
		Serial.print(eventData.batteryVoltage);
		
		if (eventData.cubeID == 0) // if message was from gateway
		{
			if (eventData.side == 61) { // if message is to drop the pig
				digitalWrite(RELAY_PIN, HIGH); // drop the pig
				delay(500);
				digitalWrite(RELAY_PIN,LOW);
        digitalWrite(RFM69_CS, HIGH);
        return true;
			}
		}
		Serial.println("");
    digitalWrite(RFM69_CS, HIGH);
    return false;
	}
}

// ftoa from http://www.ars-informatica.ca/eclectic/ftoa-convert-a-floating-point-number-to-a-character-array-on-the-arduino/
void ftoa(float f, char *str, uint8_t precision) {
  uint8_t i, j, divisor = 1;
  int8_t log_f;
  int32_t int_digits = (int)f;             //store the integer digits
  float decimals;
  char s1[12];

  memset(str, 0, sizeof(str));  
  memset(s1, 0, 10);

  if (f < 0) {                             //if a negative number 
    str[0] = '-';                          //start the char array with '-'
    f = abs(f);                            //store its positive absolute value
  }
  log_f = ceil(log10(f));                  //get number of digits before the decimal
  if (log_f > 0) {                         //log value > 0 indicates a number > 1
    if (log_f == precision) {              //if number of digits = significant figures
      f += 0.5;                            //add 0.5 to round up decimals >= 0.5
      itoa(f, s1, 10);                     //itoa converts the number to a char array
      strcat(str, s1);                     //add to the number string
    }
    else if ((log_f - precision) > 0) {    //if more integer digits than significant digits
      i = log_f - precision;               //count digits to discard
      divisor = 10;
      for (j = 0; j < i; j++) divisor *= 10;    //divisor isolates our desired integer digits 
      f /= divisor;                             //divide
      f += 0.5;                            //round when converting to int
      int_digits = (int)f;
      int_digits *= divisor;               //and multiply back to the adjusted value
      itoa(int_digits, s1, 10);
      strcat(str, s1);
    }
    else {                                 //if more precision specified than integer digits,
      itoa(int_digits, s1, 10);            //convert
      strcat(str, s1);                     //and append
    }
  }

  else {                                   //decimal fractions between 0 and 1: leading 0
    s1[0] = '0';
    strcat(str, s1);
  }

  if (log_f < precision) {                 //if precision exceeds number of integer digits,
    decimals = f - (int)f;                 //get decimal value as float
    strcat(str, ".");                      //append decimal point to char array

    i = precision - log_f;                 //number of decimals to read
    for (j = 0; j < i; j++) {              //for each,
      decimals *= 10;                      //multiply decimals by 10
      if (j == (i-1)) decimals += 0.5;     //and if it's the last, add 0.5 to round it
      itoa((int)decimals, s1, 10);         //convert as integer to character array
      strcat(str, s1);                     //append to string
      decimals -= (int)decimals;           //and remove, moving to the next
    }
  }
}
