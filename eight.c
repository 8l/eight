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
#ifndef EIGHT_MAIN
#define EIGHT_MAIN

#include "eight.h"
#include <stdlib.h>
#include <string.h>
#include "stdio.h"
#include "symbols.c"

machine *init_8VM()
{
//     test_memory();
//     error(32623653, 432747235, "got there");
     machine *m =  init_memory();
     frame *bframe = new_frame(NULL);
     m->current_frame = bframe;
     m->base_frame = bframe;

     insert_symbol("quote", QUOTE);
     insert_symbol("asterix", ASTERIX);
     insert_symbol("atpend", ATPEND);
     insert_symbol("comma", COMMA);
     insert_symbol("...", ELIPSIS);
          insert_symbol("t", T);

     internal_set(symbol(T), symbol(T), bframe, bframe);
     new_basic_commands(m);
     //m = collect();

     return m;
}	

closure *cleari()
{
     closure *cleari = symbol(CLEAR); 
     cleari->type = INTERNAL;
     return cleari;
}

closure *e_argument() 
{
     closure *e_argument = symbol(E_ARGUMENT); 
     e_argument->type = INTERNAL;
     return e_argument;
}


closure *nil() 
{
     closure *anil = new(closure);
     anil->type = NIL;
     anil->closing = anil;
     return anil;
}

int nilp(closure *x)
{
     if(x->type == NIL) return 1;
     return 0;
}

closure *symbol(symbol_id id)
{
     // builds a symbol-closure from an id.
     closure *sym = new(closure);
     sym->type = SYMBOL;
     sym->symbol_id = id;
     sym->closing = nil();
     return sym; 
}

int leakedp(closure *x)
{
     closure *y = car(x);
     if(y->type == SYMBOL && y->symbol_id == LEAKED) return 1;
  return 0;
}

closure *number(int num)
{
  closure *ret = new(closure); 
  ret->type = NUMBER;
  ret->closing = nil();
  ret->num = num;
  return ret;
}

closure *quote(closure *x)
{
      closure *q = symbol(QUOTE);
      return cheap_cons(q, cons(x, nil()));
};

closure *e_arg()
{
     closure *a = symbol(E_ARGUMENT);
     a->type = INTERNAL;
     return a;
}

int equal(closure *a, closure *b)
{
     if ((a==b) ||
	 (nilp(a) && nilp(b)) ||
	 ((a->type == b->type) && (a->obj == b->obj)))
	  return 1;
     return 0;
}


closure *cheap_cons(closure *car, closure *cdr)
{
     closure *pair = new(closure);
     cons_cell *inner = new(cons_cell);
     pair->type = CONS_PAIR;
     pair->cons = inner;
     inner->car = car;
     inner->cdr = cdr;
     inner->type = CONS_CELL;
     pair->closing = nil();
     return pair;
}

closure *copy_closure(closure *x)
{
     closure *newx = new(closure);
     memcpy(newx, x, sizeof(closure));
     return newx;
}

closure *cons(closure *car, closure *cdr)
{
     closure *pair = new(closure);
     cons_cell *cpair = new(cons_cell);
     cpair->type = CONS_CELL;
     pair->type = CONS_PAIR;
     pair->cons = cpair;
     if (nilp(cdr)) {
	  closure *newcar = copy_closure(car);
	  pair->closing = car->closing;
	  newcar->closing = nil();
	  pair->cons->car = newcar;
	  pair->cons->cdr = nil();
	  cdr->closing = nil();
     } else if (nilp(car)){
	  closure *newcdr = copy_closure(cdr);
	  pair->closing = cdr->closing;
	  newcdr->closing = nil();
	  pair->cons->cdr = newcdr;
	  pair->cons->car = nil();
	  car->closing = nil();
     } else {	
	  closure *newa = nil();
	  closure *newb = nil();
	  closure *newc = nil();
	  combine(car->closing, 
		  cdr->closing,
		  &newa, &newb, &newc);
	  combine(cdr->closing, 
		  nil(),
		  &newb, &newa, &newc);
	  closure *ncar = copy_closure(car);
	  closure *ncdr = copy_closure(cdr);
	  ncar->closing = newa;
	  ncdr->closing = newb;
	  pair->closing = newc;
	  pair->cons->car = ncar;
	  pair->cons->cdr = ncdr;
     }
     return pair;
};

