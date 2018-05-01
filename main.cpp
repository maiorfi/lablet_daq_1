#include "mbed.h"
#include "max1270.h"

#include <queue>

DigitalOut led1(LED1);

#define POLLING_TASK_INTERVAL_MSECS 10

static Thread s_thread_polling_task;
static EventQueue s_eq_polling_task;

static Timer s_main_timer;

static void initHardware()
{
    initMax1270(PE_6, PE_5, PE_2, PE_4);

    // range 10V, bipolar
    ChannelConfigurationMap[0] = {false, false};
    ChannelConfigurationMap[1] = {false, false};
    ChannelConfigurationMap[2] = {true, true};
}

void event_proc_polling_task()
{
    led1 = !led1;

    printf("%d|", s_main_timer.read_ms());

    uint8_t counter = 0;

    for (Max1270ChannelConfigurationMap::iterator it = ChannelConfigurationMap.begin(); it != ChannelConfigurationMap.end(); ++it)
    {
        counter++;
        printf("%.3f", read_max1270_volts(it->first));
        if (counter < ChannelConfigurationMap.size())
            printf("|");
    }

    printf("\n");
}

int main()
{
    printf("MAX1270 DAQ Sample Started\n\n");

    initHardware();

    s_main_timer.start();

    s_eq_polling_task.call_every(POLLING_TASK_INTERVAL_MSECS, event_proc_polling_task);
    s_thread_polling_task.start(callback(&s_eq_polling_task, &EventQueue::dispatch_forever));
}