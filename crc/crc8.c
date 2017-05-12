#include <stdio.h>

unsigned char crc8_simple(unsigned char *data, unsigned int len)
{
        const unsigned char generator = 0x1d; /* generator polynomial for CRC 8 */
        unsigned char crc = 0, msb;
        unsigned int i, j;

        for (i = 0; i < len; i++) {
                crc = (crc ^ data[i]);
                for (j = 0; j < 8; j++) {
                        msb = crc & 0x80;
                        crc = (crc << 1);       /* left shift */

                        if (msb) {      /* XOR crc if MSB == 1 */
                                crc = (crc ^ generator);
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

        printf("CRC8 value of %d (0x%.2x%.2x%.2x%.2x) = %d\n", input, data[0], data[1], data[2], data[3], crc8_simple(data, 4));
        return 0;
}
