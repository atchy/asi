
#include <stdio.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include "asi.h"

#define ARRAY_SIZE_OF(a)    ( sizeof( a ) / sizeof( a[0] ) )
#define RXD_PIN_NUMBER 15
#define WAIT_AFTER_SEND_MS 100
#define WAIT_RECEIVE_MS 3

#define DEBUG

#ifdef DEBUG
#define  DEBUG_OUT printf
#else
#define DEBUG_OUT 1 ? (void) 0 : printf
#endif

static int fd;

/**
 * GetRomVersion
 */
static unsigned char cmd_get_rom_ver[] = { 0x02, 0x00, 0x4F, 0x02, 0x90, 0x00, 0x03, 0xE6, 0x0D };
static ST_ASI_CMD get_rom_ver = {
    cmd_get_rom_ver,
    ARRAY_SIZE_OF(cmd_get_rom_ver)
};

/**
 * SetBaudrate (9600bps)
 */
static unsigned char cmd_set_baudrate_4bytes[] = {0x02, 0x00, 0x7B, 0x04, 0x39, 0x36, 0x30, 0x30, 0x03, 0x53, 0x0D};
static ST_ASI_CMD set_baudrate_4bytes = {
    cmd_set_baudrate_4bytes,
    ARRAY_SIZE_OF(cmd_set_baudrate_4bytes)
};

/**
 * SetBaudrate (19200bps to 57600bps)
 */
static unsigned char cmd_set_baudrate_5bytes[] = {0x02, 0x00, 0x7B, 0x05, 0x33, 0x38, 0x34, 0x30, 030, 0x03, 0x84, 0x0D};
static ST_ASI_CMD set_baudrate_5bytes = {
    cmd_set_baudrate_5bytes,
    ARRAY_SIZE_OF(cmd_set_baudrate_5bytes)
};

/**
 * SetBaudrate (115200bps)
 */
static unsigned char cmd_set_baudrate_6bytes[] = {0x02, 0x00, 0x7B, 0x06, 0x31, 0x31, 0x35, 0x32, 0x30, 0x30, 0x03, 0xAF, 0x0D};
static ST_ASI_CMD set_baudrate_6bytes = {
        cmd_set_baudrate_6bytes,
    ARRAY_SIZE_OF(cmd_set_baudrate_6bytes)
};

/**
 * SetFelicaMode
 */
static unsigned char cmd_set_felica_mode[] = {0x02, 0x00, 0x70, 0x01, 0x01, 0x03, 0x77, 0x0D};
static ST_ASI_CMD set_felica_mode = {
    cmd_set_felica_mode,
    ARRAY_SIZE_OF(cmd_set_felica_mode)
};

/**
 * StartFelicaPolling
 */
static unsigned char cmd_start_felica_polling[] = {0x02, 0x00, 0x72, 0x09, 0x00, 0x64, 0x20, 0x06, 0x00, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x09, 0x0D};
static ST_ASI_CMD start_felica_polling = {
    cmd_start_felica_polling,
    ARRAY_SIZE_OF(cmd_start_felica_polling)
};

/**
 * StopFelicaPolling
 */
static unsigned char cmd_stop_felica_polling[] = {0x02, 0x00, 0x72, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x0D};
static ST_ASI_CMD stop_felica_polling = {
        cmd_stop_felica_polling,
    ARRAY_SIZE_OF(cmd_stop_felica_polling)
};

static ENUM_ASI_RET doPingPongCmd(ST_ASI_CMD *, unsigned char *);
static void setSum(ST_ASI_CMD *asi_cmd);
static int receiveFromASI(unsigned char *);
static void sendToASI(ST_ASI_CMD *);
//static void callbackAtReceived(void);

static unsigned char res_buf_for_callback[RESPONSE_BUFFER_SIZE];
unsigned char set_baud_res_buf[13];
static FUNCPTR storedCallBack;

/**
 * シリアルポートのオープンとwiringPiの初期化を行います。通信開始前に一度だけ呼び出す必要があります.
 * @param  baud ボーレート
 * @return　ENUM_ASI_RET　本関数の成功/失敗
 */
