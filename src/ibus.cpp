#include "ibus.h"

//  https://github.com/betaflight/betaflight/wiki/Single-wire-FlySky-%28IBus%29-telemetry

#define PI 3.1415926535897932384626433832795f

#define IBUS_TELEMETRY_PACKET_LENGTH  4
#define IBUS_SERVO_PACKET_LENGTH     32

#define IBUS_DISCOVER_SENSOR    0x80
#define IBUS_SENSOR_TYPE        0x90
#define IBUS_SENSOR_MEASUREMENT 0xa0

#define SENSOR_ID_HEADING     0x08 // Compass heading, two bytes in degrees
#define SENSOR_ID_COG         0x0a // Two bytes degrees * 100, unknown max uint
#define SENSOR_ID_GPS_DIST    0x14 // Two bytes dist in meters from home
#define SENSOR_ID_SPEED       0x13 // Two bytes m/s * 100
#define SENSOR_ID_GPS_STATUS  0x0b // Two bytes
#define SENSOR_ID_GPS_LAT     0x80 // Four bytes signed WGS84 in degrees * 1E7
#define SENSOR_ID_GPS_LON     0x81 // Four bytes signed WGS84 in degrees * 1E7
#define SENSOR_ID_MODE        0x16 // Two bytes. First is GPS_STATUS, second is number of satellites.

#define FLIGHT_MODE_MANUAL 1 // Acro
#define FLIGHT_MODE_HOLD   7 // Circle

#define GPS_STATUS_BAD  0x01 // No GPS fix.
#define GPS_STATUS_LOW  0x02 // 4 satellites or less
#define GPS_STATUS_GOOD 0x03 // 5 or more

void Ibus::PutRxByte(uint8_t byte) {
    /*
     * Ensure garbled messages don't run away with the buffer.
     */
    if (m_length > 60) {
        m_length = 0;
    }

    /*
     * Append the new byte to the end of the already received ones.
     */
    m_rx_buffer[m_length++] = byte;

    /*
     * Have we received a whole message? Try to parse what we have.
     *
     * The first byte is the packet length. It should be 32 (for outgoing servo data) or 4 (for telemetry query).
     */
    if (m_rx_buffer[0] != IBUS_TELEMETRY_PACKET_LENGTH && m_rx_buffer[0] != IBUS_SERVO_PACKET_LENGTH) {
        /*
         * This is not a valid first byte. Start again.
         */
        m_length = 0;
        return;
    }

    /*
     * Do we have as many bytes as the packet length byte state?
     */
    if (m_length < m_rx_buffer[0]) {
        /*
         * We do not. Wait for more bytes.
         */
        return;
    }

    /*
     * What kind of packet is this?
     */
    if (m_rx_buffer[0] == IBUS_SERVO_PACKET_LENGTH) {
        ProcessServoPacket(m_rx_buffer, m_length);
    } else if (m_rx_buffer[0] == IBUS_TELEMETRY_PACKET_LENGTH) {
        ProcessTelemetryPacket(m_rx_buffer, m_length);
    }

    m_length = 0;
}

void Ibus::ProcessServoPacket(uint8_t *buffer, uint16_t length) {
    for (int i = 0; i < MAX_NUMBER_OF_CHANNELS; i++) {
        m_servo[i] = buffer[i * 2 + 2] | buffer[i * 2 + 1 + 2] << 8;
    }
    m_servo_valid = true;
}

#define HEADING_ADDRESS       1
#define COG_ADDRESS           2
#define SPEED_ADDRESS         3
#define DIST_ADDRESS          4
#define GPS_LATITUDE_ADDRESS  5
#define GPS_LONGITUDE_ADDRESS 6

