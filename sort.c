#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <float.h>
#include <ctype.h>
#include "sort.h"

struct sorts {  void (*f)(double *, unsigned int ); int slow; char *name;}lsorts[]={
{selectionsort,1,"selectionsort"},
{selsort,1,"improved selectionsort"},
{libqsort,0,"libqsort"},
{insertionsort,1,"insertionsort"},
{msort,0,"merge sort"},
{xmsort,0,"xmerge sort"},
{qs,0,"quick sort"}
};
#define LSORTS 6


#define ASIZE 100000		//default size for test array
double t[ASIZE];		// the test array
#define DEFAULT_REPEAT 100

// Test diagnostics 
void viewsort(double *, unsigned int);
void initrandomd(double *, unsigned int);
void initreverse(double *, unsigned int);
unsigned int issorted(double *, unsigned int);

void timetest(double *p, const unsigned int n,
	      void (*f) (double *, unsigned int), unsigned int repeat);
void rtimetest(double *p, const unsigned int n,
	       void (*f) (double *, unsigned int), unsigned int repeat);

struct options {
    char code;
    char *name;
} opts[] = {
    {
    's', "simple"}, {
    'p', "performance rate of growth"}, {
    'b', "complete"}, {
'a', "standard"},};

char *tname; // name of the test
int main(int argc, char **argv)
{
    unsigned int repeat = DEFAULT_REPEAT;
    char code = 'a';
    int i;

    srand(time(NULL));
    if (argc == 1)
	fprintf(stderr,
		"sort code [repeatcount]\n where code in s (simple),p (performance) ,b (include exponential sorts) ,a (standard)\n");
	tname = argv[0];
    if (argc > 1) {
	if (isdigit(argv[argc - 1][0])) {	//count 
	    repeat = atoi(argv[argc - 1]);
	    if (repeat <= 0)
		repeat = DEFAULT_REPEAT;
	}
	if (isalpha(argv[1][0])) {
	    code = argv[1][0];
	}
    }

    initrandomd(t, ASIZE);
    printf
	("Test the test: Random array first out of order in position  %d\n",
	 issorted(t, ASIZE));
    switch (code) {
    case 's':
	printf("Tests on %d elements\n", ASIZE);

	for (i = 0; i < LSORTS; i++) {
	    if (lsorts[i].slow)
		continue;
	    printf("%s %s (micros) Random: ",
		   tname,lsorts[i].name);
	    fflush(stdout);
	    timetest(t, ASIZE, lsorts[i].f, 1);
	    printf("Reversed ");
	    fflush(stdout);
	    rtimetest(t, ASIZE, lsorts[i].f, 1);
	}
	break;
    case 'b':
	for (i = 0; i < LSORTS; i++) {
	    initrandomd(t, ASIZE);
	    printf("%s %s (micros) Random: ",
		   tname,lsorts[i].name);
	    fflush(stdout);
	    timetest(t, ASIZE, lsorts[i].f, 1);
	    printf("Reversed ");
	    fflush(stdout);
	    rtimetest(t, ASIZE, lsorts[i].f, 1);
	}
	fflush(stdout);
	break;
    case 'p':
	for (int z = 100; z < ASIZE ; z *= 10) {
	    printf("Sorting %d things\n", z);
	for (i = 0; i < LSORTS; i++) {
	    initrandomd(t, z);
	    printf("%s %s (micros) Random: ",
		   tname,lsorts[i].name);
	    fflush(stdout);
	    timetest(t, z, lsorts[i].f, 1);
	    printf("Reversed ");
	    fflush(stdout);
	    rtimetest(t, z, lsorts[i].f, 1);
	}
	}
	break;
    case 'a':
	{
	    const unsigned int x = 10 * ASIZE;
	    double *z = malloc(sizeof(double) * x);
	    if (!z) {
		fprintf(stderr, "Can't allocate larger array for tests\n");
	    } else {
		printf("Testing large arrays %d times each\n", repeat);
		printf("%s msort (microseconds)\n",tname);
	for (i = 0; i < LSORTS; i++) {
	    if (lsorts[i].slow) continue;
	    printf("%s %s (micros) Random: ", tname,lsorts[i].name);
	    fflush(stdout);
	    timetest(z, x, lsorts[i].f, repeat);
	    printf("Reversed ");
	    fflush(stdout);
	    rtimetest(z, x, lsorts[i].f, repeat);
	}
	    free(z);
	}
	}
	break;
    default:
	fprintf(stderr, "No such option %c\n", code);

    }				//end switch
}


static inline double timenow(void);
static inline void tnow(struct timespec *ts);
static inline double tsince(struct timespec *a);
void
timetest(double *p, const unsigned int n,
	 void (*f) (double *, unsigned int), unsigned int repeat)
{
    char *newline;
    double t;
    int error;
    if(repeat >1)newline = "\n";
    else newline = " ";
    while (repeat-- > 0) {
	initrandomd(p, n);
	t = timenow();
	f(p, n);
	t = timenow() - t;
	if ((error = issorted(p, n))) {
	    fprintf(stderr, "sort fails at %d\n", error);
	    viewsort(p, 5);
	}
	printf(" %ld%s", (unsigned long) t,newline);
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
	if ((error = issorted(p, n))) {
	    fprintf(stderr, "sort fails at %d\n", error);
	    viewsort(p, 5);
	}
	printf("%ld\n", (unsigned long) t);
    }
}

static inline void tnow(struct timespec *ts)
{
    clock_gettime(CLOCK_REALTIME, ts);
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
	if ((error = issorted(p, n))) {
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
	*p++ = (double) rand() / (double) (RAND_MAX) * DBL_MAX;
}

void initreverse(double *p, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
	*p++ = DBL_MAX - (i + 0.2);
}

unsigned int issorted(double *p, unsigned int n)
{
    for (unsigned int i = 0; i < n - 1; i++)
	if (*(p + i + 1) < *(p + i))
	    return (i + 1);
    return 0;
}


