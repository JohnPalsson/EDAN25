#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#define MAX_THREADS 8

struct quick_args {
    double *A;
    int lo;
    int hi;
    int (*cmp)(const void*, const void*);
    int threads;
};

static double sec(void)
{
	struct timespec ts;
	int err = clock_gettime(CLOCK_MONOTONIC, &ts);
	if (err) {
		perror("Failed to get time");
		exit(1);
	}
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

int partition(double *A, int lo, int hi)
{
    double pivot = A[lo];
    int i = lo - 1;
    int j = hi + 1;
    while (1) {
        do {
            j--;
        } while (A[j] > pivot);
        do {
            i++;
        } while (A[i] < pivot);
        if (i < j) {
            double tmp = A[i];
            A[i] = A[j];
            A[j] = tmp;
        } else {
            return j;
        }
    }
}

void *quick(void *ap)
{
    struct quick_args *a = ap;
    double *A = a->A;
    int lo = a->lo;
    int hi = a->hi;
    int threads = a->threads;
    if (lo < hi) {
        int p = partition(A, lo, hi);
        struct quick_args a1 = {A, lo, p, NULL, threads/2};
	pthread_t t;
	if (threads > 1) {
	    int err = pthread_create(&t, NULL, quick, &a1);
	    if (err) {
		perror("Failed to create thread");
		exit(1);
	    }
	} else {
	    quick(&a1);
	}
        struct quick_args a2 = {A, p + 1, hi, NULL, threads/2};
        quick(&a2);
	if (threads > 1) {
	    int err = pthread_join(t, NULL);
	    if (err) {
		perror("Failed to join thread");
		exit(1);
	    }

	}
    }
    return NULL;
}

static int cmp(const void* ap, const void* bp)
{
	const double a = *(const double*)ap;
	const double b = *(const double*)bp;
	return (a > b) - (a < b);
}

int main(int ac, char** av)
{
	int		n = 2000000;
	int		i;
	double*		a;
	double		start, end;

	if (ac > 1)
		sscanf(av[1], "%d", &n);

	srand(getpid());

	a = malloc(n * sizeof a[0]);
	for (i = 0; i < n; i++) {
		a[i] = rand();
        //printf("%lf\n", a[i]);
    }
    printf("\n");

	start = sec();

#ifdef PARALLEL
	struct quick_args sa = {a, 0, n-1, NULL, MAX_THREADS};
	quick(&sa);
#else
	qsort(a, n, sizeof a[0], cmp);
#endif

	end = sec();

    for (i = 0; i < n; ++i) {
        //printf("%lf\n", a[i]);
    }

	for (i = 0; i < n-1; ++i) {
		int res = a[i] <= a[i+1];
		assert(res);
	}

	printf("%1.2f s\n", end - start);

	free(a);

	return 0;
}
