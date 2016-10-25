#include "utils.h"

/*Array struct functions*/

void initArray(Array *a, size_t initialSize) {

    if(initialSize <= 0)
    {
        printf("ERROR: Array size cannot be 0 or less.\n");
        return;
    }

    a->array = (unsigned char* )malloc(initialSize * sizeof(unsigned char));
    a->used = 0;
    a->size = initialSize;
}

void insertArray(Array *a, unsigned char element) {
  if (a->used == a->size) {
    a->size *= 2;   // double size if full
    a->array = (unsigned char*)realloc(a->array, a->size * sizeof(unsigned char));
  }
  a->array[a->used++] = element;
}

/* Puts a C array in a struct Array */
void copyArray(unsigned char* source, Array* destiny, size_t length)
{
	for(int i = 0; i < length; i++)
    {      
        insertArray(destiny, source[i]);
    }
}

void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}


void printHexArray(Array* array)
{
	for(unsigned int i = 0; i < array->used; i++)
	{
		printf ("0x%x ", array->array[i]);
	}
  printf ("\n");
}