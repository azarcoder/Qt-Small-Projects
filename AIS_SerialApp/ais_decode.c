#include "ais_decode.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define AIS_MAX_FRAG_CTX 10 /* AIS seq id 0–9 */

typedef struct
{
    char arr_cPayload[512];
    int  iExpected;
    int  ireceived;
    int  iSeq;
    int  iActive;
} S_AIS_FRAG_CTX;

static S_AIS_FRAG_CTX gS_Frag_CTX[AIS_MAX_FRAG_CTX];

/* ============================================================
 * INTERNAL BIT BUFFER
 * ============================================================ */
static uint8_t arr_cBitbuf[1024];
static int iBitlen;

/* ============================================================
 * BIT UTILITIES
 * ============================================================ */
static uint32_t get_bits(int in_iStart, int in_iLen)
{
    uint32_t uiVal = 0;
    for (int i = 0; i < in_iLen; i++)
        uiVal = (uiVal << 1) | arr_cBitbuf[in_iStart + i];
    return uiVal;
}

static int32_t get_signed(int in_iStart, int in_iLen)
{
    int32_t uiVal = get_bits(in_iStart, in_iLen);
    if (uiVal & (1 << (in_iLen - 1)))
        uiVal -= (1 << in_iLen);
    return uiVal;
}

/* ============================================================
 * 6-BIT ASCII
 * ============================================================ */
static void sixbit_to_text(int in_iStart, int in_iBits, char *out_pc)
{
    int j = 0;
    for (int i = in_iStart; i < in_iStart + in_iBits; i += 6) {
        int iChar = get_bits(i, AIS_SIXBIT_LEN);

        if (iChar == 0)
            break;
        /*if (iChar < 32)
            iChar += 64;*/
        if (iChar < AIS_ASCII_PRINTABLE_MIN) //First printable ASCII (out_Space)
            iChar += AIS_ASCII_OFFSET; //Offset to convert control range to letters

        out_pc[j++] = (char)iChar;
    }
    out_pc[j] = '\0';
}

/* ============================================================
 * NMEA pAYLOAD HANDLING
 * ============================================================ */
/*static int char_to_sixbit(char in_cChar)
{
    if (in_cChar < 48 || in_cChar > 119) return -1;
    return (in_cChar <= 87) ? (in_cChar - 48) : (in_cChar - 56);
}*/
static int char_to_sixbit(char in_cChar)
{
    if (in_cChar < AIS_NMEA_CHAR_MIN || in_cChar > AIS_NMEA_CHAR_MAX)
        return -1;

    return (in_cChar <= AIS_NMEA_RANGE1_MAX)
            ? (in_cChar - AIS_NMEA_RANGE1_OFFSET)
            : (in_cChar - AIS_NMEA_RANGE2_OFFSET);
}


static void payload_to_bits(const char *out_pcPayload)
{
    iBitlen = 0;
    for (int i = 0; out_pcPayload[i]; i++) {
        int iVal = char_to_sixbit(out_pcPayload[i]);
        if (iVal < 0) continue;
        for (int iBit = 5; iBit >= 0; iBit--)
            arr_cBitbuf[iBitlen++] = (iVal >> iBit) & 1;
    }
}

static int extract_payload(const char *in_pcNMEA, char *out_pcPayload)
{
    int iComma = 0, iIdx = 0;
    for (int i = 0; in_pcNMEA[i]; i++) {
        if (in_pcNMEA[i] == ',') {
            iComma++;
            continue;
        }
        if (iComma == 5)
        {
            if (in_pcNMEA[i] == ',' || in_pcNMEA[i] == '*')
                break;
            out_pcPayload[iIdx++] = in_pcNMEA[i];
        }
        //else if (iComma > 5)
        //  break;
    }
    out_pcPayload[iIdx] = '\0';
    return (iIdx > 0) ? AIS_SUCCESS :  AIS_ERR_FORMAT;
}

/* ============================================================
 * CHECKSUM (DIAGNOSTIC ONLY)
 * ============================================================ */
static int validate_checksum(const char *in_pcNMEA)
{
    unsigned char ucCS = 0;
    const char *cp = in_pcNMEA;

    if (*cp == '!' || *cp == '$')
        cp++;

    while (*cp && *cp != '*')
        ucCS ^= (unsigned char)(*cp++);

    if (*cp != '*')
        return 0;

    unsigned int uiGiven;
    sscanf(cp + 1, "%2X", &uiGiven);
    //printf("\n CS: %02x\n",ucCS);
    return ucCS == uiGiven;
}

int validate_type123(S_AIS_TYPE123_FULL *out_Sp)
{
    out_Sp->iValidateStatus = AIS_OK;
    out_Sp->uiErrorMask     = 0;

    /* Message ID */
    if (out_Sp->ucMsgID < 1 || out_Sp->ucMsgID > 3) {
        out_Sp->uiErrorMask |= AIS_ERR_MSG_ID;
    }

    /* Repeat Indicator: 0–3 */
    if (out_Sp->ucRepeat > 3) {
        out_Sp->uiErrorMask |= AIS_ERR_REPEAT;
    }

    /* MMSI: 1–999999999 */
    if (out_Sp->uiMMSI == 0 || out_Sp->uiMMSI > 999999999) {
        out_Sp->uiErrorMask |= AIS_ERR_MMSI;
    }

    /* Navigation Status: 0–15 */
    if (out_Sp->ucNav_Status > 15) {
        out_Sp->uiErrorMask |= AIS_ERR_NAV_STATUS;
    }

    /* ROT: -128 (not available) OR -127..127 */
    if (!(out_Sp->cROT == -128 || (out_Sp->cROT >= -127 && out_Sp->cROT <= 127))) {
        out_Sp->uiErrorMask |= AIS_ERR_ROT;
    }

    /* SOG: 0–102.2 OR 102.3 = not available */
    if (!((out_Sp->fSOG_kn >= 0.0f && out_Sp->fSOG_kn <= 102.2f) ||
          (out_Sp->fSOG_kn == 102.3f))) {
        out_Sp->uiErrorMask |= AIS_ERR_SOG;
    }

    /* Position Accuracy: 0 or 1 */
    if (out_Sp->ucPos_Acc > 1) {
        out_Sp->uiErrorMask |= AIS_ERR_POS_ACC;
    }

    /* Longitude: -180..180 OR 181 = not available */
    if (!((out_Sp->fLon >= -180.0f && out_Sp->fLon <= 180.0f) ||
          (out_Sp->fLon == 181.0f))) {
        out_Sp->uiErrorMask |= AIS_ERR_LON;
    }

    /* Latitude: -90..90 OR 91 = not available */
    if (!((out_Sp->fLat >= -90.0f && out_Sp->fLat <= 90.0f) ||
          (out_Sp->fLat == 91.0f))) {
        out_Sp->uiErrorMask |= AIS_ERR_LAT;
    }

    /* COG: 0–359.9 OR 360 = not available */
    if (!((out_Sp->fCOG_deg >= 0.0f && out_Sp->fCOG_deg < 360.0f) ||
          (out_Sp->fCOG_deg == 360.0f))) {
        out_Sp->uiErrorMask |= AIS_ERR_COG;
    }

    /* True Heading: 0–359 OR 511 = not available */
    if (!((out_Sp->usHeading <= 359) || (out_Sp->usHeading == 511))) {
        out_Sp->uiErrorMask |= AIS_ERR_HEADING;
    }

    /* Timestamp: 0–59 OR 60–63 = special */
    if (out_Sp->ucTimestamp > 63) {
        out_Sp->uiErrorMask |= AIS_ERR_TIMESTAMP;
    }

    /* Maneuver Indicator: 0–2 (3 = not used) */
    if (out_Sp->ucManeuver > 3) {
        out_Sp->uiErrorMask |= AIS_ERR_MANEUVER;
    }

    /* RAIM: 0 or 1 */
    if (out_Sp->ucRAIM > 1) {
        out_Sp->uiErrorMask |= AIS_ERR_RAIM;
    }

    /* Sync State: 0–3 */
    if (out_Sp->ucSync_State > 3) {
        out_Sp->uiErrorMask |= AIS_ERR_SYNC_STATE;
    }

    /* Slot Timeout: 0–7 */
    if (out_Sp->ucSlot_Timeout > 7) {
        out_Sp->uiErrorMask |= AIS_ERR_SLOT_TIMEOUT;
    }

    /* Sub Message */
    if (out_Sp->us_Sub_message > 16383)
        out_Sp->uiErrorMask |= AIS_ERR_SUB_MESSAGE;

    /* Spare bits MUST be zero */
    if (out_Sp->ucOut_Spare != 0)
        out_Sp->uiErrorMask |= AIS_ERR_SPARE;

    if (out_Sp->uiErrorMask != 0) {
        out_Sp->iValidateStatus = AIS_ERR_INVALID_FIELD;
        return AIS_ERR_INVALID_FIELD;
    }

    return AIS_OK;
}

int validate_type4(S_AIS_TYPE4_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 4)
        out_Sp->uiErrorMask |= AIS4_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS4_ERR_REPEAT;

    if (out_Sp->uiMMSI == 0 || out_Sp->uiMMSI > 999999999)
        out_Sp->uiErrorMask |= AIS4_ERR_MMSI;

    if (!(out_Sp->usYear == 0 || (out_Sp->usYear >= 1970 && out_Sp->usYear <= 2100)))
        out_Sp->uiErrorMask |= AIS4_ERR_YEAR;

    if (out_Sp->ucMonth > 12)
        out_Sp->uiErrorMask |= AIS4_ERR_MONTH;

    if (out_Sp->ucDay > 31)
        out_Sp->uiErrorMask |= AIS4_ERR_DAY;

    if (out_Sp->ucHour > 23)
        out_Sp->uiErrorMask |= AIS4_ERR_HOUR;

    if (out_Sp->ucMinute > 59)
        out_Sp->uiErrorMask |= AIS4_ERR_MINUTE;

    if (out_Sp->ucSecond > 59)
        out_Sp->uiErrorMask |= AIS4_ERR_SECOND;

    if (out_Sp->ucPos_Acc > 1)
        out_Sp->uiErrorMask |= AIS4_ERR_POS_ACC;

    if (!((out_Sp->fLon >= -180.0f && out_Sp->fLon <= 180.0f) || out_Sp->fLon == 181.0f))
        out_Sp->uiErrorMask |= AIS4_ERR_LON;

    if (!((out_Sp->fLat >= -90.0f && out_Sp->fLat <= 90.0f) || out_Sp->fLat == 91.0f))
        out_Sp->uiErrorMask |= AIS4_ERR_LAT;

    if (out_Sp->ucEPFS_Type > 15)
        out_Sp->uiErrorMask |= AIS4_ERR_EPFS;

    if (out_Sp->ucRAIM > 1)
        out_Sp->uiErrorMask |= AIS4_ERR_RAIM;

    out_Sp->iValidateStatus =
            (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;

    return out_Sp->iValidateStatus;
}


int validate_type5(S_AIS_TYPE5_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 5)
        out_Sp->uiErrorMask |= AIS5_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS5_ERR_REPEAT;

    if (out_Sp->uiMMSI == 0 || out_Sp->uiMMSI > 999999999)
        out_Sp->uiErrorMask |= AIS5_ERR_MMSI;

    if (out_Sp->ucAIS_Version > 3)
        out_Sp->uiErrorMask |= AIS5_ERR_AIS_VERSION;

    if (!(out_Sp->uiIMO == 0 ||
          (out_Sp->uiIMO >= 1000000 && out_Sp->uiIMO <= 9999999)))
        out_Sp->uiErrorMask |= AIS5_ERR_IMO;

    if (out_Sp->arr_cCallsign[0] == '\0')
        out_Sp->uiErrorMask |= AIS5_ERR_CALLSIGN;

    if (out_Sp->arr_cShip_name[0] == '\0')
        out_Sp->uiErrorMask |= AIS5_ERR_SHIP_NAME;

    if (out_Sp->ucShip_Type > 99)
        out_Sp->uiErrorMask |= AIS5_ERR_SHIP_TYPE;

    /* Dimensions */
    if (out_Sp->usTo_bow > 511)
        out_Sp->uiErrorMask |= AIS5_ERR_DIM_BOW;

    if (out_Sp->usTo_stern > 511)
        out_Sp->uiErrorMask |= AIS5_ERR_DIM_STERN;

    if (out_Sp->ucTo_Port > 63)
        out_Sp->uiErrorMask |= AIS5_ERR_DIM_PORT;

    if (out_Sp->ucTo_StarBoard > 63)
        out_Sp->uiErrorMask |= AIS5_ERR_DIM_STARBOARD;

    if (out_Sp->ucEPFS_Type > 15)
        out_Sp->uiErrorMask |= AIS5_ERR_EPFS;

    /* ETA */
    if (out_Sp->uiB_Eta_month > 12)
        out_Sp->uiErrorMask |= AIS5_ERR_ETA_MONTH;

    if (out_Sp->uiB_Eta_day > 31)
        out_Sp->uiErrorMask |= AIS5_ERR_ETA_DAY;

    if (out_Sp->uiB_Eta_hour > 23)
        out_Sp->uiErrorMask |= AIS5_ERR_ETA_HOUR;

    if (out_Sp->uiB_Eta_minute > 59)
        out_Sp->uiErrorMask |= AIS5_ERR_ETA_MINUTE;

    if (out_Sp->fDraught < 0.0f || out_Sp->fDraught > 25.5f)
        out_Sp->uiErrorMask |= AIS5_ERR_DRAUGHT;

    if (out_Sp->arr_cDestination[0] == '\0')
        out_Sp->uiErrorMask |= AIS5_ERR_DESTINATION;

    if (out_Sp->ucDTE > 1)
        out_Sp->uiErrorMask |= AIS5_ERR_DTE;

    if (out_Sp->ucOut_Spare != 0)
        out_Sp->uiErrorMask |= AIS5_ERR_SPARE;


    out_Sp->iValidateStatus =
            (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;

    return out_Sp->iValidateStatus;
}


