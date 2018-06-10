#include "mbed.h"

#include <map>

class Max1270
{
  public:
    typedef struct _ChannelConfiguration
    {
        bool range_10V;
        bool bipolar;
    } Max1270ChannelConfiguration;

    typedef std::map<uint8_t, Max1270ChannelConfiguration> Max1270ChannelConfigurationMap;

  private:
    Max1270::Max1270ChannelConfigurationMap _channelConfigurationMap;

    SPI &_spi;
    DigitalOut _cs;

    void initialize();
    uint16_t read_raw(uint8_t chan, bool range_10V, bool bipolar);
    float read_volts_internal(uint8_t chan, bool range_10V, bool bipolar);

  public:
    Max1270(SPI &spi, PinName csPinName) : _spi(spi), _cs(csPinName, true) { initialize(); }
    Max1270(SPI &spi) : _spi(spi), _cs(NC) { initialize(); }

    Max1270ChannelConfigurationMap &getChannelConfiguration() { return _channelConfigurationMap; }

    void setChannelConfiguration(uint8_t chan, bool range_10V, bool bipolar) {_channelConfigurationMap[chan]={range_10V, bipolar};}
    float read_volts(uint8_t chan);
};