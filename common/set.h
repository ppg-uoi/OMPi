/*
  OMPi OpenMP Compiler
  == Copyright since 2001 the OMPi Team
  == Dept. of Computer Science & Engineering, University of Ioannina

  This file is part of OMPi.

  OMPi is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  OMPi is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OMPi; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* set.h */

#ifndef __SET_H__
#define __SET_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "stddefs.h"


/**
 * THE SET, by PPG
 * ---------------
 * 
 * This is a generic hash set structure generated through preprocessor macros.
 * Designed and implemented by the OMPi team, PPG.
 *
 * In order to generate a new set type you have to use SET_TYPE_DEFINE
 * and SET_TYPE_IMPLEMENT macros (see below for more info; the first defines
 * the new set type while the second actually declares some supporting set 
 * structures and functions).
 * 
 * Then, to make a new set variable you use:
 *     set(NAME) setvariable = set_new(NAME);
 *
 * If you want to iterate over all elements in a set you create a new
 * "setelem(NAME)" variable, use setvariable->first and then
 * setelemvariable->next e.g.:
 *     set(NAME) s = ...;
 *     setelem(NAME) e;
 *     set_put(s...);
 *     for (e = s->first; e; e = e->next)
 *     {
 *       ...
 *     }
 *
 * Insertions and searches are O(1) (set_put, set_put_unique, set_get).
 * set_put_unique checks if the key is already in the set before inserting it.
 * Removals are O(1).
 * set_isempty and set_size are O(1).
 * set_copy and set_copy_filtered are O(N). set_copy_filtered copies an element
 * only if "filter" function returns true for it.
 *
 * AVAILABLE FUNCTIONS
 * -------------------
 * 
 * set     set_new(NAME);
 * void    set_init(NAME, set);
 * void    set_drain(set);
 * void    set_free(set);
 * setelem set_put(set, key);
 * setelem set_put_unique(set, key);
 * setelem set_get(set, key);
 * setelem set_remove(set, key);
 * setelem set_remove_all(set, key);
 * bool    set_isempty(set);
 * int     set_size(set);
 * void    set_copy(set, set);  // to, from
 * void    set_copy_filtered(set, set, bool (*filter)(setelem));
 * 
 * THREAD SAFETY (ΝΟΤ!)
 * --------------------
 * 
 * The sets are *not* thread safe; each set instance is meant to be used by a 
 * single thread. Period.
 * 
 * However, multiple threads may declare and use their own private sets that 
 * are all of *the same type*. There is a catch here: for efficiency,  by 
 * default there is a simple recycling list *per set type* for set elements; 
 * this is by nature non-thread safe and produces race conditions even among 
 * threads that work on different set instances (of the same type).
 * 
 * We provide two more alternatives for cases were the same set type is to be 
 * utilized by multiple threads: 
 *   a) element recycling lists specific to each set instance and 
 *   b) no recyling lists at all (plain malloc/free for set elements). 
 * When implementing the set type, use SET_TYPE_IMPLEMENT_PR for
 * the first alternative and SET_TYPE_IMPLEMENT_NR for the second alternative.
 * SET_TYPE_IMPLEMENT is the default variant, where thread safety is of no
 * concern.
 * 
 * Choose wisely.
 * 
 */


