#include "mbed.h"
#include "rtos.h"
#include "WIZnetInterface.h"

Serial pc(SERIAL_TX,SERIAL_RX);
WIZnetInterface eth(SPI_MOSI, SPI_MISO, SPI_SCK,SPI_CS,PB_4); // spi, cs, reset
TCPSocketServer TCP_high;
TCPSocketServer TCP_low;
TCPSocketConnection client_h;
TCPSocketConnection client_l;


DigitalOut led1(PA_0, 0);
DigitalOut led2(PA_1, 0);

#include "helper.h"
#include "threads.h"

int main() {

	watchdog_init();
	watchdog_status();
	watchdog_start();

	f_ethernet_init();


    Thread thread1(t_led_blink);
    Thread thread2(t_led_blink2);

    while (true) {

    }
}