void Ibus::ProcessTelemetryPacket(uint8_t *buffer, uint16_t length) {
    uint8_t address = m_rx_buffer[1] & 0x0f;
    uint8_t command = m_rx_buffer[1] & 0xf0;

    switch (command) {
        case IBUS_DISCOVER_SENSOR:
            if (address == GPS_LATITUDE_ADDRESS || address == GPS_LONGITUDE_ADDRESS || address == HEADING_ADDRESS ||
                address == COG_ADDRESS || address == SPEED_ADDRESS || address == DIST_ADDRESS) {
                Send((uint8_t) (IBUS_DISCOVER_SENSOR | address));
            }
            break;

        case IBUS_SENSOR_TYPE:
            if (address == GPS_LATITUDE_ADDRESS) {
                uint8_t buffer[] = {(uint8_t) (IBUS_SENSOR_TYPE | address), SENSOR_ID_GPS_LAT, 4};
                Send(buffer, 3);
            } else if (address == GPS_LONGITUDE_ADDRESS) {
                uint8_t buffer[] = {(uint8_t) (IBUS_SENSOR_TYPE | address), SENSOR_ID_GPS_LON, 4};
                Send(buffer, 3);
            } else if (address == HEADING_ADDRESS) {
                uint8_t buffer[] = {(uint8_t) (IBUS_SENSOR_TYPE | address), SENSOR_ID_HEADING, 2};
                Send(buffer, 3);
            } else if (address == COG_ADDRESS) {
                uint8_t buffer[] = {(uint8_t) (IBUS_SENSOR_TYPE | address), SENSOR_ID_COG, 2};
                Send(buffer, 3);
            } else if (address == SPEED_ADDRESS) {
                uint8_t buffer[] = {(uint8_t) (IBUS_SENSOR_TYPE | address), SENSOR_ID_SPEED, 2};
                Send(buffer, 3);
            } else if (address == DIST_ADDRESS) {
                uint8_t buffer[] = {(uint8_t) (IBUS_SENSOR_TYPE | address), SENSOR_ID_GPS_DIST, 2};
                Send(buffer, 3);
            }
            break;

        case IBUS_SENSOR_MEASUREMENT:
            uint8_t buffer[5] = {(uint8_t) (IBUS_SENSOR_MEASUREMENT | address)};
            if (address == GPS_LATITUDE_ADDRESS) {
                int32_t lat = (m_position_valid ? m_latitude : (double)0.0) * (double) (180.0 / PI * 10000000.0);
                buffer[1] = (lat & 0x000000ff) >> 0;
                buffer[2] = (lat & 0x0000ff00) >> 8;
                buffer[3] = (lat & 0x00ff0000) >> 16;
                buffer[4] = (lat & 0xff000000) >> 24;
                Send(buffer, 5);
            } else if (address == GPS_LONGITUDE_ADDRESS) {
                int32_t lon = (m_position_valid ? m_longitude : (double)0.0) * (double) (180.0 / PI * 10000000.0);
                buffer[1] = (lon & 0x000000ff) >> 0;
                buffer[2] = (lon & 0x0000ff00) >> 8;
                buffer[3] = (lon & 0x00ff0000) >> 16;
                buffer[4] = (lon & 0xff000000) >> 24;
                Send(buffer, 5);
            } else if (address == HEADING_ADDRESS) {
                uint16_t heading = (m_heading_valid ? m_heading : 0.0) * 180.0f / PI;
                buffer[1] = (heading & (uint16_t) 0x00ff) >> 0;
                buffer[2] = (heading & (uint16_t) 0xff00) >> 8;
                Send(buffer, 3);
            } else if (address == COG_ADDRESS) {
                if (m_cog_valid) {
                    uint16_t cog = (m_cog * 180.0f / PI) * 100;
                    buffer[1] = (cog & (uint16_t) 0x00ff) >> 0;
                    buffer[2] = (cog & (uint16_t) 0xff00) >> 8;
                } else {
                    buffer[1] = 0xff;
                    buffer[2] = 0xff;
                }
                Send(buffer, 3);
            } else if (address == SPEED_ADDRESS) {
                uint16_t speed = (m_speed_valid ? m_speed : 0.0) * 100.0f;
                buffer[1] = (speed & (uint16_t) 0x00ff) >> 0;
                buffer[2] = (speed & (uint16_t) 0xff00) >> 8;
                Send(buffer, 3);
            } else if (address == DIST_ADDRESS) {
                uint16_t distance = m_distance_valid ? m_distance : 0.0;
                buffer[1] = (distance & (uint16_t) 0x00ff) >> 0;
                buffer[2] = (distance & (uint16_t) 0xff00) >> 8;
                Send(buffer, 3);
            }
            break;
    }
}

void Ibus::Send(uint8_t byte) {
    uint8_t buffer[] = {byte};
    Send(buffer, 1);
}

void Ibus::Send(uint8_t *buffer, uint16_t length) {
    uint8_t packetLength = length + 3;
    uint16_t checksum = 0xffff - packetLength;
    for (int i = 0; i < length; i++) {
        checksum -= buffer[i];
    }
    m_usart->Write(&packetLength, 1);
    m_usart->Write(buffer, length);
    m_usart->Write((uint8_t *) &checksum, 2);
}