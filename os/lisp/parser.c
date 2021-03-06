#include "parser.h"

#include <stddef.h>
#include "types.h"

#ifdef LISP_TEST
//testin
#include <stdio.h>
#include "../tests/kernel_mappings.h"
#else
//real world
#include "../kernel/kmalloc.h"
#include "../kernel/kterm.h"
#include "../kernel/kstd.h"
#endif

reader_t* make_reader(char* str)
{
    reader_t* r = kmalloc(sizeof(reader_t));
    r->offset = 0;
    r->str = str;
    return r;
}

char reader_cur(reader_t* reader)
{
    return reader->str[reader->offset];
}

char reader_peek(reader_t* reader)
{
    return reader->str[reader->offset + 1];
}

char reader_next(reader_t* reader)
{
    return reader->str[++reader->offset];
}

char* tokenize_one(reader_t* reader)
{
    char* tok = kmalloc(sizeof(char) * (MAX_TOKEN_LEN + 1));
    int offset = 0;

    char c = reader_cur(reader);
    //loop on char != whitespace
    //add char to result str
    while((c != ' '  &&
	   c != '\n' &&
	   c != '('  &&
	   c != ')'))
    {
	tok[offset++] = c;
	c = reader_next(reader);
    }
	   

    tok[offset] = '\0';
    reader->offset--; //TODO: fix workaround for eating chars
    
    //return result in sym
    return tok;
}

int valid_hex_num(char *str)
{
    // hex ints start with '0x'
    // cheat lol
    if(str[0] == '0' && str[1] == 'x')
	return 1;
    else
	return 0;
}

int valid_dec_num(char* str)
{
    int ret = 1;
    while(*str != '\0' &&
	  ret)
    {
	if(*str < '0' ||
	   *str > '9')
	{
	    ret = 0;
	}
	str++;
    }

    return ret;
}

int valid_sym(char* str)
{
    int ret = 1;
    while(*str != '\0' &&
	  ret)
    {
	if((*str < 'a' || *str > 'z') &&
	   (*str < '0' || *str > '9') &&
	   (*str != '-' && *str != '+' &&
	    *str != '*' && *str != '/' &&
	    *str != '=' && *str != '_' &&
	    *str != '<' && *str != '>'))
	{
	    ret = 0;
	}
	str++;
    }

    return ret;
}

//TODO: put in main function, decide what type a tok is in there
lobj_t* parse_sym(reader_t* r)
{
    char* str = tokenize_one(r);
    lobj_t* newsym = NULL;
	    
    if (valid_dec_num(str))
    {
	newsym = num(atoi(str));	    
    }
    else if (valid_hex_num(str))
    {
	newsym = num(atoi16(str));
    }
    else if(valid_sym(str))
    {
	newsym = sym(str);
    }
    
    kfree(str);
    
    return newsym;	
}

lobj_t* parse_list(reader_t* r)
{
    /*
      c = '(': car = recurse
               cdr = continue loop
      c = tok: cur->car = tokenize(r)
               cur = cur->cdr               
      c = ')': cdr = nil
               return sexp
    */

    //TODO: reader macros / synatactic sugar e.g. 'x = (quote x)

    char c;
    lobj_t* ret = NULL; //maintains pointer to root
    
    while((c = reader_next(r)) &&
	   c != ')')
    {
	if(c == '(')
	{
	    lobj_t* newcons = cons(parse_list(r), NULL);
	    if(ret == NULL)
		ret = newcons;
	    else
		ret = append(ret, newcons);
	}
	else if(c == ' ' || c == '\n' || c == ' ') //TODO: is_blank
	{
	    continue;
	}
/*	else if(c == '.') // Dot pair notation
	{
	    //TODO: check if we already have car

	    ret->cdr = parse_sym(r);
	    return ret;
	    }*/
	else// if (c != ' ') //TODO: is_alpha
	{
	    //TODO: check if c = sym
	    //TODO: check if c = int

	    //TODO: with new cons is this too much work
	    if(ret == NULL)
		ret = cons(parse_sym(r), NULL);
	    else
		ret = append(ret, cons(parse_sym(r), NULL));
	}
    }

    return ret;
}

lobj_t* parse_exp(reader_t* r)
{
    char c = reader_cur(r);    
    if(c == '(')
	return parse_list(r);
    else
	return parse_sym(r);
}

lobj_t* lisp_read(char* str)
{	
    reader_t* r = make_reader(str);
    return parse_exp(r);
}

void pad_print(int padding, char* str)
{
    for(int i = 0; i < padding; i++)
      k_print(" ");

    k_print(str);
}

