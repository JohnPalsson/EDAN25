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

#define MAX_THREADS 4

struct sorting_args {
	void *base; // Array to sort.
	size_t n; // Number of elements in base.
	size_t s; // Size of each element.
	int (*cmp)(const void*, const void*); // Behaves like strcmp
	int threads;
};

static double sec(void)
{
	struct tms buf;
	times(&buf);
	long clocks_per_sec = sysconf(_SC_CLK_TCK);
	return (double) buf.tms_utime / clocks_per_sec;
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

void par_sort(struct sorting_args a)
{
	if (a.threads > 1) {
		struct sorting_args a1 = {a.base, a.n/2, a.s, a.cmp, a.threads/2};
		struct sorting_args a2 = {a.base + a.n*a.s/2, (a.n-a.n/2), a.s, a.cmp, a.threads/2};
		par_sort(a1);
		par_sort(a2);
		merge(a.base, a.n);
	} else {
		qsort(a.base, a.n, a.s, a.cmp);
	}
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
	for (i = 0; i < n; i++)
		a[i] = rand();

	/* for (i = 0; i < n; ++i) */
	/* 	printf("%lf\n", a[i]); */
	/* printf("\n"); */

	start = sec();

#ifdef PARALLEL
	struct sorting_args sa = {a, n, sizeof a[0], cmp, MAX_THREADS};
	par_sort(sa);
#else
	qsort(a, n, sizeof a[0], cmp);
#endif

	end = sec();

	for (i = 0; i < n-1; ++i) {
		//printf("%lf\n", a[i]);
		int res = a[i] <= a[i+1];
		assert(res);
	}
	//printf("%lf\n", a[n-1]);

	printf("%1.2f s\n", end - start);

	free(a);

	return 0;
}
