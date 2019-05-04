#include <Keyboard.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

String currCardId;

void setup()
{ 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  Keyboard.begin();
}
 
void loop()
{

  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
  {
    return;
  }

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
  {
    return;
  }

  currCardId = "";

  for (int i = 0; i < rfid.uid.size; i++)
  {
    //Serial.println(rfid.uid.uidByte[i]);
    currCardId += rfid.uid.uidByte[i];
  }
  //Serial.println();
  Keyboard.println(currCardId);

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}
