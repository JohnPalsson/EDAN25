#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include "dataflow.h"
#include "error.h"
#include "list.h"
#include "set.h"

#define NTHREADS 8

typedef struct vertex_t vertex_t;
typedef struct task_t   task_t;
typedef struct queue_t queue_t;
typedef struct queue_node_t queue_node_t;

struct queue_t {
	queue_node_t *first;
	queue_node_t *last;
	pthread_mutex_t insert_lock;
	pthread_mutex_t remove_lock;
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
	pthread_mutex_init(&q->insert_lock, NULL);
	pthread_mutex_init(&q->remove_lock, NULL);
	return q;
}

void q_free(queue_t *q)
{
	free(q);
}

void q_insert(queue_t *q, vertex_t *v)
{
	pthread_mutex_lock(&q->insert_lock);
	queue_node_t *n = malloc(sizeof(*n));
	n->succ = NULL;
	n->data = v;
	if (q->last)
		q->last->succ = n;
	else
		q->first = n;
	q->last = n;
	pthread_mutex_unlock(&q->insert_lock);
}

vertex_t *q_remove(queue_t *q)
{
	pthread_mutex_lock(&q->remove_lock);
	if (!q->first) {
		pthread_mutex_unlock(&q->remove_lock);
		return NULL;
	}
	queue_node_t *n = q->first;
	vertex_t *v = n->data;
	q->first = q->first->succ;
	free(n);
	if (!q->first)
		q->last = NULL;
	pthread_mutex_unlock(&q->remove_lock);
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
        bool                    listed;         /* on worklist                  */
	pthread_mutex_t mutex; /* set mutex */
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
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	int err = pthread_mutex_init(&v->mutex, &attr);
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

void *work(void *arg)
{
        vertex_t*       u;
        vertex_t*       v;
        set_t*          prev;
        size_t          j;
        list_t*         p;
        list_t*         h;
	queue_t *worklist = (queue_t *) arg;

	while ((u = q_remove(worklist)) != NULL) {
		pthread_mutex_lock(&u->mutex);
                u->listed = false;
		pthread_mutex_unlock(&u->mutex);

                reset(u->set[OUT]);
                for (j = 0; j < u->nsucc; ++j) {
			pthread_mutex_lock(&u->succ[j]->mutex);
                        or(u->set[OUT], u->set[OUT], u->succ[j]->set[IN]);
			pthread_mutex_unlock(&u->succ[j]->mutex);
		}

		pthread_mutex_lock(&u->mutex);
                prev = u->prev;
                u->prev = u->set[IN];
                u->set[IN] = prev;

                /* in our case liveness information... */
                propagate(u->set[IN], u->set[OUT], u->set[DEF], u->set[USE]);

                if (u->pred != NULL && !equal(u->prev, u->set[IN])) {
			pthread_mutex_unlock(&u->mutex);
                        p = h = u->pred;
                        do {
                                v = p->data;
				pthread_mutex_lock(&v->mutex);
                                if (!v->listed) {
                                        v->listed = true;
					pthread_mutex_unlock(&v->mutex);
                                        q_insert(worklist, v);
                                } else {
					pthread_mutex_unlock(&v->mutex);
				}

                                p = p->succ;

                        } while (p != h);
                } else {
			pthread_mutex_unlock(&u->mutex);
		}
        }
	return NULL;
}

void liveness(cfg_t* cfg)
{
        vertex_t*       u;
        size_t          i;
        queue_t*         worklist[NTHREADS];
	pthread_t threads[NTHREADS];
	int err;
        for (i = 0; i < NTHREADS; ++i) {
		worklist[i] = q_new();
        }

        for (i = 0; i < cfg->nvertex; ++i) {
                u = &cfg->vertex[i];

                q_insert(worklist[i%NTHREADS], u);
                u->listed = true;
        }

	for (i = 0; i < NTHREADS; ++i) {
		err = pthread_create(&threads[i], NULL, work, worklist[i]);
		if (err)
			error("Failed to create thread");
	}

	for (i = 0; i < NTHREADS; ++i) {
		err = pthread_join(threads[i], NULL);
		if (err)
			error("Failed to join thread");
	}
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