int validate_type6(S_AIS_TYPE6_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != TYPE6MSG)
        out_Sp->uiErrorMask |= AIS6_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS6_ERR_REPEAT;

    if (out_Sp->uiMMSI == 0 || out_Sp->uiMMSI > 999999999)
        out_Sp->uiErrorMask |= AIS6_ERR_MMSI;

    if (out_Sp->ucSeq_Num > 3)
        out_Sp->uiErrorMask |= AIS6_ERR_SEQ_NUM;

    if (out_Sp->uiDest_MMSI == 0 || out_Sp->uiDest_MMSI > 999999999)
        out_Sp->uiErrorMask |= AIS6_ERR_DEST_MMSI;

    if (out_Sp->ucRetransmit > 1)
        out_Sp->uiErrorMask |= AIS6_ERR_RETRANSMIT;

    if (out_Sp->usDAC > 1023)
        out_Sp->uiErrorMask |= AIS6_ERR_DAC;

    if (out_Sp->ucFI > 63)
        out_Sp->uiErrorMask |= AIS6_ERR_FI;

    if (out_Sp->usBinary_Len_Bits == 0 || out_Sp->usBinary_Len_Bits > 920)
        out_Sp->uiErrorMask |= AIS6_ERR_BINARY_LEN;

    out_Sp->iValidateStatus =
            (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;

    return out_Sp->iValidateStatus;
}

int validate_type7(S_AIS_TYPE7_FULL *out_Sp)
{
    int i;

    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != TYPE7MSG)
        out_Sp->uiErrorMask |= AIS7_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS7_ERR_REPEAT;

    if (out_Sp->uiMMSI == 0 || out_Sp->uiMMSI > 999999999)
        out_Sp->uiErrorMask |= AIS7_ERR_MMSI;

    if (out_Sp->ucAck_Count == 0 || out_Sp->ucAck_Count > 4)
        out_Sp->uiErrorMask |= AIS7_ERR_ACK_COUNT;

    for (i = 0; i < out_Sp->ucAck_Count; i++)
    {
        if (out_Sp->arr_uiDest_MMSI[i] == 0 || out_Sp->arr_uiDest_MMSI[i] > 999999999)
            out_Sp->uiErrorMask |= AIS7_ERR_DEST_MMSI;

        if (out_Sp->arr_ucSeq_Num[i] > 3)
            out_Sp->uiErrorMask |= AIS7_ERR_SEQ_NUM;
    }

    out_Sp->iValidateStatus =
            (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;

    return out_Sp->iValidateStatus;
}


int validate_type8(S_AIS_TYPE8_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != TYPE8MSG)
        out_Sp->uiErrorMask |= AIS8_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS8_ERR_REPEAT;

    if (out_Sp->uiMMSI == 0 || out_Sp->uiMMSI > 999999999)
        out_Sp->uiErrorMask |= AIS8_ERR_MMSI;

    if (out_Sp->usDAC > 1023)
        out_Sp->uiErrorMask |= AIS8_ERR_DAC;

    if (out_Sp->ucFI > 63)
        out_Sp->uiErrorMask |= AIS8_ERR_FI;

    if (out_Sp->usBinary_Len_Bits == 0 || out_Sp->usBinary_Len_Bits > 952)
        out_Sp->uiErrorMask |= AIS8_ERR_BINARY_LEN;

    out_Sp->iValidateStatus =
            (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;

    return out_Sp->iValidateStatus;
}

int validate_type9(S_AIS_TYPE9_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != TYPE9MSG)
        out_Sp->uiErrorMask |= AIS9_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS9_ERR_REPEAT;

    if (out_Sp->uiMMSI == 0 || out_Sp->uiMMSI > 999999999)
        out_Sp->uiErrorMask |= AIS9_ERR_MMSI;

    if (out_Sp->usAltitude > 4095)
        out_Sp->uiErrorMask |= AIS9_ERR_ALTITUDE;

    if (out_Sp->ucSOG > 102)
        out_Sp->uiErrorMask |= AIS9_ERR_SOG;

    if (out_Sp->ucPos_Acc > 1)
        out_Sp->uiErrorMask |= AIS9_ERR_POS_ACC;

    if (out_Sp->fLon < -180.0f || out_Sp->fLon > 180.0f)
        out_Sp->uiErrorMask |= AIS9_ERR_LON;

    if (out_Sp->fLat < -90.0f || out_Sp->fLat > 90.0f)
        out_Sp->uiErrorMask |= AIS9_ERR_LAT;

    if (out_Sp->usCog > 3600)
        out_Sp->uiErrorMask |= AIS9_ERR_COG;

    if (out_Sp->ucTimestamp > 59)
        out_Sp->uiErrorMask |= AIS9_ERR_TIMESTAMP;

    if (out_Sp->ucDTE > 1)
        out_Sp->uiErrorMask |= AIS9_ERR_DTE;

    if (out_Sp->ucRAIM > 1)
        out_Sp->uiErrorMask |= AIS9_ERR_RAIM;

    out_Sp->iValidateStatus =
            (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;

    return out_Sp->iValidateStatus;
}


int validate_type10(S_AIS_TYPE10_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != TYPE10MSG)
        out_Sp->uiErrorMask |= AIS10_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS10_ERR_REPEAT;

    if (out_Sp->uiMMSI == 0 || out_Sp->uiMMSI > 999999999)
        out_Sp->uiErrorMask |= AIS10_ERR_MMSI;

    if (out_Sp->uiDest_MMSI == 0 || out_Sp->uiDest_MMSI > 999999999)
        out_Sp->uiErrorMask |= AIS10_ERR_DEST_MMSI;

    if (out_Sp->ucOut_Spare != 0)
        out_Sp->uiErrorMask |= AIS10_ERR_SPARE;

    out_Sp->iValidateStatus =
            (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;

    return out_Sp->iValidateStatus;
}


void validate_type11(S_AIS_TYPE11_FULL *out_Sp)
{
    out_Sp->iValidateStatus = AIS_OK;
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 11) out_Sp->uiErrorMask |= AIS11_ERR_MSG_ID;
    if (out_Sp->ucRepeat > 3)   out_Sp->uiErrorMask |= AIS11_ERR_REPEAT;
    if (out_Sp->uiMMSI == 0)      out_Sp->uiErrorMask |= AIS11_ERR_MMSI;

    if (out_Sp->usYear > 4095)    out_Sp->uiErrorMask |= AIS11_ERR_YEAR;
    if (out_Sp->ucMonth > 12)   out_Sp->uiErrorMask |= AIS11_ERR_MONTH;
    if (out_Sp->ucDay > 31)     out_Sp->uiErrorMask |= AIS11_ERR_DAY;
    if (out_Sp->ucHour > 23)    out_Sp->uiErrorMask |= AIS11_ERR_HOUR;
    if (out_Sp->ucMinute > 59)  out_Sp->uiErrorMask |= AIS11_ERR_MINUTE;
    if (out_Sp->ucSecond > 59)  out_Sp->uiErrorMask |= AIS11_ERR_SECOND;

    if (out_Sp->fLon < -180.0 || out_Sp->fLon > 180.0) out_Sp->uiErrorMask |= AIS11_ERR_LON;
    if (out_Sp->fLat < -90.0  || out_Sp->fLat > 90.0)  out_Sp->uiErrorMask |= AIS11_ERR_LAT;

    if (out_Sp->ucRAIM > 1) out_Sp->uiErrorMask |= AIS11_ERR_RAIM;

    if (out_Sp->uiErrorMask)
        out_Sp->iValidateStatus = AIS_ERR_INVALID_FIELD;
}

void validate_type12(S_AIS_TYPE12_FULL *out_Sp)
{
    out_Sp->iValidateStatus = AIS_OK;
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 12) out_Sp->uiErrorMask |= AIS12_ERR_MSG_ID;
    if (out_Sp->ucRepeat > 3)   out_Sp->uiErrorMask |= AIS12_ERR_REPEAT;
    if (out_Sp->uiMMSI == 0)      out_Sp->uiErrorMask |= AIS12_ERR_MMSI;

    if (out_Sp->usSeq_Num > 3)  out_Sp->uiErrorMask |= AIS12_ERR_SEQ_NUM;
    if (out_Sp->uiDest_MMSI == 0) out_Sp->uiErrorMask |= AIS12_ERR_DEST_MMSI;

    if (strlen(out_Sp->arr_cText) > 156)
        out_Sp->uiErrorMask |= AIS12_ERR_TEXT_LEN;

    if (out_Sp->uiErrorMask)
        out_Sp->iValidateStatus = AIS_ERR_INVALID_FIELD;
}

void validate_type13(S_AIS_TYPE13_FULL *out_Sp)
{
    int i;

    out_Sp->iValidateStatus = AIS_OK;
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 13) out_Sp->uiErrorMask |= AIS13_ERR_MSG_ID;
    if (out_Sp->ucAck_Count > 4)  out_Sp->uiErrorMask |= AIS13_ERR_ACK_COUNT;

    for (i = 0; i < out_Sp->ucAck_Count; i++) {
        if (out_Sp->arr_uiDest_MMSI[i] == 0)
            out_Sp->uiErrorMask |= AIS13_ERR_DEST_MMSI;
        if (out_Sp->arr_ucSeq_Num[i] > 3)
            out_Sp->uiErrorMask |= AIS13_ERR_SEQ_NUM;
    }

    if (out_Sp->uiErrorMask)
        out_Sp->iValidateStatus = AIS_ERR_INVALID_FIELD;
}

void validate_type14(S_AIS_TYPE14_FULL *out_Sp)
{
    out_Sp->iValidateStatus = AIS_OK;
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 14) out_Sp->uiErrorMask |= AIS14_ERR_MSG_ID;
    if (strlen(out_Sp->arr_cText) > 156)
        out_Sp->uiErrorMask |= AIS14_ERR_TEXT_LEN;

    if (out_Sp->uiErrorMask)
        out_Sp->iValidateStatus = AIS_ERR_INVALID_FIELD;
}

void validate_type15(S_AIS_TYPE15_FULL *out_Sp)
{
    out_Sp->iValidateStatus = AIS_OK;
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 15) out_Sp->uiErrorMask |= AIS15_ERR_MSG_ID;

    if (out_Sp->uiDest_MMSI1 && out_Sp->ucMsgID1 > 27)
        out_Sp->uiErrorMask |= AIS15_ERR_MSG_ID1;

    if (out_Sp->uiDest_MMSI2 && out_Sp->ucMsgID2 > 27)
        out_Sp->uiErrorMask |= AIS15_ERR_MSG_ID2;

    if (out_Sp->uiErrorMask)
        out_Sp->iValidateStatus = AIS_ERR_INVALID_FIELD;
}

void validate_type16(S_AIS_TYPE16_FULL *out_Sp)
{
    out_Sp->iValidateStatus = AIS_OK;
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != TYPE16MSG)
        out_Sp->uiErrorMask |= AIS16_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS16_ERR_REPEAT;

    if (out_Sp->uiMMSI < 100000000 || out_Sp->uiMMSI > 999999999)
        out_Sp->uiErrorMask |= AIS16_ERR_MMSI;

    if (out_Sp->uiDest_MMSI1 &&
            (out_Sp->uiDest_MMSI1 < 100000000 || out_Sp->uiDest_MMSI1 > 999999999))
        out_Sp->uiErrorMask |= AIS16_ERR_DEST1_MMSI;

    if (out_Sp->uiOffset1 > 4095)
        out_Sp->uiErrorMask |= AIS16_ERR_OFFSET1;

    if (out_Sp->uiIncrement1 > 2047)
        out_Sp->uiErrorMask |= AIS16_ERR_INCREMENT1;

    if (out_Sp->uiDest_MMSI2 &&
            (out_Sp->uiDest_MMSI2 < 100000000 || out_Sp->uiDest_MMSI2 > 999999999))
        out_Sp->uiErrorMask |= AIS16_ERR_DEST2_MMSI;

    if (out_Sp->uiOffset2 > 4095)
        out_Sp->uiErrorMask |= AIS16_ERR_OFFSET2;

    if (out_Sp->uiIncrement2 > 2047)
        out_Sp->uiErrorMask |= AIS16_ERR_INCREMENT2;

    if (out_Sp->uiErrorMask)
        out_Sp->iValidateStatus = AIS_ERR_INVALID_FIELD;
}

void validate_type17(S_AIS_TYPE17_FULL *out_Sp)
{
    out_Sp->iValidateStatus = AIS_OK;
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != TYPE17MSG)
        out_Sp->uiErrorMask |= AIS17_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS17_ERR_REPEAT;

    if (out_Sp->uiMMSI < 100000000 || out_Sp->uiMMSI > 999999999)
        out_Sp->uiErrorMask |= AIS17_ERR_MMSI;

    if (out_Sp->fLon < -180.0 || out_Sp->fLon > 180.0)
        out_Sp->uiErrorMask |= AIS17_ERR_LON;

    if (out_Sp->fLat < -90.0 || out_Sp->fLat > 90.0)
        out_Sp->uiErrorMask |= AIS17_ERR_LAT;

    if (out_Sp->usBinary_Len_Bits == 0)
        out_Sp->uiErrorMask |= AIS17_ERR_BINARY;

    if (out_Sp->uiErrorMask)
        out_Sp->iValidateStatus = AIS_ERR_INVALID_FIELD;
}

void validate_type18(S_AIS_TYPE18_FULL *out_Sp)
{
    out_Sp->iValidateStatus = AIS_OK;
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != TYPE18MSG)
        out_Sp->uiErrorMask |= AIS18_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS18_ERR_REPEAT;

    if (out_Sp->uiMMSI < 100000000 || out_Sp->uiMMSI > 999999999)
        out_Sp->uiErrorMask |= AIS18_ERR_MMSI;

    if (out_Sp->fSOG < 0 || out_Sp->fSOG > 102.2)
        out_Sp->uiErrorMask |= AIS18_ERR_SOG;

    if (out_Sp->fLon < -180.0 || out_Sp->fLon > 180.0)
        out_Sp->uiErrorMask |= AIS18_ERR_LON;

    if (out_Sp->fLat < -90.0 || out_Sp->fLat > 90.0)
        out_Sp->uiErrorMask |= AIS18_ERR_LAT;

    if (out_Sp->fCOG < 0 || out_Sp->fCOG >= 360.0)
        out_Sp->uiErrorMask |= AIS18_ERR_COG;

    if (out_Sp->usHeading > 359)
        out_Sp->uiErrorMask |= AIS18_ERR_HEADING;

    if (out_Sp->uiErrorMask)
        out_Sp->iValidateStatus = AIS_ERR_INVALID_FIELD;
}

