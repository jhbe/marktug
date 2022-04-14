#include "string.h"
#include "gps.h"

void Gps::PutRxByte(uint8_t byte) {
    /*
     * Ensure we don't overwrite the buffer.
     */
    if (m_bufferLength >= GPS_BUFFER_LENGTH) {
        m_bufferLength = 0;
    }

    /*
     * Are we looking at the beginning of a new sentence?
     */
    if (byte == '$') {
        m_bufferLength = 0;
    }

    /*
     * Add the new character to the buffer.
     */
    m_buffer[m_bufferLength++] = byte;

    /*
     * If this is not the *, then there's nothing more to do.
     */
    if (byte != '*') {
        return;
    }

    /*
     * Is this a GLL sentence?
     */
    if (m_buffer[0] == '$'
        && m_buffer[3] == 'G'
        && m_buffer[4] == 'L'
        && m_buffer[5] == 'L') {
        ProcessGLL();
    } else if (m_buffer[0] == '$'
               && m_buffer[3] == 'G'
               && m_buffer[4] == 'S'
               && m_buffer[5] == 'A') {
        ProcessGSA();
    }
}

void Gps::ProcessGLL() {
    /*
     * Latitude starts at offset 7.
     */
    int index = 7;

    /*
     * We assume the sentence will not contain a valid position.
     */
    m_positionIsValid = false;

    /*
     * Extract the latitude.
     */
    double latitude;
    if (!RadianOf(index, false, latitude)) {
        return;
    }

    if (m_buffer[index++] != ',') {
        return;
    }

    if (m_buffer[index] == 'S') {
        latitude *= -1;
    } else if (m_buffer[index] != 'N') {
        return;
    }
    index++;

    if (m_buffer[index++] != ',') {
        return;
    }

    /*
     * Extract the longitude.
     */
    double longitude;
    if (!RadianOf(index, true, longitude)) {
        return;
    }

    if (m_buffer[index++] != ',') {
        return;
    }

    if (m_buffer[index] == 'W') {
        latitude *= -1;
    } else if (m_buffer[index] != 'E') {
        return;
    }
    index++;

    if (m_buffer[index++] != ',') {
        return;
    }

    /*
     * Skip the time.
     */
    while (m_buffer[index] != ',') {
        index++;
    }

    if (m_buffer[index++] != ',') {
        return;
    }

    /*
     * Is the position valid or not?
     */
    if (m_buffer[index] != 'A') {
        /*
         * Is it not.
         */
        return;
    }

    m_position.Set(latitude, longitude);
    m_positionIsValid = true;
}

void Gps::ProcessGSA() {
    /*
     * The fix mode is at offset 9.
     */
    m_has_lock = m_buffer[9] == '2' || m_buffer[9] == '3';
}

bool Gps::RadianOf(int &index, bool threeDigitDegrees, double &value) {
    value = 0.0;
    if (threeDigitDegrees) {
        if (IsDigit(m_buffer[index])) {
            value += ToRadian(100.0 * (m_buffer[index] - '0'));
        } else if (m_buffer[index] == ',') {
            return false;
        }
        index++;
    }
    if (IsDigit(m_buffer[index])) {
        value += ToRadian(10.0 * (m_buffer[index] - '0'));
    } else if (m_buffer[index] == ',') {
        return false;
    }
    index++;
    if (IsDigit(m_buffer[index])) {
        value += ToRadian(m_buffer[index] - '0');
    } else if (m_buffer[index] == ',') {
        return false;
    }
    index++;
    if (IsDigit(m_buffer[index])) {
        value += ToRadian(10.0 * (m_buffer[index] - '0') / 60.0);
    } else if (m_buffer[index] == ',') {
        return false;
    }
    index++;
    if (IsDigit(m_buffer[index])) {
        value += ToRadian((m_buffer[index] - '0') / 60.0);
    } else if (m_buffer[index] == ',') {
        return false;
    }
    index++;
    if (m_buffer[index] != '.') {
        return false;
    }
    index++;
    if (IsDigit(m_buffer[index])) {
        value += ToRadian((m_buffer[index] - '0') / 600.0);
    } else if (m_buffer[index] == ',') {
        return false;
    }
    index++;
    if (IsDigit(m_buffer[index])) {
        value += ToRadian((m_buffer[index] - '0') / 6000.0);
        index++;
    }
    if (IsDigit(m_buffer[index])) {
        value += ToRadian((m_buffer[index] - '0') / 60000.0);
        index++;
    }
    if (IsDigit(m_buffer[index])) {
        value += ToRadian((m_buffer[index] - '0') / 600000.0);
        index++;
    }
    if (IsDigit(m_buffer[index])) {
        value += ToRadian((m_buffer[index] - '0') / 6000000.0);
        index++;
    }
    if (IsDigit(m_buffer[index])) {
        value += ToRadian((m_buffer[index] - '0') / 60000000.0);
        index++;
    }
    return true;
}


