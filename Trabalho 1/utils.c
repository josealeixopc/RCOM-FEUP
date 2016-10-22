#include "utils.h"


/** Check SET and UA state machines **/

int badSET(char* set)
{
	if (set[0] != FLAG)
		return -1;

	if (set[1] != A_SND)
		return -2;

	if (set[2] != C_SET)
		return -3;

	if (set[3] != (A_SND ^ C_SET))
		return -4;

	if (set[4] != FLAG)
		return -5;

	return 0;
}

int badUA(char *ua)
{
  if (ua[0] != FLAG)
    return -1;

  if (ua[1] != A_SND)
    return -2;

  if (ua[2] != C_UA)
    return -3;

  if (ua[3] != (A_SND^C_UA))
    return -4;

  if (ua[4] != FLAG)
    return -5;

  return 0;
}


/** Array struct functions **/

void initArray(Array *a, size_t initialSize) {
  a->array = (char* )malloc(initialSize * sizeof(char*));
  a->used = 0;
  a->size = initialSize;

  if(initialSize <= 0)
  {
      printf("ERROR: Array size cannot be 0 or less.\n");
  }
}

void insertArray(Array *a, char element) {
  if (a->used == a->size) {
    a->size *= 1;
    a->array = (char*)realloc(a->array, a->size * sizeof(char*));
  }
  a->array[a->used++] = element;
}

void copyArray(char* source, Array* destiny, size_t length)
{
	printf ("%d, ", 10);

	for(int i = 0; i < length; i++)
  {      
		printf ("%d, ", i);
  	insertArray(destiny, source[i]);
  }
}

void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

/** Print hex array function **/


int printHexArray(char* array, size_t length)
{
	for(unsigned int i = 0; i < length; i++)
	{
		printf ("0x%x ", array[i]);
	}
  printf ("\n");
}