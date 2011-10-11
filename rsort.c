#include <stdio.h>
#include <stdlib.h>

struct range {
    int lo;
    int hi;
};

static void _swp(void *a, void *b, size_t s)
{
    unsigned char t;
    size_t c = 0;

    printf("Swap %p (%d) and %p (%d)\n", a, *(int *)a, b, *(int *)b);

    do {
//        printf("%d Swap %x / %x: %x %x\n", c, p, q, *p, *q);
        t = *(unsigned char *)(a + c);
        *(unsigned char *)(a + c) = *(unsigned char *)(b + c);
        *(unsigned char *)(b + c) = t;
    } while(++c < s);
}

int part(void *p, size_t s, int low, int high, int (*cmp)(void *, void *))
{
    void *k;
    int i, j;
    int piv = (high + low) / 2;

    printf("Pivot: %d (%d)\n", piv, *(int *)(p + piv * s));

    _swp((p + s * piv), (p + s * (high)), s);
    k = (p + s * (high));
    j = low;
    for(i = low; i < high; i++) {
        if(cmp((p + s * i), k) <= 0)   {
            _swp((p + s * i), (p + s * j), s);
            j++;
        }
    }
    _swp((p + s * j), (p + s * (high)), s);
    return j;
}

int _qs(void *p, size_t s, int low, int high, int (*cmp)(void *, void *))
{
    int i;
    printf("Low: %d, High: %d\n", low, high);
    if(low < high)  {
        int pivot;
        //
        printf("\nBefore Partitioning:\n");
        for(i = low; i < high; i++)      {
            printf("(%2d) %p: %d\n", i, (int *)(p + i * s), *(int *)(p + i * s));
        }
        pivot = part(p, s, low, high, cmp);
        printf("\nAfter Partitioning, np=%d:\n", pivot);
        for(i = low; i < high; i++)      {
            printf("(%2d) %p: %d\n", i, (int *)(p + i * s), *(int *)(p + i * s));
        }
        _qs(p, s, low, pivot - 1, cmp);
        _qs(p, s, pivot + 1, high, cmp);
    }
    return 0;
}


void qsrt(void *p, size_t len, size_t nelm, int (*cmp)(void *, void *))
{
    if(len > 1 && p != NULL)
        _qs(p, len, 0, nelm - 1, cmp);
}
/*
int _qsrt(void *r, size_t s, size_t len, int low, int high, int (*cmp)(void *, void *))
{
    void *p, *t;
    int i,j,k;

    i = low;
    j = high;
    p = (r + high);

    if(low < high)  {
        do {
            while ((i < j) && (cmp(*(r + i), p) <= 0))
                i++;
            while ((j > i) && (cmp(*(r + j), p) >= 0))
                j--;
            if (i < j) {
                swp(r + i, r + j, s);
            }
        } while (i < j);
        *(r + high) = *(r + i);
        *(r + i) = p;
        qsortrange(r, low, i - 1);
        qsortrange(r, j + 1, high);
    }
}
*/
int rcmp(void *a, void *b)
{
    struct range *r, *v;
    r = (struct range *)a;
    v = (struct range *)b;
    if(r->lo < v->lo)
        return -1;
    else if(r->lo == v->lo)
        return 0;
    else
        return 1;
}

int icmp(void *a, void *b)
{
    int *i = (int *)a;
    int *j = (int *)b;
    printf("Compare: %d w/ %d\n", *i, *j);
    if(*i < *j)
        return -1;
    else if(*i == *j)
        return 0;
    else
        return 1;
}

#define NUM 20

int main(int argc, char *argv[])
{
    struct range r[] = { {4,8}, {2,6}, {40,43}, {30,111}, {322,334}, {11,121} };
    //int ints[] = {21, 2, 11, 33, 20, 19, 10, 6, 8, 12, 29, 32, 13};
    int ints[NUM];
    //struct range r[30];
    int i;

    srand((unsigned)time(NULL) + getpid());
    //srand(23212);
    for(i = 0; i < NUM; i++)      {
        //printf("%p: %d\n", &ints[i], ints[i]);
        ints[i] = rand() % 100;
        printf("%d ", ints[i]);
    }

    //_swp(&ints[8], &ints[4], sizeof(int));
    qsrt(ints, sizeof(int), NUM, icmp);

    printf("\n---------------------\n");

    for(i = 0; i < NUM; i++)      {
        printf("%p: %d\n", &ints[i], ints[i]);
    }
/*
    for(i = 0; i < 6; i++)      {
        printf("%p: %d-%d\n", &r[i], r[i].lo, r[i].hi);
    }

    //swp(&r[atoi(argv[1])], &r[atoi(argv[2])], sizeof(struct range));
    qsrt(r, sizeof(struct range), 6, rcmp);

    printf("\n---------------------\n");

    for(i = 0; i < 6; i++)      {
        printf("%p: %d-%d\n", &r[i], r[i].lo, r[i].hi);
    }*/

//    qsrt(r, 6, sizeof(struct range), rcmp);

    /*srand(time(NULL));

    for(i = 0; i < 30; i++) {
        r[i].lo = rand() % 100;
        r[i].hi = rand() % 200;
    }*/

/*    for(i = 0; i < 30; i++)
        printf("%d %d\n", r[i].lo, r[i].hi);
    printf("*****\n");
    qsortrange(r, sizeof(struct range), 0, 29, rcmp);
    for(i = 0; i < 30; i++)
        printf("%d %d\n", r[i].lo, r[i].hi);


*/
    return 0;
}
