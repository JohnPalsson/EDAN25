#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"

list_t* new_list(void *data)
{
	list_t*		list;

	list = malloc(sizeof(list_t));

	if (list == NULL) {
		fprintf(stderr, "out of memory in %s\n", __func__);
		exit(1);
	}

	list->succ = list->pred = list;
	list->data = data;

	return list;
}

void insert_last(list_t** list1, void *p)
{
	list_t*		tmp;
	list_t*		list2;

	list2 = new_list(p);

	if (*list1 == NULL)
		*list1 = list2;
	else if (list2 != NULL) {
		(*list1)->pred->succ = list2;
		list2->pred->succ = *list1;
		tmp	= (*list1)->pred;
		(*list1)->pred = list2->pred;
		list2->pred = tmp;
	}
}


void delete_list(list_t *list)
{
	list->pred->succ = list->succ;
	list->succ->pred = list->pred;
	free(list);
}

void* remove_first(list_t** list)
{
	void*		data;
	list_t*		p;

	if (*list == NULL)
		return NULL;
	p = *list;
	data = p->data;

	if (*list == (*list)->succ)
		*list = NULL;
	else
		*list = p->succ;

	delete_list(p);

	return data;
}

void free_list(list_t** list)
{
	list_t*		p;
	list_t*		q;

	p = *list;

	if (p == NULL)
		return;

	p->pred->succ = NULL;

	while (p != NULL) {
		q = p;
		p = p->succ;
		free(q);
	}

	*list = NULL;
}
