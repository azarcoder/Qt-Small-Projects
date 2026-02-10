#ifndef AIS_DECODE_H
#define AIS_DECODE_H

#include <stdint.h>

#define AIS_SIXBIT_LEN            6
#define AIS_ASCII_PRINTABLE_MIN   32
#define AIS_ASCII_OFFSET          64

/* AIS NMEA 6-bit ASCII ranges */
#define AIS_NMEA_CHAR_MIN        48   /* '0' Start of valid AIS payload chars*/ 
#define AIS_NMEA_CHAR_MAX        119  /* 'w' End of AIS payload chars*/

#define AIS_NMEA_RANGE1_MAX      87   /* 'W' End of first AIS range */

#define AIS_NMEA_RANGE1_OFFSET   48  //Offset for first range
#define AIS_NMEA_RANGE2_OFFSET   56 //Offset for second range

#define AIS_SUCCESS                0

#define AIS_ERR_INVALID_ARGUMENT  -1  /* NULL pointer, bad input */
#define AIS_ERR_FORMAT            -2  /* Invalid NMEA / unsupported message */
#define AIS_ERR_INCOMPLETE        -3  /* Waiting for more fragments */

#define TYPE1MSG 1
#define TYPE2MSG 2
#define TYPE3MSG 3
#define TYPE4MSG 4
#define TYPE5MSG 5
#define TYPE6MSG 6
#define TYPE7MSG 7
#define TYPE8MSG 8
#define TYPE9MSG 9
#define TYPE10MSG 10
#define TYPE11MSG 11
#define TYPE12MSG 12
#define TYPE13MSG 13
#define TYPE14MSG 14
#define TYPE15MSG 15
#define TYPE16MSG 16
#define TYPE17MSG 17
#define TYPE18MSG 18
#define TYPE19MSG 19
#define TYPE20MSG 20
#define TYPE21MSG 21
#define TYPE22MSG 22
#define TYPE23MSG 23
#define TYPE24MSG 24
#define TYPE25MSG 25
#define TYPE26MSG 26
#define TYPE27MSG 27

#define AIS_OK                    0
#define AIS_ERR_INVALID_FIELD    -5


/* ===== Type 1/2/3 field error bits ===== */
#define AIS_ERR_MSG_ID        (1U << 0)
#define AIS_ERR_REPEAT        (1U << 1)
#define AIS_ERR_MMSI          (1U << 2)
#define AIS_ERR_NAV_STATUS    (1U << 3)
#define AIS_ERR_ROT           (1U << 4)
#define AIS_ERR_SOG           (1U << 5)
#define AIS_ERR_POS_ACC       (1U << 6)
#define AIS_ERR_LON           (1U << 7)
#define AIS_ERR_LAT           (1U << 8)
#define AIS_ERR_COG           (1U << 9)
#define AIS_ERR_HEADING       (1U << 10)
#define AIS_ERR_TIMESTAMP     (1U << 11)
#define AIS_ERR_MANEUVER      (1U << 12)
#define AIS_ERR_RAIM          (1U << 13)
#define AIS_ERR_SYNC_STATE    (1U << 14)
#define AIS_ERR_SLOT_TIMEOUT  (1U << 15)
#define AIS_ERR_SUB_MESSAGE   (1U << 16)
#define AIS_ERR_SPARE         (1U << 17)

/* ===== Type 4 field error bits ===== */
#define AIS4_ERR_MSG_ID     (1U << 0)
#define AIS4_ERR_REPEAT     (1U << 1)
#define AIS4_ERR_MMSI       (1U << 2)

#define AIS4_ERR_YEAR       (1U << 3)
#define AIS4_ERR_MONTH      (1U << 4)
#define AIS4_ERR_DAY        (1U << 5)
#define AIS4_ERR_HOUR       (1U << 6)
#define AIS4_ERR_MINUTE     (1U << 7)
#define AIS4_ERR_SECOND     (1U << 8)

#define AIS4_ERR_POS_ACC    (1U << 9)
#define AIS4_ERR_LON        (1U << 10)
#define AIS4_ERR_LAT        (1U << 11)

#define AIS4_ERR_EPFS       (1U << 12)
#define AIS4_ERR_RAIM       (1U << 13)
#define AIS4_ERR_SPARE      (1U << 14)

