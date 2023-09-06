#include<p30fxxxx.h>
#include "fotootpornik.h"

unsigned int sirovi;

int fotootpornik(sirovi)
{
    if(sirovi<3850)
    {
        return 0;
    }  
    else
    {
        return 1;
    }
}
