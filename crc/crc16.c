#include <stdio.h>

#define TABLE_SIZE (256 * 256)

unsigned short crc_table[TABLE_SIZE];
const unsigned short generator = 0x1021;   /* generator polynomial for CRC 16 */

void gen_lookuptable()
{
        unsigned int i, j;
        unsigned short crc, msb;

        for (i = 0; i < TABLE_SIZE; i++) {
                crc = i;
                for (j = 0; j < 8; j ++) {
                        msb = crc & 0x8000;
                        crc = crc << 1;
                        if (msb) {
                                crc = crc ^ generator;
                        }
                }
                crc_table[i] = crc;
        }
}

/* Lookup table based solution */
unsigned short crc16_lookuptable(unsigned char *data, unsigned int len)
{
        unsigned short crc = 0;
        unsigned int i;

        for (i = 0; i < len; i++) {
                crc = crc ^ ((unsigned short)data[i] << 8);     /* XOR-in next input byte */
                crc = crc_table[(int)crc];      /* get CRC result based on lookup table */
        }

        return crc;
}

/* General CRC-16 bitwise implementation */
unsigned short crc16_simple(unsigned char *data, unsigned int len)
{
        unsigned short crc = 0, msb;
        unsigned int i, j;

        for (i = 0; i < len; i++) {
                crc = crc ^ ((unsigned short)data[i] << 8);     /* XOR-in next input byte */

                for (j = 0; j < 8; j++) {
                        msb = crc & 0x8000;
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

        printf("CRC16 (bitwise) of %d (0x%.2x%.2x%.2x%.2x) = 0x%.4x\n", input, data[0], data[1], data[2], data[3], crc16_simple(data, 4));
        printf("CRC16 (lookup table) of %d (0x%.2x%.2x%.2x%.2x) = 0x%.4x\n", input, data[0], data[1], data[2], data[3], crc16_lookuptable(data, 4));

        return 0;
}
