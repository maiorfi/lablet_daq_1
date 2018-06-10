#include "max1270.h"

void Max1270::initialize()
{
    _spi.frequency(5000000); //5MHz Max
    _cs.write(true);
}

uint16_t Max1270::read_raw(uint8_t chan, bool range_10V, bool bipolar)
{
    uint8_t cword = 0x80; //set the start bit

    cword |= (chan << 4); //shift channel
    cword |= (((uint8_t)range_10V) << 3);
    cword |= (((uint8_t)bipolar) << 2);

    _spi.format(8, 0); // 8 data bits, CPOL0, and CPHA0 (datasheet Digital Interface)

    if(_cs.is_connected()) _cs.write(false);

    _spi.write(cword);

    wait_us(15); //15us

    _spi.format(16, 0);
    uint16_t result = _spi.write(0);

    if(_cs.is_connected()) _cs.write(true);

    return result >> 4;
}

float Max1270::read_volts_internal(uint8_t chan, bool range_10V, bool bipolar)
{
    float rangevolts = 0.0;
    float volts = 0.0;
    uint16_t adc_res;

    //read the ADC converter
    adc_res = read_raw(chan, range_10V, bipolar) & 0xFFF;

    //Determine the voltage range
    rangevolts = range_10V ? 10.0 : 5.0;

    //bi-polar input range
    if (bipolar)
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

float Max1270::read_volts(uint8_t chan)
{
    return read_volts_internal(chan, _channelConfigurationMap[chan].range_10V, _channelConfigurationMap[chan].bipolar);
}