void validate_type19(S_AIS_TYPE19_FULL *out_Sp)
{
    out_Sp->iValidateStatus = AIS_OK;
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != TYPE19MSG)
        out_Sp->uiErrorMask |= AIS18_ERR_MSG_ID;

    if (out_Sp->arr_cShip_name[0] == '\0')
        out_Sp->uiErrorMask |= AIS19_ERR_SHIPNAME;

    if (out_Sp->ucShip_Type > 99)
        out_Sp->uiErrorMask |= AIS19_ERR_SHIPTYPE;

    if (out_Sp->uiErrorMask)
        out_Sp->iValidateStatus = AIS_ERR_INVALID_FIELD;
}

void validate_type20(S_AIS_TYPE20_FULL *out_Sp)
{
    int i;

    out_Sp->iValidateStatus = AIS_OK;
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != TYPE20MSG)
        out_Sp->uiErrorMask |= AIS20_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS20_ERR_REPEAT;

    if (out_Sp->uiMMSI < 100000000 || out_Sp->uiMMSI > 999999999)
        out_Sp->uiErrorMask |= AIS20_ERR_MMSI;

    if (out_Sp->ucEntry_Count == 0 || out_Sp->ucEntry_Count > 4)
        out_Sp->uiErrorMask |= AIS20_ERR_ENTRY_CNT;

    for (i = 0; i < out_Sp->ucEntry_Count; i++)
    {
        if (out_Sp->arr_usOffset[i] > 4095)
            out_Sp->uiErrorMask |= AIS20_ERR_OFFSET;

        if (out_Sp->arr_ucSlots[i] == 0)
            out_Sp->uiErrorMask |= AIS20_ERR_SLOTS;

        if (out_Sp->arr_ucTimeout[i] > 7)
            out_Sp->uiErrorMask |= AIS20_ERR_TIMEOUT;

        if (out_Sp->arr_ucIncrement[i] > 8191)
            out_Sp->uiErrorMask |= AIS20_ERR_INCREMENT;
    }

    if (out_Sp->uiErrorMask)
        out_Sp->iValidateStatus = AIS_ERR_INVALID_FIELD;
}

int validate_type21(S_AIS_TYPE21_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 21)
        out_Sp->uiErrorMask |= AIS21_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS21_ERR_REPEAT;

    if (out_Sp->uiMMSI < 100000000)
        out_Sp->uiErrorMask |= AIS21_ERR_MMSI;

    if (out_Sp->ucAtoN_Type > 31)
        out_Sp->uiErrorMask |= AIS21_ERR_ATON_TYPE;

    if (out_Sp->fLon < -180.0 || out_Sp->fLon > 180.0)
        out_Sp->uiErrorMask |= AIS21_ERR_LON;

    if (out_Sp->fLat < -90.0 || out_Sp->fLat > 90.0)
        out_Sp->uiErrorMask |= AIS21_ERR_LAT;

    if (out_Sp->ucEPFD > 15)
        out_Sp->uiErrorMask |= AIS21_ERR_EPFD;

    if (out_Sp->ucSpare != 0)
        out_Sp->uiErrorMask |= AIS21_ERR_SPARE;

    out_Sp->iValidateStatus = (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;
    return out_Sp->iValidateStatus;
}

int validate_type22(S_AIS_TYPE22_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 22)
        out_Sp->uiErrorMask |= AIS22_ERR_MSG_ID;

    if (out_Sp->ucRepeat > 3)
        out_Sp->uiErrorMask |= AIS22_ERR_REPEAT;

    if (out_Sp->uiMMSI < 100000000)
        out_Sp->uiErrorMask |= AIS22_ERR_MMSI;

    if (out_Sp->ucChannelA > 2087 || out_Sp->ucChannelB > 2087)
        out_Sp->uiErrorMask |= AIS22_ERR_CHANNEL;

    if (out_Sp->ucAddressed)
    {
        if (out_Sp->uiDestMMSI1 < 100000000)
            out_Sp->uiErrorMask |= AIS22_ERR_DEST_MMSI;
    }
    else
    {
        if (out_Sp->fLon1 < -180 || out_Sp->fLon2 > 180 ||
                out_Sp->fLat1 < -90  || out_Sp->fLat2 > 90)
            out_Sp->uiErrorMask |= AIS22_ERR_LON_LAT;
    }

    if (out_Sp->ucSpare != 0)
        out_Sp->uiErrorMask |= AIS22_ERR_SPARE;

    out_Sp->iValidateStatus = (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;
    return out_Sp->iValidateStatus;
}

int validate_type23(S_AIS_TYPE23_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 23)
        out_Sp->uiErrorMask |= AIS23_ERR_MSG_ID;

    if (out_Sp->fLon1 < -180 || out_Sp->fLon2 > 180 ||
            out_Sp->fLat1 < -90  || out_Sp->fLat2 > 90)
        out_Sp->uiErrorMask |= AIS23_ERR_LON_LAT;

    if (out_Sp->ucInterval > 15)
        out_Sp->uiErrorMask |= AIS23_ERR_INTERVAL;

    if (out_Sp->ucSpare != 0)
        out_Sp->uiErrorMask |= AIS23_ERR_SPARE;

    out_Sp->iValidateStatus = (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;
    return out_Sp->iValidateStatus;
}

int validate_type24(S_AIS_TYPE24_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 24)
        out_Sp->uiErrorMask |= AIS24_ERR_MSG_ID;

    if (out_Sp->ucPartNO > 1)
        out_Sp->uiErrorMask |= AIS24_ERR_PART_NO;

    if (out_Sp->ucPartNO == 0 && out_Sp->arr_cShip_name[0] == '\0')
        out_Sp->uiErrorMask |= AIS24_ERR_SHIPNAME;

    if (out_Sp->ucPartNO == 1)
    {
        if (out_Sp->ucShip_Type > 99)
            out_Sp->uiErrorMask |= AIS24_ERR_SHIPTYPE;
        if (out_Sp->arr_cCallsign[0] == '\0')
            out_Sp->uiErrorMask |= AIS24_ERR_CALLSIGN;
    }

    out_Sp->iValidateStatus = (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;
    return out_Sp->iValidateStatus;
}

int validate_type25(S_AIS_TYPE25_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 25)
        out_Sp->uiErrorMask |= AIS25_ERR_MSG_ID;

    if (out_Sp->ucAddressed && out_Sp->uiDestMMSI < 100000000)
        out_Sp->uiErrorMask |= AIS25_ERR_DEST_MMSI;

    if (out_Sp->usBinaryLen == 0)
        out_Sp->uiErrorMask |= AIS25_ERR_BINARY;

    out_Sp->iValidateStatus = (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;
    return out_Sp->iValidateStatus;
}

int validate_type26(S_AIS_TYPE26_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 26)
        out_Sp->uiErrorMask |= AIS26_ERR_MSG_ID;

    if (out_Sp->ucAddressed && out_Sp->uiDestMMSI < 100000000)
        out_Sp->uiErrorMask |= AIS26_ERR_DEST_MMSI;

    if (out_Sp->usBinaryLen == 0)
        out_Sp->uiErrorMask |= AIS26_ERR_BINARY;

    out_Sp->iValidateStatus = (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;
    return out_Sp->iValidateStatus;
}

int validate_type27(S_AIS_TYPE27_FULL *out_Sp)
{
    out_Sp->uiErrorMask = 0;

    if (out_Sp->ucMsgID != 27)
        out_Sp->uiErrorMask |= AIS27_ERR_MSG_ID;

    if (out_Sp->ucNav_Status > 15)
        out_Sp->uiErrorMask |= AIS27_ERR_NAV_STATUS;

    if (out_Sp->fLon < -180 || out_Sp->fLon > 180)
        out_Sp->uiErrorMask |= AIS27_ERR_LON;

    if (out_Sp->fLat < -90 || out_Sp->fLat > 90)
        out_Sp->uiErrorMask |= AIS27_ERR_LAT;

    if (out_Sp->ucSpare != 0)
        out_Sp->uiErrorMask |= AIS27_ERR_SPARE;

    out_Sp->iValidateStatus = (out_Sp->uiErrorMask == 0) ? AIS_OK : AIS_ERR_INVALID_FIELD;
    return out_Sp->iValidateStatus;
}



/* ============================================================
 * TYpE 1 / 2 / 3 DECODER (168 BITS)
 * ============================================================ */
static void decode_type123(S_AIS_TYPE123_FULL *out_Sp)
{
    out_Sp->ucMsgID      = get_bits(0, 6);
    out_Sp->ucRepeat      = get_bits(6, 2);
    out_Sp->uiMMSI          = get_bits(8, 30);
    out_Sp->ucNav_Status  = get_bits(38, 4);
    out_Sp->cROT          = get_signed(42, 8);
    out_Sp->fSOG_kn         = get_bits(50, 10) / 10.0f;
    out_Sp->ucPos_Acc     = get_bits(60, 1);
    out_Sp->fLon            = get_signed(61, 28) / 600000.0f;
    out_Sp->fLat            = get_signed(89, 27) / 600000.0f;
    out_Sp->fCOG_deg        = get_bits(116, 12) / 10.0f;
    out_Sp->usHeading       = get_bits(128, 9);
    out_Sp->ucTimestamp   = get_bits(137, 6);
    out_Sp->ucManeuver    = get_bits(143, 2);
    out_Sp->ucOut_Spare   = get_bits(145, 3);
    out_Sp->ucRAIM        = get_bits(148, 1);
    out_Sp->uiRadio         = get_bits(149, 19);

    out_Sp->ucSync_State   = (out_Sp->uiRadio >> 17) & 0x03;
    out_Sp->ucSlot_Timeout = (out_Sp->uiRadio >> 14) & 0x07;
    out_Sp->us_Sub_message   = out_Sp->uiRadio & 0x3FFF;
}

static void decode_type4(S_AIS_TYPE4_FULL *out_Sp)
{
    out_Sp->ucMsgID   = get_bits(0, 6);
    out_Sp->ucRepeat   = get_bits(6, 2);
    out_Sp->uiMMSI       = get_bits(8, 30);

    out_Sp->usYear       = get_bits(38, 14);
    out_Sp->ucMonth    = get_bits(52, 4);
    out_Sp->ucDay      = get_bits(56, 5);
    out_Sp->ucHour     = get_bits(61, 5);
    out_Sp->ucMinute  = get_bits(66, 6);
    out_Sp->ucSecond  = get_bits(72, 6);

    out_Sp->ucPos_Acc = get_bits(78, 1);
    out_Sp->fLon        = get_signed(79, 28) / 600000.0f;
    out_Sp->fLat        = get_signed(107, 27) / 600000.0f;

    out_Sp->ucEPFS_Type = get_bits(134, 4);
    out_Sp->ucRAIM      = get_bits(148, 1);
    out_Sp->uiRadio       = get_bits(149, 19);
}

static void decode_type5(S_AIS_TYPE5_FULL *out_Sp)
{
    out_Sp->ucMsgID      = get_bits(0, 6);
    out_Sp->ucRepeat      = get_bits(6, 2);
    out_Sp->uiMMSI          = get_bits(8, 30);

    out_Sp->ucAIS_Version = get_bits(38, 2);
    out_Sp->uiIMO           = get_bits(40, 30);

    sixbit_to_text(70, 42,  out_Sp->arr_cCallsign);
    sixbit_to_text(112,120, out_Sp->arr_cShip_name);

    out_Sp->ucShip_Type   = get_bits(232, 8);
    out_Sp->usTo_bow        = get_bits(240, 9);
    out_Sp->usTo_stern      = get_bits(249, 9);
    out_Sp->ucTo_Port     = get_bits(258, 6);
    out_Sp->ucTo_StarBoard= get_bits(264, 6);

    out_Sp->ucEPFS_Type   = get_bits(270, 4);
    out_Sp->uiB_Eta_month   = get_bits(274, 4);
    out_Sp->uiB_Eta_day     = get_bits(278, 5);
    out_Sp->uiB_Eta_hour    = get_bits(283, 5);
    out_Sp->uiB_Eta_minute  = get_bits(288, 6);

    out_Sp->fDraught        = get_bits(294, 8) / 10.0f;
    sixbit_to_text(302,120, out_Sp->arr_cDestination);

    out_Sp->ucDTE         = get_bits(422, 1);
    out_Sp->ucOut_Spare   = get_bits(423, 1);
}

static void decode_type6(S_AIS_TYPE6_FULL *out_Sp)
{
    int iBitpos = 0;

    memset(out_Sp, 0, sizeof(*out_Sp));

    out_Sp->ucMsgID   = get_bits(iBitpos, 6);   iBitpos += 6;
    out_Sp->ucRepeat   = get_bits(iBitpos, 2);   iBitpos += 2;
    out_Sp->uiMMSI       = get_bits(iBitpos, 30);  iBitpos += 30;

    out_Sp->ucSeq_Num  = get_bits(iBitpos, 2);   iBitpos += 2;
    out_Sp->uiDest_MMSI  = get_bits(iBitpos, 30);  iBitpos += 30;
    out_Sp->ucRetransmit = get_bits(iBitpos, 1); iBitpos += 1;
    out_Sp->ucOut_Spare  = get_bits(iBitpos, 1); iBitpos += 1;

    out_Sp->usDAC = get_bits(iBitpos, 10); iBitpos += 10;
    out_Sp->ucFI  = get_bits(iBitpos, 6);  iBitpos += 6;

    /* Remaining bits = binary payload */
    int iRemaining_Bits = iBitlen - iBitpos;
    out_Sp->usBinary_Len_Bits = iRemaining_Bits;

    int iByte_Len = (iRemaining_Bits + 7) / 8;

    for (int i = 0; i < iByte_Len; i++) {
        out_Sp->arr_ucBinary[i] = get_bits(iBitpos + (i * 8), 8);
    }
}

static void decode_type7(S_AIS_TYPE7_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);

    int iPos = 38;
    out_Sp->ucAck_Count = 0;

    while (iPos + 32 <= iBitlen && out_Sp->ucAck_Count < 4) {
        out_Sp->arr_uiDest_MMSI[out_Sp->ucAck_Count] = get_bits(iPos,30);
        out_Sp->arr_ucSeq_Num[out_Sp->ucAck_Count] = get_bits(iPos+30,2);
        out_Sp->ucAck_Count++;
        iPos += 32;
    }
}

static void decode_type8(S_AIS_TYPE8_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);

    out_Sp->usDAC = get_bits(40,10);
    out_Sp->ucFI  = get_bits(50,6);

    out_Sp->usBinary_Len_Bits = iBitlen - 56;
}

