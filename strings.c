/***************************************************************************
                                 .ooooo.          
                                d88'   `8. 
                                Y88..  .8' 
                                 `88888b.  
                                .8'  ``88b 
                                `8.   .88P 
                                 `boood8'  
                                      
 EightLisp, by Sam Bleckley (diiq, stm31415@gmail.com)

***************************************************************************/

#include "eight.h"
#include <math.h>

closure* character(wchar_t a)
{
     closure *ret = new(closure);
     ret->in = new(doubleref);
     ret->in->type = CHARACTER;
     ret->type = DREF;
     ret->in->obj = NULL;
     ret->in->character = a;
     ret->closing = nil();
     ret->in->info = nil();
     return ret;
}

closure *string(wchar_t * str)
{
     if(*str == 0)
	  return nil();
     return cons(character(*str), string(str+1));
}

int stringp(closure *a)
{
     if ((a->in->type == CONS_PAIR) && (car(a)->in->type == CHARACTER))
	  return stringp_internal(a);
     return 0;
}

int stringp_internal(closure *a)
{
     if (!nilp(a))
	  return 1;
     if ((a->in->type == CONS_PAIR) && (car(a)->in->type == CHARACTER))
	  return stringp_internal(cdr(a));
     return 0;
}

wchar_t* string_to_c_MALLOC(closure *a){
    // Watch it! MUST BE FREED
    if (!stringp(a))
	error(121, 121, "Oh man, I tried to convert a non-string.");
    int l = length(a);
    wchar_t *name = calloc(l+1, sizeof(wchar_t));
    int i = 0;
    for(i=0; i<l; i++){
	name[i] = car(a)->in->character;
	a = cdr(a);
    }
    return name;
}

/* closure *string_to_number(closure *a){ */
/*     if (!stringp(a)) */
/* 	error(121, 121, "Oh man, I tried to convert a number from a non-string."); */
/*     int l = length(a); */
/*     int num = 0; */
/*     int i; */
/*     for(i=l; i>0; i--){ */
/* 	num += (car(a)->in->num) * pow(10,i); */
/* 	a = cdr(a); */
/*     } */
/*     return number(num); */

/* } */


