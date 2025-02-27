/* mean.c: Implementation of a mean function of testable component.
   See test/test_mean.c for the associated unit test.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "example.h"


/**
 * @brief Calculates the mean of an array of integers.
 *
 * This function takes an array of integers and its count, then calculates
 * and returns the mean (average) of the values in the array.
 *
 * @param values A pointer to an array of integers.
 * @param count The number of elements in the array.
 * @return The mean of the values in the array as an integer.
 */
int testable_mean(const int* values, int count)
{
    if (count == 0) {
        return 0;
    }
    int sum = 0;
    for (int i = 0; i < count; ++i) {
        sum += values[i];
    }
    return sum / count;
}
