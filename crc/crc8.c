#include <stdio.h>

unsigned char crc_table[256];
const unsigned char generator = 0x1d;   /* generator polynomial for CRC 8 */

void gen_lookuptable()
{
        unsigned i, j;
        unsigned char crc, msb;

        for (i = 0; i < 256; i++) {
                crc = i;
                for (j = 0; j < 8; j ++) {
                        msb = crc & 0x80;
                        crc = crc << 1;
                        if (msb) {
                                crc = crc ^ generator;
                        }
                }
                crc_table[i] = crc;
        }
}

/* Lookup table based solution */
unsigned char crc8_lookuptable(unsigned char *data, unsigned int len)
{
        unsigned char crc = 0;
        unsigned int i;

        for (i = 0; i < len; i++) {
                crc = crc ^ data[i];    /* XOR-in next input byte */
                crc = crc_table[(int)crc];      /* get CRC result based on lookup table */
        }

        return crc;
}

/* General CRC-8 bitwise implementation */
unsigned char crc8_simple(unsigned char *data, unsigned int len)
{
        unsigned char crc = 0, msb;
        unsigned int i, j;

        for (i = 0; i < len; i++) {
                crc = crc ^ data[i];    /* XOR-in next input byte */
                for (j = 0; j < 8; j++) {
                        msb = crc & 0x80;
                        crc = crc << 1; /* left shift */

                        if (msb) {      /* XOR crc if MSB == 1 */
                                crc = crc ^ generator;
                        }
                }
        }

        return crc;
}

int main(int argc, char **argv)
{
        int input;
        unsigned char data[4] = {0, 0, 0, 0};

        if (argc != 2) {
                printf("Usage: %s [int]\n", argv[0]);
                return 0;
        }

        input = atoi(argv[1]);
        data[0] = (input & 0xff000000) >> 24;
        data[1] = (input & 0x00ff0000) >> 16;
        data[2] = (input & 0x0000ff00) >> 8;
        data[3] = input & 0x000000ff;

        gen_lookuptable();      /* generate lookup table */

        printf("CRC8 (bitwise) of %d (0x%.2x%.2x%.2x%.2x) = %d\n", input, data[0], data[1], data[2], data[3], crc8_simple(data, 4));
        printf("CRC8 (lookup table) of %d (0x%.2x%.2x%.2x%.2x) = %d\n", input, data[0], data[1], data[2], data[3], crc8_lookuptable(data, 4));

        return 0;
}
