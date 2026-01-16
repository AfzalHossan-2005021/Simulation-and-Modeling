#include "table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct Table
{
    char *title;
    int ncols;
    char **colnames; /* length ncols, can contain NULL */

    /* rows: dynamic array of row pointers. Each row is array of ncols char* strings */
    char ***rows;
    int nrows;
    int capacity;
};

/* Helper: safe strdup (portable) */
static char *xstrdup(const char *s)
{
    if (!s)
        return NULL;
    char *p = malloc(strlen(s) + 1);
    if (p)
        strcpy(p, s);
    return p;
}

Table *table_create(const char *title, int ncols, const char *colnames[])
{
    if (ncols <= 0)
        return NULL;
    Table *t = malloc(sizeof(Table));
    if (!t)
        return NULL;
    t->title = title ? xstrdup(title) : NULL;
    t->ncols = ncols;
    t->nrows = 0;
    t->capacity = 4;
    t->rows = malloc(sizeof(char **) * t->capacity);
    t->colnames = malloc(sizeof(char *) * ncols);
    for (int i = 0; i < ncols; ++i)
    {
        t->colnames[i] = colnames && colnames[i] ? xstrdup(colnames[i]) : NULL;
    }
    return t;
}

int table_add_row(Table *t, const char *values[])
{
    if (!t)
        return -1;
    if (t->nrows >= t->capacity)
    {
        int nc = t->capacity * 2;
        char ***newrows = realloc(t->rows, sizeof(char **) * nc);
        if (!newrows)
            return -1;
        t->rows = newrows;
        t->capacity = nc;
    }
    char **row = malloc(sizeof(char *) * t->ncols);
    if (!row)
        return -1;
    for (int i = 0; i < t->ncols; ++i)
    {
        row[i] = values && values[i] ? xstrdup(values[i]) : xstrdup("");
    }
    t->rows[t->nrows++] = row;
    return 0;
}

static int max_int(int a, int b) { return a > b ? a : b; }

void table_print(Table *t, FILE *out)
{
    if (!t || !out)
        return;
    /* Compute column widths */
    int *widths = malloc(sizeof(int) * t->ncols);
    for (int j = 0; j < t->ncols; ++j)
    {
        widths[j] = 0;
        if (t->colnames[j])
            widths[j] = (int)strlen(t->colnames[j]);
    }
    for (int i = 0; i < t->nrows; ++i)
    {
        for (int j = 0; j < t->ncols; ++j)
        {
            int len = (int)strlen(t->rows[i][j]);
            widths[j] = max_int(widths[j], len);
        }
    }

    /* Total line length for borders: 1 + sum(width[j] + 2 + 1) = 1 + sum(widths) + 3*ncols */
    int total_len = 1;
    for (int j = 0; j < t->ncols; ++j)
        total_len += widths[j] + 5;

    /* Print title centered in a bordered line if title exists */
    if (t->title)
    {
        /* top border */
        {
            fputc('+', out);
            for (int j = 0; j < t->ncols; ++j)
            {
                for (int k = 0; k < widths[j] + 4; ++k)
                    fputc('-', out);
                if (j < t->ncols - 1)
                    fputc('-', out);
                else
                    fputc('+', out);
            }
            fputc('\n', out);
        }
        int content_width = total_len - 2; /* excluding starting and ending '|' */
        /* print title line */
        fputc('|', out);
        int tlen = (int)strlen(t->title);
        int left = (content_width - tlen) / 2;
        int right = content_width - tlen - left;
        for (int i = 0; i < left; ++i)
            fputc(' ', out);
        fputs(t->title, out);
        for (int i = 0; i < right; ++i)
            fputc(' ', out);
        fputc('|', out);
        fputc('\n', out);
        /* separator after title */
        fputc('+', out);
        for (int j = 0; j < t->ncols; ++j)
        {
            for (int k = 0; k < widths[j] + 4; ++k)
                fputc('-', out);
            fputc('+', out);
        }
        fputc('\n', out);
    }

    /* Header row */
    fputc('|', out);
    for (int j = 0; j < t->ncols; ++j)
    {
        int w = widths[j];
        const char *label = t->colnames[j] ? t->colnames[j] : "";
        int len = (int)strlen(label);
        int left = (w - len) / 2;
        int right = w - len - left;
        fputc(' ', out);
        fputc(' ', out);
        for (int k = 0; k < left; ++k)
            fputc(' ', out);
        fputs(label, out);
        for (int k = 0; k < right; ++k)
            fputc(' ', out);
        fputc(' ', out);
        fputc(' ', out);
        fputc('|', out);
    }
    fputc('\n', out);

    /* header-data separator */
    fputc('+', out);
    for (int j = 0; j < t->ncols; ++j)
    {
        for (int k = 0; k < widths[j] + 4; ++k)
            fputc('-', out);
        fputc('+', out);
    }
    fputc('\n', out);

    /* data rows */
    for (int i = 0; i < t->nrows; ++i)
    {
        fputc('|', out);
        for (int j = 0; j < t->ncols; ++j)
        {
            int w = widths[j];
            const char *cell = t->rows[i][j] ? t->rows[i][j] : "";
            int len = (int)strlen(cell);
            int left = (w - len) / 2;
            int right = w - len - left;
            fputc(' ', out);
            fputc(' ', out);
            for (int k = 0; k < left; ++k)
                fputc(' ', out);
            fputs(cell, out);
            for (int k = 0; k < right; ++k)
                fputc(' ', out);
            fputc(' ', out);
            fputc(' ', out);
            fputc('|', out);
        }
        fputc('\n', out);
        /* row separator */
        fputc('+', out);
        for (int j = 0; j < t->ncols; ++j)
        {
            for (int k = 0; k < widths[j] + 4; ++k)
                fputc('-', out);
            fputc('+', out);
        }
        fputc('\n', out);
    }

    free(widths);
}

void table_free(Table *t)
{
    if (!t)
        return;
    free(t->title);
    if (t->colnames)
    {
        for (int j = 0; j < t->ncols; ++j)
            free(t->colnames[j]);
        free(t->colnames);
    }
    for (int i = 0; i < t->nrows; ++i)
    {
        for (int j = 0; j < t->ncols; ++j)
            free(t->rows[i][j]);
        free(t->rows[i]);
    }
    free(t->rows);
    free(t);
}
