#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <inttypes.h>
#include <pthread.h>
#include "dataflow.h"
#include "error.h"
#include "list.h"
#include "set.h"

#define NTHREADS 4

typedef struct vertex_t vertex_t;
typedef struct task_t   task_t;
typedef struct queue_t queue_t;
typedef struct queue_node_t queue_node_t;

struct queue_t {
	queue_node_t *first;
	pthread_spinlock_t remove_lock;
};

struct queue_node_t {
	queue_node_t *succ;
	vertex_t *data;
};

queue_t *q_new(void)
{
	queue_t *q = calloc(1, sizeof(queue_t));
	if (!q)
		error("Failed to allocate memory");
	pthread_spin_init(&q->remove_lock, PTHREAD_PROCESS_PRIVATE);
	return q;
}

void q_free(queue_t *q)
{
	free(q);
}

void q_insert(queue_t *q, vertex_t *v)
{
	pthread_spin_lock(&q->remove_lock);
	queue_node_t *n = malloc(sizeof(*n));
	n->succ = q->first;
	n->data = v;
	q->first = n;
	pthread_spin_unlock(&q->remove_lock);
}

vertex_t *q_remove(queue_t *q)
{
	pthread_spin_lock(&q->remove_lock);
	if (!q->first) {
		pthread_spin_unlock(&q->remove_lock);
		return NULL;
	}
	queue_node_t *n = q->first;
	vertex_t *v = n->data;
	q->first = q->first->succ;
	free(n);
	pthread_spin_unlock(&q->remove_lock);
	return v;
}

/* cfg_t: a control flow graph. */
struct cfg_t {
	size_t                  nvertex;        /* number of vertices           */
	size_t                  nsymbol;        /* width of bitvectors          */
	vertex_t*               vertex;         /* array of vertex              */
};

/* vertex_t: a control flow graph vertex. */
struct vertex_t {
	size_t                  index;          /* can be used for debugging    */
	set_t*                  set[NSETS];     /* live in from this vertex     */
	set_t*                  prev;           /* alternating with set[IN]     */
	size_t                  nsucc;          /* number of successor vertices */
	vertex_t**              succ;           /* successor vertices           */
	list_t*                 pred;           /* predecessor vertices         */
	_Atomic bool            listed;         /* on worklist                  */
	pthread_spinlock_t listmutex; /* set mutex */
	pthread_spinlock_t inmutex; /* set mutex */
};

static void clean_vertex(vertex_t* v);
static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ);

cfg_t* new_cfg(size_t nvertex, size_t nsymbol, size_t max_succ)
{
	size_t          i;
	cfg_t*          cfg;

	cfg = calloc(1, sizeof(cfg_t));
	if (cfg == NULL)
		error("out of memory");

	cfg->nvertex = nvertex;
	cfg->nsymbol = nsymbol;

	cfg->vertex = calloc(nvertex, sizeof(vertex_t));
	if (cfg->vertex == NULL)
		error("out of memory");

	for (i = 0; i < nvertex; i += 1)
		init_vertex(&cfg->vertex[i], i, nsymbol, max_succ);

	return cfg;
}

static void clean_vertex(vertex_t* v)
{
	int             i;

	for (i = 0; i < NSETS; i += 1)
		free_set(v->set[i]);
	free_set(v->prev);
	free(v->succ);
	free_list(&v->pred);
}

static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ)
{
	int             i;

	v->index        = index;
	v->succ         = calloc(max_succ, sizeof(vertex_t*));

	if (v->succ == NULL)
		error("out of memory");

	for (i = 0; i < NSETS; i += 1)
		v->set[i] = new_set(nsymbol);

	v->prev = new_set(nsymbol);
	int err = pthread_spin_init(&v->inmutex, PTHREAD_PROCESS_PRIVATE);
	if (err)
		error("Failed to init mutex");
	err = pthread_spin_init(&v->listmutex, PTHREAD_PROCESS_PRIVATE);
	if (err)
		error("Failed to init mutex");
}

void free_cfg(cfg_t* cfg)
{
	size_t          i;

	for (i = 0; i < cfg->nvertex; i += 1)
		clean_vertex(&cfg->vertex[i]);
	free(cfg->vertex);
	free(cfg);
}

void connect(cfg_t* cfg, size_t pred, size_t succ)
{
	vertex_t*       u;
	vertex_t*       v;

	u = &cfg->vertex[pred];
	v = &cfg->vertex[succ];

	u->succ[u->nsucc++ ] = v;
	insert_last(&v->pred, u);
}

bool testbit(cfg_t* cfg, size_t v, set_type_t type, size_t index)
{
	return test(cfg->vertex[v].set[type], index);
}

