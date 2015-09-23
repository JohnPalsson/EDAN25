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

struct sorting_args {
	void *base; // Array to sort.
	size_t n; // Number of elements in base.
	size_t s; // Size of each element.
	int (*cmp)(const void*, const void*); // Behaves like strcmp
	int threads;
};

struct quick_args {
    double *A;
    int lo;
    int hi;
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

void merge(double *base, size_t n)
{
	double *ap, *bp, *dp;
	double *unsorted = malloc(n * sizeof(double));
	if (unsorted == NULL) {
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}
	memcpy(unsorted, base, n * sizeof(double));
	ap = unsorted;
	bp = unsorted + n/2;
	dp = base;
	while (dp < base + n) {
		if (*ap < *bp && ap < unsorted + n/2 || bp == unsorted + n) {
			*dp = *ap;
			++ap;
		} else {
			*dp = *bp;
			++bp;
		}
		++dp;
	}
	free(unsorted);
}

int partition(double *A, int lo, int hi)
{
    double pivot = A[hi];
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
    if (lo < hi) {
        int p = partition(A, lo, hi);
        struct quick_args a1 = {A, lo, p};
        quick(&a1);
        struct quick_args a2 = {A, p + 1, hi};
        quick(&a2);
    }
    return NULL;
}

void *par_sort(void *ap)
{
	struct sorting_args *a = ap;
	if (a->threads > 1) {
        int pivot = partition(a->base, 0, a->n-1);
        //printf("Pivot: %d=%lf\n", pivot, ((double *)a->base)[pivot]);
		struct sorting_args a1 = {a->base, pivot+1, a->s, a->cmp, a->threads/2};
		struct sorting_args a2 = {a->base + (pivot+1)*a->s, (a->n-(pivot+1)), a->s, a->cmp, a->threads/2};
		pthread_t t;
		int err = pthread_create(&t, NULL, par_sort, &a1);
		if (err) {
			perror("Failed to create thread");
			exit(1);
		}
		par_sort(&a2);
		err = pthread_join(t, NULL);
		if (err) {
			perror("Failed to join thread");
			exit(1);
		}
		//merge(a->base, a->n);
	} else {
		qsort(a->base, a->n, a->s, a->cmp);
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
	struct sorting_args sa = {a, n, sizeof a[0], cmp, MAX_THREADS};
	par_sort(&sa);
#else
	//qsort(a, n, sizeof a[0], cmp);
    struct quick_args sa = {a, 0, n-1};
    quick(sa);
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
