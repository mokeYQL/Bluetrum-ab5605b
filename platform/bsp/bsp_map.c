#include "include.h"

#if BT_MAP_EN

#define MAX_RETRY_CNT    20

int nibble_for_char(char c);
void bt_map_start(void);
uint32_t big_endian_read_16( const uint8_t * buffer, int pos);

static uint8_t map_retry_cnt;

static uint16_t get_number(uint8_t *ptr, uint8_t start, uint8_t *end)
{
    uint8_t pos = 0;
    uint16_t number = 0;

    while (ptr[start + pos] != '/') {
        number *= 10;
        number += nibble_for_char(ptr[start + pos]);
        pos++;
    }

    *end = start + pos;
    return number;
}

void bt_get_time(char *ptr, uint32_t len, uint8_t format)
{
#if UART0_PRINTF_SEL
    u16 year = 0;
    u8 month = 0;
    u8 day = 0;
    u8 hour = 0;
    u8 min = 0;
    u8 sec = 0;

    my_printf("-->set time\n");
    if (format == 0) {
        year = nibble_for_char(ptr[0])*1000 + nibble_for_char(ptr[1])*100 + nibble_for_char(ptr[2])*10 + nibble_for_char(ptr[3]);
        month = nibble_for_char(ptr[4])*10 + nibble_for_char(ptr[5]);
        day = nibble_for_char(ptr[6])*10 + nibble_for_char(ptr[7]);
        hour = nibble_for_char(ptr[9])*10 + nibble_for_char(ptr[10]);
        min = nibble_for_char(ptr[11])*10 + nibble_for_char(ptr[12]);
        sec = nibble_for_char(ptr[13])*10 + nibble_for_char(ptr[14]);
    } else if (format == 1) {
        uint8_t start_pos = 0;
        uint8_t end_pos;

        year = 2000 + get_number((uint8_t *)ptr, start_pos, &end_pos);
        start_pos = end_pos + 1;
        month = get_number((uint8_t *)ptr, start_pos, &end_pos);
        start_pos = end_pos + 1;
        day = get_number((uint8_t *)ptr, start_pos, &end_pos);
        start_pos = end_pos + 1;
        hour = get_number((uint8_t *)ptr, start_pos, &end_pos);
        start_pos = end_pos + 1;
        min = get_number((uint8_t *)ptr, start_pos, &end_pos);
        start_pos = end_pos + 1;
        sec = get_number((uint8_t *)ptr, start_pos, &end_pos);
    }

    if ((year >= 2000) && (year <= 3000) && (month <= 12) && (day <= 31) && (hour <= 24) && (min <= 60) && sec <= 60) {
        printf("get time:\n");
        printf("date:%04d.%02d.%02d time:%02d:%02d:%02d\n",year,month,day,hour,min,sec);
    }
#endif
}

void bt_map_data_callback(uint8_t *packet)     //获取时间例程
{
    bool time_find = false;
    uint8_t time_len;
    uint8_t *ptr;
    uint16_t packet_len;

    packet_len = big_endian_read_16(packet, 1);
    if (packet_len <= 3) {
        return;
    }

    packet_len -= 3;
    ptr = packet + 3;

    while (packet_len) {
        uint8_t header_id;
        uint16_t section_len;

        header_id = ptr[0];
        if (header_id == 0x4c) {
            section_len = big_endian_read_16(ptr, 1);
            if (section_len <= 3) {
                break;
            }
            section_len -= 3;
            ptr += 3;
            while (section_len) {
                uint8_t tag_id;
                uint8_t tag_len;

                tag_id = ptr[0];
                tag_len = ptr[1];
                if (tag_id == 0x19) {
                    ptr += 2;
                    time_len = tag_len;
                    time_find = true;
                    break;
                }
                if (section_len >= tag_len + 2) {
                    section_len -= tag_len + 2;
                    ptr += tag_len + 2;
                } else {
                    section_len = 0;
                }
            }
            break;
        } else if (header_id == 0xcb) {
            section_len = 5;
        } else {
            break;
        }

        if (packet_len >= section_len) {
            packet_len -= section_len;
            ptr += section_len;
        } else {
            packet_len = 0;
        }
    }

    if (time_find) {
        map_retry_cnt = 0;
        bt_get_time((char *)ptr, time_len, 0);
    } else {
        map_retry_cnt++;
        if (map_retry_cnt > MAX_RETRY_CNT) {
            map_retry_cnt = 0;
        } else {
            bt_map_start();
        }
    }
}
#endif