void combine(closure *a, 
	     closure *b, 
	     closure **newa, 
	     closure **newb,
	     closure **ret)
{
     // So each closing should look like:
     // ([(sym val . ())]* . ())
     // So car(car( is sym;
     // car(cdr(car is val
     // cdr( is rest
     if (nilp(a)){ // halt condition
	  return;
     }
     closure *seen = assoc(car(car(a)), *ret); // look for symbol
                                               // in the new closure;
     if(nilp(seen)){ // if it isn't there, then check for conflicts...
                     // look for symbol in b...

	  closure *duplicate = assoc(car(car(a)), b);
	  if(nilp(duplicate) || 
	     equal(car(duplicate), second(car(a)))){ 
	       // No conflict? Then toss it in ret.
	       *ret = cheap_cons(car(a), *ret);
	  } else {
	       *newa = cheap_cons(car(a), *newa);
	       *newb = cheap_cons(cheap_cons(car(car(a)), duplicate), *newb);
	  }
     }
     combine(cdr(a), b, newa, newb, ret); 
}

	       
closure *car(closure *x)
{
     if (nilp(x)) return x;
     //if (x->type != CONS_PAIR) return NULL;
     if (nilp(x->closing)){
	  return x->cons->car;
     } else {
	  closure *clo = copy_closure(x->cons->car);
	  clo->closing = append(clo->closing, x->closing);
     	  return clo;
     } 
}

closure *cdr(closure *x)
{
     if (nilp(x)) return x;
     //if (x->type != CONS_PAIR) return NULL;
     if (nilp(x->closing)){
	  return x->cons->cdr;
     } else {
	  closure *clo = copy_closure(x->cons->cdr);
	  clo->closing = append(clo->closing, x->closing);
     	  return clo;
     } 
}


closure *append(closure *a, closure *b)
{
     // TODO sanity test a & b for listness.
     if (nilp(a)){
	  return b;
     } else {
	  return cons(car(a), append(cdr(a), b));
     }
};

closure *second(closure* list)
{
     return car(cdr(list));
}

closure *assoc(closure *sym, closure* closing)
{
     if (nilp(closing)) {
	  return closing;
     }
     if (equal(sym, car(car(closing)))) {
	  return cdr(car(closing));
     }
     return assoc(sym, cdr(closing));
};

closure *acons(closure *sym, closure *val, closure *closing)
{
     closure *ret =  cons(cons(sym, cons(val, nil())), closing);
     return ret;
};

int length(closure *xs)
{
  int i = 0;
  while(!nilp(xs)){
    i++;
    xs = cdr(xs);
  }
  return i;
}

closure *looker_up(closure *sym, frame *aframe)
{
     // note that the value itself is the car of
     // what is returned; by setting the car of
     // what is returned, you can alter the value
     // destructively.
     closure *local = assoc(sym, sym->closing);
     if (nilp(local) || leakedp(local)) 
       return looker_up_internal(sym, aframe);
     return local;
};

closure *looker_up_internal(closure *sym, frame *aframe)
{
     closure* local =  assoc(sym, aframe->scope);
     if (nilp(local) || leakedp(local)) {
       if (aframe->below == NULL)
	    return nil();
       return looker_up_internal(sym, aframe->below);
     }
     return local;
};

void internal_set(closure *sym,
		  closure *value,
		  frame   *aframe,
		  frame   *base_frame)
{
     closure *old = looker_up(sym, aframe);
     if (nilp(old)){
	  base_frame->scope = acons(sym,
				    value,
				    base_frame->scope);
     } else {
         old->cons->car = value;
     };
}

