#ifndef PRIMITIVE_POLYS_H_
#define PRIMITIVE_POLYS_H_

#include <vector>
#include <map>

std::map<long, std::vector<long>> PRIMITIVE_POLYS  = {
    {2, {2, 1, 0}},
    {3, {3, 1, 0}},
    {4, {4, 1, 0}},
    {5, {5, 2, 0}},
    {6, {6, 1, 0}},
    {7, {7, 1, 0}},
    {8, {8, 7, 2, 1, 0}},
    {9, {9, 4, 0}},
    {10, {10, 3, 0}},
    {11, {11, 2, 0}},
    {12, {12, 10, 2, 1, 0}},
    {13, {13, 8, 5, 3, 0}},
    {14, {14, 12, 11, 1, 0}},
    {15, {15, 1, 0}},
    {16, {16, 15, 12, 10, 0}},
    {17, {17, 3, 0}},
    {18, {18, 7, 0}},
    {19, {19, 10, 9, 3, 0}},
    {20, {20, 3, 0}},
    {21, {21, 2, 0}},
    {22, {22, 1, 0}},
    {23, {23, 5, 0}},
    {24, {24, 11, 5, 2, 0}},
    {25, {25, 3, 0}},
    {26, {26, 23, 15, 13, 0}},
    {27, {27, 23, 22, 17, 0}},
    {28, {28, 3, 0}},
    {42, {42, 7, 4, 3, 0}}, // Hansen and Mullen, "PRIMITIVE POLYNOMIALS OVER FINITE FIELDS".
    {45, {45, 39, 28, 4, 0}},
    // {48, {48, 19, 9, 1, 0}}, // This doesn't work for 'gf2n.cpp' because of violation of the requirement "2*(n-1)-64+l[1]<64"
    {48, {48, 9, 7, 4, 0}}, // Hansen and Mullen, "PRIMITIVE POLYNOMIALS OVER FINITE FIELDS".
    {66, {66, 55, 48, 39, 0}}
};

#endif /* PRIMITIVE_POLYS_H_ */