#include <stdint.h>
#include "utility.h"


void bubble_sort(int32_t *arr, uint8_t n) {
    uint8_t i, j;
    int32_t temp;

    for (i = 0; i < n - 1; i++)
    {
        for (j = 0; j < n - i - 1; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}
