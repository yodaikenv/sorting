
static void libqsort(double *p, unsigned int n);
static void selectionsort(double *p, unsigned int n);
static void insertionsort(double *p, unsigned int n);
static void selsort(double *p, unsigned int n);
static void msort(double *p, unsigned int n);
static void qs(double *p, unsigned int n);

// to use libc with the standard API used here
int dcmp(const void *a, const void *b)
{
    return (*(double *) a > *(double *) b);
}

static void libqsort(double *p, unsigned int n)
{
    qsort((void *) p, n, sizeof(double), dcmp);
}

typedef int (*cmpfun) (const void *, const void *);

// swap utility used by mergesort and quicksort
static inline void pswap(double *a, double *b)
{
    double t = *a;
    *a = *b;
    *b = t;
}

//Selection sort
// https://stackoverflow.com/questions/31691118/what-sorting-algorithm-is-this-3-liner
static void selectionsort(double *p, unsigned int n)
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

static void insertionsort(double *p, unsigned int n)
{
    for (int i = 1; i < n; i++) {
	for (int j = i; j > 0; j--)
	    if (*(p + j - 1) > *(p + j))
		pswap(p + j - 1, p + j);
	    else
		break;
    }
}

static inline double *thebest(double *p, unsigned int n)
{
    unsigned int b = 0;
    for (int i = 0; i < n; i++)
	if (*(p + i) < *(p + b))
	    b = i;
    return p + b;
}

static void selsort(double *p, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
	pswap(p + i, thebest(p + i, n - i));


}

//Mergesort
static void merge(double *src, unsigned int n1, unsigned int n2,
		  double *scratch)
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
static void rmsort(double *p, unsigned int n, double *s)
{
    /* if (n < 2)
       return; */
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
	    rmsort(p + half, otherhalf, s + half);
	merge(p, half, otherhalf, s);
    }
}

static void msort(double *p, unsigned int n)
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

// improved mergesort

//Mergesort
static void xmerge(double *src, unsigned int n1, unsigned int n2,
		   double *scratch)
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
//no copy back - result is always in scratch
    return;
}


// improved recursive mergesort with the scratch array as a parameter
// 1/2 as many copies: data is in p and s is scratch
// if phase is 1, data ends up in s (scratch) 
static void xrmsort(double *p, unsigned int n, double *s, int phase)
{
    if (n == 1) {
	if (phase)
	    *s = *p;
    } else if (n == 2) {
	if (*p > *(p + 1)) {
	    pswap(p, p + 1);
	}
	if (phase) {		// have to copy out
	    *s = *p;
	    *(s + 1) = *(p + 1);
	}
    } else {
	int newphase = (phase ? 0 : 1);
	unsigned int half = n / 2;	// done in sequence 
	unsigned int otherhalf = half + (n & 1 ? 1 : 0);
	if (half > 1)
	    xrmsort(p, half, s, newphase);
	else {
//	    if (half == 1)
		*(s) = *(p);
	}
	if (otherhalf <= 1) {
	    fprintf(stderr, "otherhalf =%d\n", otherhalf);
	    exit(0);
	}
	xrmsort(p + half, otherhalf, s + half, newphase);
	if (phase == 0) {
	    xmerge(s, half, otherhalf, p);
	} else {
	    xmerge(p, half, otherhalf, s);	//for(int i=0;i < n;i++) *(p+i)= *(s+i);
	}
    }
}


static void xmsort(double *p, unsigned int n)
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

    xrmsort(p, n, scratch, 0);
    free(scratch);
}



//quicksort
static unsigned int partition(double *p, int n)
{
    unsigned int alpha = 0;
    unsigned int beta = n - 1;
    double pivot = *p;

    if (n < 2)
	return 0;
    do {
	//initialize alpha
	while ((*(p + alpha + 1) <= pivot)) {
	    alpha++;
	    if ((alpha == n - 1) || (alpha == beta))
		goto done;
	}
	// initialize beta
	while ((*(p + beta) > pivot)) {
	    beta--;
	    if ((beta == alpha))
		goto done;
	}
	pswap(p + alpha + 1, p + beta);	// *(p+beta) <= pivot 
    }
    while (1);
  done:
    pswap(p, p + alpha);	// move pivot
    return alpha;
}

static void qs(double *p, unsigned int n)
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
