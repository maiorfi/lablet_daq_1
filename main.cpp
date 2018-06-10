#include "mbed.h"
#include "max1270.h"

#include <vector>

DigitalOut led1(PH_0);
DigitalOut led2(PH_1);

#define POLLING_TASK_INTERVAL_MSECS_MAX1270 20
#define POLLING_TASK_INTERVAL_MSECS_IO 500

static Thread s_thread_polling_task_max1270;
static EventQueue s_eq_polling_task_max1270;

static Thread s_thread_polling_task_io;
static EventQueue s_eq_polling_task_io;

static Timer s_main_timer;

static Serial s_serialgw(PA_9, PA_10, 921600);
static DigitalOut s_max1270_on(PC_0, true);    // collegato a /SHUTDOWN

static std::vector<DigitalOut*> s_douts;
static std::vector<DigitalIn*> s_dins;

static SPI _spi(PB_15, PB_14, PB_13);

static Max1270 _max1270(_spi, PC_1);

static void initHardware()
{
    s_serialgw.set_flow_control(Serial::Flow::RTSCTS, PA_12, PA_11);

    // range 10V, bipolar
    _max1270.setChannelConfiguration(0, true, true);
    _max1270.setChannelConfiguration(1, true, true);

    s_douts.push_back(new DigitalOut(PA_4));
    s_douts.push_back(new DigitalOut(PA_5));
    s_douts.push_back(new DigitalOut(PA_6));
    s_douts.push_back(new DigitalOut(PA_7));

    s_dins.push_back(new DigitalIn(PA_0));
    s_dins.push_back(new DigitalIn(PA_1));
    s_dins.push_back(new DigitalIn(PA_2));
    s_dins.push_back(new DigitalIn(PA_3));

    s_serialgw.printf("{TIMEPLOT|CLEAR}\n");
}

void event_proc_polling_task_max1270()
{
    led1.write(!led1.read());

    for (auto it = _max1270.getChannelConfiguration().begin(); it != _max1270.getChannelConfiguration().end(); ++it)
    {
        s_serialgw.printf("{TIMEPLOT|DATA|MAX1270-CH-%d|T|%.3f}\n", (*it).first, _max1270.read_volts((*it).first));
    }
}

void event_proc_polling_task_io()
{
    led2.write(!led2.read());

    static uint32_t index=0;

    s_douts[index]->write(!s_douts[index]->read());

    index++;

    if(index==s_douts.size()) index=0;

    printf("DIGITAL INPUTS: ");

    for(std::vector<DigitalIn*>::iterator it=s_dins.begin();it!=s_dins.end();++it)
    {
        printf("%s ",(*it)->read() ? "HI" : "LO");
    }

    printf("\n");
}

int main()
{
    printf("MAX1270 DAQ Sample Started\n\n");

    initHardware();

    s_main_timer.start();

    s_eq_polling_task_max1270.call_every(POLLING_TASK_INTERVAL_MSECS_MAX1270, event_proc_polling_task_max1270);
    s_thread_polling_task_max1270.start(callback(&s_eq_polling_task_max1270, &EventQueue::dispatch_forever));

    s_eq_polling_task_io.call_every(POLLING_TASK_INTERVAL_MSECS_IO, event_proc_polling_task_io);
    s_thread_polling_task_io.start(callback(&s_eq_polling_task_io, &EventQueue::dispatch_forever));
}