int commap(closure *arg)
{
     if ((arg->type == CONS_PAIR) &&
	 (car(arg)->type == SYMBOL) &&
	 (car(arg)->symbol_id == COMMA)) return 1;
     return 0;
};

int asterixp(closure *arg)
{
     // returns true if argument == *x
     if ((arg->type == CONS_PAIR) &&
	 (car(arg)->type == SYMBOL) &&
	 (car(arg)->symbol_id == ASTERIX)) return 1;
     return 0;
};
int atpendp(closure *arg)
{
     // returns true if argument == *x
     if ((arg->type == CONS_PAIR) &&
	 (car(arg)->type == SYMBOL) &&
	 (car(arg)->symbol_id == ATPEND)) return 1;
     return 0;
};
int quotep(closure *sym)
{
     // returns true if argument == *x
     if ((sym->type == CONS_PAIR) &&
	 (car(sym)->type == SYMBOL) &&
	 (car(sym)->symbol_id == QUOTE)) return 1;
     return 0;
};
int elipsisp(closure *sym)
{
     //returns true if sym == ...
     if ((sym->type == SYMBOL) &&
	 (sym->symbol_id == ELIPSIS)) return 1;
     return 0;
}

int optional_argp(closure *sym)
{
     if ((sym->type == CONS_PAIR) &&
	 (car(sym)->type == SYMBOL) &&
	 (car(sym)->symbol_id != QUOTE))  return 1;
     return 0;
};

int e_argp(closure *sym)
{
     if (((sym->type == INTERNAL) && 
	  (sym->symbol_id == E_ARGUMENT)) ||
	 ((sym->type == CONS_PAIR) &&
	  ((second(sym)->type == INTERNAL) && 
	   (second(sym)->symbol_id == E_ARGUMENT)))) return 1;
     return 0;
}


operation *build_argument_chain(closure *lambda_list,
				closure *arg_list,
				operation *current)
{
     // builds the operations necessary to evaluate the arguments of a fn.
     if (lambda_list->type == NIL) {
	  if (!nilp(arg_list)) {
	       printf("Too many args!"); // need an actual action here.
	  }
	  // If we're out of lambda list, we're done!
	  return current;
     } else if (asterixp(car(arg_list))){
	  // accum->(closure_op, x)->(continue_apply,
	  //                          (lambda_list . arg_list->cdr))

	  operation *tnext = new(operation);
	  tnext->type = CLOSURE_OP;
	  tnext->closure = second(car(arg_list));
	  current->next = tnext;
	  current = tnext;

	  operation *bnext = new(operation);
	  bnext->type = MACHINE_FLAG;
	  bnext->flag = CONTINUE_APPLY;
	  bnext->closure = cons(lambda_list, cdr(arg_list));
	  current->next = bnext;
	  current = bnext;

	  return current;

     } else if (atpendp(car(arg_list))){
	  // accum->(closure_op, x)->(continue_apply,
	  //                          (lambda_list . arg_list->cdr))
	  operation *tnext = new(operation);
	  tnext->type = CLOSURE_OP;
	  tnext->closure = second(car(arg_list));
	  current->next = tnext;
	  current = tnext;

	  operation *bnext = new(operation);
	  bnext->type = MACHINE_FLAG;
	  bnext->flag = ATPEND_APPLY;
	  bnext->closure = cons(lambda_list, cdr(arg_list));
	  current->next = bnext;
	  current = bnext;

	  return current;

     } else if (elipsisp(car(lambda_list))){
	  // the trick here is that you have to signal yourself
	  // that you're elipsising, even THROUGH a continue_apply
	  closure *argm;
	  if (quotep(second(lambda_list))){
	       // if the next arg is quoted, keep the quote.
	       argm = cons(quote(e_argument()), nil());
	  } else {
	       argm = cons(e_argument(), nil());
	  }
	  if (nilp(arg_list)){
	       return build_argument_chain(nil(),
					   arg_list,
					   make_arg(second(lambda_list),
						    nil(),
						    current));
	  } else {
	       return build_argument_chain(argm,
					   arg_list,
					   make_arg(second(lambda_list),
						    nil(),
						    current));
	  }	
     } else if (optional_argp(car(lambda_list))){
	  // if there is a parameter given, remove the 'optional' and rec.
	  // if there isn't, push the default form on to arg_list, and rec.
	  if (!nilp(arg_list)) {
	       return build_argument_chain(cons(car(car(lambda_list)),
						cdr(lambda_list)),
					   arg_list,
					   current);
	  } else {
	       return build_argument_chain(cons(car(car(lambda_list)),
						cdr(lambda_list)),
					   cons(second(car(lambda_list)),
						nil()),
					   current);
	  }
     } else {
	  // e_argument makes this kinda a mess, but not too bad:
	  // just check for e-arg and if it's present, DON'T shrink the
	  // lambda list until there are no more arguments in arg_list.
	  if (e_argp(car(lambda_list)) && !nilp(cdr(arg_list))){
	       return build_argument_chain(lambda_list,
					   cdr(arg_list),
					   make_arg(car(lambda_list),
						    car(arg_list),
						    current));
	  } else {
	       return build_argument_chain(cdr(lambda_list),
					   cdr(arg_list),
					   make_arg(car(lambda_list),
						    car(arg_list),
						    current));
	  }
     }
}