static void decode_type9(S_AIS_TYPE9_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);

    out_Sp->usAltitude = get_bits(38,12);
    out_Sp->ucSOG    = get_bits(50,10);
    out_Sp->ucPos_Acc= get_bits(60,1);

    out_Sp->fLon = get_signed(61,28) / 600000.0f;
    out_Sp->fLat = get_signed(89,27) / 600000.0f;

    out_Sp->usCog = get_bits(116,12);
    out_Sp->ucTimestamp = get_bits(128,6);
    out_Sp->ucAltitude_Sensor = get_bits(134,1);
    out_Sp->ucOut_Spare = get_bits(135,7);
    out_Sp->ucDTE = get_bits(142,1);
    out_Sp->ucRAIM = get_bits(148,1);
    out_Sp->uiRadio = get_bits(149,19);
}

static void decode_type10(S_AIS_TYPE10_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);
    out_Sp->uiDest_MMSI = get_bits(40,30);
    out_Sp->ucOut_Spare = get_bits(70,2);
}

static void decode_type11(S_AIS_TYPE11_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);

    out_Sp->usYear  = get_bits(38,14);
    out_Sp->ucMonth = get_bits(52,4);
    out_Sp->ucDay   = get_bits(56,5);
    out_Sp->ucHour  = get_bits(61,5);
    out_Sp->ucMinute= get_bits(66,6);
    out_Sp->ucSecond= get_bits(72,6);

    out_Sp->ucPos_Acc = get_bits(78,1);
    out_Sp->fLon = get_signed(79,28) / 600000.0f;
    out_Sp->fLat = get_signed(107,27) / 600000.0f;

    out_Sp->ucOut_Spare = get_bits(134,4);
    out_Sp->ucRAIM = get_bits(148,1);
    out_Sp->uiRadio  = get_bits(149,19);
}

static void decode_type12(S_AIS_TYPE12_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);
    out_Sp->usSeq_Num= get_bits(38,2);
    out_Sp->uiDest_MMSI= get_bits(40,30);
    out_Sp->ucRetransmit = get_bits(70,1);
    sixbit_to_text(72, iBitlen-72, out_Sp->arr_cText);
}

static void decode_type13(S_AIS_TYPE13_FULL *out_Sp)
{
    int iPos = 38;
    out_Sp->ucAck_Count = 0;
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);

    while (iPos + 32 <= iBitlen && out_Sp->ucAck_Count < 4) {
        out_Sp->arr_uiDest_MMSI[out_Sp->ucAck_Count] = get_bits(iPos,30);
        out_Sp->arr_ucSeq_Num[out_Sp->ucAck_Count] = get_bits(iPos+30,2);
        out_Sp->ucAck_Count++;
        iPos += 32;
    }
}

static void decode_type14(S_AIS_TYPE14_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);
    sixbit_to_text(40, iBitlen-40, out_Sp->arr_cText);
}

static void decode_type15(S_AIS_TYPE15_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);
    out_Sp->uiDest_MMSI1 = get_bits(40,30);
    out_Sp->ucMsgID1  = get_bits(70,6);
    out_Sp->ucSlot_Offset1 = get_bits(76,12);
}

static void decode_type16(S_AIS_TYPE16_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);
    out_Sp->uiDest_MMSI1 = get_bits(40,30);
    out_Sp->uiOffset1 = get_bits(70,12);
    out_Sp->uiIncrement1 = get_bits(82,10);
}

static void decode_type17(S_AIS_TYPE17_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);
    out_Sp->fLon = get_signed(40,18)/600.0f;
    out_Sp->fLat = get_signed(58,17)/600.0f;
    out_Sp->usBinary_Len_Bits = iBitlen - 80;
}

static void decode_type18(S_AIS_TYPE18_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);
    out_Sp->fSOG = get_bits(46,10)/10.0f;
    out_Sp->fLon = get_signed(57,28)/600000.0f;
    out_Sp->fLat = get_signed(85,27)/600000.0f;
}

static void decode_type19(S_AIS_TYPE19_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);
    sixbit_to_text(143,120,out_Sp->arr_cShip_name);
    out_Sp->ucShip_Type = get_bits(263,8);
}

static void decode_type20(S_AIS_TYPE20_FULL *out_Sp)
{
    int iPos = 38;
    out_Sp->ucEntry_Count = 0;
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);

    while (iPos + 30 <= iBitlen && out_Sp->ucEntry_Count < 4) {
        int i = out_Sp->ucEntry_Count;
        out_Sp->arr_usOffset[i] = get_bits(iPos,12);
        out_Sp->arr_ucSlots[i]  = get_bits(iPos+12,4);
        out_Sp->arr_ucTimeout[i]= get_bits(iPos+16,3);
        out_Sp->arr_ucIncrement[i]=get_bits(iPos+19,11);
        iPos += 30;
        out_Sp->ucEntry_Count++;
    }
}

void decode_type21(S_AIS_TYPE21_FULL *out_Sp)
{
    out_Sp->ucMsgID   = get_bits(0,6);
    out_Sp->ucRepeat   = get_bits(6,2);
    out_Sp->uiMMSI       = get_bits(8,30);
    out_Sp->ucAtoN_Type= get_bits(38,5);

    sixbit_to_text(43,120,out_Sp->arr_cName);

    out_Sp->ucPos_Acc  = get_bits(163,1);
    out_Sp->fLon         = get_bits(164,28)/600000.0f;
    out_Sp->fLat         = get_bits(192,27)/600000.0f;

    out_Sp->usDimBow     = get_bits(219,9);
    out_Sp->usDimStern   = get_bits(228,9);
    out_Sp->ucDimport    = get_bits(237,6);
    out_Sp->ucDimStarboard = get_bits(243,6);

    out_Sp->ucEPFD     = get_bits(249,4);
    out_Sp->ucTimestamp= get_bits(253,6);
    out_Sp->ucOFF_Position = get_bits(259,1);
    out_Sp->ucRAIM     = get_bits(268,1);
    out_Sp->ucVirtual_AtoN = get_bits(269,1);
    out_Sp->ucAssigned = get_bits(270,1);
}

void decode_type22(S_AIS_TYPE22_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);

    out_Sp->ucChannelA = get_bits(40,12);
    out_Sp->ucChannelB = get_bits(52,12);

    out_Sp->ucTX_RX_Mode = get_bits(64,4);
    out_Sp->ucPower    = get_bits(68,1);

    out_Sp->fLon1 = get_bits(69,18)/600.0f;
    out_Sp->fLat1 = get_bits(87,17)/600.0f;
    out_Sp->fLon2 = get_bits(104,18)/600.0f;
    out_Sp->fLat2 = get_bits(122,17)/600.0f;

    out_Sp->ucAddressed = get_bits(139,1);
    out_Sp->ucBand      = get_bits(140,1);
    out_Sp->ucZoneSize  = get_bits(141,3);
}

void decode_type23(S_AIS_TYPE23_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);

    out_Sp->fLon1 = get_bits(40,18)/600.0f;
    out_Sp->fLat1 = get_bits(58,17)/600.0f;
    out_Sp->fLon2 = get_bits(75,18)/600.0f;
    out_Sp->fLat2 = get_bits(93,17)/600.0f;

    out_Sp->ucStation_Type = get_bits(110,4);
    out_Sp->ucShip_Type    = get_bits(114,8);
    out_Sp->ucTX_RX_Mode     = get_bits(122,2);
    out_Sp->ucInterval     = get_bits(124,4);
    out_Sp->ucQuietTime    = get_bits(128,4);
}

/* ============================================================
 * TYpE 24 DECODER
 * ============================================================ */
static void decode_type24(S_AIS_TYPE24_FULL *out_Sp)
{
    out_Sp->ucMsgID   = get_bits(0, 6);
    out_Sp->ucRepeat   = get_bits(6, 2);
    out_Sp->uiMMSI       = get_bits(8, 30);
    out_Sp->ucPartNO  = get_bits(38, 2);

    if (out_Sp->ucPartNO == 0) {
        sixbit_to_text(40, 120, out_Sp->arr_cShip_name);
    } else {
        out_Sp->ucShip_Type = get_bits(40, 8);
        sixbit_to_text(48, 42, out_Sp->arr_cVendor_id);
        sixbit_to_text(90, 42, out_Sp->arr_cCallsign);
        out_Sp->usTo_bow       = get_bits(132, 9);
        out_Sp->usTo_stern     = get_bits(141, 9);
        out_Sp->ucTo_Port    = get_bits(150, 6);
        out_Sp->ucTo_StarBoard = get_bits(156, 6);
        out_Sp->ucEPFS_Type  = get_bits(162, 4);
        out_Sp->ucOut_Spare      = get_bits(166, 2);
    }
}

void decode_type25(S_AIS_TYPE25_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);

    out_Sp->ucAddressed = get_bits(38,1);
    out_Sp->ucStructured= get_bits(39,1);

    if (out_Sp->ucAddressed)
        out_Sp->uiDestMMSI = get_bits(40,30);

    out_Sp->usBinaryLen = (iBitlen - 70)/8;
    //memcpy(out_Sp->arr_ucBinaryData, &arr_cBitbuf[70], out_Sp->usBinaryLen);
    for (int i = 0; i < out_Sp->usBinaryLen; i++)
    {
        out_Sp->arr_ucBinaryData[i] = get_bits(70 + i * 8, 8);
    }

}

void decode_type26(S_AIS_TYPE26_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);

    out_Sp->ucAddressed = get_bits(38,1);
    out_Sp->ucStructured= get_bits(39,1);

    if (out_Sp->ucAddressed)
        out_Sp->uiDestMMSI = get_bits(40,30);

    out_Sp->usBinaryLen = (iBitlen - 70)/8;
    //memcpy(out_Sp->arr_ucBinaryData, &arr_cBitbuf[70], out_Sp->usBinaryLen);
    for (int i = 0; i < out_Sp->usBinaryLen; i++)
    {
        out_Sp->arr_ucBinaryData[i] = get_bits(70 + i * 8, 8);
    }

}

void decode_type27(S_AIS_TYPE27_FULL *out_Sp)
{
    out_Sp->ucMsgID = get_bits(0,6);
    out_Sp->ucRepeat = get_bits(6,2);
    out_Sp->uiMMSI     = get_bits(8,30);

    out_Sp->ucPos_Acc = get_bits(38,1);
    out_Sp->ucRAIM    = get_bits(39,1);
    out_Sp->ucNav_Status = get_bits(40,4);

    out_Sp->fLon = get_bits(44,18)/600.0f;
    out_Sp->fLat = get_bits(62,17)/600.0f;

    out_Sp->ucSOG = get_bits(79,6);
    out_Sp->ucCOG = get_bits(85,9);
    out_Sp->ucGNSS_PosStatus = get_bits(94,1);
    out_Sp->ucSpare         = get_bits(95,1);

}


const char *ais_strip_tagblock(const char *in_pcNMEA)
{
    const char *p_cPtr = strchr(in_pcNMEA, '!');
    if (!p_cPtr)
        return NULL;
    return p_cPtr;
}

void ais_reset_bits(void)
{
    iBitlen = 0;
    memset(arr_cBitbuf, 0, sizeof(arr_cBitbuf));
}


int ais_decode(const char *in_pcNmea, int *out_piMsgid, U_AIS_MSG_FULL *out_pU_Msg)
{
    char arr_cPayload[512];
    int iTotal = 1, iNum = 1;
    char arr_cSeqstr[8] = {0};
    char cChan = 0;
    int iSeq = -1;

    if (!in_pcNmea || !out_piMsgid || !out_pU_Msg)
        return AIS_ERR_INVALID_ARGUMENT;

    const char *p_cAIS = ais_strip_tagblock(in_pcNmea);
    if (!p_cAIS)
        return AIS_ERR_FORMAT;

    if (!validate_checksum(p_cAIS))
        printf("WARNING: checksum mismatch\n");

    /* FIXED sscanf: seq field may be EMPTY */
    int iParsedFields = sscanf(p_cAIS, "!AIVDM,%d,%d,%7[^,],%c",
                               &iTotal, &iNum, arr_cSeqstr, &cChan);

    if (iParsedFields < 2)
        return -3;
    //convert seq id "3"-> 3
    if (arr_cSeqstr[0] != '\0')
        iSeq = atoi(arr_cSeqstr);

    if (extract_payload(p_cAIS, arr_cPayload) != 0)
        return AIS_ERR_FORMAT;

    /* ---------- SINGLE FRAGMENT ---------- */
    if (iTotal == 1)
    {
        ais_reset_bits();
        payload_to_bits(arr_cPayload);
    }
    /* ---------- MULTI FRAGMENT ---------- */
    else
    {
        S_AIS_FRAG_CTX *S_CTX = &gS_Frag_CTX[iSeq];

        if (iNum == 1)
        {
            memset(S_CTX, 0, sizeof(*S_CTX));
            strcpy(S_CTX->arr_cPayload, arr_cPayload);
            S_CTX->iExpected = iTotal;
            S_CTX->ireceived = 1;
            S_CTX->iSeq      = iSeq;
            S_CTX->iActive   = 1;
            return AIS_ERR_INCOMPLETE;   /* wait for next fragment */
        }

        //middle fragment
        if (!S_CTX->iActive || S_CTX->iSeq != iSeq)
            return AIS_ERR_INCOMPLETE;

        //last fragemnt
        strcat(S_CTX->arr_cPayload, arr_cPayload);
        S_CTX->ireceived++;

        if (S_CTX->ireceived < S_CTX->iExpected)
            return AIS_ERR_INCOMPLETE;

        ais_reset_bits();
        payload_to_bits(S_CTX->arr_cPayload);
        memset(S_CTX, 0, sizeof(*S_CTX));
    }

    /* ---------- DECODE ---------- */
    *out_piMsgid = get_bits(0, 6);

    switch (*out_piMsgid)
    {
    case TYPE1MSG:
    case TYPE2MSG:
    case TYPE3MSG:
        decode_type123(&out_pU_Msg->mSt123);
        validate_type123(&out_pU_Msg->mSt123);
        break;

    case TYPE4MSG:  decode_type4(&out_pU_Msg->mSt4); validate_type4(&out_pU_Msg->mSt4);break;
    case TYPE5MSG:  decode_type5(&out_pU_Msg->mSt5); validate_type5(&out_pU_Msg->mSt5);break;
    case TYPE6MSG:  decode_type6(&out_pU_Msg->mSt6); validate_type6(&out_pU_Msg->mSt6);break;
    case TYPE7MSG:  decode_type7(&out_pU_Msg->mSt7); validate_type7(&out_pU_Msg->mSt7);break;
    case TYPE8MSG:  decode_type8(&out_pU_Msg->mSt8); validate_type8(&out_pU_Msg->mSt8);break;
    case TYPE9MSG:  decode_type9(&out_pU_Msg->mSt9); validate_type9(&out_pU_Msg->mSt9);break;
    case TYPE10MSG: decode_type10(&out_pU_Msg->mSt10); validate_type10(&out_pU_Msg->mSt10);break;
    case TYPE11MSG: decode_type11(&out_pU_Msg->mSt11); validate_type11(&out_pU_Msg->mSt11);break;
    case TYPE12MSG: decode_type12(&out_pU_Msg->mSt12); validate_type12(&out_pU_Msg->mSt12);break;
    case TYPE13MSG: decode_type13(&out_pU_Msg->mSt13); validate_type13(&out_pU_Msg->mSt13);break;
    case TYPE14MSG: decode_type14(&out_pU_Msg->mSt14); validate_type14(&out_pU_Msg->mSt14);break;
    case TYPE15MSG: decode_type15(&out_pU_Msg->mSt15); validate_type15(&out_pU_Msg->mSt15);break;
    case TYPE16MSG: decode_type16(&out_pU_Msg->mSt16); validate_type16(&out_pU_Msg->mSt16);break;
    case TYPE17MSG: decode_type17(&out_pU_Msg->mSt17); validate_type17(&out_pU_Msg->mSt17);break;
    case TYPE18MSG: decode_type18(&out_pU_Msg->mSt18); validate_type18(&out_pU_Msg->mSt18);break;
    case TYPE19MSG: decode_type19(&out_pU_Msg->mSt19); validate_type19(&out_pU_Msg->mSt19);break;
    case TYPE20MSG: decode_type20(&out_pU_Msg->mSt20); validate_type20(&out_pU_Msg->mSt20);break;
    case TYPE21MSG: decode_type21(&out_pU_Msg->mSt21); validate_type21(&out_pU_Msg->mSt21);break;
    case TYPE22MSG: decode_type22(&out_pU_Msg->mSt22); validate_type22(&out_pU_Msg->mSt22);break;
    case TYPE23MSG: decode_type23(&out_pU_Msg->mSt23); validate_type23(&out_pU_Msg->mSt23);break;
    case TYPE24MSG: decode_type24(&out_pU_Msg->mSt24); validate_type24(&out_pU_Msg->mSt24);break;
    case TYPE25MSG: decode_type25(&out_pU_Msg->mSt25); validate_type25(&out_pU_Msg->mSt25);break;
    case TYPE26MSG: decode_type26(&out_pU_Msg->mSt26); validate_type26(&out_pU_Msg->mSt26);break;
    case TYPE27MSG: decode_type27(&out_pU_Msg->mSt27); validate_type27(&out_pU_Msg->mSt27);break;

    default:
        return AIS_ERR_FORMAT;
    }

    return AIS_SUCCESS;

}




