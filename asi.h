

#define UART_DEV_TTY "/dev/ttyAMA0"
#define RESPONSE_BUFFER_SIZE 50


typedef enum  {
    ASI_RET_ACK = 0,
    ASI_RET_NACK,
} ENUM_ASI_RET;

typedef enum  {
    UART_BAUD_9600 = 0, 
    UART_BAUD_19200,
    UART_BAUD_38400,
    UART_BAUD_57600,
    UART_BAUD_115200,
} ENUM_UART_BAUD;

typedef enum  {
    CMD_TYPE_GETROMVER = 0,
} ENUM_CMD_TYPE;

typedef struct ASI_CMD {
    unsigned char *cmd;
    unsigned char cmd_len;
} ST_ASI_CMD;

typedef void (*FUNCPTR)(unsigned char* response);


ENUM_ASI_RET InitASI(ENUM_UART_BAUD);
void DisposeASI(void);
ENUM_ASI_RET GetRomVersion(unsigned char *response);
ENUM_ASI_RET SetFelicaMode(unsigned char *response);
ENUM_ASI_RET StartFelicaPollingSync(unsigned char detect_count, unsigned char *response);
ENUM_ASI_RET StartFelicaPollingAsync(unsigned char detect_count, unsigned char *response, FUNCPTR callback);
ENUM_ASI_RET StopFelicaPolling(unsigned char *response);