/* ===== Type 5 field error bits ===== */
#define AIS5_ERR_MSG_ID        (1U << 0)
#define AIS5_ERR_REPEAT        (1U << 1)
#define AIS5_ERR_MMSI          (1U << 2)

#define AIS5_ERR_AIS_VERSION   (1U << 3)
#define AIS5_ERR_IMO           (1U << 4)

#define AIS5_ERR_CALLSIGN      (1U << 5)
#define AIS5_ERR_SHIP_NAME     (1U << 6)

#define AIS5_ERR_SHIP_TYPE     (1U << 7)

#define AIS5_ERR_DIM_BOW       (1U << 8)
#define AIS5_ERR_DIM_STERN     (1U << 9)
#define AIS5_ERR_DIM_PORT      (1U << 10)
#define AIS5_ERR_DIM_STARBOARD (1U << 11)

#define AIS5_ERR_EPFS          (1U << 12)

#define AIS5_ERR_ETA_MONTH     (1U << 13)
#define AIS5_ERR_ETA_DAY       (1U << 14)
#define AIS5_ERR_ETA_HOUR      (1U << 15)
#define AIS5_ERR_ETA_MINUTE    (1U << 16)

#define AIS5_ERR_DRAUGHT       (1U << 17)
#define AIS5_ERR_DESTINATION  (1U << 18)

#define AIS5_ERR_DTE           (1U << 19)
#define AIS5_ERR_SPARE         (1U << 20)

/* ===== Type 6 field error bits ===== */
#define AIS6_ERR_MSG_ID      (1U << 0)
#define AIS6_ERR_REPEAT      (1U << 1)
#define AIS6_ERR_MMSI        (1U << 2)

#define AIS6_ERR_SEQ_NUM     (1U << 3)
#define AIS6_ERR_DEST_MMSI   (1U << 4)
#define AIS6_ERR_RETRANSMIT  (1U << 5)

#define AIS6_ERR_DAC         (1U << 6)
#define AIS6_ERR_FI          (1U << 7)

#define AIS6_ERR_BINARY_LEN  (1U << 8)

/* ===== Type 7 field error bits ===== */
#define AIS7_ERR_MSG_ID     (1U << 0)
#define AIS7_ERR_REPEAT     (1U << 1)
#define AIS7_ERR_MMSI       (1U << 2)

#define AIS7_ERR_ACK_COUNT  (1U << 3)
#define AIS7_ERR_DEST_MMSI  (1U << 4)
#define AIS7_ERR_SEQ_NUM    (1U << 5)

/* ===== Type 8 field error bits ===== */
#define AIS8_ERR_MSG_ID      (1U << 0)
#define AIS8_ERR_REPEAT      (1U << 1)
#define AIS8_ERR_MMSI        (1U << 2)

#define AIS8_ERR_DAC         (1U << 3)
#define AIS8_ERR_FI          (1U << 4)

#define AIS8_ERR_BINARY_LEN  (1U << 5)

/* ===== Type 9 field error bits ===== */
#define AIS9_ERR_MSG_ID     (1U << 0)
#define AIS9_ERR_REPEAT     (1U << 1)
#define AIS9_ERR_MMSI       (1U << 2)

#define AIS9_ERR_ALTITUDE   (1U << 3)
#define AIS9_ERR_SOG        (1U << 4)
#define AIS9_ERR_POS_ACC    (1U << 5)

#define AIS9_ERR_LON        (1U << 6)
#define AIS9_ERR_LAT        (1U << 7)

#define AIS9_ERR_COG        (1U << 8)
#define AIS9_ERR_TIMESTAMP  (1U << 9)

#define AIS9_ERR_DTE        (1U << 10)
#define AIS9_ERR_RAIM       (1U << 11)

/* ===== Type 10 field error bits ===== */
#define AIS10_ERR_MSG_ID     (1U << 0)
#define AIS10_ERR_REPEAT     (1U << 1)
#define AIS10_ERR_MMSI       (1U << 2)

#define AIS10_ERR_DEST_MMSI  (1U << 3)
#define AIS10_ERR_SPARE      (1U << 4)

