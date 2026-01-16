#ifndef TABLE_H
#define TABLE_H

#include <stdio.h>

typedef struct Table Table;

/*
 * Create a table.
 * title: optional (NULL allowed)
 * ncols: number of columns (>0)
 * colnames: optional array of ncols strings (NULL means no names)
 */
Table *table_create(const char *title, int ncols, const char *colnames[]);

/* Add a row. values: array of ncols strings (NULL treated as empty string). */
int table_add_row(Table *t, const char *values[]);

/* Print table to provided stream (stdout, file, etc.). */
void table_print(Table *t, FILE *out);

/* Free all memory used by table. */
void table_free(Table *t);

#endif /* TABLE_H */