operation *make_arg(closure *sym, closure *val, operation *current)
{
     operation* arg  = new(operation);
     operation* arg2 = new(operation);
     current->next = arg;
     current = arg;
     current->type = CLOSURE_OP;
     current->next = arg2;
     // it's possible these can be simplified to conses not lists.
     if (quotep(sym)){
	  if (commap(val)){
	       //(closure_op (second val))->(machine_op, argument (second sym))
	       current->closure = second(val);
	       current = arg2;
	       current->type = MACHINE_FLAG;
	       current->flag = ARGUMENT;
	       current->closure = second(sym);
	  } else {
	       //(closure_op (quote val))->(machine_op, argument (second sym)))
	       current->closure = quote(val);
	       current = arg2;
	       current->type = MACHINE_FLAG;
	       current->flag = ARGUMENT;
	       current->closure = second(sym);
	  }
     } else {
	  //(closure_op val)-> (machine_op argument sym)
	  current->closure = val;
	  current = arg2;
	  current->type = MACHINE_FLAG;
	  current->flag = ARGUMENT;
	  current->closure = sym;
     }
     return current;
}

closure *clear_list(closure *args)
{
     if (args->type != CONS_PAIR){
	  printf("YOU'RE A POOP HEAD: senseless args to clear_list.");
	  return nil();
     } else {
	  if (!nilp(cdr(args))) 
	       return cons(cons(cleari(), cons(car(args), nil())), 
			   clear_list(cdr(args)));
	  return  cons(cons(cleari(), cons(car(args), nil())), nil());
     }
}

int free_varp(closure *token,
	      closure *accum,
	      frame *current_frame)
{
     if((token->type == SYMBOL) &&
	(nilp(assoc(token, accum))))  return 1;
     return 0;
}
	   
closure *find_free_variables(closure *code,
			     frame* current_frame,
			     closure *accum)
{
     if (free_varp(code, accum, current_frame)){
	  closure* value = looker_up(code, current_frame);
	  if (!nilp(value)){
	       accum = cheap_cons(cheap_cons(code, 
					     cheap_cons(car(value), 
							nil())), accum);
	  }
     } else if (code->type == CONS_PAIR && !quotep(code)){
	  accum = find_free_variables(code->cons->car,
				      current_frame, 
				      accum);
	  accum = find_free_variables(code->cons->cdr, 
				      current_frame, 
				      accum);
     }
     return accum;
}

