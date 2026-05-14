#ifndef INC_CELLULAR_DRIVER_H_
#define INC_CELLULAR_DRIVER_H_

#include "at_command.h"

/* Connection Configurations */
#define CELL_APN            "super"
#define CELL_BROKER_IP      "your.broker.ip"
#define CELL_BROKER_PORT    1883

/* State Definitions */
typedef enum {
    CELL_STATE_IDLE,
    CELL_STATE_NET_READY,
    CELL_STATE_TCP_OPEN,
    CELL_STATE_ERROR
} CellState_t;

/* Exported Functions */
atCommandErrorCodes_t Cellular_InitDevice(void);
atCommandErrorCodes_t Cellular_SetupNetwork(void);
atCommandErrorCodes_t Cellular_ConnectBroker(void);
atCommandErrorCodes_t Cellular_TransmitRaw(uint8_t* data, uint16_t len);

#endif