ENUM_ASI_RET InitASI(ENUM_UART_BAUD baud) {

    unsigned long lBaud;
    ST_ASI_CMD *set_baud_cmd;

    switch (baud) {
    case UART_BAUD_9600:
        lBaud = 9600;
        cmd_set_baudrate_4bytes[4] = 0x39;
        cmd_set_baudrate_4bytes[5] = 0x36;
        cmd_set_baudrate_4bytes[6] = 0x30;
        cmd_set_baudrate_4bytes[7] = 0x30;
        setSum(&set_baudrate_4bytes);
        set_baud_cmd = &set_baudrate_4bytes;
        break;
    case UART_BAUD_19200:
        lBaud = 19200;
        cmd_set_baudrate_5bytes[4] = 0x31;
        cmd_set_baudrate_5bytes[5] = 0x39;
        cmd_set_baudrate_5bytes[6] = 0x32;
        cmd_set_baudrate_5bytes[7] = 0x30;
        cmd_set_baudrate_5bytes[8] = 0x30;
        setSum(&set_baudrate_5bytes);
        set_baud_cmd = &set_baudrate_5bytes;
        break;
    case UART_BAUD_57600:
        cmd_set_baudrate_5bytes[4] = 0x35;
        cmd_set_baudrate_5bytes[5] = 0x37;
        cmd_set_baudrate_5bytes[6] = 0x36;
        cmd_set_baudrate_5bytes[7] = 0x30;
        cmd_set_baudrate_5bytes[8] = 0x30;
        setSum(&set_baudrate_5bytes);
        set_baud_cmd = &set_baudrate_5bytes;
        break;
    case UART_BAUD_115200:
        lBaud = 115200;
        cmd_set_baudrate_6bytes[4] = 0x31;
        cmd_set_baudrate_6bytes[5] = 0x31;
        cmd_set_baudrate_6bytes[6] = 0x35;
        cmd_set_baudrate_6bytes[7] = 0x32;
        cmd_set_baudrate_6bytes[8] = 0x30;
        cmd_set_baudrate_6bytes[9] = 0x30;
        setSum(&set_baudrate_6bytes);
        set_baud_cmd = &set_baudrate_6bytes;
        break;
    case UART_BAUD_38400:
    default:
        lBaud = 38400;
        cmd_set_baudrate_5bytes[4] = 0x33;
        cmd_set_baudrate_5bytes[5] = 0x38;
        cmd_set_baudrate_5bytes[6] = 0x34;
        cmd_set_baudrate_5bytes[7] = 0x30;
        cmd_set_baudrate_5bytes[8] = 0x30;
        setSum(&set_baudrate_5bytes);
        set_baud_cmd = &set_baudrate_5bytes;

        break;
    }

    fd = -1;

    if ((fd = serialOpen (UART_DEV_TTY, 38400)) < 0) {
        DEBUG_OUT ("Unable to open serial\n");
        return ASI_RET_NACK;
    }

    if (baud != UART_BAUD_38400) {
        //　38400bps以外ならボーレート設定コマンドを送信し、シリアルをオープンし直す
        if (ASI_RET_NACK == doPingPongCmd(set_baud_cmd, set_baud_res_buf)) {
            DEBUG_OUT("set baud error\n");
            while(1);
        }
        serialClose(fd);
        if ((fd = serialOpen (UART_DEV_TTY, lBaud)) < 0) {
            DEBUG_OUT ("Unable to open serial\n");
            return ASI_RET_NACK;
        }
   }


    //if (wiringPiSetup () == -1) {
    if (wiringPiSetupSys () == -1) {
    //if(wiringPiSetupGpio() == -1) {
        DEBUG_OUT("Unable to start wiringPi\n");
        return ASI_RET_NACK ;
    }

    return ASI_RET_ACK;
}

/**
 * NFCリーダ/ライタ上のROMバージョンを返します
 * @param  response レスポンスバッファ
 * @return　ENUM_ASI_RET　本関数の成功/失敗
 */
ENUM_ASI_RET GetRomVersion(unsigned char *response) {

    return doPingPongCmd(&get_rom_ver, response);
}
/**
 * 通信モードをFelicaにします。
 * Felicaとの通信前に一度だけ呼び出す必要があります
 * @param  response レスポンスバッファ
 * @return　ENUM_ASI_RET　本関数の成功/失敗
 */
ENUM_ASI_RET SetFelicaMode(unsigned char *response) {

    return doPingPongCmd(&set_felica_mode, response);
}

/**
 * Felicaからの受信待ちに遷移します（同期型）
 * @param　detect_count　検出する回数（1回から7回まで指定可能）
 * @param  response レスポンスバッファ
 * @return　ENUM_ASI_RET　本関数の成功/失敗
 */