closure *enclose(closure *code, frame *current_frame)
{
     closure *ret = copy_closure(code);
     ret->closing = find_free_variables(code, 
					current_frame, 
					code->closing);
     return ret;
};

machine *stack_copy(machine *m){
     machine *ret = new(machine);
     ret->type = MACHINE;
     ret->current_frame = copy_frame(m->current_frame);     
     ret->base_frame = m->base_frame;
     return ret;
}

frame *copy_frame(frame *fram)
{
     // Todo --- this can be made a loop, but not by the compiler.
     if (fram->below == NULL)
	  return fram;
     frame *ret = new_frame(NULL);
     ret->scope = fram->scope;
     ret->rib = fram->rib;
     ret->next = fram->next;
     ret->below = copy_frame(fram->below);
     return ret;
}

closure *fn_lambda_list(closure *fn)
{
     return car(fn); // This may change in the FUTURE! Duhn duhn duuuuuhhhnnnn.
}

operation *fn_instructions(closure *fn)
{
     operation* cl = new(operation);
     cl->type = CLOSURE_OP;
     instruction_list(cdr(fn), cl);
     return cl->next;
}

operation *instruction_list(closure *list, operation *doit)
{
     if(nilp(list)){
	  return NULL;
     } else {
	  operation* cl = new(operation);
	  cl->type = CLOSURE_OP;
 	  cl->closure = car(list);
	  doit->next = cl;
	  cl->next = instruction_list(cdr(list), cl);
	  return cl;
     }
}

frame *new_frame(frame *below)
{
     frame *ret = new(frame);
     ret->type = FRAME;
     ret->below = below;
     ret->scope = nil();
     ret->rib = nil();
     return ret;
}