/*
          0 24: $GNRMC,,V,,,,,,,,,,N*4D
         1 24: $GNRMC,,V,,,,,,,,,,N*4D
         2 20: $GNVTG,,,,,,,,,N*2E
         3 44: $GNGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*2E
         4 44: $GNGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*2E
         5 17: $GLGSV,1,1,00*65
         6 19: $GNGLL,,,,,,V,N*7A
         7 24: $GNRMC,,V,,,,,,,,,,N*4D
         8 20: $GNVTG,,,,,,,,,N*2E
         9 44: $GNGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*2E
        10 44: $GNGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*2E
        11 17: $GLGSV,1,1,00*65
        12 19: $GNGLL,,,,,,V,N*7A
        13 24: $GNRMC,,V,,,,,,,,,,N*4D
        14 20: $GNVTG,,,,,,,,,N*2E
        15 44: $GNGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*2E
        16 44: $GNGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*2E
        17 17: $GLGSV,1,1,00*65
        18 19: $GNGLL,,,,,,V,N*7A
        19 24: $GNRMC,,V,,,,,,,,,,N*4D
        20 20: $GNVTG,,,,,,,,,N*2E
^C




      67015 59: $GNGSA,A,3,17,30,24,06,13,01,15,19,14,,,,1.44,0.77,1.22*13
     67016 47: $GNGSA,A,3,81,82,73,,,,,,,,,,1.44,0.77,1.22*1B
     67017 52: $GLGSV,3,3,11,82,51,127,20,83,57,034,,84,07,352,*5C
     67018 51: $GNGLL,3454.11942,S,13829.77948,E,070233.00,A,A*62
     67019 34: $GNVTG,,T,,M,0.339,N,0.628,K,A*38
     67020 59: $GNGSA,A,3,17,30,24,06,13,01,15,19,14,,,,1.73,0.89,1.49*1B
     67021 47: $GNGSA,A,3,81,82,73,,,,,,,,,,1.73,0.89,1.49*13
     67022 52: $GLGSV,3,3,11,82,51,127,20,83,57,034,,84,07,352,*5C
     67023 51: $GNGLL,3454.11924,S,13829.77929,E,070234.00,A,A*62
     67024 34: $GNVTG,,T,,M,0.465,N,0.861,K,A*35
     67025 59: $GNGSA,A,3,17,30,24,06,13,01,15,19,14,,,,1.44,0.77,1.22*13
     67026 47: $GNGSA,A,3,81,82,73,,,,,,,,,,1.44,0.77,1.22*1B
     67027 52: $GLGSV,3,3,11,82,51,127,19,83,57,034,,84,07,352,*56
     67028 51: $GNGLL,3454.11918,S,13829.77932,E,070235.00,A,A*66
     67029 34: $GNVTG,,T,,M,0.223,N,0.413,K,A*38
     67030 59: $GNGSA,A,3,17,30,24,06,13,01,15,19,14,,,,1.44,0.77,1.22*13
     67031 47: $GNGSA,A,3,81,82,73,,,,,,,,,,1.44,0.77,1.22*1B
     67032 52: $GLGSV,3,3,11,82,51,127,19,83,57,034,,84,07,352,*56
     67033 51: $GNGLL,3454.11922,S,13829.77938,E,070236.00,A,A*66
     67034 34: $GNVTG,,T,,M,0.585,N,1.083,K,A*3F
     67035 57: $GNGSA,A,3,17,30,24,06,13,01,15,19,,,,,1.47,0.81,1.23*1D
     67036 47: $GNGSA,A,3,81,82,73,,,,,,,,,,1.47,0.81,1.23*10
     67037 52: $GLGSV,3,3,11,82,51,127,20,83,57,034,,84,07,352,*5C
     67038 51: $GNGLL,3454.11910,S,13829.77953,E,070237.00,A,A*6B
     67039 34: $GNVTG,,T,,M,0.783,N,1.450,K,A*31
     67040 57: $GNGSA,A,3,17,30,24,06,13,01,15,19,,,,,1.47,0.81,1.23*1D
     67041 47: $GNGSA,A,3,81,82,73,,,,,,,,,,1.47,0.81,1.23*10
     67042 52: $GLGSV,3,3,11,82,51,127,22,83,57,034,,84,07,352,*5E
     67043 51: $GNGLL,3454.11899,S,13829.77914,E,070238.00,A,A*67
     67044 34: $GNVTG,,T,,M,0.459,N,0.849,K,A*30
     67045 55: $GNGSA,A,3,17,30,24,06,13,15,19,,,,,,1.61,0.84,1.38*17
     67046 47: $GNGSA,A,3,81,82,73,,,,,,,,,,1.61,0.84,1.38*1B
     67047 63: $GPGSV,3,1,12,01,07,143,,02,08,002,,06,25,033,19,11,08,009,*78
     67048 52: $GLGSV,3,3,11,82,51,127,22,83,57,034,,84,07,352,*5E
     67049 51: $GNGLL,3454.11904,S,13829.77921,E,070239.00,A,A*65
     67050 34: $GNVTG,,T,,M,0.870,N,1.611,K,A*35
     67051 55: $GNGSA,A,3,17,30,24,06,13,15,19,,,,,,1.61,0.84,1.38*17
     67052 47: $GNGSA,A,3,81,82,73,,,,,,,,,,1.61,0.84,1.38*1B
     67053 52: $GLGSV,3,3,11,82,51,127,22,83,57,034,,84,07,352,*5E
     67054 51: $GNGLL,3454.11896,S,13829.77959,E,070240.00,A,A*6E

 */