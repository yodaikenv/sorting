/* (c) Victor Yodaiken, 2017
   a curated set of artisanal sorting algorithms and a test/performance
   framework.

   selection standard
   selection redone
   insertion
   quicksort
   mergesort

   compared agains libc/quicksort

   for simplicity this is all about sorting an array of doubles.

   An experiment in (informal) mathematical specification 
   See http://www.yodaiken.com/2017/10/29/quicksort-without-pseudo-code/

   All of the sorting code was written and modified against the functional spec.
   I didn't think of specifying the test frame - that would probably be
   harder.

*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <float.h>

#define ASIZE 100000
double t[ASIZE];		// the test array

// Test diagnostics 
void viewsort(double *p, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
	printf("p[%d] = %g\n", i, *(p + i));
    return;
}

// testing 

void initrandomd(double *p, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
	p[i] = DBL_MAX / (double) rand();
}

void initreverse(double *p, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
	*p++ = DBL_MAX - (i + 0.2);
}

unsigned int notsorted(double *p, unsigned int n)
{
    for (unsigned int i = 0; i < n - 1; i++)
	if (*(p + i + 1) < *(p + i))
	    return (i + 1);
    return 0;
}


void libqsort(double *p, unsigned int n);
void selectionsort(double *p, unsigned int n);
void selsort(double *p, unsigned int n);
void msort(double *p, unsigned int n);
void qs(double *p, unsigned int n);
#define NAMESIZE 100 // this is for a string naming the method
struct list { void (*f)(double *, unsigned int); char name[NAMESIZE]; };
struct list methods[] = { {libqsort, "Libc quicksort"},
				{selectionsort, "Standard Selection Sort"},
				{selsort, "Functional Selection Sort"},
				{msort, "Merge Sort"},
				{qs, "Quick Sort"},
				{0 },
				};

// to use libc quicksort with the standard API used here
int dcmp(const void *a, const void *b)
{
    return (*(double *) a > *(double *) b);
}

void libqsort(double *p, unsigned int n)
{
    qsort((void *) p, n, sizeof(double), dcmp);
}

// swap utility used by mergesort and quicksort
static inline void pswap(double *a, double *b)
{
    double t = *a;
    *a = *b;
    *b = t;
}

//Selection sort
// https://stackoverflow.com/questions/31691118/what-sorting-algorithm-is-this-3-liner
void selectionsort(double *p, unsigned int n)
{
    for (int i = 0; i < n; i++) {
	int best = i;
	for (int j = i + 1; j < n; j++)
	    if (*(p + best) > *(p + j))
		best = j;
	if (best != i)
	    pswap(p + best, p + i);
    }
}

// a nicer version of selection sort based on the math
static inline double *thebest(double *p, unsigned int n)
{
    unsigned int b = 0;
    for (int i = 0; i < n; i++)
	if (*(p + i) < *(p + b))
	    b = i;
    return p + b;
}

void selsort(double *p, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
	pswap(p + i, thebest(p + i, n - i));


}

//insertion sort
void insertionsort(double *p, unsigned int n)
{
    for (int i = 1; i < n; i++) {
	for (int j = i; j > 0; j--)
	    if (*(p + j - 1) > *(p + j))
		pswap(p + j - 1, p + j);
	    else
		break;
    }
}

//Mergesort
void merge(double *src, unsigned int n1, unsigned int n2, double *scratch)
{
// adjacent sequences (src[0] ... src[n1-1])(src[n1],... src[n1+n2 -1])

    int n = n1 + n2;
    double *s1, *s2, *d;
    for (s1 = src, s2 = src + n1, d = scratch;
	 s1 < src + n1 && s2 < src + n;) {
	if (*s1 <= *s2)
	    *d++ = *s1++;
	else
	    *d++ = *s2++;
    }				//at this point one subsequence is done- only one of the following runs
    while (s1 < src + n1)
	*d++ = *s1++;
    while (s2 < src + n)
	*d++ = *s2++;
//now copy it back - 
    for (s1 = src, d = scratch; s1 < src + n;)
	*s1++ = *d++;
    return;
}


// recursive mergesort with the scratch array as a parameter
// there's an efficiency improvement to be made to alternate arrays so no copying back.
// but I'm not making it yet.
//  does not check for length < 2 because that is done at a higher level
void rmsort(double *p, unsigned int n, double *s)
{
    if (n == 2) {
	if (*p > *(p + 1)) {
	    pswap(p, p + 1);
	}
	return;
    } else {
	unsigned int half = n / 2;	// done in sequence 
	unsigned int otherhalf = half + (n & 1 ? 1 : 0);
	if (half > 1)
	    rmsort(p, half, s);
	if (otherhalf > 1)
	    rmsort(p + half, otherhalf, s);
	merge(p, half, otherhalf, s);
    }
}

void msort(double *p, unsigned int n)
{

    if (n < 2)
	return;
    if (n == 2) {
	if (*p > *(p + 1)) {
	    pswap(p, p + 1);
	}
	return;
    }
    double *scratch = malloc(n * sizeof(double));	//won't work if parallelized
    if (!scratch) {
	fprintf(stderr, "Merge sort cannot allocate scratch\n");
	return;
    }

    rmsort(p, n, scratch);
    free(scratch);
}

//quicksort
// would be better to randomize pivot selection
unsigned int partition(double *p, int n)
{
    unsigned int L = 0;
    unsigned int R = n - 1;
    double pivot = *p;

    if (n < 2)
	return 0;

    do {
	//initialize L
	while ((*(p + L + 1) <= pivot)) {
	    L++;
	    if ((L == n - 1) || (L == R))
		goto done;
	}
	// initialize R
	while ((*(p + R) > pivot)) {
	    R--;
	    if (R == L)
		goto done;
	}
	pswap(p + L + 1, p + R);	// *(p+R) <= pivot 
    }
    while (1);
  done:
    pswap(p, p + L);		// move pivot
    return L;
}

// the actual sort
void qs(double *p, unsigned int n)
{
    unsigned int pivotindex;
    if (n < 2)
	return;
    pivotindex = partition(p, n);
    if (pivotindex > 0) {
	qs(p, pivotindex);
    }
    if (pivotindex + 1 < n) {
	qs(p + pivotindex + 1, n - (pivotindex + 1));
    }

}







struct tsince {
    unsigned long us;
    int overflows;
    int overflowns;
};

void timetest(double *p, const unsigned int n,
	      void (*f) (double *, unsigned int), unsigned int repeat);
void rtimetest(double *p, const unsigned int n,
	       void (*f) (double *, unsigned int), unsigned int repeat);


#include <ctype.h>
#define DEFAULT_REPEAT 100
int main(int argc, char **argv)
{
    unsigned int repeat = DEFAULT_REPEAT;
    char *code = "a";
#define DOSIMPLE (strchr(code,'s') != NULL)
#define DOBRUTE (strchr(code,'b') != NULL)

    /* arguments
       help -- print help message
       none - do default tests with default repeat
       [code] [count]

       code is fpalqm 
       s - simple 
       performance - tests to show rate of growth
       b - test the slow guys too
       a - all standard
       l - libqsort
       q - qs
       m - merge

       count is repeatcount for standard tests
     */
    printf("%s \n", argv[0]);
    if (argc > 1) {
	if (argc == 2 && !strncmp("help", argv[1], 4)) {
	    printf
		("sort [options] \n Tests sort algorithms for time and accuracy. Lqsort is the libc quicksort which is used for comparison\n");
	    printf
		("Standard tests use randomly intialized arrays, reverse tests use arrays initial in reverse order\n");
	    printf
		("Options:\n s [b]  simple tests (option b forces tests of insertion/selectionsort)\n");
	    printf
		("Options:\n a [count]  big tests (count  is number of repeats of each test)\n");
	    exit(0);
	}
	if (isdigit(argv[argc - 1][0])) {	//count 
	    repeat = atoi(argv[argc - 1]);
	    if (repeat <= 0)
		repeat = DEFAULT_REPEAT;
	}
	if (isalpha(argv[1][0])) {
	    code = argv[1];
	}
    }

    srand(time(NULL));

    if (DOSIMPLE) {		//checking to see if they work;
	printf("Making sure methods work on small example %d elements\n",
	       ASIZE);

	initrandomd(t, ASIZE);
	if (!notsorted(t, ASIZE)) {
	    fprintf(stderr,
		    "Fatal ERROR: Randomized array is in order notsorted=%d\n",
		    notsorted(t, ASIZE));
	    exit(-1);
	}

	printf("\nTime testing libqsort (microseconds) Random: ");
	timetest(t, ASIZE, libqsort, 1);
	printf("Reversed ");
	rtimetest(t, ASIZE, libqsort, 1);
	printf("\nTime testing merge sort (microseconds) Random: ");
	timetest(t, ASIZE, msort, 1);
	printf(" Reversed ");
	rtimetest(t, ASIZE, msort, 1);
	printf("\nTime testing qs (microseconds) Random: ");
	timetest(t, ASIZE, qs, 1);
	printf(" Reversed ");
	rtimetest(t, ASIZE, qs, 1);

	if (DOBRUTE)		// this is damn slow
	{
	    printf("\n Testing exponential time sorts (slow)");
	    printf("\n--------------\n insertion sort");
	    for (int z = 100; z < ASIZE * 10; z *= 10) {
		printf("\n Sorting %d things ", z);
		timetest(t, z, insertionsort, 1);
		printf("Reversed ");
		rtimetest(t, z, insertionsort, 1);
	    }

	    printf("\n -------------------\n selectionsort");
	    for (int z = 100; z < ASIZE * 10; z *= 10) {
		printf("\n Sorting %d things ", z);
		timetest(t, z, selectionsort, 1);
		printf("Reversed ");
		rtimetest(t, z, selectionsort, 1);
	    }

	    printf("\n -------------------\nselectionsort version 2");
	    for (int z = 100; z < ASIZE * 10; z *= 10) {
		printf("\n Sorting %d things ", z);
		timetest(t, z, selsort, 1);
		printf("Reversed ");
		rtimetest(t, z, selsort, 1);
	    }
	    printf("\n The end \n");

	}
	return 0;
    }


    {
	const unsigned int x = 10 * ASIZE;
	double *z = malloc(sizeof(double) * x);
	if (!z) {
	    fprintf(stderr, "Can't allocate larger array for tests\n");
	    exit(-1);
	} else {
	    printf("Testing large arrays %d times each\n", repeat);
	    printf
		("\nTime testing libqsort (microseconds) for reference\n");
	    timetest(z, x, libqsort, repeat);
	    printf("\nTime testing msort (microseconds)\n");
	    timetest(z, x, msort, repeat);
	    printf("\nTime testing qs (microseconds)\n");
	    timetest(z, x, qs, repeat);
	}
	putchar('\n');
	free(z);
    }

}


