#include "WIZnetInterface.h"
#include "stm32f4xx_hal_iwdg.h"

IWDG_HandleTypeDef hiwdg;	//Watchdog timer
int ret;


void watchdog_init(void){

	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
	hiwdg.Init.Reload = 1350; //10 seconds
	HAL_IWDG_Init(&hiwdg);

}

void watchdog_start(void){
	HAL_IWDG_Start(&hiwdg);
}

void watchdog_refresh(void){
	HAL_IWDG_Refresh(&hiwdg);
}

void watchdog_status(void){
	switch (HAL_IWDG_GetState(&hiwdg)){
		case HAL_IWDG_STATE_RESET:
			pc.printf("IWDG not yet initialized or disabled\r\n");
			break;
		case HAL_IWDG_STATE_READY:
			pc.printf("IWDG initialized and ready for use\r\n");
			break;
		case HAL_IWDG_STATE_BUSY:
			pc.printf("IWDG internal process is ongoing\r\n");
			break;
		case HAL_IWDG_STATE_TIMEOUT:
			pc.printf("IWDG timeout state\r\n");
			break;
		case HAL_IWDG_STATE_ERROR:
			pc.printf("IWDG error state\r\n");
			break;
		default:
			pc.printf("Unknown state\n\r");
	}
}

void f_ethernet_init()
{
	watchdog_refresh();
    uint8_t mac[]={0x90,0xa2,0xDa,0x0d,0x42,0xe0};
    // mbed_mac_address((char *)mac);
    pc.printf("\n\r\t####Starting Ethernet Server#### \n\r");
    wait(1.0);
    ret = eth.init(mac);
    if(!ret)
    {
        pc.printf("Initialized, MAC= %s\n\r",eth.getMACAddress());
    }
    else
    {
        pc.printf("Communication Failure  ... Restart devices ...\n\r");
    }
    pc.printf("Connecting...\r\n");
    ret = eth.connect();
    if(!ret)
    {
        pc.printf("Connection Established!\n\n\r");
        wait(1);
        pc.printf("IP=%s\n\rMASK=%s\n\rGW=%s\n\r",eth.getIPAddress(), eth.getNetworkMask(), eth.getGateway());
    }
    else
    {
        pc.printf("Communication Failure  ... Restart devices ...\n\r");
    }
}

void f_server_init(TCPSocketServer server, int port)
{
	server.bind(port);
	server.listen();
}
