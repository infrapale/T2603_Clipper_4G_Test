#ifndef __MAIN_H__
#define __MAIN_H__


// TFT definitions selects the correct TFT library settings
// Must be set in the TFT_eSPI Library
#define BOARD_PICO_TFT_4KEYS        1
#define BOARD_TFT_4_QUADCORE_PICO   2
#define TFT_TARGET_BOARD            BOARD_TFT_4_QUADCORE_PICO


#define PIRPANA
// #define LILLA_ASTRID
// #define VILLA_ASTRID

#include "io.h"
#include "atask.h"

#define LABEL_LEN           12
#define TXT_LEN             40
#define TIME_ZONE_OFFS      2
#define UNIT_LABEL_LEN      10
#define MEASURE_LABEL_LEN   16

#define APP_NAME   "T2602_PicoTFT28_AIO_v1"
#define MAIN_TITLE "TFT 2.8 Inc Console"

typedef enum 
{
  AIO_PUBL_VA_HOME_MODE = 0,
  AIO_PUBL_VA_AC_TEMP,
  AIO_PUBL_NBR_OF
} aio_publ_et;


typedef enum
{
    UNIT_TEMPERATURE = 0,
    UNIT_HUMIDITY,
    UNIT_AIR_PRESSURE,
    UNIT_LIGHT,
    UNIT_LDR,
    UNIT_VOLTAGE,
    UNIT_TIME,
    UNIT_CO2,
    UNIT_LUX,
    UNIT_NBR_OF
} unit_et;


typedef struct date_time
{
    uint16_t  year;
    uint8_t   month;
    uint8_t   day;
    uint8_t   hour;
    uint8_t   minute;
    uint8_t   second;
 } date_time_st;


 
#endif