void print_cons_iter(lobj_t* root, int depth, int debug)
{
    lobj_t* elem = root;
    
    if(elem == NULL)
    {
	k_print(" nil");
	return;
    }

    if(debug)
    {
	if(elem->type == Cons)
	{
	    k_print("{");
	    k_print("Cons");
	    k_print(":");
	    k_print_hex((size_t)(elem->car));
	    k_print(":");
	    k_print_hex((size_t)(elem->cdr));
	    k_print("}");
	}
	else if(elem->type == Sym)
	{
	    k_print("{");
	    k_print("Sym");
	    k_print(":");
	    k_print_hex((size_t)(elem->val));
	    k_print("} ");
	}
	else if(elem->type == Num)
	{
	    k_print("{");
	    k_print("Num");
	    k_print(":");
	    k_print_hex((size_t)(elem->numl));
	    k_print("} ");
	}
    }

    if(elem->type == Sym)
    {
	//its sym
	/*if(i != 0)
	  k_print(" ");*/

	pad_print(1, elem->val);
    }
    else if(elem->type == Num)
    {
	k_print(" ");
	k_print_num(elem->numl);
    }
    else if(elem->type == Cons)
    {
	k_print("-\\");

	//its list
	k_print("\n");
	pad_print(depth + 2, "|-");
	print_cons_iter(elem->car, depth + 3, debug);

	k_print("\n");
	pad_print(depth + 2, "\\-");
	print_cons_iter(elem->cdr, depth + 3, debug);
    }
    else if(elem->type == Func)
    {
	k_print("-\\ func");

	//its list
	k_print("\n");
	pad_print(depth + 2, "|-");
	print_cons_iter(elem->args, depth + 3, debug);

	k_print("\n");
	pad_print(depth + 2, "\\-");
	print_cons_iter(elem->body, depth + 3, debug);

    }
    else if(elem->type == Err)
    {
	pad_print(depth + 2, " (Err ");
	k_print_num(elem->errcode);
	k_print(": ");
	k_print(elem->errmsg);
	k_println(")");
    }

}

void print_cons(lobj_t* root)
{
    print_cons_iter(root, 0, 0);
    k_print("\n");
}

void print_cons_debug(lobj_t* root)
{
    print_cons_iter(root, 0, 1);
    k_print("\n");
}

void print_sexp_iter(lobj_t* root, int depth, int debug)
{
    lobj_t* elem = root;

    /*
     * each elem->car.type =
     *    sym: print val, space
     *    cons: recur
     */

    if(elem == NULL)
    {
	pad_print(depth, "nil");
	return;
    }
	
    if(elem->type == Cons)
	pad_print(depth, "(");

    uint8_t first = 1;
    
    while(elem)
    {
	if(debug && elem)
	{
	    if(elem->type == Cons)
	    {
		k_print("{");
		k_print("Cons");
		k_print(":");
		k_print_hex((size_t)(elem->car));
		k_print(":");
		k_print_hex((size_t)(elem->cdr));
		k_print("}");
	    }
	    else if(elem->type == Sym)
	    {
		k_print("{");
		k_print("Sym");
		k_print(":");
		k_print_hex((size_t)(elem->val));
		k_print("} ");
	    }
	    else if(elem->type == Num)
	    {
		k_print("{");
		k_print("Num");
		k_print(":");
		k_print_hex((size_t)(elem->numl));
		k_print("} ");
	    }

	}

	if(!first)
	    k_print(" ");

	if(elem == NULL)
	{
	    break;
	}
	else if(elem->type == Sym)
	{
	    k_print(elem->val);
	    return;
	}
	else if(elem->type == Num)
	{
	    k_print_num(elem->numl);
	    return;
	}
	else if(elem->type == Cons)
	{
	    if(elem->car->type == Sym)
	    {
		k_print(elem->car->val);
	    }
	    else if(elem->car->type == Num)
	    {
		k_print_num(elem->car->numl);
	    }
	    else if(elem->car->type == Cons)
	    {
		//its list
		k_print("\n");
		print_sexp_iter(elem->car, depth + 2, debug);
	    }
	}
	else if(elem->type == Func)
	{
	    k_print("func");
	    k_print("\n");
	    print_sexp_iter(elem->args, depth + 2, debug);
	    print_sexp_iter(elem->body, depth + 2, debug);
	}
	else if(elem->type == Err)
	{
	    k_print("[Err ");
	    k_print_num(elem->errcode);
	    k_print(": ");
	    k_print(elem->errmsg);
	    k_println("]");
	    return NULL;
	}

	// flag that we aren't the first in the list anymore
	first = 0;
	elem = elem->cdr;

	// If the cdr is a cons, loop again. If not, we're a dot pair (a . b)
	/*if(elem->cdr->type == Cons)
	    elem = elem->cdr;
	else
	{
	    //TODO: implement print lobj function (unwraps num, sym)
	    k_print(" . ");

	    //TODO URGENT: DON'T KEEP THIS WHAT IF A DOT PAIR IS A SYM
	    k_print(elem->numl);
	    break;
	    }*/
    }

    k_print(")");
}

void print_sexp(lobj_t* root)
{
    print_sexp_iter(root, 0, 0);
}

void print_sexp_debug(lobj_t* root)
{
    print_sexp_iter(root, 0, 1);
}
