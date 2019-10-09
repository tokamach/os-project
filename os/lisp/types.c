#include "types.h"

#ifdef LISP_TEST
//testin
#include <stdio.h>
#include "../tests/kernel_mappings.h"
#else
//real world
#include "../kernel/kmalloc.h"
#include "../kernel/kterm.h"
#endif

atom_t* make_atom(const char* str)
{
    atom_t* ret = kmalloc(sizeof(atom_t));
    ret->str = kmalloc(strlen(str));
    strcopy(str, ret->str);
    return ret;
}

void free_atom(atom_t* atom)
{
    kfree(atom);
}

sexp_elem_t* make_sexp_elem_atom(atom_t* atom)
{
    sexp_elem_t* tmp = kmalloc(sizeof(sexp_elem_t));
    tmp->type = Atom;
    tmp->val.atom = atom;
    tmp->next = NULL;
    return tmp;
}

sexp_elem_t* make_sexp_elem_list(sexp_t* list)
{
    sexp_elem_t* tmp = kmalloc(sizeof(sexp_elem_t));
    tmp->type = List;
    tmp->val.list = list;
    tmp->next = NULL;
    return tmp;
}

void free_sexp_elem(sexp_elem_t* elem)
{
    if(elem->type == List)
	free_sexp(elem->val.list);
/*    else
      kfree(elem->val.atom); */
    
    kfree(elem);
}

sexp_t* make_sexp()
{
    sexp_t* tmp = kmalloc(sizeof(sexp_t));
    tmp->first = NULL;
    tmp->size  = 0;
    return tmp;
}

void free_sexp(sexp_t* sexp)
{
    sexp_elem_t* nextnode = sexp->first;
    sexp_elem_t* tmp = nextnode;

    //waht the fuck is this
    while(nextnode)
    {
	tmp = nextnode;
	nextnode = nextnode->next;
	free_sexp_elem(tmp);
    }

    kfree(sexp);
}

sexp_elem_t* sexp_elem_at(sexp_t* sexp, int index)
{
    sexp_elem_t* tmp = sexp->first;
    
    if(!tmp)
	return NULL;
    
    for(int i = 0; i < index; i++)
    {
	tmp = tmp->next;
	/*if(!tmp)
	    while(1) {k_print("LISTERR");} //out of range, hang TODO: actual err
	*/
    }

    return tmp;
}

void sexp_add_elem(sexp_t* sexp, sexp_elem_t* new)
{
    sexp_elem_t* end = sexp_elem_at(sexp, sexp->size - 1);
    
    if(!end)
    {
	//list is empty
	sexp->first = new;
    }
    else
    {
	end->next = new;	
    }
    
    sexp->size++;
}

void sexp_pop(sexp_t* sexp)
{
    //get last elem
    sexp_elem_t* tmp = sexp_elem_at(sexp, sexp->size - 1);
    //free it
    free_sexp_elem(tmp);

    //set penultimate elem's next to null, reduce sexp size
    sexp_elem_at(sexp, sexp->size - 2)->next = NULL;
    sexp->size--;
}
