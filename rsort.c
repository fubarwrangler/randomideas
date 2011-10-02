#include <stdio.h>
#include <stdlib.h>

struct range {
    int lo;
    int hi;
};

void swap_r(struct range *a, struct range *b)
{
    struct range t;
    t = *a;
    *a = *b;
    *b = t;
}

int part(struct range *arr, int low, int high, int idx)
{
    int i, j;
    struct range k;

    k = arr[idx];

    swap_r(&arr[idx], &arr[high]);

    for(i = low, j = low; i < high; i++) {
        if(arr[i].lo < k.lo)   {
            swap_r(&arr[i], &arr[j]);
            j++;
        }
    }
    swap_r(&arr[j], &arr[high]);
    return j;
}

int sortrange(struct range *range, int low, int high)
{
    if(low + 1 < high)  {
        int pivot = (high + low) / 2, newpivot;
        newpivot = part(range, low, high, pivot);
        sortrange(range, low, newpivot - 1);
        sortrange(range, newpivot + 1, high);
    }
}


int qsortrange(void *r, int low, int high, int (*cmp)(void *, void*))
{
    void *p, *t;
    int i,j,k;

    i = low;
    j = high;
    p = *(r + high);

    if(low < high)  {
        do {
            while ((i < j) && (cmp(*(r + i), p) <= 0))
                i++;
            while ((j > i) && (cmp(*(r + j), p) >= 0))
                j--;
            if (i < j) {
                t = (r + i);
                *(r + i) = *(r + j);
                *(r + j) = t;
            }
        } while (i < j);
        *(r + high) = *(r + i);
        *(r + i) = p;
        qsortrange(r, low, i - 1);
        qsortrange(r, j + 1, high);
    }
}

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



int main(int argc, char *argv[])
{
    //struct range r[] = { {4,8}, {2,6}, {40,43}, {30,111}, {322,334}, {11, 121} };
    struct range r[30];
    int i;

    srand(time(NULL));

    for(i = 0; i < 30; i++) {
        r[i].lo = rand() % 100;
        r[i].hi = rand() % 200;
    }

    for(i = 0; i < 30; i++)
        printf("%d %d\n", r[i].lo, r[i].hi);
    printf("*****\n");
    qsortrange(r, sizeof(struct range), 0, 29, rcmp);
    for(i = 0; i < 30; i++)
        printf("%d %d\n", r[i].lo, r[i].hi);



    return 0;
}
