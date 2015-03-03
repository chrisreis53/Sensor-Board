#include "rtos.h"

Mutex sdcard;

void t_led_blink(void const *args) {
    while (true) {
    	watchdog_refresh();
        led2 = !led2;
        Thread::wait(1000);
    }
}

void t_led_blink2(void const *args) {
    while (true) {
    	watchdog_refresh();
        led1 = !led1;
        Thread::wait(500);
    }
}

void t_server_high(void const *args) {
	while (true) {
		watchdog_refresh();
		int acc = TCP_high.accept();


	}
}