ENUM_ASI_RET StartFelicaPollingSync(unsigned char detect_count, unsigned char *response) {

    // set detect count
    int received_count;

    // set detect count
    cmd_start_felica_polling[6] = detect_count << 5;
    // set SUM
    //cmd_start_felica_polling[start_felica_polling.cmd_len - 2] = calcSum(&start_felica_polling);
    setSum(&start_felica_polling);
    sendToASI(&start_felica_polling);
    delay(WAIT_AFTER_SEND_MS);
    DEBUG_OUT("\nWaitForDataComing...\n");
    while(0 >= serialDataAvail(fd));

    received_count = receiveFromASI(response);

    if (0 >= received_count) {
        return ASI_RET_NACK;
    }
    return ASI_RET_ACK;

}

/**
 *
 */
PI_THREAD (waitForRXI) {
    unsigned char res_counter;
    unsigned char buf_counter;
    res_counter = 0;
    buf_counter = 0;

    (void)piHiPri (10) ;    // Set this thread to be high priority
    //


    while(0 >= serialDataAvail(fd));
    DEBUG_OUT("RXI occured\n");
    while (serialDataAvail(fd)) {
        piLock(0);
        res_buf_for_callback[buf_counter++] = serialGetchar(fd);
        piUnlock(0);
        DEBUG_OUT("0x%02X,",res_buf_for_callback[buf_counter -1] );
        delay(WAIT_RECEIVE_MS);
    }
    DEBUG_OUT("\n");
    storedCallBack(res_buf_for_callback);
}

/**
 * [StartFelicaPollingAsync description]
 * @param  detect_count [description]
 * @param  response     [description]
 * @param  callback     [description]
 * @return              [description]
 */
ENUM_ASI_RET StartFelicaPollingAsync(unsigned char detect_count, unsigned char *response, FUNCPTR callback) {

    // set detect count
    cmd_start_felica_polling[6] = detect_count << 5;
    // set SUM
    //cmd_start_felica_polling[start_felica_polling.cmd_len - 2] = calcSum(&start_felica_polling);
    setSum(&start_felica_polling);

    storedCallBack = callback;

    sendToASI(&start_felica_polling);
    delay(WAIT_AFTER_SEND_MS);
    DEBUG_OUT("\nwaitForRXI...\n");
    piThreadCreate(waitForRXI);

    return ASI_RET_ACK;
}

/**
 * Felicaのポーリングを停止します
 * @param  response レスポンスバッファ
 * @return　ENUM_ASI_RET　本関数の成功/失敗
 */
ENUM_ASI_RET StopFelicaPolling(unsigned char *response) {
    return doPingPongCmd(&stop_felica_polling, response);
}
/**
 *
 */
static ENUM_ASI_RET doPingPongCmd(ST_ASI_CMD *asi_cmd, unsigned char *response) {

    int received_count;

    // calcurate SUM value in command
    //asi_cmd->cmd[asi_cmd->cmd_len - 2] = calcSum(asi_cmd);
    setSum(asi_cmd);
    sendToASI(asi_cmd);
    delay(WAIT_AFTER_SEND_MS);
    received_count = receiveFromASI(response);

    if (0 >= received_count) {
        return ASI_RET_NACK;
    }
    return ASI_RET_ACK;
}

/**
 * [DisposeASI description]
 */
void DisposeASI(void) {
    serialClose(fd);
}

static void setSum(ST_ASI_CMD *asi_cmd) {

    unsigned long sum;
    int i;

    for (i = 0,sum = 0; i < asi_cmd->cmd_len - 2; i++) {
        sum += asi_cmd->cmd[i];
    }
    asi_cmd->cmd[asi_cmd->cmd_len - 2] = (unsigned char)(0x000000FF & sum);
}


/**
 * [sendToASI description]
 * @param asi_cmd [description]
 */
static void sendToASI(ST_ASI_CMD *asi_cmd) {

    unsigned char cmd_counter;

    for (cmd_counter = 0; cmd_counter < asi_cmd->cmd_len; cmd_counter++) {
        serialPutchar(fd, asi_cmd->cmd[cmd_counter]);
        DEBUG_OUT("0x%02X, ", asi_cmd->cmd[cmd_counter]);
    }
    DEBUG_OUT("\n");
}

/**
 * [receiveFromASI description]
 * @param  response [description]
 * @return          [description]
 */
static int receiveFromASI(unsigned char *response) {

    unsigned char res_counter;
    res_counter = 0;

    while (0 < serialDataAvail(fd)) {
        response[res_counter++] = serialGetchar(fd);
        DEBUG_OUT("0x%02X,",response[res_counter -1] );
        delay(WAIT_RECEIVE_MS);
    }
    DEBUG_OUT("\n");
    return res_counter;
}
