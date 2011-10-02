#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct range {
    int lo;
    int hi;
};

int getint(char *s, int *n)
{
    char *p;
    p = s;
    while(isdigit(*p++))
        ;
    if(*(p - 1) != '\0')
        return 0;

    *n = strtol(s, &p, 10);
    if(*p != '\0')
        return 0;
    return 1;
}

int getrange(char *s, struct range *r)
{
    char *p, *q;

    p = s;
    while(isdigit(*s++))
        ;
    if(*(s-1) != '-')
        return 0;
    r->lo = strtol(p, &q, 10);
    if(*q != '-' || !isdigit(*(q + 1)))
        return 0;
    p = s;
    while(isdigit(*s++))
        ;
    if(*(s-1) != '\0')
        return 0;
    r->hi = strtol(p, &q, 10);
    if(*q != '\0')
        return 0;
    if(r->lo >= r->hi)
        return 0;
    return 1;
}

void qsortrange(struct range *r, int low, int high)
{
    struct range p, t;
    int i,j;

    i = low;
    j = high;
    p = r[high];

    if(low < high)  {
        do {
            while ((i < j) && (r[i].lo <= p.lo))
                i++;
            while ((j > i) && (r[j].lo >= p.lo))
                j--;
            if (i < j) {
                t = r[i];
                r[i] = r[j];
                r[j] = t;
            }
        } while (i < j);
        r[high] = r[i];
        r[i] = p;
        qsortrange(r, low, i - 1);
        qsortrange(r, j + 1, high);
    }
}

static int cmp_r(const void *n, const void *m)
{
    struct range *a = (struct range *)n;
    struct range *b = (struct range *)m;

    if(a->lo < b->lo)
        return -1;
    else if(a->lo > b->lo)
        return 1;
    else
        return 0;
}

static int cmp_i(const void *n, const void *m)
{
    return *(int *)n - *(int *)m;
}


int main(int argc, char *argv[])
{
    int nr, nn, i, j, k;
    int nums[20], new_nums[20];
    struct range ranges[20], new_ranges[20];
    char *p, *s;
    int n_nums = 0, n_ranges = 0, hi_water;

    if(argc < 2)    {
        printf("Error, please enter range-spec as argument\n");
        exit(EXIT_FAILURE);
    }

    for(s = argv[1], i = 0; i < 20; s = NULL, i++)    {
        p = strtok(s, ",");
        if(p == NULL)
            break;
        if(getint(p, &nums[n_nums]))
            n_nums++;
        else if(getrange(p, &ranges[n_ranges]))
            n_ranges++;
        else    {
            printf("Invalid token found: '%s'\n", p);
            exit(EXIT_FAILURE);
        }
    }

    qsort(ranges, n_ranges, sizeof(struct range), cmp_r);
    qsort(nums, n_nums, sizeof(int), cmp_i);

    for(i = 0; i < n_nums; i++)
        printf("Num: %d\n", nums[i]);
    for(i = 0; i < n_ranges; i++)
        printf("Range: %d to %d\n", ranges[i].lo, ranges[i].hi);
    printf("\n*****\n");
    i = nr = 0;
    while(i < n_ranges) {

        /* How many ranges can be consolidated */
        new_ranges[nr].lo = ranges[i].lo;
        hi_water = ranges[i].hi;

        for(j = i; ranges[j + 1].lo <= hi_water + 1 && j < n_ranges - 1; j++)  {
            printf("Hw: %d\n", hi_water);
            if(ranges[j + 1].hi > hi_water)
                hi_water = ranges[j + 1].hi;
        }
        new_ranges[nr].hi = hi_water;
        printf("j: %d, r.hi: %d", j, ranges[j].hi);
        nr++;
        i = j + 1;
    }






/*
        while(nums[k] < new_ranges[nr].lo && k < n_nums)  {
            new_nums[nn] = nums[k++];
        }

        for(j = k; j < n_nums && nums[j] < new_ranges[nr].hi; j++)
            ;
        k = j;
        while(nums[k] < ranges[i + 1].lo && k < n_nums)  {
            new_nums[nn] = nums[k++];
        }*/

    printf("\n******\n");
//    for(i = 0; i < nn; i++)
//        printf("New num: %d\n", new_nums[i]);
    for(i = 0; i < nr; i++)
        printf("New range: %d to %d\n", new_ranges[i].lo, new_ranges[i].hi);




    return 0;
}