int virtual_machine_step(machine *m)
{
     /* printf("\ninstruction: "); */
     /*     if(m->current_frame->next){ */
     /* 	  print_op(m->current_frame->next); */
     /*    } */
     /* if(m->accum){ */
    /*   //	  print_stack(m->current_frame); */
     /* 	  printf(", accum: "); */
     /* 	  print_closure(m->accum); */
     /*    } */
     /*   printf("\n");   */
     
     /* print_stack(m->current_frame); */
     /* printf("\ndon printing\n\n"); */
     // pop/halt condition
     if (m->current_frame->next == NULL){
	  if (m->current_frame->below != NULL){
	       m->current_frame = m->current_frame->below;
	       return 0;
	  } else {
	       return 1; // halt instead of popping the final frame
	  }
     }
     operation *instruction = m->current_frame->next;
     m->current_frame->next = instruction->next;
     if (instruction->type == CLOSURE_OP){
	  if (instruction->closure->type == BUILTIN){

	       builtin fn = instruction->closure->builtin_fn;
	       fn(m);

	  } else if (instruction->closure->type == CONTINUATION){
	       m->accum = car(looker_up(symbol(string_to_symbol_id("a")), 
					m->current_frame));
	       m->base_frame = instruction->closure->mach->base_frame;
	       m->current_frame = instruction->closure->mach->current_frame;    

	  } else if (instruction->closure->type == SYMBOL){
	       closure *res =  looker_up(instruction->closure,
					 m->current_frame);
	       if (nilp(res)) {
		    printf("nada\n");
		    print_closure(instruction->closure);
//		    closure *sig = build_signal(cons(string("\n\n\nthere's an old man in town\nwho puts his spoon in his mouth\nand swallows\nbut there's nothing there\n\n\nerror: I attempted to look up a symbol that was undefined: "), instruction->closure), m);
//		    toss_signal(sig, m);
	       } else {
		    m->accum = car(res);
	       }

	  } else if (!nilp(instruction->closure->closing)){

               // new frame, as if a function application
	       m->current_frame = new_frame(m->current_frame);

	       closure* sub = copy_closure(instruction->closure);
	       sub->closing = nil();
	       operation* cl = new(operation);
	       cl->type = CLOSURE_OP;
	       cl->closure = sub;

	       m->current_frame->next = cl;
	       m->current_frame->scope = instruction->closure->closing;

	  } else if (instruction->closure->type != CONS_PAIR){

	       // shortcut for char, num and other directs.
	       m->accum = instruction->closure;

	  } else if (quotep(instruction->closure)){
	       m->accum = enclose(second(instruction->closure), 
				  m->current_frame);

	       // TODO sanity check, is lonely cdr?
	  } else if (car(instruction->closure)->symbol_id == CLEAR){

	       m->accum = car(cdr(instruction->closure));

	  } else if (stringp(instruction->closure)){

	       m->accum = instruction->closure;

	  } else {
	       // fn application
	       operation* fn=new(operation);
	       operation* apply=new(operation);
	       fn->type = CLOSURE_OP;
	       fn->closure = car(instruction->closure);
	       fn->next = apply;
	       apply->type = MACHINE_FLAG;
	       apply->flag = APPLY;
	       apply->closure = cdr(instruction->closure);
	       apply->next = m->current_frame->next;
	       m->current_frame->next = fn;

	  };


     } else if (instruction->type == MACHINE_FLAG){


	  if (instruction->flag == APPLY) {

	       frame* n_frame;
	       if ((m->current_frame->next == NULL) && 
		   (m->current_frame != m->base_frame)){
		    n_frame = m->current_frame;
	       } else {
		    n_frame = new_frame(m->current_frame);
	       }
	       n_frame->rib = m->accum->closing;

	       operation* fn=new(operation);
	       operation* chain=build_argument_chain(fn_lambda_list(m->accum),
						     instruction->closure,
						     fn);
	       fn->type = MACHINE_FLAG;
	       fn->flag = DO; 
	       fn->closure = m->accum;
	       if (fn->next != NULL){
		    n_frame->next = fn->next;
		    chain->next = fn;
		    fn->next = NULL;
	       } else {
		    n_frame->next = fn;
	       }
	       m->current_frame = n_frame;

	  } else if (instruction->flag == ARGUMENT){
	       if (e_argp(instruction->closure)){
		    closure *last = cdr(car(m->current_frame->rib));
		    last->cons->car = append(last->cons->car,
					     cons(m->accum, nil()));
			 
	       } else {
		    m->current_frame->rib = acons(instruction->closure,
						  m->accum,
						  m->current_frame->rib);
	       }

	  } else if (instruction->flag == CONTINUE_APPLY){

	       operation* fn=new(operation);
	       fn->type = CLOSURE_OP;
	       operation* boo = build_argument_chain(
		    car(instruction->closure),
		    append(m->accum, cdr(instruction->closure)),
		    fn);
	       boo->next = m->current_frame->next;
	       m->current_frame->next = fn->next;

	  } else if (instruction->flag == ATPEND_APPLY){

	       closure *args = clear_list(m->accum);
	       operation* fn=new(operation);
	       fn->type = CLOSURE_OP;
	       operation* boo = build_argument_chain(
		    car(instruction->closure),
		    append(args, cdr(instruction->closure)),
		    fn);
	       boo->next = m->current_frame->next;
	       m->current_frame->next = fn->next;

	  } else if (instruction->flag == DO){

	       operation* ins = fn_instructions(instruction->closure);
	       m->current_frame->next = ins;
	       m->current_frame->scope = m->current_frame->rib;
	  }
     }
     return 0;
}
  


machine* eval(closure *form, machine *m){
     operation* newop = new(operation);
     newop->type = CLOSURE_OP;
     newop->closure = form;
     newop->next = m->current_frame->next;
     m->current_frame->next = newop;
     int i = 0; 
     while (i == 0){
 	  i = virtual_machine_step(m);
	  m = collect();
     }

     return m;
}



#endif