/* ============================================================
 * pRINT FUNCTIONS
 * ============================================================ */
void ais_print_type123(const S_AIS_TYPE123_FULL *in_pS)
{
    printf("\n--- AIS TYpE %u pOSITION REpORT ---\n", in_pS->ucMsgID);
    printf("Repeat Indicator : %u\n", in_pS->ucRepeat);
    printf("MMSI             : %u\n", in_pS->uiMMSI);
    printf("Nav Status       : %u\n", in_pS->ucNav_Status);
    printf("ROT              : %d\n", in_pS->cROT);
    printf("SOG              : %.1f kn\n", in_pS->fSOG_kn);
    printf("position Acc     : %u\n", in_pS->ucPos_Acc);
    printf("Longitude        : %.5f\n", in_pS->fLon);
    printf("Latitude         : %.5f\n", in_pS->fLat);
    printf("COG              : %.1f deg\n", in_pS->fCOG_deg);
    printf("Heading          : %u\n", in_pS->usHeading);
    printf("Timestamp        : %u\n", in_pS->ucTimestamp);
    printf("Maneuver         : %u\n", in_pS->ucManeuver);
    printf("RAIM             : %u\n", in_pS->ucRAIM);
    printf("Sync State       : %u\n", in_pS->ucSync_State);
    printf("Slot Timeout     : %u\n", in_pS->ucSlot_Timeout);
    printf("Sub Message      : %u\n", in_pS->us_Sub_message);
    printf("ucOut_Spare    : %u\n", in_pS->ucOut_Spare);

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS_ERR_MSG_ID)
            printf("  - Invalid MESSAGE ID\n");

        if (in_pS->uiErrorMask & AIS_ERR_REPEAT)
            printf("  - Invalid REPEAT INDICATOR\n");

        if (in_pS->uiErrorMask & AIS_ERR_MMSI)
            printf("  - Invalid MMSI\n");

        if (in_pS->uiErrorMask & AIS_ERR_NAV_STATUS)
            printf("  - Invalid NAVIGATION STATUS\n");

        if (in_pS->uiErrorMask & AIS_ERR_ROT)
            printf("  - Invalid RATE OF TURN\n");

        if (in_pS->uiErrorMask & AIS_ERR_SOG)
            printf("  - Invalid SPEED OVER GROUND\n");

        if (in_pS->uiErrorMask & AIS_ERR_POS_ACC)
            printf("  - Invalid POSITION ACCURACY\n");

        if (in_pS->uiErrorMask & AIS_ERR_LON)
            printf("  - Invalid LONGITUDE\n");

        if (in_pS->uiErrorMask & AIS_ERR_LAT)
            printf("  - Invalid LATITUDE\n");

        if (in_pS->uiErrorMask & AIS_ERR_COG)
            printf("  - Invalid COURSE OVER GROUND\n");

        if (in_pS->uiErrorMask & AIS_ERR_HEADING)
            printf("  - Invalid TRUE HEADING\n");

        if (in_pS->uiErrorMask & AIS_ERR_TIMESTAMP)
            printf("  - Invalid TIMESTAMP\n");

        if (in_pS->uiErrorMask & AIS_ERR_MANEUVER)
            printf("  - Invalid MANEUVER INDICATOR\n");

        if (in_pS->uiErrorMask & AIS_ERR_RAIM)
            printf("  - Invalid RAIM FLAG\n");

        if (in_pS->uiErrorMask & AIS_ERR_SYNC_STATE)
            printf("  - Invalid SYNC STATE\n");

        if (in_pS->uiErrorMask & AIS_ERR_SLOT_TIMEOUT)
            printf("  - Invalid SLOT TIMEOUT\n");

        if (in_pS->uiErrorMask & AIS_ERR_SUB_MESSAGE)
            printf("  - Invalid SUB MESSAGE\n");

        if (in_pS->uiErrorMask & AIS_ERR_SPARE)
            printf("  - Spare bits not ZERO\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }

}


