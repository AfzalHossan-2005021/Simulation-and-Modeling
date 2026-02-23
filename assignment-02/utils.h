#ifndef UTILS_H
#define UTILS_H

/* Convert integer to string. Caller is responsible for freeing the returned string. */
char *int_to_string(int value);

/* Convert float to string. Caller is responsible for freeing the returned string. */
char *float_to_string(float value);

/* Convert array to string. Caller is responsible for freeing the returned string. */
char *array_to_string(float arr[], int n);

/* Convert pair to string. Caller is responsible for freeing the returned string. */
char *pair_to_string(int smalls, int bigs);

/* Convert range to string. Caller is responsible for freeing the returned string. */
char *range_to_string(float min, float max);

/* Concatenate two strings. Caller is responsible for freeing the returned string. */
char *concat_strings(const char *s1, const char *s2);

#endif /* UTILS_H */