/* ===== Type 11 field error bits ===== */
#define AIS11_ERR_MSG_ID     (1U << 0)
#define AIS11_ERR_REPEAT     (1U << 1)
#define AIS11_ERR_MMSI       (1U << 2)

#define AIS11_ERR_YEAR       (1U << 3)
#define AIS11_ERR_MONTH      (1U << 4)
#define AIS11_ERR_DAY        (1U << 5)
#define AIS11_ERR_HOUR       (1U << 6)
#define AIS11_ERR_MINUTE     (1U << 7)
#define AIS11_ERR_SECOND     (1U << 8)

#define AIS11_ERR_POS_ACC    (1U << 9)
#define AIS11_ERR_LON        (1U << 10)
#define AIS11_ERR_LAT        (1U << 11)

#define AIS11_ERR_RAIM       (1U << 12)
#define AIS11_ERR_SPARE      (1U << 13)

/* ===== Type 12 field error bits ===== */
#define AIS12_ERR_MSG_ID      (1U << 0)
#define AIS12_ERR_REPEAT      (1U << 1)
#define AIS12_ERR_MMSI        (1U << 2)

#define AIS12_ERR_SEQ_NUM     (1U << 3)
#define AIS12_ERR_DEST_MMSI   (1U << 4)
#define AIS12_ERR_RETRANSMIT  (1U << 5)

#define AIS12_ERR_TEXT_LEN    (1U << 6)
#define AIS12_ERR_TEXT_CHAR   (1U << 7)

/* ===== Type 13 field error bits ===== */
#define AIS13_ERR_MSG_ID     (1U << 0)
#define AIS13_ERR_REPEAT     (1U << 1)
#define AIS13_ERR_MMSI       (1U << 2)

#define AIS13_ERR_ACK_COUNT  (1U << 3)
#define AIS13_ERR_DEST_MMSI  (1U << 4)
#define AIS13_ERR_SEQ_NUM    (1U << 5)

/* ===== Type 14 field error bits ===== */
#define AIS14_ERR_MSG_ID     (1U << 0)
#define AIS14_ERR_REPEAT     (1U << 1)
#define AIS14_ERR_MMSI       (1U << 2)

#define AIS14_ERR_TEXT_LEN   (1U << 3)
#define AIS14_ERR_TEXT_CHAR  (1U << 4)

/* ===== Type 15 field error bits ===== */
#define AIS15_ERR_MSG_ID      (1U << 0)
#define AIS15_ERR_REPEAT      (1U << 1)
#define AIS15_ERR_MMSI        (1U << 2)

#define AIS15_ERR_DEST1_MMSI  (1U << 3)
#define AIS15_ERR_MSG_ID1     (1U << 4)
#define AIS15_ERR_SLOT1       (1U << 5)

#define AIS15_ERR_DEST2_MMSI  (1U << 6)
#define AIS15_ERR_MSG_ID2     (1U << 7)
#define AIS15_ERR_SLOT2       (1U << 8)

//TYPE 16 – Assigned Mode Command

#define AIS16_ERR_MSG_ID        (1U << 0)
#define AIS16_ERR_REPEAT        (1U << 1)
#define AIS16_ERR_MMSI          (1U << 2)
#define AIS16_ERR_DEST1_MMSI    (1U << 3)
#define AIS16_ERR_OFFSET1      (1U << 4)
#define AIS16_ERR_INCREMENT1   (1U << 5)
#define AIS16_ERR_DEST2_MMSI    (1U << 6)
#define AIS16_ERR_OFFSET2      (1U << 7)
#define AIS16_ERR_INCREMENT2   (1U << 8)

//TYPE 17 – GNSS Broadcast Binary
#define AIS17_ERR_MSG_ID   (1U << 0)
#define AIS17_ERR_REPEAT   (1U << 1)
#define AIS17_ERR_MMSI     (1U << 2)
#define AIS17_ERR_LON      (1U << 3)
#define AIS17_ERR_LAT      (1U << 4)
#define AIS17_ERR_BINARY   (1U << 5)

//TYPE 18 – Class-B Position Report
#define AIS18_ERR_MSG_ID   (1U << 0)
#define AIS18_ERR_REPEAT   (1U << 1)
#define AIS18_ERR_MMSI     (1U << 2)
#define AIS18_ERR_SOG      (1U << 3)
#define AIS18_ERR_LON      (1U << 4)
#define AIS18_ERR_LAT      (1U << 5)
#define AIS18_ERR_COG      (1U << 6)
#define AIS18_ERR_HEADING  (1U << 7)
#define AIS18_ERR_RAIM     (1U << 8)