static inline double timenow(void);
void
timetest(double *p, const unsigned int n,
	 void (*f) (double *, unsigned int), unsigned int repeat)
{

    double t;
    int error;
    while (repeat-- > 0) {
	initrandomd(p, n);
	t = timenow();
	f(p, n);
	t = timenow() - t;
	if ((error = notsorted(p, n))) {
	    fprintf(stderr, "sort fails at %d\n", error);
	    viewsort(p, 5);
	}
	printf("%ld ", (unsigned long) t);
    }
}

void
rtimetest(double *p, const unsigned int n,
	  void (*f) (double *, unsigned int), unsigned int repeat)
{

    double t;
    int error;
    while (repeat-- > 0) {
	initreverse(p, n);
	t = timenow();
	f(p, n);
	t = timenow() - t;
	if ((error = notsorted(p, n))) {
	    fprintf(stderr, "sort fails at %d\n", error);
	    viewsort(p, 5);
	}
	printf("%ld", (unsigned long) t);
    }
}

void
xtimetest(double *p, const unsigned int n,
	  void (*f) (double *, unsigned int), unsigned int repeat,
	  int code)
{

    double t;
    int error;
    while (repeat-- > 0) {
	initrandomd(p, n);
	t = timenow();
	f(p, n);
	t = timenow() - t;
	if ((error = notsorted(p, n))) {
	    fprintf(stderr, "sort fails at %d\n", error);
	    viewsort(p, 5);
	}
	printf("%ld\n", (unsigned long) t);
    }
}


static inline double timenow(void)
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((double) tp.tv_sec * 1000000.0) +
	(((double) tp.tv_nsec) / 1000.0);
}
