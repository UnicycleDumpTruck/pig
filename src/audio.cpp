#include <Arduino.h>
#include <audio.h>

Adafruit_VS1053_FilePlayer musicPlayer =
    Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

/// File listing helper
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}


void vsAudioSetup()
{
    pinMode(CARDCS, OUTPUT);
    pinMode(SHIELD_CS, OUTPUT);

    digitalWrite(SHIELD_CS, LOW);
    delay(1000);

    if (!musicPlayer.begin())
    { // initialise the music player
        Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
        while (1)
            ;
    }
    digitalWrite(SHIELD_CS, HIGH);
    Serial.println(F("VS1053 found"));

    digitalWrite(CARDCS, LOW);
    if (!SD.begin(CARDCS))
    {
        Serial.println(F("SD failed, or not present"));
        while (1)
            ; // don't do anything more
    }
    // list files
    printDirectory(SD.open("/"), 0);
    digitalWrite(CARDCS, HIGH);

    // Set volume for left, right channels. lower numbers == louder volume!
    digitalWrite(SHIELD_CS, LOW);
    musicPlayer.setVolume(0, 0);
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT); // DREQ int
    digitalWrite(SHIELD_CS, HIGH);

    startAudio();
    delay(2000);
    stopAudio();
    Serial.println(F("Audio Setup Complete."));
}

void startAudio()
{
    Serial.println(F("Playing Sound"));
    digitalWrite(SHIELD_CS, LOW);
    musicPlayer.startPlayingFile("/oink.mp3");
    digitalWrite(SHIELD_CS, HIGH);
}

void stopAudio()
{
    digitalWrite(SHIELD_CS, LOW);
    musicPlayer.stopPlaying();
    digitalWrite(SHIELD_CS, HIGH);
}