//TYPE 19 – Class-B Extended
#define AIS19_ERR_SHIPNAME (1U << 9)
#define AIS19_ERR_SHIPTYPE (1U << 10)

//TYPE 20 – Data Link Management
#define AIS20_ERR_MSG_ID    (1U << 0)
#define AIS20_ERR_REPEAT    (1U << 1)
#define AIS20_ERR_MMSI      (1U << 2)
#define AIS20_ERR_ENTRY_CNT (1U << 3)
#define AIS20_ERR_OFFSET    (1U << 4)
#define AIS20_ERR_SLOTS     (1U << 5)
#define AIS20_ERR_TIMEOUT   (1U << 6)
#define AIS20_ERR_INCREMENT (1U << 7)

//TYPE 21 ERROR BITS
#define AIS21_ERR_MSG_ID        (1<<0)
#define AIS21_ERR_REPEAT        (1<<1)
#define AIS21_ERR_MMSI          (1<<2)
#define AIS21_ERR_ATON_TYPE     (1<<3)
#define AIS21_ERR_NAME          (1<<4)
#define AIS21_ERR_LON           (1<<5)
#define AIS21_ERR_LAT           (1<<6)
#define AIS21_ERR_DIMENSION     (1<<7)
#define AIS21_ERR_EPFD          (1<<8)
#define AIS21_ERR_TIMESTAMP     (1<<9)
#define AIS21_ERR_RAIM          (1<<10)
#define AIS21_ERR_SPARE         (1<<11)

//TYPE 22 ERROR BITS
#define AIS22_ERR_MSG_ID        (1<<0)
#define AIS22_ERR_REPEAT        (1<<1)
#define AIS22_ERR_MMSI          (1<<2)
#define AIS22_ERR_CHANNEL       (1<<3)
#define AIS22_ERR_TXRX_MODE     (1<<4)
#define AIS22_ERR_POWER         (1<<5)
#define AIS22_ERR_LON_LAT       (1<<6)
#define AIS22_ERR_ZONE          (1<<7)
#define AIS22_ERR_DEST_MMSI     (1<<8)
#define AIS22_ERR_SPARE         (1<<9)

//TYPE 23 ERROR BITS
#define AIS23_ERR_MSG_ID        (1<<0)
#define AIS23_ERR_REPEAT        (1<<1)
#define AIS23_ERR_MMSI          (1<<2)
#define AIS23_ERR_LON_LAT       (1<<3)
#define AIS23_ERR_STATION_TYPE  (1<<4)
#define AIS23_ERR_SHIP_TYPE     (1<<5)
#define AIS23_ERR_INTERVAL      (1<<6)
#define AIS23_ERR_QUIET         (1<<7)
#define AIS23_ERR_SPARE         (1<<8)

//TYPE 24
#define AIS24_ERR_MSG_ID     (1U << 0)
#define AIS24_ERR_REPEAT     (1U << 1)
#define AIS24_ERR_MMSI       (1U << 2)
#define AIS24_ERR_PART_NO    (1U << 3)
#define AIS24_ERR_SHIPNAME   (1U << 4)
#define AIS24_ERR_SHIPTYPE   (1U << 5)
#define AIS24_ERR_CALLSIGN   (1U << 6)
#define AIS24_ERR_DIM        (1U << 7)
#define AIS24_ERR_EPFD       (1U << 8)

//TYPE 25 ERROR BITS
#define AIS25_ERR_MSG_ID        (1<<0)
#define AIS25_ERR_REPEAT        (1<<1)
#define AIS25_ERR_MMSI          (1<<2)
#define AIS25_ERR_ADDRESS       (1<<3)
#define AIS25_ERR_DEST_MMSI     (1<<4)
#define AIS25_ERR_BINARY        (1<<5)

//TYPE 26 ERROR BITS
#define AIS26_ERR_MSG_ID        (1<<0)
#define AIS26_ERR_REPEAT        (1<<1)
#define AIS26_ERR_MMSI          (1<<2)
#define AIS26_ERR_ADDRESS       (1<<3)
#define AIS26_ERR_DEST_MMSI     (1<<4)
#define AIS26_ERR_BINARY        (1<<5)

