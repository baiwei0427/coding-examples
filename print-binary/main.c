/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>

using namespace std;

void print_binary(unsigned char *array, int size)
{
    unsigned char mask;
    
    for (int i = 0; i < size; i++) {
        for (mask = 0x01; mask != 0x00; mask <<= 1) {
            if (int(array[i] & mask) != 0) {
                cout << 1;
            } else {                
                cout << 0;
            }
        }
        cout << " ";        
    }    
}

int main()
{
    unsigned char str[] = {0x08, 0x04, 0x02, 0x01};
    print_binary(str, sizeof(str) / sizeof(unsigned char));
    return 0;
}



