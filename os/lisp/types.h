#pragma once

#include <stdint.h>

#define MAX_ATOM_LEN 128

//Atom
typedef struct atom {
    char* str;
    //TODO: add lisp data type
} atom_t;

atom_t* make_atom(const char* str);
void free_atom(atom_t* atom);

typedef enum e_elem_type {
    Atom,
    List
} elem_type;

typedef struct sexp sexp_t;
//Boxed void pointer with "type" annontation
typedef struct cons {
    elem_type type;
    union {
	atom_t* atom;
	sexp_t* list;
    } val; //either List_t or Atom
    struct cons* next;
    
} cons_t;

cons_t* make_sexp_elem_atom(atom_t* atom);
cons_t* make_sexp_elem_list(sexp_t* list);
void free_sexp_elem(cons_t* elem);

//Sexp
typedef struct sexp {
    //List_t list;
    cons_t* first;
    int size;
} sexp_t;

sexp_t* make_sexp();
void free_sexp(sexp_t* sexp);
void sexp_add_elem(sexp_t* sexp, cons_t* elem);
cons_t* sexp_elem_at(sexp_t* sexp, int index);
