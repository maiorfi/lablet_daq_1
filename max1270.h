#include "mbed.h"

#include <map>

typedef struct _ChannelConfiguration
{
    bool range_10V;
    bool bipolar;
} Max1270ChannelConfiguration;

typedef std::map<uint8_t, Max1270ChannelConfiguration> Max1270ChannelConfigurationMap;

extern Max1270ChannelConfigurationMap ChannelConfigurationMap;

void initMax1270(PinName mosi, PinName miso, PinName sclk, PinName ssel);
void deinitMax1270();
float read_max1270_volts(uint8_t chan);