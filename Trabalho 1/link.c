#include "link.h"

/******************** VERIFYING COMMAND FUNCTIONS *******************/

int badSET(unsigned char* set)
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

int badUA(unsigned char *ua)
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

/******************** STUFFING FUNCTIONS *******************/

int byteStuff(Array* inArray, Array* outArray)
{
    int count = 0;

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

    return count;
}

int byteUnstuff(Array* inArray, Array* outArray)
{
    int count = 0;

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

    return count;
}

/******************** SEND & RECEIVE FUNCTIONS *******************/