static void *ssmalloc(int size)
{
	void *m;

	if ((m = (void *) calloc(1, size)) == NULL)
	{
		fprintf(stderr, "[ssmalloc]: memory allocation failed\n");
		exit(1);
	}
	return (m);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *            TYPE DEFINITIONS & FUNCTION PROTOTYPES             *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * SET_TYPE_DEFINE generates the structs and the prototypes for the functions.
 *
 * It should be called from a .h file and it should be accessible (visible) 
 * from all the files that are using the set.
 *
 * @param NAME          A name given to the set type. It is used for creating
 *                      new sets and set elements
 * @param KEYTYPE       The type of the key (e.g. long int)
 * @param VALUETYPE     The type of the value (e.g. void *).
 *                      Use 'char' for nill
 * @param HASHTABLESIZE The size of the internal hash table. It must be a prime.
 *                      Good values are said to be in approximately equal
 *                      distance from two consecutive powers of two
 *                      (53, 97, 193, 389, 769, ...)
 */
#define SET_TYPE_DEFINE(NAME, KEYTYPE, VALUETYPE, HASHTABLESIZE)               \
typedef KEYTYPE NAME##setkey_t;        /* Typedef used by SET_TYPE_IMPLEMENT */\
enum { NAME##htsize = HASHTABLESIZE }; /* The size of the hash table */        \
                                                                               \
/* Definition of set elements */                                               \
typedef struct NAME##setelem_ *NAME##setelem;                                  \
struct NAME##setelem_                                                          \
{                                                                              \
    KEYTYPE       key;                                                         \
    VALUETYPE     value;                                                       \
    NAME##setelem bucketnext;   /* for the HT bucket */                        \
    NAME##setelem next;         /* for the element list */                     \
    NAME##setelem prev;         /* for the element list */                     \
};                                                                             \
                                                                               \
/* Definition of set */                                                        \
struct __##NAME##SETFUNCTIONS;                                                 \
typedef struct NAME##set_                                                      \
{                                                                              \
    NAME##setelem table[NAME##htsize];                                         \
    NAME##setelem first;                                                       \
    NAME##setelem last;                                                        \
    int           nelem;                                                       \
    struct __##NAME##SETFUNCTIONS *__funcs;                                    \
    NAME##setelem __epool;     /* For private, per-instance recyclers */       \
} *NAME##set;                                                                  \
                                                                               \
/* A struct containing pointers to all the available set functions.            \
 * One instance is declared in SET_TYPE_IMPLEMENTS and all sets contain a      \
 * pointer to it. It is later used from the generic functions (see the bottom  \
 * of this file) in order to call the actual functions of the set.             \
 */                                                                            \
struct __##NAME##SETFUNCTIONS {                                                \
    void          (*set_drain)        (NAME##set s);                           \
    void          (*set_free)         (NAME##set s);                           \
    NAME##setelem (*set_put)          (NAME##set s, KEYTYPE k);                \
    NAME##setelem (*set_put_unique)   (NAME##set s, KEYTYPE k);                \
    NAME##setelem (*set_get)          (NAME##set s, KEYTYPE k);                \
    NAME##setelem (*set_remove)       (NAME##set s, KEYTYPE k);                \
    NAME##setelem (*set_remove_all)   (NAME##set s, KEYTYPE k);                \
    void          (*set_copy)         (NAME##set to, NAME##set from);          \
    void          (*set_copy_filtered)(NAME##set to, NAME##set from,           \
                                      bool (*filter)(NAME##setelem));          \
};                                                                             \
                                                                               \
NAME##set      NAME##set_new();                                                \
void           NAME##set_init(NAME##set *s);                                   \
void           NAME##set_drain(NAME##set s);                                   \
void           NAME##set_free(NAME##set s);                                    \
NAME##setelem  NAME##set_put(NAME##set s, KEYTYPE k);                          \
NAME##setelem  NAME##set_put_unique(NAME##set s, KEYTYPE k);                   \
NAME##setelem  NAME##set_get(NAME##set s, KEYTYPE k);                          \
NAME##setelem  NAME##set_remove(NAME##set s, KEYTYPE k);                       \
NAME##setelem  NAME##set_remove_all(NAME##set s, KEYTYPE k);                   \
void           NAME##set_copy(NAME##set to, NAME##set from);                   \
void           NAME##set_copy_filtered(NAME##set to, NAME##set from,           \
                                      bool (*filter)(NAME##setelem));


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *                DECLARATIONS & FUNCTION DEFINITIONS            *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* SET_TYPE_IMPLEMENT[_PR|_NR] generates the actual code for the set.
 * The _PR/_NR version allows multiple threads to use private sets of the same 
 * type by utilizing private element recylers/no recyclers at all.
 * There is no thread safety when accessing a particular set instance.
 *
 * This should be called from a single .c (non-header) source file.
 *
 * @param NAME The same name you gave when calling the SET_TYPE_DEFINE macro.
 * 
 * Perhaps SET_TYPE_IMPLEMENT should get a function for hashing and comparing
 */


/* For non-thread safe set types 
 */
#define SET_TYPE_IMPLEMENT(NAME)                                               \
/* Recycle bin for set elements (Note: this is NOT THREADSAFE) */              \
static NAME##setelem NAME##setepool = NULL;                                    \
static NAME##setelem get##NAME##setelem(NAME##set s)                           \
{                                                                              \
    if (NAME##setepool)                                                        \
    {                                                                          \
        NAME##setelem e = NAME##setepool;                                      \
        NAME##setepool = NAME##setepool->next;                                 \
        return (e);                                                            \
    }                                                                          \
    return (NAME##setelem) ssmalloc(sizeof(struct NAME##setelem_));            \
}                                                                              \
static void free##NAME##setelem(NAME##setelem e, NAME##set s)                  \
{                                                                              \
    e->next = NAME##setepool;                                                  \
    NAME##setepool = e;                                                        \
}                                                                              \
SET_TYPE_IMPLEMENTATION_BASE(NAME)

/* For thread safe set types (not sets!); per-instance recycling 
 */
#define SET_TYPE_IMPLEMENT_PR(NAME)                                            \
static NAME##setelem get##NAME##setelem(NAME##set s)                           \
{                                                                              \
    if (s->__epool)                                                            \
    {                                                                          \
        NAME##setelem e = s->__epool;                                          \
        s->__epool = e->next;                                                  \
        return (e);                                                            \
    }                                                                          \
    return (NAME##setelem) ssmalloc(sizeof(struct NAME##setelem_));            \
}                                                                              \
static void free##NAME##setelem(NAME##setelem e, NAME##set s)                  \
{                                                                              \
    e->next = s->__epool;                                                      \
    s->__epool = e;                                                            \
}                                                                              \
SET_TYPE_IMPLEMENTATION_BASE(NAME)

/* For thread safe set types (not sets!); no element recycling 
 */
#define SET_TYPE_IMPLEMENT_NR(NAME)                                            \
static NAME##setelem get##NAME##setelem(NAME##set s)                           \
{                                                                              \
    return (NAME##setelem) ssmalloc(sizeof(struct NAME##setelem_));            \
}                                                                              \
static void free##NAME##setelem(NAME##setelem e, NAME##set s)                  \
{                                                                              \
    free(e);               /* TODO: provide element-specific freeing */        \
}                                                                              \
SET_TYPE_IMPLEMENTATION_BASE(NAME)


/* For non-thread safe set types */
#define SET_TYPE_IMPLEMENTATION_BASE(NAME)                                     \
/* A zeroed element used for initializing new elements*/                       \
struct NAME##setelem_ Zero##NAME##setelem = {0};                               \
                                                                               \
/* Initialize a struct containing pointers to the set functions */             \
static struct __##NAME##SETFUNCTIONS NAME##set__functions = {                  \
    NAME##set_drain,                                                           \
    NAME##set_free,                                                            \
    NAME##set_put,                                                             \
    NAME##set_put_unique,                                                      \
    NAME##set_get,                                                             \
    NAME##set_remove,                                                          \
    NAME##set_remove_all,                                                      \
    NAME##set_copy,                                                            \
    NAME##set_copy_filtered                                                    \
};                                                                             \
                                                                               \
static NAME##setelem NAME##Setelem(NAME##set s, NAME##setkey_t k,              \
                                   NAME##setelem bnext, NAME##setelem last)    \
{                                                                              \
    NAME##setelem e = get##NAME##setelem(s);                                   \
    *e = Zero##NAME##setelem;                                                  \
    e->key = k;                                                                \
    e->bucketnext = bnext;                                                     \
    e->prev = last;                                                            \
    e->next = NULL;                                                            \
    if (last)                                                                  \
        last->next = e;                                                        \
    return (e);                                                                \
}                                                                              \
                                                                               \
/* An empty set */                                                             \
NAME##set NAME##set_new()                                                      \
{                                                                              \
    NAME##set s = ssmalloc(sizeof(struct NAME##set_));                         \
    int i;                                                                     \
                                                                               \
    for (i = 0; i < NAME##htsize; i++)                                         \
        s->table[i] = NULL;                                                    \
    s->first = NULL;                                                           \
    s->last  = NULL;                                                           \
    s->nelem = 0;                                                              \
    s->__funcs = &NAME##set__functions;                                        \
    s->__epool = NULL;                                                         \
    return (s);                                                                \
}                                                                              \
                                                                               \
/* Create or drain a set */                                                    \
void NAME##set_init(NAME##set *s)                                              \
{                                                                              \
    if (*s == NULL)                                                            \
        *s = NAME##set_new();                                                  \
    else                                                                       \
        NAME##set_drain(*s);                                                   \
}                                                                              \
                                                                               \
/* Empty out a set */                                                          \
void NAME##set_drain(NAME##set s)                                              \
{                                                                              \
    NAME##setelem e;                                                           \
    int i;                                                                     \
    for (; s->first;)                                                          \
    {                                                                          \
        e = s->first;                                                          \
        s->first = e->next;                                                    \
        free##NAME##setelem(e, s);                                             \
    }                                                                          \
    for (i = 0; i < NAME##htsize; i++)                                         \
        s->table[i] = NULL;                                                    \
    s->last  = NULL;                                                           \
    s->nelem = 0;                                                              \
}                                                                              \
                                                                               \
void NAME##set_free(NAME##set s)                                               \
{                                                                              \
    NAME##set_drain(s);                                                        \
    if (s->__epool)   /* for per-instance element recyling */                  \
    {                                                                          \
       NAME##setelem e = s->__epool, ne;                                       \
       for (; e; e = ne)                                                       \
       {                                                                       \
           ne = e->next;                                                       \
           free(e);        /* TODO: provide element-specific freeing */        \
       }                                                                       \
    }                                                                          \
    free(s);                                                                   \
}                                                                              \
                                                                               \
/* Put a key in the table */                                                   \
NAME##setelem NAME##set_put(NAME##set s, NAME##setkey_t k)                     \
{                                                                              \
    NAME##setelem *bucket;                                                     \
    bucket = &(s->table[((unsigned long int) k) % NAME##htsize]);              \
    *bucket = s->last = NAME##Setelem(s, k, *bucket, s->last);                 \
    if (!s->first)                                                             \
        s->first = s->last;                                                    \
    s->nelem ++;                                                               \
    return (s->last);                                                          \
}                                                                              \
                                                                               \
/* Put a key in the table only if it doesn't already exist */                  \
NAME##setelem NAME##set_put_unique(NAME##set s, NAME##setkey_t k)              \
{                                                                              \
    NAME##setelem tmp = NAME##set_get(s, k);                                   \
    if (!tmp)                                                                  \
        return NAME##set_put(s, k);                                            \
    return tmp;                                                                \
}                                                                              \
                                                                               \
/* Find and return an element. Notice that we return the most recent entry. */ \
NAME##setelem NAME##set_get(NAME##set s, NAME##setkey_t k)                     \
{                                                                              \
    NAME##setelem e;                                                           \
    for (e = s->table[((unsigned long int) k) % NAME##htsize]; e;              \
         e = e->bucketnext)                                                    \
        if (e->key == k) /* found it */                                        \
            return (e);                                                        \
    return (NULL);                                                             \
}                                                                              \
                                                                               \
/* Get & remove a key (NULL if not found).                                     \
 * Notice if there are multiple keys only the last one is removed.             \
 */                                                                            \
NAME##setelem NAME##set_remove(NAME##set s, NAME##setkey_t k)                  \
{                                                                              \
    NAME##setelem e, f = NULL;                                                 \
                                                                               \
    for (e = s->table[((unsigned long int) k) % NAME##htsize]; e;              \
         f = e, e = e->bucketnext)                                             \
        if (e->key == k) /* found it */                                        \
        {                                                                      \
            /* Get it off the bucket */                                        \
            if (f == NULL) /* Head of bucket list */                           \
                s->table[((unsigned long int) k)%NAME##htsize] = e->bucketnext;\
            else                                                               \
                f->bucketnext = e->bucketnext;                                 \
                                                                               \
            /* Get it off the list */                                          \
            if (e->prev)                                                       \
                e->prev->next = e->next;                                       \
            else                                                               \
                s->first = e->next;                                            \
                                                                               \
            if (e->next)                                                       \
                e->next->prev = e->prev;                                       \
            else                                                               \
                s->last = e->prev;                                             \
                                                                               \
            s->nelem --;                                                       \
            return (e);                                                        \
        };                                                                     \
    return (NULL);                                                             \
}                                                                              \
                                                                               \
/* Get & remove a key (NULL if not found)                                      \
 * If there are multiple, all of them are removed and the oldest is returned.  \
 */                                                                            \
NAME##setelem NAME##set_remove_all(NAME##set s, NAME##setkey_t k)              \
{                                                                              \
    NAME##setelem e, f = NULL, g = NULL;                                       \
                                                                               \
    for (e = s->table[((unsigned long int) k) % NAME##htsize]; e;              \
         f = e, e = e->bucketnext)                                             \
        if (e->key == k) /* found it */                                        \
        {                                                                      \
            /* Get it off the bucket */                                        \
            if (f == NULL) /* Head of bucket list */                           \
                s->table[((unsigned long int) k)%NAME##htsize] = e->bucketnext;\
            else                                                               \
                f->bucketnext = e->bucketnext;                                 \
                                                                               \
            /* Get it off the list */                                          \
            if (e->prev)                                                       \
                e->prev->next = e->next;                                       \
            else                                                               \
                s->first = e->next;                                            \
                                                                               \
            if (e->next)                                                       \
                e->next->prev = e->prev;                                       \
            else                                                               \
                s->last = e->prev;                                             \
                                                                               \
            g = e;                                                             \
            s->nelem --;                                                       \
        };                                                                     \
    return (g);                                                                \
}                                                                              \
                                                                               \
void NAME##set_copy(NAME##set to, NAME##set from)                              \
{                                                                              \
    NAME##setelem e, new;                                                      \
                                                                               \
    for (e = from->first; e; e = e->next)                                      \
        NAME##set_put(to, e->key)->value = e->value;                           \
}                                                                              \
                                                                               \
void NAME##set_copy_filtered(NAME##set to, NAME##set from,                     \
                            bool (*filter)(NAME##setelem))                     \
{                                                                              \
    NAME##setelem e;                                                           \
                                                                               \
    for (e = from->first; e; e = e->next)                                      \
    {                                                                          \
        if ((*filter)(e))                                                      \
            NAME##set_put(to, e->key)->value = e->value;                       \
    }                                                                          \
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *                   THE MAIN INTERFACE                          *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Types */
#define set(NAME) NAME##set
#define setelem(NAME) NAME##setelem

/* Functions */
#define set_new(NAME) NAME##set_new()
#define set_init(NAME,s) NAME##set_init(s)

/* Generic functions */
#define set_drain(s) \
   (assert((s) != NULL), (s)->__funcs->set_drain(s))
#define set_free(s) \
   (assert((s) != NULL), (s)->__funcs->set_free(s), s = NULL)
#define set_put(s, k) \
   (assert((s) != NULL), (s)->__funcs->set_put(s, k))
#define set_put_unique(s,k) \
   (assert((s) != NULL), (s)->__funcs->set_put_unique(s,k))
#define set_get(s, k) \
   (assert((s) != NULL), (s)->__funcs->set_get(s, k))
#define set_remove(s, k) \
   (assert((s) != NULL), (s)->__funcs->set_remove(s, k))
#define set_remove_all(s,k) \
   (assert((s) != NULL), (s)->__funcs->set_remove_all(s,k))
#define set_isempty(s) \
   (assert((s) != NULL), (s)->nelem == 0)
#define set_size(s) \
   (assert((s) != NULL), (s)->nelem)
#define set_copy(to, from) \
   (assert((to) != NULL), assert((from) != NULL), \
    (to)->__funcs->set_copy(to, from))
#define set_copy_filtered(to, from, filter) \
   (assert((to) != NULL), assert((from) != NULL), \
    (to)->__funcs->set_copy_filtered(to, from, filter))


#endif /* __SET_H__ */
