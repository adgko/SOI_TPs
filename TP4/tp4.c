/** @file
		@brief tp4
*/

#include "/usr/include/x86_64-linux-gnu/sys/types.h"
#include "/usr/include/unistd.h"
#include <stdio.h>

#define align4(x) (((((x) - 1) >> 2) << 2) + 4)
#define BLOCK_SIZE 20

struct s_block
{
	size_t size;
	struct s_block *next;
	struct s_block *prev;
	int free;
	void *ptr;
	char data[1];
};

typedef struct s_block *t_block;

void *base = NULL;

/** Devuelve el bloque de una dirección determinada
*/
t_block 
get_block (void *p)
{
	char *tmp;
	tmp = p;
	return (p = tmp -= BLOCK_SIZE);
}

/** Permite saber si una dirección es válida para realizar un free
*/
int
valid_addr (void *p)
{
	if (base)
		{
			if ((p > base) && (p < sbrk (0)))
				{
					return (p == (get_block (p))->ptr);
				}
		}
	return (0);
}

/** Busca un bloque del tamaño adecuado y lo devuelve
*/
t_block 
find_block (t_block *last, size_t size)
{
	t_block b = base;
	while (b && !(b -> free && b->size >= size))
		{
			*last = b;
			b = b->next;
		}
	return(b);
}

/** Añade un nuevo bloque a la heap
*/
t_block 
extend_heap (t_block last, size_t s)
{
	long sb;
	t_block b;
	b = sbrk (0);
	sb = (long) sbrk(BLOCK_SIZE + s);
	if (sb < 0)
		{
			return (NULL);
		}
	b->size = s;
	b->next = NULL;
	b->prev = last;
	b->ptr = b->data;
	if (last)
		{
			last->next = b;
		}
	b->free = 0;
	return (b);
}

/** Divide un bloque de acuerdo al tamaño
*/
void
split_block (t_block b, size_t s)
{
	t_block new;
	new = (t_block) (b->data + s);
	new->size = (b->size - s - BLOCK_SIZE);
	new->next = b->next;
	new->prev = b;
	new->free = 1;
	new->ptr = new->data;
	b->size = s;
	b->next = new;
	if (new->next)
		{
			new->next->prev = new;
		}
}

/** Reserva bloques de memoria de un determinado tamaño, y devuelve un puntero
	 	al espacio reservado
*/
void
*a_malloc (size_t size)
{
	t_block	b, last;
	size_t s;
	s = align4 (size);
	if (base)
		{
			last = base;
			b = find_block (&last, s);
			if (b)
				{
					if ((b->size - s) >= (BLOCK_SIZE + 4))
						{
							split_block (b, s);
						}
					b->free = 0;
				}
			else
				{
					b = extend_heap (last, s);
					if (!b)
						{
							return (NULL);
						}
				}
		}
	else
		{
			b = extend_heap (NULL, s);
			if (!b)
				{
					return (NULL);
				}
			base = b;
		}
	return (b->data);
}

/** Fusiona bloques libres, en caso de que haya varios que lo estén
*/
t_block
fusion (t_block b)
{
	if (b->next && b->next->free)
		{
			b->size += (BLOCK_SIZE + b->next->size);
			b->next = b->next->next;
			if (b->next)
				{
					b->next->prev = b;
				}
		}
	return (b);
}

/** Libera un determinado espacio de memoria
*/
void
a_free (void *p)
{
	if (valid_addr (p))
		{
			t_block b;
			b = get_block (p);
			b->free = 1;
			if ((b->prev) && (b->prev->free))
				{
					b = fusion (b->prev);
				}
			if (b->next)
				{
					fusion(b);
				}
			else
				{
					if (b->prev)
						{
							b->prev->next = NULL;
						}
					else
						{
							base = NULL;
						}
					brk (b);
				}
		}
}

int 
main()
{
	void *i;
	void *j;
	i = a_malloc (1);
	j = a_malloc (1);

	printf ("i = %p \n", i);
	printf ("j = %p \n", j);
	
	a_free (i);
	printf ("i liberada\n");
	a_free (j);
	printf ("j liberada\n");
	return 0;
}