void ais_print_type4(const S_AIS_TYPE4_FULL *in_pS)
{
    printf("\n--- AIS TYPE 4 BASE STATION REPORT ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("MMSI              : %u\n", in_pS->uiMMSI);

    printf("UTC               : %04u-%02u-%02u %02u:%02u:%02u\n",
           in_pS->usYear,
           in_pS->ucMonth,
           in_pS->ucDay,
           in_pS->ucHour,
           in_pS->ucMinute,
           in_pS->ucSecond);

    printf("Position Accuracy : %u\n", in_pS->ucPos_Acc);
    printf("Longitude         : %.5f\n", in_pS->fLon);
    printf("Latitude          : %.5f\n", in_pS->fLat);
    printf("EPFD Type         : %u\n", in_pS->ucEPFS_Type);
    printf("RAIM              : %u\n", in_pS->ucRAIM);

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS4_ERR_MSG_ID)  printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS4_ERR_REPEAT)  printf("  - Invalid REPEAT\n");
        if (in_pS->uiErrorMask & AIS4_ERR_MMSI)    printf("  - Invalid MMSI\n");
        if (in_pS->uiErrorMask & AIS4_ERR_YEAR)    printf("  - Invalid YEAR\n");
        if (in_pS->uiErrorMask & AIS4_ERR_MONTH)   printf("  - Invalid MONTH\n");
        if (in_pS->uiErrorMask & AIS4_ERR_DAY)     printf("  - Invalid DAY\n");
        if (in_pS->uiErrorMask & AIS4_ERR_HOUR)    printf("  - Invalid HOUR\n");
        if (in_pS->uiErrorMask & AIS4_ERR_MINUTE)  printf("  - Invalid MINUTE\n");
        if (in_pS->uiErrorMask & AIS4_ERR_SECOND)  printf("  - Invalid SECOND\n");
        if (in_pS->uiErrorMask & AIS4_ERR_LON)     printf("  - Invalid LONGITUDE\n");
        if (in_pS->uiErrorMask & AIS4_ERR_LAT)     printf("  - Invalid LATITUDE\n");
        if (in_pS->uiErrorMask & AIS4_ERR_EPFS)    printf("  - Invalid EPFS\n");
        if (in_pS->uiErrorMask & AIS4_ERR_RAIM)    printf("  - Invalid RAIM\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type5(const S_AIS_TYPE5_FULL *in_pS)
{
    printf("\n--- AIS TYPE 5 STATIC & VOYAGE DATA ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("MMSI              : %u\n", in_pS->uiMMSI);
    printf("AIS Version       : %u\n", in_pS->ucAIS_Version);
    printf("IMO Number        : %u\n", in_pS->uiIMO);
    printf("Callsign          : %s\n", in_pS->arr_cCallsign);
    printf("Ship Name         : %s\n", in_pS->arr_cShip_name);
    printf("Ship Type         : %u\n", in_pS->ucShip_Type);

    printf("Dimensions (m)\n");
    printf("  Bow             : %u\n", in_pS->usTo_bow);
    printf("  Stern           : %u\n", in_pS->usTo_stern);
    printf("  Port            : %u\n", in_pS->ucTo_Port);
    printf("  Starboard       : %u\n", in_pS->ucTo_StarBoard);

    printf("EPFS Type         : %u\n", in_pS->ucEPFS_Type);

    printf("ETA               : %02u-%02u %02u:%02u\n",
           in_pS->uiB_Eta_month,
           in_pS->uiB_Eta_day,
           in_pS->uiB_Eta_hour,
           in_pS->uiB_Eta_minute);

    printf("Draught           : %.1f m\n", in_pS->fDraught);
    printf("Destination       : %s\n", in_pS->arr_cDestination);
    printf("DTE               : %u\n", in_pS->ucDTE);
    printf("Spare             : %u\n", in_pS->ucOut_Spare);

    /* ============== VALIDATION ============== */
    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS5_ERR_MSG_ID)        printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS5_ERR_REPEAT)        printf("  - Invalid REPEAT\n");
        if (in_pS->uiErrorMask & AIS5_ERR_MMSI)          printf("  - Invalid MMSI\n");
        if (in_pS->uiErrorMask & AIS5_ERR_AIS_VERSION)   printf("  - Invalid AIS VERSION\n");
        if (in_pS->uiErrorMask & AIS5_ERR_IMO)           printf("  - Invalid IMO\n");
        if (in_pS->uiErrorMask & AIS5_ERR_SHIP_TYPE)     printf("  - Invalid SHIP TYPE\n");
        if (in_pS->uiErrorMask & AIS5_ERR_DIM_BOW ||
                in_pS->uiErrorMask & AIS5_ERR_DIM_STERN ||
                in_pS->uiErrorMask & AIS5_ERR_DIM_PORT ||
                in_pS->uiErrorMask & AIS5_ERR_DIM_STARBOARD)
            printf("  - Invalid DIMENSIONS\n");
        if (in_pS->uiErrorMask & AIS5_ERR_EPFS)          printf("  - Invalid EPFS\n");
        if (in_pS->uiErrorMask & AIS5_ERR_ETA_MONTH ||
                in_pS->uiErrorMask & AIS5_ERR_ETA_DAY ||
                in_pS->uiErrorMask & AIS5_ERR_ETA_HOUR ||
                in_pS->uiErrorMask & AIS5_ERR_ETA_MINUTE)
            printf("  - Invalid ETA\n");
        if (in_pS->uiErrorMask & AIS5_ERR_DRAUGHT)       printf("  - Invalid DRAUGHT\n");
        if (in_pS->uiErrorMask & AIS5_ERR_DTE)           printf("  - Invalid DTE\n");
        if (in_pS->uiErrorMask & AIS5_ERR_SPARE)         printf("  - Spare bits not zero\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}

void ais_print_type6(const S_AIS_TYPE6_FULL *in_pS)
{
    printf("\n--- AIS TYPE 6 ADDRESSED BINARY MESSAGE ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);
    printf("Sequence Number   : %u\n", in_pS->ucSeq_Num);
    printf("Destination MMSI  : %u\n", in_pS->uiDest_MMSI);
    printf("Retransmit Flag   : %u\n", in_pS->ucRetransmit);
    printf("Spare             : %u\n", in_pS->ucOut_Spare);
    printf("DAC               : %u\n", in_pS->usDAC);
    printf("FI                : %u\n", in_pS->ucFI);
    printf("Binary Length     : %u bits\n", in_pS->usBinary_Len_Bits);

    /* ============== VALIDATION ============== */

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS6_ERR_MSG_ID)     printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS6_ERR_REPEAT)     printf("  - Invalid REPEAT\n");
        if (in_pS->uiErrorMask & AIS6_ERR_MMSI)       printf("  - Invalid MMSI\n");
        if (in_pS->uiErrorMask & AIS6_ERR_SEQ_NUM)    printf("  - Invalid SEQUENCE NUMBER\n");
        if (in_pS->uiErrorMask & AIS6_ERR_DEST_MMSI)  printf("  - Invalid DEST MMSI\n");
        if (in_pS->uiErrorMask & AIS6_ERR_RETRANSMIT) printf("  - Invalid RETRANSMIT FLAG\n");
        if (in_pS->uiErrorMask & AIS6_ERR_DAC)        printf("  - Invalid DAC\n");
        if (in_pS->uiErrorMask & AIS6_ERR_FI)         printf("  - Invalid FI\n");
        if (in_pS->uiErrorMask & AIS6_ERR_BINARY_LEN) printf("  - Invalid BINARY LENGTH\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}



void ais_print_type7(const S_AIS_TYPE7_FULL *in_pS)
{
    printf("\n--- AIS TYPE 7 BINARY ACKNOWLEDGE ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);
    printf("ACK Count         : %u\n", in_pS->ucAck_Count);

    for (int i = 0; i < in_pS->ucAck_Count; i++)
    {
        printf("  ACK %d\n", i + 1);
        printf("    Destination MMSI : %u\n", in_pS->arr_uiDest_MMSI[i]);
        printf("    Sequence Number  : %u\n", in_pS->arr_ucSeq_Num[i]);
    }

    /* ============== VALIDATION ============== */

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS7_ERR_MSG_ID)    printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS7_ERR_REPEAT)    printf("  - Invalid REPEAT\n");
        if (in_pS->uiErrorMask & AIS7_ERR_MMSI)      printf("  - Invalid MMSI\n");
        if (in_pS->uiErrorMask & AIS7_ERR_ACK_COUNT) printf("  - Invalid ACK COUNT\n");
        if (in_pS->uiErrorMask & AIS7_ERR_DEST_MMSI) printf("  - Invalid DEST MMSI\n");
        if (in_pS->uiErrorMask & AIS7_ERR_SEQ_NUM)   printf("  - Invalid SEQUENCE NUMBER\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}

void ais_print_type8(const S_AIS_TYPE8_FULL *in_pS)
{
    printf("\n--- AIS TYPE 8 BINARY BROADCAST MESSAGE ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);
    printf("DAC               : %u\n", in_pS->usDAC);
    printf("FI                : %u\n", in_pS->ucFI);
    printf("Binary Length     : %u bits\n", in_pS->usBinary_Len_Bits);

    /* ============== VALIDATION ============== */

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS8_ERR_MSG_ID)     printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS8_ERR_REPEAT)     printf("  - Invalid REPEAT\n");
        if (in_pS->uiErrorMask & AIS8_ERR_MMSI)       printf("  - Invalid MMSI\n");
        if (in_pS->uiErrorMask & AIS8_ERR_DAC)        printf("  - Invalid DAC\n");
        if (in_pS->uiErrorMask & AIS8_ERR_FI)         printf("  - Invalid FI\n");
        if (in_pS->uiErrorMask & AIS8_ERR_BINARY_LEN) printf("  - Invalid BINARY LENGTH\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type9(const S_AIS_TYPE9_FULL *in_pS)
{
    printf("\n--- AIS TYPE 9 SAR AIRCRAFT POSITION REPORT ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("MMSI              : %u\n", in_pS->uiMMSI);
    printf("Altitude          : %u m\n", in_pS->usAltitude);
    printf("SOG               : %u kn\n", in_pS->ucSOG);
    printf("Position Accuracy : %u\n", in_pS->ucPos_Acc);
    printf("Longitude         : %.5f\n", in_pS->fLon);
    printf("Latitude          : %.5f\n", in_pS->fLat);
    printf("COG               : %u\n", in_pS->usCog);
    printf("Timestamp         : %u\n", in_pS->ucTimestamp);
    printf("Altitude Sensor   : %u\n", in_pS->ucAltitude_Sensor);
    printf("DTE               : %u\n", in_pS->ucDTE);
    printf("RAIM              : %u\n", in_pS->ucRAIM);
    printf("Spare             : %u\n", in_pS->ucOut_Spare);

    /* ============== VALIDATION ============== */

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS9_ERR_ALTITUDE) printf("  - Invalid ALTITUDE\n");
        if (in_pS->uiErrorMask & AIS9_ERR_SOG)      printf("  - Invalid SOG\n");
        if (in_pS->uiErrorMask & AIS9_ERR_LON)      printf("  - Invalid LONGITUDE\n");
        if (in_pS->uiErrorMask & AIS9_ERR_LAT)      printf("  - Invalid LATITUDE\n");
        if (in_pS->uiErrorMask & AIS9_ERR_COG)      printf("  - Invalid COG\n");
        if (in_pS->uiErrorMask & AIS9_ERR_TIMESTAMP)printf("  - Invalid TIMESTAMP\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type10(const S_AIS_TYPE10_FULL *in_pS)
{
    printf("\n--- AIS TYPE 10 UTC / DATE INQUIRY ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);
    printf("Destination MMSI  : %u\n", in_pS->uiDest_MMSI);
    printf("Spare             : %u\n", in_pS->ucOut_Spare);

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS10_ERR_MSG_ID)    printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS10_ERR_REPEAT)    printf("  - Invalid REPEAT\n");
        if (in_pS->uiErrorMask & AIS10_ERR_MMSI)      printf("  - Invalid MMSI\n");
        if (in_pS->uiErrorMask & AIS10_ERR_DEST_MMSI) printf("  - Invalid DEST MMSI\n");
        if (in_pS->uiErrorMask & AIS10_ERR_SPARE)     printf("  - Spare bits not zero\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type11(const S_AIS_TYPE11_FULL *in_pS)
{
    printf("\n--- AIS TYPE 11 UTC / DATE RESPONSE ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);

    printf("UTC Date & Time   : %04u-%02u-%02u %02u:%02u:%02u\n",
           in_pS->usYear,
           in_pS->ucMonth,
           in_pS->ucDay,
           in_pS->ucHour,
           in_pS->ucMinute,
           in_pS->ucSecond);

    printf("Position Accuracy: %u\n", in_pS->ucPos_Acc);
    printf("Longitude         : %.5f\n", in_pS->fLon);
    printf("Latitude          : %.5f\n", in_pS->fLat);

    printf("RAIM              : %u\n", in_pS->ucRAIM);
    printf("Radio Status      : %u\n", in_pS->uiRadio);

    /* ============== VALIDATION ============== */

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS11_ERR_MSG_ID)
            printf("  - Invalid MESSAGE ID\n");

        if (in_pS->uiErrorMask & AIS11_ERR_REPEAT)
            printf("  - Invalid REPEAT INDICATOR\n");

        if (in_pS->uiErrorMask & AIS11_ERR_MMSI)
            printf("  - Invalid MMSI\n");

        if (in_pS->uiErrorMask & AIS11_ERR_YEAR)
            printf("  - Invalid YEAR\n");

        if (in_pS->uiErrorMask & AIS11_ERR_MONTH)
            printf("  - Invalid MONTH\n");

        if (in_pS->uiErrorMask & AIS11_ERR_DAY)
            printf("  - Invalid DAY\n");

        if (in_pS->uiErrorMask & AIS11_ERR_HOUR)
            printf("  - Invalid HOUR\n");

        if (in_pS->uiErrorMask & AIS11_ERR_MINUTE)
            printf("  - Invalid MINUTE\n");

        if (in_pS->uiErrorMask & AIS11_ERR_SECOND)
            printf("  - Invalid SECOND\n");

        if (in_pS->uiErrorMask & AIS11_ERR_LON)
            printf("  - Invalid LONGITUDE\n");

        if (in_pS->uiErrorMask & AIS11_ERR_LAT)
            printf("  - Invalid LATITUDE\n");

        if (in_pS->uiErrorMask & AIS11_ERR_RAIM)
            printf("  - Invalid RAIM FLAG\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type12(const S_AIS_TYPE12_FULL *in_pS)
{
    printf("\n--- AIS TYPE 12 ADDRESSED SAFETY MESSAGE ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);

    printf("Sequence Number  : %u\n", in_pS->usSeq_Num);
    printf("Destination MMSI : %u\n", in_pS->uiDest_MMSI);
    printf("Retransmit Flag  : %u\n", in_pS->ucRetransmit);
    printf("Spare            : %u\n", in_pS->ucOut_Spare);

    printf("Text Message     : %s\n", in_pS->arr_cText);

    /* ============== VALIDATION ============== */

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS12_ERR_MSG_ID)
            printf("  - Invalid MESSAGE ID\n");

        if (in_pS->uiErrorMask & AIS12_ERR_REPEAT)
            printf("  - Invalid REPEAT INDICATOR\n");

        if (in_pS->uiErrorMask & AIS12_ERR_MMSI)
            printf("  - Invalid SOURCE MMSI\n");

        if (in_pS->uiErrorMask & AIS12_ERR_SEQ_NUM)
            printf("  - Invalid SEQUENCE NUMBER\n");

        if (in_pS->uiErrorMask & AIS12_ERR_DEST_MMSI)
            printf("  - Invalid DESTINATION MMSI\n");

        if (in_pS->uiErrorMask & AIS12_ERR_RETRANSMIT)
            printf("  - Invalid RETRANSMIT FLAG\n");

        if (in_pS->uiErrorMask & AIS12_ERR_TEXT_LEN)
            printf("  - Text length exceeds limit\n");

        if (in_pS->uiErrorMask & AIS12_ERR_TEXT_CHAR)
            printf("  - Invalid character in text\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type13(const S_AIS_TYPE13_FULL *in_pS)
{
    int i;

    printf("\n--- AIS TYPE 13 SAFETY ACKNOWLEDGEMENT ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);

    printf("ACK Count         : %u\n", in_pS->ucAck_Count);

    for (i = 0; i < in_pS->ucAck_Count; i++)
    {
        printf("ACK %d Dest MMSI  : %u\n", i + 1, in_pS->arr_uiDest_MMSI[i]);
        printf("ACK %d Seq Num    : %u\n", i + 1, in_pS->arr_ucSeq_Num[i]);
    }

    /* ============== VALIDATION ============== */

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS13_ERR_MSG_ID)
            printf("  - Invalid MESSAGE ID\n");

        if (in_pS->uiErrorMask & AIS13_ERR_REPEAT)
            printf("  - Invalid REPEAT INDICATOR\n");

        if (in_pS->uiErrorMask & AIS13_ERR_MMSI)
            printf("  - Invalid SOURCE MMSI\n");

        if (in_pS->uiErrorMask & AIS13_ERR_ACK_COUNT)
            printf("  - Invalid ACK COUNT\n");

        if (in_pS->uiErrorMask & AIS13_ERR_DEST_MMSI)
            printf("  - Invalid DESTINATION MMSI\n");

        if (in_pS->uiErrorMask & AIS13_ERR_SEQ_NUM)
            printf("  - Invalid SEQUENCE NUMBER\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type14(const S_AIS_TYPE14_FULL *in_pS)
{
    printf("\n--- AIS TYPE 14 SAFETY BROADCAST ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);

    printf("Broadcast Text    : %s\n", in_pS->arr_cText);

    /* ============== VALIDATION ============== */

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS14_ERR_MSG_ID)
            printf("  - Invalid MESSAGE ID\n");

        if (in_pS->uiErrorMask & AIS14_ERR_REPEAT)
            printf("  - Invalid REPEAT INDICATOR\n");

        if (in_pS->uiErrorMask & AIS14_ERR_MMSI)
            printf("  - Invalid SOURCE MMSI\n");

        if (in_pS->uiErrorMask & AIS14_ERR_TEXT_LEN)
            printf("  - Text length exceeds limit\n");

        if (in_pS->uiErrorMask & AIS14_ERR_TEXT_CHAR)
            printf("  - Invalid character in text\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type15(const S_AIS_TYPE15_FULL *in_pS)
{
    printf("\n--- AIS TYPE 15 INTERROGATION ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);

    printf("Dest MMSI 1       : %u\n", in_pS->uiDest_MMSI1);
    printf("Msg ID 1          : %u\n", in_pS->ucMsgID1);
    printf("Slot Offset 1     : %u\n", in_pS->ucSlot_Offset1);

    printf("Dest MMSI 2       : %u\n", in_pS->uiDest_MMSI2);
    printf("Msg ID 2          : %u\n", in_pS->ucMsgID2);
    printf("Slot Offset 2     : %u\n", in_pS->ucSlot_Offset2);

    /* ============== VALIDATION ============== */

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS15_ERR_MSG_ID)
            printf("  - Invalid MESSAGE ID\n");

        if (in_pS->uiErrorMask & AIS15_ERR_REPEAT)
            printf("  - Invalid REPEAT INDICATOR\n");

        if (in_pS->uiErrorMask & AIS15_ERR_MMSI)
            printf("  - Invalid SOURCE MMSI\n");

        if (in_pS->uiErrorMask & AIS15_ERR_DEST1_MMSI)
            printf("  - Invalid DESTINATION MMSI 1\n");

        if (in_pS->uiErrorMask & AIS15_ERR_MSG_ID1)
            printf("  - Invalid MESSAGE ID 1\n");

        if (in_pS->uiErrorMask & AIS15_ERR_SLOT1)
            printf("  - Invalid SLOT OFFSET 1\n");

        if (in_pS->uiErrorMask & AIS15_ERR_DEST2_MMSI)
            printf("  - Invalid DESTINATION MMSI 2\n");

        if (in_pS->uiErrorMask & AIS15_ERR_MSG_ID2)
            printf("  - Invalid MESSAGE ID 2\n");

        if (in_pS->uiErrorMask & AIS15_ERR_SLOT2)
            printf("  - Invalid SLOT OFFSET 2\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type16(const S_AIS_TYPE16_FULL *in_pS)
{
    printf("\n--- AIS TYPE 16 ASSIGNED MODE COMMAND ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);

    printf("Dest MMSI 1       : %u\n", in_pS->uiDest_MMSI1);
    printf("Offset 1          : %u\n", in_pS->uiOffset1);
    printf("Increment 1       : %u\n", in_pS->uiIncrement1);

    printf("Dest MMSI 2       : %u\n", in_pS->uiDest_MMSI2);
    printf("Offset 2          : %u\n", in_pS->uiOffset2);
    printf("Increment 2       : %u\n", in_pS->uiIncrement2);

    /* ============== VALIDATION ============== */
    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS16_ERR_MSG_ID)
            printf("  - Invalid MESSAGE ID\n");

        if (in_pS->uiErrorMask & AIS16_ERR_REPEAT)
            printf("  - Invalid REPEAT INDICATOR\n");

        if (in_pS->uiErrorMask & AIS16_ERR_MMSI)
            printf("  - Invalid SOURCE MMSI\n");

        if (in_pS->uiErrorMask & AIS16_ERR_DEST1_MMSI)
            printf("  - Invalid DESTINATION MMSI 1\n");

        if (in_pS->uiErrorMask & AIS16_ERR_OFFSET1)
            printf("  - Invalid OFFSET 1\n");

        if (in_pS->uiErrorMask & AIS16_ERR_INCREMENT1)
            printf("  - Invalid INCREMENT 1\n");

        if (in_pS->uiErrorMask & AIS16_ERR_DEST2_MMSI)
            printf("  - Invalid DESTINATION MMSI 2\n");

        if (in_pS->uiErrorMask & AIS16_ERR_OFFSET2)
            printf("  - Invalid OFFSET 2\n");

        if (in_pS->uiErrorMask & AIS16_ERR_INCREMENT2)
            printf("  - Invalid INCREMENT 2\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}



void ais_print_type17(const S_AIS_TYPE17_FULL *in_pS)
{
    printf("\n--- AIS TYPE 17 GNSS BROADCAST BINARY ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);
    printf("Longitude         : %.5f\n", in_pS->fLon);
    printf("Latitude          : %.5f\n", in_pS->fLat);
    printf("Binary Length(bit): %u\n", in_pS->usBinary_Len_Bits);

    /* ============== VALIDATION ============== */
    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS17_ERR_MSG_ID)
            printf("  - Invalid MESSAGE ID\n");

        if (in_pS->uiErrorMask & AIS17_ERR_REPEAT)
            printf("  - Invalid REPEAT INDICATOR\n");

        if (in_pS->uiErrorMask & AIS17_ERR_MMSI)
            printf("  - Invalid SOURCE MMSI\n");

        if (in_pS->uiErrorMask & AIS17_ERR_LON)
            printf("  - Invalid LONGITUDE\n");

        if (in_pS->uiErrorMask & AIS17_ERR_LAT)
            printf("  - Invalid LATITUDE\n");

        if (in_pS->uiErrorMask & AIS17_ERR_BINARY)
            printf("  - Invalid BINARY PAYLOAD\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type18(const S_AIS_TYPE18_FULL *in_pS)
{
    printf("\n--- AIS TYPE 18 CLASS B POSITION REPORT ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);
    printf("SOG               : %.1f\n", in_pS->fSOG);
    printf("COG               : %.1f\n", in_pS->fCOG);
    printf("Heading           : %u\n", in_pS->usHeading);
    printf("Longitude         : %.5f\n", in_pS->fLon);
    printf("Latitude          : %.5f\n", in_pS->fLat);
    printf("Timestamp         : %u\n", in_pS->ucTimestamp);
    printf("RAIM              : %u\n", in_pS->ucRAIM);

    /* ============== VALIDATION ============== */
    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS18_ERR_MSG_ID)
            printf("  - Invalid MESSAGE ID\n");

        if (in_pS->uiErrorMask & AIS18_ERR_REPEAT)
            printf("  - Invalid REPEAT INDICATOR\n");

        if (in_pS->uiErrorMask & AIS18_ERR_MMSI)
            printf("  - Invalid SOURCE MMSI\n");

        if (in_pS->uiErrorMask & AIS18_ERR_SOG)
            printf("  - Invalid SOG\n");

        if (in_pS->uiErrorMask & AIS18_ERR_COG)
            printf("  - Invalid COG\n");

        if (in_pS->uiErrorMask & AIS18_ERR_HEADING)
            printf("  - Invalid HEADING\n");

        if (in_pS->uiErrorMask & AIS18_ERR_LON)
            printf("  - Invalid LONGITUDE\n");

        if (in_pS->uiErrorMask & AIS18_ERR_LAT)
            printf("  - Invalid LATITUDE\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}

void ais_print_type19(const S_AIS_TYPE19_FULL *in_pS)
{
    printf("\n--- AIS TYPE 19 CLASS B EXTENDED POSITION ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);
    printf("Ship Name         : %s\n", in_pS->arr_cShip_name);
    printf("Ship Type         : %u\n", in_pS->ucShip_Type);
    printf("Longitude         : %.5f\n", in_pS->fLon);
    printf("Latitude          : %.5f\n", in_pS->fLat);

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS_ERR_MSG_ID)
            printf("  - Invalid MESSAGE ID\n");

        if (in_pS->uiErrorMask & AIS19_ERR_SHIPNAME)
            printf("  - Invalid SHIP NAME\n");

        if (in_pS->uiErrorMask & AIS19_ERR_SHIPTYPE)
            printf("  - Invalid SHIP TYPE\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type20(const S_AIS_TYPE20_FULL *in_pS)
{
    int i;

    printf("\n--- AIS TYPE 20 DATA LINK MANAGEMENT ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("Source MMSI       : %u\n", in_pS->uiMMSI);
    printf("Entry Count       : %u\n", in_pS->ucEntry_Count);

    for (i = 0; i < in_pS->ucEntry_Count; i++)
    {
        printf("Entry %d Offset    : %u\n", i + 1, in_pS->arr_usOffset[i]);
        printf("Entry %d Slots     : %u\n", i + 1, in_pS->arr_ucSlots[i]);
        printf("Entry %d Timeout   : %u\n", i + 1, in_pS->arr_ucTimeout[i]);
        printf("Entry %d Increment : %u\n", i + 1, in_pS->arr_ucIncrement[i]);
    }

    /* ============== VALIDATION ============== */
    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");

        if (in_pS->uiErrorMask & AIS20_ERR_MSG_ID)
            printf("  - Invalid MESSAGE ID\n");

        if (in_pS->uiErrorMask & AIS20_ERR_REPEAT)
            printf("  - Invalid REPEAT INDICATOR\n");

        if (in_pS->uiErrorMask & AIS20_ERR_MMSI)
            printf("  - Invalid SOURCE MMSI\n");

        if (in_pS->uiErrorMask & AIS20_ERR_ENTRY_CNT)
            printf("  - Invalid ENTRY COUNT\n");

        if (in_pS->uiErrorMask & AIS20_ERR_OFFSET)
            printf("  - Invalid OFFSET\n");

        if (in_pS->uiErrorMask & AIS20_ERR_SLOTS)
            printf("  - Invalid SLOTS\n");

        if (in_pS->uiErrorMask & AIS20_ERR_TIMEOUT)
            printf("  - Invalid TIMEOUT\n");

        if (in_pS->uiErrorMask & AIS20_ERR_INCREMENT)
            printf("  - Invalid INCREMENT\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}

void ais_print_type21(const S_AIS_TYPE21_FULL *in_pS)
{
    printf("\n--- AIS TYPE 21 AID TO NAVIGATION ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("MMSI              : %u\n", in_pS->uiMMSI);

    printf("AtoN Type         : %u\n", in_pS->ucAtoN_Type);
    printf("Name              : %s\n", in_pS->arr_cName);

    printf("Position Accuracy : %u\n", in_pS->ucPos_Acc);
    printf("Longitude         : %.6f\n", in_pS->fLon);
    printf("Latitude          : %.6f\n", in_pS->fLat);

    printf("Dimension Bow     : %u\n", in_pS->usDimBow);
    printf("Dimension Stern   : %u\n", in_pS->usDimStern);
    printf("Dimension Port    : %u\n", in_pS->ucDimport);
    printf("Dimension Starbd  : %u\n", in_pS->ucDimStarboard);

    printf("EPFD Type         : %u\n", in_pS->ucEPFD);
    printf("Timestamp         : %u\n", in_pS->ucTimestamp);
    printf("Off Position      : %u\n", in_pS->ucOFF_Position);
    printf("RAIM              : %u\n", in_pS->ucRAIM);
    printf("Virtual AtoN      : %u\n", in_pS->ucVirtual_AtoN);
    printf("Assigned Mode     : %u\n", in_pS->ucAssigned);
    printf("Spare             : %u\n", in_pS->ucSpare);

    /* ============== VALIDATION ============== */
    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");
        if (in_pS->uiErrorMask & AIS21_ERR_MSG_ID)   printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS21_ERR_REPEAT)   printf("  - Invalid REPEAT INDICATOR\n");
        if (in_pS->uiErrorMask & AIS21_ERR_MMSI)     printf("  - Invalid MMSI\n");
        if (in_pS->uiErrorMask & AIS21_ERR_ATON_TYPE)printf("  - Invalid AtoN TYPE\n");
        if (in_pS->uiErrorMask & AIS21_ERR_LON)      printf("  - Invalid LONGITUDE\n");
        if (in_pS->uiErrorMask & AIS21_ERR_LAT)      printf("  - Invalid LATITUDE\n");
        if (in_pS->uiErrorMask & AIS21_ERR_EPFD)     printf("  - Invalid EPFD TYPE\n");
        if (in_pS->uiErrorMask & AIS21_ERR_SPARE)    printf("  - Non-zero SPARE\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type22(const S_AIS_TYPE22_FULL *in_pS)
{
    printf("\n--- AIS TYPE 22 CHANNEL MANAGEMENT ---\n");

    printf("Message ID        : %u\n", in_pS->ucMsgID);
    printf("Repeat Indicator  : %u\n", in_pS->ucRepeat);
    printf("MMSI              : %u\n", in_pS->uiMMSI);

    printf("Channel A         : %u\n", in_pS->ucChannelA);
    printf("Channel B         : %u\n", in_pS->ucChannelB);
    printf("Tx/Rx Mode        : %u\n", in_pS->ucTX_RX_Mode);
    printf("Power             : %u\n", in_pS->ucPower);
    printf("Addressed         : %u\n", in_pS->ucAddressed);

    if (in_pS->ucAddressed)
    {
        printf("Dest MMSI 1       : %u\n", in_pS->uiDestMMSI1);
        printf("Dest MMSI 2       : %u\n", in_pS->uiDestMMSI2);
    }
    else
    {
        printf("Lon1              : %.6f\n", in_pS->fLon1);
        printf("Lat1              : %.6f\n", in_pS->fLat1);
        printf("Lon2              : %.6f\n", in_pS->fLon2);
        printf("Lat2              : %.6f\n", in_pS->fLat2);
    }

    printf("Band              : %u\n", in_pS->ucBand);
    printf("Zone Size         : %u\n", in_pS->ucZoneSize);
    printf("Spare             : %u\n", in_pS->ucSpare);

    /* ============== VALIDATION ============== */
    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");
        if (in_pS->uiErrorMask & AIS22_ERR_MSG_ID) printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS22_ERR_REPEAT) printf("  - Invalid REPEAT\n");
        if (in_pS->uiErrorMask & AIS22_ERR_MMSI) printf("  - Invalid MMSI\n");
        if (in_pS->uiErrorMask & AIS22_ERR_CHANNEL) printf("  - Invalid CHANNEL\n");
        if (in_pS->uiErrorMask & AIS22_ERR_DEST_MMSI) printf("  - Invalid DEST MMSI\n");
        if (in_pS->uiErrorMask & AIS22_ERR_LON_LAT) printf("  - Invalid LAT/LON\n");
        if (in_pS->uiErrorMask & AIS22_ERR_SPARE) printf("  - Non-zero SPARE\n");
    }
    else
    {
        printf("VALIDATION: OK\n");
    }
}


void ais_print_type23(const S_AIS_TYPE23_FULL *in_pS)
{
    printf("\n--- AIS TYPE 23 GROUP ASSIGNMENT ---\n");

    printf("Message ID   : %u\n", in_pS->ucMsgID);
    printf("Repeat       : %u\n", in_pS->ucRepeat);
    printf("MMSI         : %u\n", in_pS->uiMMSI);

    printf("Lon1         : %.6f\n", in_pS->fLon1);
    printf("Lat1         : %.6f\n", in_pS->fLat1);
    printf("Lon2         : %.6f\n", in_pS->fLon2);
    printf("Lat2         : %.6f\n", in_pS->fLat2);

    printf("Station Type : %u\n", in_pS->ucStation_Type);
    printf("Ship Type    : %u\n", in_pS->ucShip_Type);
    printf("Tx/Rx Mode   : %u\n", in_pS->ucTX_RX_Mode);
    printf("Interval     : %u\n", in_pS->ucInterval);
    printf("Quiet Time   : %u\n", in_pS->ucQuietTime);
    printf("Spare        : %u\n", in_pS->ucSpare);

    /* VALIDATION */
    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");
        if (in_pS->uiErrorMask & AIS23_ERR_MSG_ID) printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS23_ERR_LON_LAT) printf("  - Invalid LAT/LON\n");
        if (in_pS->uiErrorMask & AIS23_ERR_INTERVAL) printf("  - Invalid INTERVAL\n");
        if (in_pS->uiErrorMask & AIS23_ERR_SPARE) printf("  - Non-zero SPARE\n");
    }
    else printf("VALIDATION: OK\n");
}


void ais_print_type24(const S_AIS_TYPE24_FULL *in_pS)
{
    printf("\n--- AIS TYPE 24 CLASS B STATIC DATA ---\n");

    printf("Message ID   : %u\n", in_pS->ucMsgID);
    printf("Repeat       : %u\n", in_pS->ucRepeat);
    printf("MMSI         : %u\n", in_pS->uiMMSI);
    printf("Part Number  : %u\n", in_pS->ucPartNO);

    if (in_pS->ucPartNO == 0)
    {
        printf("Ship Name    : %s\n", in_pS->arr_cShip_name);
    }
    else
    {
        printf("Ship Type    : %u\n", in_pS->ucShip_Type);
        printf("Vendor ID    : %s\n", in_pS->arr_cVendor_id);
        printf("Callsign     : %s\n", in_pS->arr_cCallsign);
        printf("Bow          : %u\n", in_pS->usTo_bow);
        printf("Stern        : %u\n", in_pS->usTo_stern);
        printf("Port         : %u\n", in_pS->ucTo_Port);
        printf("Starboard    : %u\n", in_pS->ucTo_StarBoard);
        printf("EPFD         : %u\n", in_pS->ucEPFS_Type);
    }

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");
        if (in_pS->uiErrorMask & AIS24_ERR_MSG_ID) printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS24_ERR_PART_NO) printf("  - Invalid PART NUMBER\n");
        if (in_pS->uiErrorMask & AIS24_ERR_SHIPNAME) printf("  - Missing SHIP NAME\n");
        if (in_pS->uiErrorMask & AIS24_ERR_SHIPTYPE) printf("  - Invalid SHIP TYPE\n");
        if (in_pS->uiErrorMask & AIS24_ERR_CALLSIGN) printf("  - Missing CALLSIGN\n");
    }
    else printf("VALIDATION: OK\n");
}


void ais_print_type25(const S_AIS_TYPE25_FULL *in_pS)
{
    printf("\n--- AIS TYPE 25 SINGLE SLOT BINARY ---\n");

    printf("Message ID  : %u\n", in_pS->ucMsgID);
    printf("Repeat      : %u\n", in_pS->ucRepeat);
    printf("MMSI        : %u\n", in_pS->uiMMSI);
    printf("Addressed   : %u\n", in_pS->ucAddressed);
    printf("Structured  : %u\n", in_pS->ucStructured);
    printf("Dest MMSI   : %u\n", in_pS->uiDestMMSI);
    printf("Binary Len  : %u bits\n", in_pS->usBinaryLen);

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");
        if (in_pS->uiErrorMask & AIS25_ERR_MSG_ID) printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS25_ERR_DEST_MMSI) printf("  - Invalid DEST MMSI\n");
        if (in_pS->uiErrorMask & AIS25_ERR_BINARY) printf("  - Invalid BINARY DATA\n");
    }
    else printf("VALIDATION: OK\n");
}

void ais_print_type26(const S_AIS_TYPE26_FULL *in_pS)
{
    printf("\n--- AIS TYPE 26 MULTI SLOT BINARY ---\n");

    printf("Message ID  : %u\n", in_pS->ucMsgID);
    printf("Repeat      : %u\n", in_pS->ucRepeat);
    printf("MMSI        : %u\n", in_pS->uiMMSI);
    printf("Addressed   : %u\n", in_pS->ucAddressed);
    printf("Structured  : %u\n", in_pS->ucStructured);
    printf("Dest MMSI   : %u\n", in_pS->uiDestMMSI);
    printf("Binary Len  : %u bits\n", in_pS->usBinaryLen);

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");
        if (in_pS->uiErrorMask & AIS26_ERR_MSG_ID) printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS26_ERR_DEST_MMSI) printf("  - Invalid DEST MMSI\n");
        if (in_pS->uiErrorMask & AIS26_ERR_BINARY) printf("  - Invalid BINARY DATA\n");
    }
    else printf("VALIDATION: OK\n");
}


void ais_print_type27(const S_AIS_TYPE27_FULL *in_pS)
{
    printf("\n--- AIS TYPE 27 LONG RANGE BROADCAST ---\n");

    printf("Message ID    : %u\n", in_pS->ucMsgID);
    printf("Repeat        : %u\n", in_pS->ucRepeat);
    printf("MMSI          : %u\n", in_pS->uiMMSI);

    printf("Position Acc  : %u\n", in_pS->ucPos_Acc);
    printf("RAIM          : %u\n", in_pS->ucRAIM);
    printf("Nav Status    : %u\n", in_pS->ucNav_Status);
    printf("Longitude     : %.6f\n", in_pS->fLon);
    printf("Latitude      : %.6f\n", in_pS->fLat);
    printf("SOG           : %u\n", in_pS->ucSOG);
    printf("COG           : %u\n", in_pS->ucCOG);
    printf("GNSS Status   : %u\n", in_pS->ucGNSS_PosStatus);
    printf("Spare         : %u\n", in_pS->ucSpare);

    if (in_pS->iValidateStatus != AIS_OK)
    {
        printf("VALIDATION: FAILED\n");
        if (in_pS->uiErrorMask & AIS27_ERR_MSG_ID) printf("  - Invalid MESSAGE ID\n");
        if (in_pS->uiErrorMask & AIS27_ERR_NAV_STATUS) printf("  - Invalid NAV STATUS\n");
        if (in_pS->uiErrorMask & AIS27_ERR_LON) printf("  - Invalid LONGITUDE\n");
        if (in_pS->uiErrorMask & AIS27_ERR_LAT) printf("  - Invalid LATITUDE\n");
        if (in_pS->uiErrorMask & AIS27_ERR_SPARE) printf("  - Non-zero SPARE\n");
    }
    else printf("VALIDATION: OK\n");
}


/* ============================================================
 * MAIN
 * ============================================================ */

/*
int main(void)
{
    const char *arr_pNmea_string[] =
    {
        "!AIVDM,1,1,,B,15MuqJ001oK?TpH>Dp@EwvN00000,0*14", // type 1
        "!AIVDM,1,1,,A,25N?;p000pD;88MD5MTDww@0TKH,0*62", // type 2
        "!AIVDM,1,1,,A,35N?;p000pD;88MD5MTDww@0TKH,0*63", //type 3
        "!AIVDM,1,1,,B,403Owpiuho;M7bO`E@0,0*78",        // type 4
        "!AIVDM,2,1,6,B,55NBsi02>tJ@E@L4p4@E=@0000000000000000000,0*35", // type 5 (part 1)
        "!AIVDM,2,2,6,B,0000000000000000000000000000000000000,2*21",   // type 5 (part 2)
        "!AIVDM,1,1,,B,6>jppp0000G?tO8p0@E=;0,0*1f",  // type 6
        "!AIVDM,1,1,,B,7>jppp0000G?tO8p0@E=;0,0*1e",  // type 7
        "!AIVDM,1,1,,B,8>jppp0000G?tO8p0@E=;0,0*11",  // type 8
        "!AIVDM,1,1,,B,9>jppp0000G?tO8p0@E=;0,0*10",  // type 9
        "!AIVDM,1,1,,B,:5N9p40001G?tO8p00000000000,0*23",  // type 10
        "!AIVDM,1,1,,B,;>jppp0000G?tO8p0@E=;0,0*12",   // type 11
        "!AIVDM,1,1,,B,<03Owpiuho;M7bO`E@0,0*70", // type 12
        "!AIVDM,1,1,,B,=03Owpiuho;M7bO`E@0,0*71", // type 13
        "!AIVDM,1,1,,B,>03Owpiuho;M7bO`E@0,0*72", // type 14
        "!AIVDM,1,1,,B,?03Owpiuho;M7bO`E@0,0*73", // type 15
        "!AIVDM,1,1,,B,@03Owpiuho;M7bO`E@0,0*0c", // type 16
        "!AIVDM,1,1,,B,A03Owpiuho;M7bO`E@0,0*0d", // type 17
        "!AIVDM,1,1,,B,B03Owpiuho;M7bO`E@0,0*0e", // type 18
        "!AIVDM,1,1,,B,C03Owpiuho;M7bO`E@0,0*0f", // type 19
        "!AIVDM,1,1,,B,D03Owpiuho;M7bO`E@0,0*08",  // type 20
        "!AIVDM,1,1,,A,E>l1h@0000G?tO8p0000000000000000,2*01", //type 21
        "!AIVDM,1,1,,A,F03Owpiuho;M7bO`E@0,0*09", //type 22
        "!AIVDM,1,1,,A,G03Owpiuho;M7bO`E@0,0*08", //type 23
        "!AIVDM,1,1,,B,H5NBsi@E=@000000000000000,0*06",  // type 24 A
        "!AIVDM,1,1,,B,H5NBsi@E=@D0000000000000,0*42",  // type 24 B
        "!AIVDM,1,1,,A,I03Owpiuho;M7bO`E@0,0*06", //type 25
        "!AIVDM,1,1,,A,J5Muq@p0000000000000000,0*00", //type 26
        "!AIVDM,1,1,,A,K03Owpiuho;M7bO`E@0,0*04", // type 27
        */

//test simulator
/*"!AIVDM,1,1,,A,26?UkTQOg`UQ2gh7KWd0I0OJ0000,0*XX",
        "!AIVDM,1,1,,A,31mg=5Ng8h78nB0fgtH0qQJb0000,0*XX",
        "!AIVDM,1,1,,A,1Amg=5A607Qh9g0Irsp2WQ6dc401,0*7B",
        */
/*
        "!AIVDO,1,1,,,14cwdlhP005=E<t:l>Ftowvf2000,0*11",
        "!AIVDM,1,1,,B,16@LkJ@000U=`1f:hjspoj0f00SB,0*59",
        "!AIVDM,2,1,1,A,56?UhB@2:U?<5HuV220d4pLp622222222222221?1Q2666tPN63EC@PB,0*6E",
        "!AIVDM,2,2,1,A,H88888888888888,2*5D",
        "!AIVDM,1,1,,B,86@Lf@P0G@:?>G1?6600,0*14",
        "!AIVDM,1,1,,B,B6@LlK@0GQCK;UReFJfI5r;P0D00,0*36",
        "!AIVDO,1,1,,C,Klcwdll2bbQHVPDT,0*4C",
        "!AIVDM,1,1,,A,16?Ul2@P005=r<8:mutawwvj0D4N,0*5E",
        "!AIVDM,1,1,,B,16?Ulg@0hvU<rOJ:Tw9=pK:J0@>L,0*67",
        "!AIVDM,1,1,,A,16?Uk5hP005=rHf:n04mw?vj04kD,0*3B",
        "!AIVDM,1,1,,A,9001BthEQfU8lsV;StoB6F0244kD,0*4A",
        "!AIVDO,1,1,,,14cwdlhP005=E<p:l>G<owvh2000,0*52",
        "!AIVDM,1,1,,A,13QpMR@P00U=Kf`:m9@f4?vj28>q,0*35",
        */

/*!AIVDM,1,1,,B,16?a7t30015=ReF:mnGmnWdj04kD,0*4B
        !AIVDM,1,1,,B,16@LgthP005=QI@:jR1v4?vj2@?4,0*7E
        !AIVDM,1,1,,A,H6@Ll956?>G1000pD;>j00000000,0*1E
        !AIVDM,1,1,,B,16?Um@001@5=A1J:jEeqV7bh0<8N,0*36
        !AIVDM,1,1,,A,36?Ulgiv@1U=Lv4:jAIhcHdR0Dg:,0*4E
        !AIVDM,1,1,,B,16?Un7P0005=run:mu>QlPlh0<48,0*65
        !AIVDM,1,1,,B,403wol1v`TVmJU=Chb:kwB302<91,0*24
        !AIVDM,1,1,,B,38KuCv5000U=stT:nJ`o1VJV0000,0*71
        !AIVDO,1,1,,,14cwdlhP005=E<n:l>G<owvj2000,0*4E
        !AIVDM,1,1,,A,17WK:@PP?w<tSF0l4Q@>4?wv1PSM,0*61
        !AIVDM,1,1,,A,16?UkFP000U=Jmp:m;0f47Ll20Rs,0*03
        !AIVDM,2,1,2,B,58KuCv02AF6SUKWGGR0l4E9<f0<50F1@uLr22217GhsK:4V>NHRCSThO,0*20
        !AIVDM,2,2,2,B,RCSEC`888888880,2*01
        !AIVDM,1,1,,B,16?aem0P005=QsH:mvJ<gwvj04kD,0*06
        !AIVDM,1,1,,A,16?UnGP0025=RNV:mw?niWdl0<4q,0*4C
        !AIVDO,1,1,,,14cwdlhP005=E<h:l>GLowvl2000,0*3E
        !AIVDM,1,1,,A,36@LkJ@000U=`1d:hjs`oB0n0200,0*79
        !AIVDM,1,1,,A,16?Un4P0005=rJN:mwhWUi@l0D3t,0*33
        !AIVDM,1,1,,B,16?UlNH000U=Ljh:mTS8@V6n0@@E,0*67
        !AIVDM,1,1,,A,169UTF000mU=es@:mRiqCWJn0hGM,0*43*/
//    };


// github strings
/*const char *arr_pNmea_string[] =
    {

        "!AIVDM,1,1,,A,15Mwd<PP00ISfGpA7jBr??vP0<3:,0*04",
        "!AIVDM,2,1,4,B,55MwW7P00001L@?;GS0<51B08Thj0TdpE800000P0hD556IE07RlSm6P0000,0*0B", //type 5
        "!AIVDM,1,1,,A,15Mv5v?P00IS0J`A86KTROvN0<5k,0*12",
        "!AIVDM,2,2,4,B,00000000000,2*23", //type 5
        "!AIVDM,1,1,,A,15N7th0P00ISsi4A5I?:fgvP2<40,0*06",
    "!AIVDM,1,1,,A,15NIEcP000ISrjPA8tEIBq<P089=,0*63",
    "!AIVDM,1,1,,B,15MuS0PP00IS00HA8gEtSgvN0<3U,0*61",
    "\\c:1609841515,s:r3669961*78\\!AIVDM,1,1,,A,13ukmN7@0<0pRcHPTkn4P33f0000,0*58",
    "\\c:1609841515,s:r3669961,g:1-2-1234*0E\\!AIVDM,1,1,,A,13ukmN7@0<0pRcHPTkn4P33f0000,0*58"
    };*/

/*
    U_AIS_MSG_FULL U_Msg;
    int iMsgid;

    int iCount = sizeof(arr_pNmea_string) / sizeof(arr_pNmea_string[0]);

    for (int i = 0; i < iCount; i++)
    {
        int iRet = ais_decode(arr_pNmea_string[i], &iMsgid, &U_Msg);

        if (iRet == -3)
            continue;   // waiting for next fragment

        if (iRet != 0)
            continue;   // error

        if (iMsgid >= 1 && iMsgid <= 3)
            ais_print_type123(&U_Msg.mSt123);

        else if (iMsgid == 4)
            ais_print_type4(&U_Msg.mSt4);

        else if (iMsgid == 5)
            ais_print_type5(&U_Msg.mSt5);

        else if (iMsgid == 6)
            ais_print_type6(&U_Msg.mSt6);

        else if (iMsgid == 7)
            ais_print_type7(&U_Msg.mSt7);

        else if (iMsgid == 8)
            ais_print_type8(&U_Msg.mSt8);

        else if (iMsgid == 9)
            ais_print_type9(&U_Msg.mSt9);

        else if (iMsgid == 10)
            ais_print_type10(&U_Msg.mSt10);

        else if (iMsgid == 11)
            ais_print_type11(&U_Msg.mSt11);

        else if (iMsgid == 12) ais_print_type12(&U_Msg.mSt12);
        else if (iMsgid == 13) ais_print_type13(&U_Msg.mSt13);
        else if (iMsgid == 14) ais_print_type14(&U_Msg.mSt14);
        else if (iMsgid == 15) ais_print_type15(&U_Msg.mSt15);
        else if (iMsgid == 16) ais_print_type16(&U_Msg.mSt16);
        else if (iMsgid == 17) ais_print_type17(&U_Msg.mSt17);
        else if (iMsgid == 18) ais_print_type18(&U_Msg.mSt18);
        else if (iMsgid == 19) ais_print_type19(&U_Msg.mSt19);
        else if (iMsgid == 20) ais_print_type20(&U_Msg.mSt20);
        else if (iMsgid == 21) ais_print_type21(&U_Msg.mSt21);
        else if (iMsgid == 22) ais_print_type22(&U_Msg.mSt22);
        else if (iMsgid == 23) ais_print_type23(&U_Msg.mSt23);
        else if (iMsgid == 24) ais_print_type24(&U_Msg.mSt24);
        else if (iMsgid == 25) ais_print_type25(&U_Msg.mSt25);
        else if (iMsgid == 26) ais_print_type26(&U_Msg.mSt26);
                else if (iMsgid == 27)
            ais_print_type27(&U_Msg.mSt27);
    }

    return 0;
}

*/
