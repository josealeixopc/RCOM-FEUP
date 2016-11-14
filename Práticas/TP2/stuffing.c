#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include "utils.h"

#define DEBUG 1

typedef struct {
  unsigned char* array;
  size_t used;
  size_t size;
} Array;

void initArray(Array *a, size_t initialSize) {
  a->array = (unsigned char* )malloc(initialSize * sizeof(unsigned char*));
  a->used = 0;
  a->size = initialSize;

  if(initialSize <= 0)
  {
      printf("ERROR: Array size cannot be 0 or less.\n");
  }
}

void insertArray(Array *a, unsigned char element) {
  if (a->used == a->size) {
    a->size *= 1;
    a->array = (unsigned char*)realloc(a->array, a->size * sizeof(unsigned char*));
  }
  a->array[a->used++] = element;
}

void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

/*
    Returns the number of bytes that was either a FLAG or a ESCAPE and has been altered.
*/
int byteStuff(Array* inArray, Array* outArray)
{
    int count = 0;

    initArray(outArray, 1); // to reset the array, so information can't be corrupted

    for(int i = 0; i < inArray->used; i++)
    {
        if (inArray->array[i] == FLAG)
        {
            insertArray(outArray, ESCAPE);
            insertArray(outArray, (FLAG^XOR_BYTE));
            count++;
        }

        else if (inArray->array[i] == ESCAPE)
        {
            insertArray(outArray, ESCAPE);
            insertArray(outArray, (ESCAPE^XOR_BYTE));
            count++;
        }

        else
        {
            insertArray(outArray, inArray->array[i]);
        }
    }

    if(DEBUG)
    {
        printf("%d bytes altered.\n", count);
    }

    return count;
}

int byteUnstuff(Array* inArray, Array* outArray)
{
    int count = 0;

    initArray(outArray, 1);

    for(int i = 0; i < inArray->used; i++)
    {
        if ((inArray->array[i] == ESCAPE) && (inArray->array[i+1] == (FLAG^XOR_BYTE)))
        {
            insertArray(outArray, FLAG);
           
            i++; // to skip next byte evaluation
            count++;
        }

        else if ((inArray->array[i] == ESCAPE) && (inArray->array[i+1] == (ESCAPE^XOR_BYTE)))
        {
            insertArray(outArray, ESCAPE);
            
            i++;
            count++;
        }

        else
        {
            insertArray(outArray, inArray->array[i]);
        }
    }

    if(DEBUG)
    {
        printf("%d bytes altered.\n", count);
    }

    return count;
}

int main()
{
    Array in, out;

    initArray(&in, 1);
    initArray(&out, 1);

    insertArray(&in, FLAG);
    insertArray(&in, ESCAPE);

    byteStuff(&in, &out);

    if(DEBUG)
    {
        printf("In (length = %lu): 0x%x, 0x%x, 0x%x\n", in.used, in.array[0], in.array[1], in.array[2]);
        printf("Out (length = %lu): 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", out.used, out.array[0], out.array[1], out.array[2], out.array[3], out.array[4]);
    }

    byteUnstuff(&out, &in);

    if(DEBUG)
    {
        printf("In (length = %lu): 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", out.used, out.array[0], out.array[1], out.array[2], out.array[3], out.array[4]);
        printf("Out (length = %lu): 0x%x, 0x%x, 0x%x\n", in.used, in.array[0], in.array[1], in.array[2]);
    }

    freeArray(&in);
    freeArray(&out);

    return 0;
}