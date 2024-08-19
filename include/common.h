#ifndef _COMMON_H_
#define _COMMON_H_

/*
*   Returns an index to a 1D array with an (x,y) coordinate
*/
static inline int __array_idx(int w, int x, int y)
{
    return y * w + x;
}

#endif