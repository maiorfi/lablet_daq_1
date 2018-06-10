#include "max1270.h"

static SPI* spi_max1270;
static DigitalOut* spi_max1270_cs;

Max1270ChannelConfigurationMap ChannelConfigurationMap;

void initMax1270(PinName mosi, PinName miso, PinName sclk, PinName ssel)
{
    spi_max1270=new SPI(mosi, miso, sclk, ssel);
    spi_max1270->frequency(5000000); //5MHz Max
}

void initMax1270_no_hw_cs(PinName mosi, PinName miso, PinName sclk, PinName ssel)
{
    spi_max1270=new SPI(mosi, miso, sclk);
    spi_max1270->frequency(5000000); //5MHz Max

    spi_max1270_cs=new DigitalOut(ssel, true);
}

void deinitMax1270()
{
    delete spi_max1270_cs;
    delete spi_max1270;
}

static uint16_t read_max1270(uint8_t chan, bool range_10V, bool bipol)
{
    uint8_t cword = 0x80; //set the start bit

    cword |= (chan << 4); //shift channel
    cword |= (((uint8_t)range_10V) << 3);
    cword |= (((uint8_t)bipol) << 2);

    spi_max1270->format(8, 0); // 8 data bits, CPOL0, and CPHA0 (datasheet Digital Interface)

    if(spi_max1270_cs) spi_max1270_cs->write(false);

    spi_max1270->write(cword);

    wait_us(15); //15us

    spi_max1270->format(16, 0);
    uint16_t result = spi_max1270->write(0);

    if(spi_max1270_cs) spi_max1270_cs->write(true);

    return result >> 4;
}

static float read_max1270_volts(uint8_t chan, bool range_10V, bool bipol)
{
    float rangevolts = 0.0;
    float volts = 0.0;
    uint16_t adc_res;

    //read the ADC converter
    adc_res = read_max1270(chan, range_10V, bipol) & 0xFFF;

    //Determine the voltage range
    rangevolts = range_10V ? 10.0 : 5.0;

    //bi-polar input range
    if (bipol)
    { //BIP is set, input is +/-
        if (adc_res < 0x800)
        { //if result was positive
            volts = ((float)adc_res / 0x7FF) * rangevolts;
        }
        else
        { //result was negative
            volts = -(-((float)adc_res / 0x7FF) * rangevolts) - (rangevolts * 2.0);
        }
    }
    else
    { //input is positive polarity only
        volts = ((float)adc_res / 0xFFF) * rangevolts;
    }

    return volts;
}

float read_max1270_volts(uint8_t chan)
{
    return read_max1270_volts(chan, ChannelConfigurationMap[chan].range_10V, ChannelConfigurationMap[chan].bipolar);
}