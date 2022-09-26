//////////////////////////////////////////////////////////////////////////////////////////
//
//    Demo code for the MAX30001 breakout board
//
//    This example plots the ECG through serial UART on openview processing GUI.
//    GUI URL: https://github.com/Protocentral/protocentral_openview.git
//
//    Arduino connections:
//
//  |MAX30001 pin label| Pin Function         |Arduino Connection|
//  |----------------- |:--------------------:|-----------------:|
//  | MISO             | Slave Out            |  D12             |
//  | MOSI             | Slave In             |  D11             |
//  | SCLK             | Serial Clock         |  D13             |
//  | CS               | Chip Select          |  D7              |
//  | VCC              | Digital VDD          |  +5V             |
//  | GND              | Digital Gnd          |  Gnd             |
//  | FCLK             | 32K CLOCK            |  -               |
//  | INT1             | Interrupt1           |  02              |
//  | INT2             | Interrupt2           |  -               |
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//   For information on how to use, visit https://github.com/Protocentral/protocentral_max30001
//
/////////////////////////////////////////////////////////////////////////////////////////

#include<SPI.h>
#include "protocentral_max30001.h"

#define CES_CMDIF_PKT_START_1   0x0A
#define CES_CMDIF_PKT_START_2   0xFA
#define CES_CMDIF_TYPE_DATA     0x02
#define CES_CMDIF_PKT_STOP      0x0B
#define DATA_LEN                0x0C
#define ZERO                    0

volatile char DataPacket[DATA_LEN];
const char DataPacketFooter[2] = {ZERO, CES_CMDIF_PKT_STOP};
const char DataPacketHeader[5] = {CES_CMDIF_PKT_START_1, CES_CMDIF_PKT_START_2, DATA_LEN, ZERO, CES_CMDIF_TYPE_DATA};

uint8_t data_len = 0x0C;

#define MAX30001_CS_PIN 6
MAX30001 max30001(MAX30001_CS_PIN);

void sendDataThroughUART(void){

  DataPacket[5] = max30001.ecg_data;
  DataPacket[6] = max30001.ecg_data>>8;
  DataPacket[7] = max30001.ecg_data>>16;
  DataPacket[8] = max30001.ecg_data>>24;

  DataPacket[9] =  max30001.bioz_data;
  DataPacket[10] = max30001.bioz_data >>8;
  DataPacket[11] = max30001.bioz_data>>16;
  DataPacket[12] = max30001.bioz_data>>24;

  DataPacket[13] = max30001.heartRate ;
  DataPacket[14] = max30001.heartRate >>8;
  DataPacket[15] = 0x00;
  DataPacket[16] = 0x00;

  //send packet header
  for(int i=0; i<5; i++){

    Serial.write(DataPacketHeader[i]);
  }

  //send 30003 data
  for(int i=0; i<DATA_LEN; i++) // transmit the data
  {
    Serial.write(DataPacket[i]);
  }

  //send packet footer
  for(int i=0; i<2; i++){

    Serial.write(DataPacketFooter[i]);
  }
}


void setup()
{
    Serial.begin(57600); //Serial begin

    pinMode(MAX30001_CS_PIN,OUTPUT);
    digitalWrite(MAX30001_CS_PIN,HIGH); //disable device

    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);

    bool ret = max30001.max30001ReadInfo();
    if(ret){
      Serial.println("MAX 30001 read ID Success");
    }else{

      while(!ret){
        //stay here untill the issue is fixed.
        ret = max30001.max30001ReadInfo();
        Serial.println("Failed to read ID, please make sure all the pins are connected");
        delay(5000);
      }
    }

    Serial.println("Initialising the chip ...");
    max30001.BeginBioZ();   // initialize MAX30001
    //max30001.Begin(); 

}

void loop()
{
    max30001.getECGSamples();   //It reads the ecg sample and stores it to max30001.ecgdata .
    //max30001.getHRandRR();   //It will store HR to max30001.heartRate and rr to max30001.RRinterval.
    max30001.getBioZSamples();
    sendDataThroughUART();
    delay(8);
}
