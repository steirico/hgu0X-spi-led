// Build with command: gcc -Wall -I. -o hgu0X-led main.c bcm2835.c
//


#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_spi()
{
  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST); // The default
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0); // The default
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_512); // The default
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0); // The default
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW); // the default
  // Send a byte to the slave and simultaneously read a byte back from the slave
  // If you tie MISO to MOSI, you should read back what was sent
}

void init_max6966()
{
  uint8_t cWriteBuf[8];
  uint8_t *pcWriteBuf;

  cWriteBuf[0]  = 0x15;			// Set Global Full Current
  cWriteBuf[1]  = 0x00;
  pcWriteBuf = &cWriteBuf[0];
  bcm2835_spi_writenb(pcWriteBuf,2);

  cWriteBuf[2]  = 0x12;			// Set Ramp Up Time
  cWriteBuf[3]  = 0x02;
  pcWriteBuf = &cWriteBuf[2];
  bcm2835_spi_writenb(pcWriteBuf,2);

  cWriteBuf[4]  = 0x11;			// Set Ramp Down Time
  cWriteBuf[5]  = 0x02;
  pcWriteBuf = &cWriteBuf[4];
  bcm2835_spi_writenb(pcWriteBuf,2);

  cWriteBuf[6]  = 0x10;			// Set Normal Mode
  cWriteBuf[7]  = 0x01;
  pcWriteBuf = &cWriteBuf[6];
  bcm2835_spi_writenb(pcWriteBuf,2);
}

uint8_t calcLEDValueForMaxDriver(uint8_t ucValue)
{
    int iColorTmp = (int)ucValue * 251;
    iColorTmp = iColorTmp >> 8;
    iColorTmp += 3;

    int iBrightnessTmp = 100 * 255;
    iBrightnessTmp = iBrightnessTmp >> 8;

    iColorTmp *= iBrightnessTmp;
    iColorTmp /= 100;

    if(iColorTmp > 0xFE)            // Value out of upper range
        return 0xFE;
    else if(iColorTmp <= 0x03)      // Value is zero ==> return 0xFF for forcing Port to high impedance
        return 0xFF;
    else                            // Value is in Range from 0x03 to 0xFE
        return (uint8_t)iColorTmp;
}


void writeLedValues(uint8_t ucLEDNumber, uint8_t ucValue_R, uint8_t ucValue_G, uint8_t ucValue_B)
{
  uint8_t cWriteBuf[2];
  uint8_t *pcWriteBuf;
  uint8_t ucLEDValueTmp;

  uint8_t ucCnt = 0;
  for(ucCnt = 0; ucCnt < 3; ucCnt++)
  {
    cWriteBuf[0] = (ucLEDNumber * 3) + ucCnt;		// Set RGB LED Adress
    switch(ucCnt)
    {
      case 0:
        ucLEDValueTmp = ucValue_R; break;
      case 1:
        ucLEDValueTmp = ucValue_G; break;
      case 2:
        ucLEDValueTmp = ucValue_B; break;
      default:
        ucLEDValueTmp = 0; break;
    }
    cWriteBuf[1] = calcLEDValueForMaxDriver(ucLEDValueTmp);

    pcWriteBuf = &cWriteBuf[0];
    bcm2835_spi_writenb(pcWriteBuf,2);
  }
}

int main(int argc, char *argv[])
{
  if(argc < 10) {
    printf("Programm need 9 Values 3 for each LED in following Format from 0 to 255:\n");
    printf("LED1_R LED1_G LED1_B LED2_R LED2_G LED2_B LED3_R LED3_G LED3_B");
    printf("\n");
    return 0;
  }

  uint8_t uiLEDValues[9];
  int iCnt = 0;
  long lResult;
  for(iCnt = 0; iCnt < 9; iCnt++) {
    lResult = strtol(argv[iCnt+1], NULL, 10);
    if(lResult > 255)
      lResult = 255;
    if(lResult < 0)
      lResult = 0;
    uiLEDValues[iCnt] = (uint8_t)lResult;
  }

  // If you call this, it will not actually access the GPIO
  // Use for testing
  // bcm2835_set_debug(1);
  if (!bcm2835_init())
    return 1;

  init_spi();
  init_max6966();

  writeLedValues(0,uiLEDValues[0],uiLEDValues[1],uiLEDValues[2]);
  writeLedValues(1,uiLEDValues[3],uiLEDValues[4],uiLEDValues[5]);
  writeLedValues(2,uiLEDValues[6],uiLEDValues[7],uiLEDValues[8]);

  bcm2835_spi_end();
  bcm2835_close();
  return 0;
}
