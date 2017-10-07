/*
 * main.c
 *
 *  Created on: 2015/05/29
 *      Author: shirakami
 */

#include <stdio.h>
#include <string.h>
#include "wiringPi.h"
#include "asi.h"

#define LED_GPIO_NO 19

void CallBackFromASI(unsigned char *asi_res_buf);
void CallBackFromASIatOnce(unsigned char *asi_res_buf);

static unsigned char res_buf[RESPONSE_BUFFER_SIZE];
static unsigned char callbacked_flag;

int main(void) {

    unsigned char timer;
    unsigned char loop;

    printf("InitASI\n");

    //if (ASI_RET_NACK == InitASI(UART_BAUD_9600)) {
    //if (ASI_RET_NACK == InitASI(UART_BAUD_19200)) {
    //if (ASI_RET_NACK == InitASI(UART_BAUD_38400)) {
    //if (ASI_RET_NACK == InitASI(UART_BAUD_57600)) {
    if (ASI_RET_NACK == InitASI(UART_BAUD_115200)) {
        printf("init error");
        while(1);
    }

    pinMode(LED_GPIO_NO, OUTPUT);
#if 0
    printf("\nGetRomVersion\n");
    if (ASI_RET_NACK == GetRomVersion(res_buf)) {
        printf("cmd error");
        while(1);
    }

    for (loop = 0;loop < 20;loop++)
    {
        printf("0x%02X,",res_buf[loop]);
    }

    while(1);
#endif
    printf("\nSetFelicaMode\n");
    if (ASI_RET_NACK == SetFelicaMode(res_buf)) {
        printf("cmd error");
        while(1);
    }

#if 0
    printf("\nStartFelicaPollingSync\n");
    if (ASI_RET_NACK == StartFelicaPollingSync(1,res_buf)) {
        printf("cmd error");
        while(1);
    }
    printf("\nStopFelicaPolling\n");
    if (ASI_RET_NACK == StopFelicaPolling(res_buf)) {
        printf("cmd error");
        while(1);
    }

    for (loop = 0;loop < 20;loop++)
    {
        printf("0x%02X,",res_buf[loop]);
    }
    printf("\n");


    printf("\nDisposeASI\n");
    DisposeASI();

    timer = 0;
    while(1) {
        digitalWrite(LED_GPIO_NO, 0x01 & timer++);
        delay(500);
    }

#else

    while(1) {
        callbacked_flag = 0;
        printf("\nStartFelicaPollingAsync\n");
        if (ASI_RET_NACK == StartFelicaPollingAsync(1,res_buf, CallBackFromASI)) {
            printf("cmd error");
            while(1);
        }
        // wait for call-backed
        while(!callbacked_flag) {
            digitalWrite(LED_GPIO_NO, 0x01 & timer++);
            delay(500);
        }
        printf("\n-----------------------------\n");
        delay(1000);
    }
#endif

}

void CallBackFromASIatOnce(unsigned char *asi_res_buf) {

    int i;

    printf("\ncall back func has called\n");
    memcpy(res_buf, asi_res_buf, RESPONSE_BUFFER_SIZE);

    for (i = 0; i < 20; i++) {
        printf("0x%02X,", res_buf[i]);
    }

    printf("\nStopFelicaPolling\n");
    if (ASI_RET_NACK == StopFelicaPolling(res_buf)) {
        printf("cmd error");
        while(1);
    }
    printf("\nDisposeASI\n");
    DisposeASI();
}

void CallBackFromASI(unsigned char *asi_res_buf) {

    int i;

    printf("\ncall back func has called\n");
    memcpy(res_buf, asi_res_buf, RESPONSE_BUFFER_SIZE);

    for (i = 0; i < 20 ; i++) {
        printf("0x%02X,", res_buf[i]);
    }
    callbacked_flag = 1;

}
