#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// There are at least 7 memory management related bugs in this code
typedef struct
{
    int *data;
    int  length;
    int  capacity;
} Vec;

Vec *vec_new()
{
    Vec vec;
    vec.data     = NULL;
    vec.length   = 0;
    vec.capacity = 0;
    return &vec;
}

void vec_push(Vec *vec, int n)
{
    if (vec->length == vec->capacity)
    {
        int  new_capacity = vec->capacity * 2;
        int *new_data     = (int *)malloc(new_capacity);
        if (new_data == NULL)
            printf("Error couldn't allocate data");

        for (int i = 0; i < vec->length; ++i)
        {
            new_data[i] = vec->data[i];
        }

        vec->data     = new_data;
        vec->capacity = new_capacity;
    }

    vec->data[vec->length] = n;
    vec->length += 1;
}

void vec_free(Vec *vec)
{
    free(vec);
    free(vec->data);
}

void main()
{
    Vec *vec = vec_new();
    vec_push(vec, 107);

    int *n = &vec->data[0];
    vec_push(vec, 110);
    printf("%d\n", *n);

    free(vec->data);
    vec_free(vec);
}