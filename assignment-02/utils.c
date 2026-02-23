#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *int_to_string(int value) /* Convert integer to string. */
{
    int size = 128;
    char *str = (char *)malloc(size * sizeof(char)); /* Enough for 32-bit int */
    snprintf(str, size, "%d", value);
    return str;
}

char *float_to_string(float value) /* Convert float to string. */
{
    int size = 128;
    char *str = (char *)malloc(size * sizeof(char)); /* Enough for float */
    snprintf(str, size, "%.3f", value);
    return str;
}

char *array_to_string(float arr[], int n) /* Convert array to string. */
{
    int size = 1024;
    char *str = (char *)malloc(size * sizeof(char));
    int pos = 0;
    pos += snprintf(str + pos, size - pos, "[");
    for (int i = 1; i <= n && pos < size - 16; ++i)
        pos += snprintf(str + pos, size - pos, "%.3f%s", arr[i], (i < n) ? ", " : "");
    pos += snprintf(str + pos, size - pos, "]");
    return str;
}

char *pair_to_string(int smalls, int bigs) /* Convert pair to string. */
{
    int size = 128;
    char *str = (char *)malloc(size * sizeof(char));
    snprintf(str, size, "(%3d,%3d)", smalls, bigs);
    return str;
}

char *range_to_string(float min, float max) /* Convert range to string. */
{
    int size = 128;
    char *str = (char *)malloc(size * sizeof(char));
    snprintf(str, size, "%.2f to %.2f", min, max);
    return str;
}

char *concat_strings(const char *s1, const char *s2)
{
    int size = strlen(s1) + strlen(s2) + 1;
    char *str = (char *)malloc(size * sizeof(char));
    snprintf(str, size, "%s%s", s1, s2);
    return str;
}