//TYPE 27 ERROR BITS
#define AIS27_ERR_MSG_ID        (1<<0)
#define AIS27_ERR_REPEAT        (1<<1)
#define AIS27_ERR_MMSI          (1<<2)
#define AIS27_ERR_NAV_STATUS    (1<<3)
#define AIS27_ERR_LON           (1<<4)
#define AIS27_ERR_LAT           (1<<5)
#define AIS27_ERR_SOG           (1<<6)
#define AIS27_ERR_COG           (1<<7)
#define AIS27_ERR_RAIM          (1<<8)
#define AIS27_ERR_SPARE         (1<<9)

/* ================= STRUCTURES ================= */

/* AIS Type 1 / 2 / 3 : Class A position Report */
typedef struct 
{
    uint8_t  ucMsgID;             // 6 bits
    uint8_t  ucRepeat;             // 2 bits
    uint32_t uiMMSI;               // 30 bits
    uint8_t  ucNav_Status;         // 4 bits
    int8_t   cROT;                // 8 bits (signed)
    float    fSOG_kn;                // 10 bits (x0.1 kn)
    uint8_t  ucPos_Acc;            // 1 bit
    float    fLon;                // 28 bits (1/600000°)
    float    fLat;                // 27 bits (1/600000°)
    float    fCOG_deg;                // 12 bits (0.1°)
    uint16_t usHeading;            // 9 bits (degrees)
    uint8_t  ucTimestamp;          // 6 bits (UTC sec)
    uint8_t  ucManeuver;           // 2 bits
    uint8_t  ucOut_Spare;              // 3 bits
    uint8_t  ucRAIM;               // 1 bit
    uint32_t uiRadio;              // 19 bits
    // COMMUNICATION STATE:
    uint8_t  ucSync_State;         // 2 bits (bits 168-169 in the AIS payload)
    uint8_t  ucSlot_Timeout;       // 3 bits (bits 170-172)
    uint16_t us_Sub_message;        // 11 bits (bits 173-183)

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE123_FULL;

/* AIS Type 24 : Class B Static Data */

typedef struct {
    uint8_t  ucMsgID;             // 6 bits
    uint8_t  ucRepeat;             // 2 bits
    uint32_t uiMMSI;               // 30 bits
    uint8_t  ucPartNO;            // 2 bits

    // part A fields
    char arr_cShip_name[21];          // 120 bits (20 6-bit chars + NUL)
    
    // part B fields
    uint8_t  ucShip_Type;          // 8 bits
    char     arr_cVendor_id[8];       // 42 bits
    char     arr_cCallsign[8];        // 42 bits
    uint16_t usTo_bow;             // 9 bits
    uint16_t usTo_stern;           // 9 bits
    uint8_t  ucTo_Port;            // 6 bits
    uint8_t  ucTo_StarBoard;       // 6 bits
    uint8_t  ucEPFS_Type;          // 4 bits (type of electronic position fixing system) :contentReference[oaicite:1]{index=1}
    uint8_t  ucOut_Spare;              // 2 bits

    int       iValidateStatus;
    uint32_t  uiErrorMask;
} S_AIS_TYPE24_FULL;


/* ============================================================
 * AIS TYpE 4 : BASE STATION REpORT
 * ============================================================ */
typedef struct {
    uint8_t  ucMsgID;        // 6
    uint8_t  ucRepeat;        // 2
    uint32_t uiMMSI;            // 30

    uint16_t usYear;            // 14
    uint8_t  ucMonth;         // 4
    uint8_t  ucDay;           // 5
    uint8_t  ucHour;          // 5
    uint8_t  ucMinute;        // 6
    uint8_t  ucSecond;        // 6

    uint8_t  ucPos_Acc;       // 1
    float    fLon;              // 28 (signed, 1/600000)
    float    fLat;              // 27 (signed, 1/600000)

    uint8_t  ucEPFS_Type;     // 4
    uint8_t  ucRAIM;          // 1
    uint32_t uiRadio;           // 19
    int      iValidateStatus;   // AIS_OK / AIS_ERR
    uint32_t uiErrorMask;       // bitmask of field-level errors

} S_AIS_TYPE4_FULL;


/* ============================================================
 * AIS TYpE 5 : STATIC & VOYAGE DATA
 * ============================================================ */
typedef struct {
    uint8_t  ucMsgID;        // 6
    uint8_t  ucRepeat;        // 2
    uint32_t uiMMSI;            // 30

    uint8_t  ucAIS_Version;   // 2
    uint32_t uiIMO;             // 30

    char     arr_cCallsign[8];  // 42
    char     arr_cShip_name[21];// 120

    uint8_t  ucShip_Type;     // 8
    uint16_t usTo_bow;          // 9
    uint16_t usTo_stern;        // 9
    uint8_t  ucTo_Port;       // 6
    uint8_t  ucTo_StarBoard;  // 6

    uint8_t  ucEPFS_Type;     // 4
    uint8_t  uiB_Eta_month;     // 4
    uint8_t  uiB_Eta_day;       // 5
    uint8_t  uiB_Eta_hour;      // 5
    uint8_t  uiB_Eta_minute;    // 6

    float    fDraught;          // 8 (x0.1 m)
    char     arr_cDestination[21]; // 120

    uint8_t  ucDTE;           // 1
    uint8_t  ucOut_Spare;     // 1

    int       iValidateStatus;
    uint32_t  uiErrorMask;
} S_AIS_TYPE5_FULL;

//TYpE 6 – Addressed Binary Message
typedef struct {
    uint8_t  ucMsgID;        // 6
    uint8_t  ucRepeat;        // 2 bits
    uint32_t uiMMSI;            // 30 source MMSI

    uint8_t  ucSeq_Num;       // 2 sequence number
    uint32_t uiDest_MMSI;       // 30 destination MMSI
    uint8_t  ucRetransmit;    // 1 = retransmitted
    uint8_t  ucOut_Spare;     // 1

    uint16_t usDAC;             // 10 (Application ID)Designated Area Cod
    uint8_t  ucFI;              // 6 Function Identifier

    uint8_t  arr_ucBinary[920]; // variable (up to ~920 bits)
    uint16_t usBinary_Len_Bits;

    int       iValidateStatus;
    uint32_t  uiErrorMask;
} S_AIS_TYPE6_FULL;

//TYpE 7 – Binary Acknowledge
typedef struct {
    uint8_t  ucMsgID;        // 6
    uint8_t  ucRepeat;        // 2
    uint32_t uiMMSI;            // 30

    uint32_t arr_uiDest_MMSI[4];    // 4 × 30 up to 4 acknowledgements
    uint8_t  arr_ucSeq_Num[4];    // 4 × 2
    uint8_t  ucAck_Count;       // how many are valid

    int       iValidateStatus;
    uint32_t  uiErrorMask;
} S_AIS_TYPE7_FULL;

//TYpE 8 – Binary Broadcast Message
typedef struct {
    uint8_t  ucMsgID;        // 6
    uint8_t  ucRepeat;        // 2
    uint32_t uiMMSI;            // 30

    uint16_t usDAC;             // 10
    uint8_t  ucFI;              // 6

    uint8_t  arr_ucBinary[952];
    uint16_t usBinary_Len_Bits;
    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE8_FULL;

//TYpE 9 – SAR Aircraft position Report
typedef struct {
    uint8_t  ucMsgID;        // 9
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    uint16_t usAltitude;       // meters
    uint8_t  ucSOG;           // knots
    uint8_t  ucPos_Acc;

    float    fLon;              // 1/10000 minute
    float    fLat;              // 1/10000 minute

    uint16_t usCog;
    uint8_t  ucTimestamp;         // UTC second

    uint8_t  ucAltitude_Sensor; // barometric/GNSS
    uint8_t  ucOut_Spare;
    uint8_t  ucDTE;           // data terminal ready
    uint8_t  ucRAIM;
    uint32_t uiRadio;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE9_FULL;

//TYpE 10 – UTC / Date Inquiry
typedef struct {
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    uint32_t uiDest_MMSI;
    uint8_t  ucOut_Spare;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE10_FULL;

//TYpE 11 – UTC / Date Response (Base Station)
typedef struct {
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    uint16_t usYear;
    uint8_t  ucMonth;
    uint8_t  ucDay;
    uint8_t  ucHour;
    uint8_t  ucMinute;
    uint8_t  ucSecond;

    uint8_t  ucPos_Acc;
    float    fLon;
    float    fLat;

    uint8_t  ucOut_Spare;
    uint8_t  ucRAIM;
    uint32_t uiRadio;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE11_FULL;

/* TYpE 12 – Addressed Safety Related Message */
typedef struct {
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    uint16_t usSeq_Num;
    uint32_t uiDest_MMSI;
    uint8_t  ucRetransmit;
    uint8_t  ucOut_Spare;

    char     arr_cText[161];   // up to 156 chars

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE12_FULL;


/* TYpE 13 – Safety Related Acknowledgement */
typedef struct {
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    uint32_t arr_uiDest_MMSI[4];
    uint8_t  arr_ucSeq_Num[4];
    uint8_t  ucAck_Count;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE13_FULL;

/* TYpE 14 – Safety Related Broadcast */
typedef struct {
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    char     arr_cText[161];

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE14_FULL;

/* TYpE 15 – Interrogation */
typedef struct {
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    uint32_t uiDest_MMSI1;
    uint8_t  ucMsgID1;
    uint8_t  ucSlot_Offset1;

    uint32_t uiDest_MMSI2;
    uint8_t  ucMsgID2;
    uint8_t  ucSlot_Offset2;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE15_FULL;

/* TYpE 16 – Assigned Mode Command */
typedef struct {
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    uint32_t uiDest_MMSI1;
    uint16_t uiOffset1;
    uint16_t uiIncrement1;

    uint32_t uiDest_MMSI2;
    uint16_t uiOffset2;
    uint16_t uiIncrement2;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE16_FULL;

/* TYpE 17 – GNSS Broadcast Binary Message */
typedef struct {
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    float    fLon;
    float    fLat;

    uint8_t  arr_ucBinary[736];
    uint16_t usBinary_Len_Bits;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE17_FULL;

/* TYpE 18 – Class B position Report */
typedef struct {
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    float    fSOG;
    uint8_t  ucPos_Acc;
    float    fLon;
    float    fLat;
    float    fCOG;
    uint16_t usHeading;
    uint8_t  ucTimestamp;
    uint8_t  ucRAIM;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE18_FULL;

/* TYpE 19 – Class B Extended position Report */
typedef struct {
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    float    fSOG;
    uint8_t  ucPos_Acc;
    float    fLon;
    float    fLat;
    float    fCOG;
    uint16_t usHeading;

    char     arr_cShip_name[21];
    uint8_t  ucShip_Type;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE19_FULL;

/* TYpE 20 – Data Link Management */
typedef struct {
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    uint16_t arr_usOffset[4];
    uint8_t  arr_ucSlots[4];
    uint8_t  arr_ucTimeout[4];
    uint8_t  arr_ucIncrement[4];
    uint8_t  ucEntry_Count;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE20_FULL;

//TYpE 21 – Aids-to-Navigation Report
typedef struct
{
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    uint8_t  ucAtoN_Type;
    char     arr_cName[21];        // 120 bits = 20 chars + '\0'

    uint8_t  ucPos_Acc;
    float    fLon;
    float    fLat;

    uint16_t usDimBow;
    uint16_t usDimStern;
    uint8_t  ucDimport;
    uint8_t  ucDimStarboard;

    uint8_t  ucEPFD;
    uint8_t  ucTimestamp;
    uint8_t  ucOFF_Position;
    uint8_t  ucRAIM;
    uint8_t  ucVirtual_AtoN;
    uint8_t  ucAssigned;
    uint8_t  ucSpare;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE21_FULL;

//TYpE 22 – Channel Management

typedef struct
{
    uint8_t  ucMsgID;      // 6
    uint8_t  ucRepeat;      // 2
    uint32_t uiMMSI;          // 30

    uint8_t  ucChannelA;    // 12
    uint8_t  ucChannelB;    // 12

    uint8_t  ucTX_RX_Mode;    // 4
    uint8_t  ucPower;       // 1

    /* Either geographic OR addressed */
    float    fLon1;
    float    fLat1;
    float    fLon2;
    float    fLat2;

    uint32_t uiDestMMSI1;     // used if Addressed = 1
    uint32_t uiDestMMSI2;     // optional

    uint8_t  ucAddressed;   // 1
    uint8_t  ucBand;        // 1
    uint8_t  ucZoneSize;    // 3
    uint8_t  ucSpare;       // 1

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE22_FULL;


//TYpE 23 – Group Assignment Command
typedef struct
{
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    float    fLon1;
    float    fLat1;
    float    fLon2;
    float    fLat2;

    uint8_t  ucStation_Type;
    uint8_t  ucShip_Type;
    uint8_t  ucTX_RX_Mode;
    uint8_t  ucInterval;
    uint8_t  ucQuietTime;
    uint8_t  ucSpare;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE23_FULL;

//TYpE 25 – Single Slot Binary Message
typedef struct
{
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    uint8_t  ucAddressed;
    uint8_t  ucStructured;
    uint32_t uiDestMMSI;

    uint8_t  arr_ucBinaryData[32];
    uint16_t usBinaryLen;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE25_FULL;

//TYpE 26 – Multiple Slot Binary Message
typedef struct
{
    uint8_t  ucMsgID;
    uint8_t  ucRepeat;
    uint32_t uiMMSI;

    uint8_t  ucAddressed;
    uint8_t  ucStructured;
    uint32_t uiDestMMSI;

    uint8_t  arr_ucBinaryData[128];
    uint16_t usBinaryLen;

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE26_FULL;

//TYpE 27 – Long Range AIS Broadcast

typedef struct
{
    uint8_t  ucMsgID;        // 6
    uint8_t  ucRepeat;        // 2
    uint32_t uiMMSI;            // 30

    uint8_t  ucPos_Acc;       // 1
    uint8_t  ucRAIM;          // 1
    uint8_t  ucNav_Status;    // 4

    float    fLon;              // 18 bits (1/10 min)
    float    fLat;              // 17 bits (1/10 min)

    uint8_t  ucSOG;           // 6
    uint8_t  ucCOG;           // 9

    uint8_t  ucGNSS_PosStatus; // 1
    uint8_t  ucSpare;         // 1

    int       iValidateStatus;
    uint32_t  uiErrorMask;

} S_AIS_TYPE27_FULL;

/* Union for all AIS messages */
typedef union {
    S_AIS_TYPE123_FULL mSt123;
    S_AIS_TYPE24_FULL  mSt24;
    S_AIS_TYPE4_FULL   mSt4;
    S_AIS_TYPE5_FULL   mSt5;
    S_AIS_TYPE6_FULL   mSt6;
    S_AIS_TYPE7_FULL   mSt7;
    S_AIS_TYPE8_FULL   mSt8;
    S_AIS_TYPE9_FULL   mSt9;
    S_AIS_TYPE10_FULL  mSt10;
    S_AIS_TYPE11_FULL  mSt11;
    S_AIS_TYPE12_FULL mSt12;
    S_AIS_TYPE13_FULL mSt13;
    S_AIS_TYPE14_FULL mSt14;
    S_AIS_TYPE15_FULL mSt15;
    S_AIS_TYPE16_FULL mSt16;
    S_AIS_TYPE17_FULL mSt17;
    S_AIS_TYPE18_FULL mSt18;
    S_AIS_TYPE19_FULL mSt19;
    S_AIS_TYPE20_FULL mSt20;
    S_AIS_TYPE21_FULL mSt21;
    S_AIS_TYPE22_FULL mSt22;
    S_AIS_TYPE23_FULL mSt23;
    S_AIS_TYPE25_FULL mSt25;
    S_AIS_TYPE26_FULL mSt26;
    S_AIS_TYPE27_FULL mSt27;
} U_AIS_MSG_FULL;



/* ================= ApI ================= */

/* Decode full AIS NMEA sentence */
int ais_decode(const char *in_pcNmea, int *out_piMsgid, U_AIS_MSG_FULL *out_pU_Msg);

/* Debug print helpers */
void ais_print_type123(const S_AIS_TYPE123_FULL *in_pS);
//void ais_print_type24(const S_AIS_TYPE24_FULL *out_Sp);
//void ais_print_type6(const S_AIS_TYPE6_FULL *p);


#endif