void setbit(cfg_t* cfg, size_t v, set_type_t type, size_t index)
{
	set(cfg->vertex[v].set[type], index);
}

void single(vertex_t *u, queue_t *worklist){
	vertex_t*       v;
	set_t*          prev;
	size_t          j;
	list_t*         p;
	list_t*         h;
	atomic_store(&u->listed, false);

	reset(u->set[OUT]);
	for (j = 0; j < u->nsucc; ++j) {
		pthread_spin_lock(&u->succ[j]->inmutex);
		or(u->set[OUT], u->set[OUT], u->succ[j]->set[IN]);
		pthread_spin_unlock(&u->succ[j]->inmutex);
	}

	pthread_spin_lock(&u->inmutex);
	prev = u->prev;
	u->prev = u->set[IN];
	u->set[IN] = prev;

	/* in our case liveness information... */
	propagate(u->set[IN], u->set[OUT], u->set[DEF], u->set[USE]);

	if (u->pred != NULL && !equal(u->prev, u->set[IN])) {
		pthread_spin_unlock(&u->inmutex);
		p = h = u->pred;
		do {
			v = p->data;
			bool expected = false;
			if (atomic_compare_exchange_strong(&v->listed, &expected, true)) {
				v->listed = true;
				q_insert(worklist, v);
			} else {
			}

			p = p->succ;

		} while (p != h);
	} else {
		pthread_spin_unlock(&u->inmutex);
	}
}

void first(vertex_t *u, queue_t *worklist){
	vertex_t*       v;
	set_t*          prev;
	size_t          j;
	list_t*         p;
	list_t*         h;
	// printf("%s\n", "entering first" );

	for (j = 0; j < u->nsucc; ++j) {
		bool expected = false;
		// printf("%s\n", "found predecessor" );
		v = u-> succ[j];
		if (atomic_compare_exchange_strong(&v->listed, &expected, true)) {
			// printf("%s\n", "was not listed" );

			first(v, worklist);
		}
	}
	atomic_store(&u->listed, false);

	reset(u->set[OUT]);
	for (j = 0; j < u->nsucc; ++j) {
		pthread_spin_lock(&u->succ[j]->inmutex);
		or(u->set[OUT], u->set[OUT], u->succ[j]->set[IN]);
		pthread_spin_unlock(&u->succ[j]->inmutex);
	}

	pthread_spin_lock(&u->inmutex);
	prev = u->prev;
	u->prev = u->set[IN];
	u->set[IN] = prev;

	/* in our case liveness information... */
	propagate(u->set[IN], u->set[OUT], u->set[DEF], u->set[USE]);

	if (u->pred != NULL && !equal(u->prev, u->set[IN])) {
		pthread_spin_unlock(&u->inmutex);
		p = h = u->pred;
		do {
			v = p->data;
			bool expected = false;
			if (atomic_compare_exchange_strong(&v->listed, &expected, true)) {
				q_insert(worklist, v);
			}
			p = p->succ;
		} while (p != h);
	} else {
		pthread_spin_unlock(&u->inmutex);
	}
}


void *work(void *arg)
{
	vertex_t*       u;
	queue_t *worklist = (queue_t *) arg;
	u = q_remove(worklist);
	if(u!=NULL){
		first( u, worklist );
	}
	while ((u = q_remove(worklist)) != NULL) {
		single(u, worklist);
	}
	return NULL;
}

void liveness(cfg_t* cfg)
{
	vertex_t*       u;
	size_t          i;
	queue_t*         worklist = q_new();
	pthread_t threads[NTHREADS];
	int err;

	for (i = 0; i < NTHREADS; ++i) {
		u = &cfg->vertex[i];
		q_insert(worklist, u);
		u->listed = true;
	}

	for (i = 0; i < NTHREADS; ++i) {
		err = pthread_create(&threads[i], NULL, work, worklist);
		if (err)
			error("Failed to create thread");
	}

	for (i = 0; i < NTHREADS; ++i) {
		err = pthread_join(threads[i], NULL);
		if (err)
			error("Failed to join thread");
	}
	q_free(worklist);
}

void print_sets(cfg_t* cfg, FILE *fp)
{
	size_t          i;
	vertex_t*       u;

	for (i = 0; i < cfg->nvertex; ++i) {
		u = &cfg->vertex[i];
		fprintf(fp, "use[%zu] = ", u->index);
		print_set(u->set[USE], fp);
		fprintf(fp, "def[%zu] = ", u->index);
		print_set(u->set[DEF], fp);
		fputc('\n', fp);
		fprintf(fp, "in[%zu] = ", u->index);
		print_set(u->set[IN], fp);
		fprintf(fp, "out[%zu] = ", u->index);
		print_set(u->set[OUT], fp);
		fputc('\n', fp);
	}
}
