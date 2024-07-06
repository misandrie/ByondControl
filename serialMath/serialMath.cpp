#define _CRT_SECURE_NO_WARNINGS
// serialMath.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "serialMath.h"
#include "limits.h"

#pragma warning( push )
#pragma warning( disable : 4309)
#pragma warning( disable : 4838)
char specialNumsBytes[] = {0, 0, 0, 0, 175, 0, 0, 0, 94, 1, 0, 0, 241, 1, 0, 0, 188, 2, 0, 0, 19, 2, 0, 0, 226, 3, 0, 0, 77, 3, 0, 0, 120, 5, 0, 0, 215, 5, 0, 0, 38, 4, 0, 0, 137, 4, 0, 0, 196, 7, 0, 0, 107, 7, 0, 0, 154, 6, 0, 0, 53, 6, 0, 0, 240, 10, 0, 0, 95, 10, 0, 0, 174, 11, 0, 0, 1, 11, 0, 0, 76, 8, 0, 0, 227, 8, 0, 0, 18, 9, 0, 0, 189, 9, 0, 0, 136, 15, 0, 0, 39, 15, 0, 0, 214, 14, 0, 0, 121, 14, 0, 0, 52, 13, 0, 0, 155, 13, 0, 0, 106, 12, 0, 0, 197, 12, 0, 0, 224, 21, 0, 0, 79, 21, 0, 0, 190, 20, 0, 0, 17, 20, 0, 0, 92, 23, 0, 0, 243, 23, 0, 0, 2, 22, 0, 0, 173, 22, 0, 0, 152, 16, 0, 0, 55, 16, 0, 0, 198, 17, 0, 0, 105, 17, 0, 0, 36, 18, 0, 0, 139, 18, 0, 0, 122, 19, 0, 0, 213, 19, 0, 0, 16, 31, 0, 0, 191, 31, 0, 0, 78, 30, 0, 0, 225, 30, 0, 0, 172, 29, 0, 0, 3, 29, 0, 0, 242, 28, 0, 0, 93, 28, 0, 0, 104, 26, 0, 0, 199, 26, 0, 0, 54, 27, 0, 0, 153, 27, 0, 0, 212, 24, 0, 0, 123, 24, 0, 0, 138, 25, 0, 0, 37, 25, 0, 0, 192, 43, 0, 0, 111, 43, 0, 0, 158, 42, 0, 0, 49, 42, 0, 0, 124, 41, 0, 0, 211, 41, 0, 0, 34, 40, 0, 0, 141, 40, 0, 0, 184, 46, 0, 0, 23, 46, 0, 0, 230, 47, 0, 0, 73, 47, 0, 0, 4, 44, 0, 0, 171, 44, 0, 0, 90, 45, 0, 0, 245, 45, 0, 0, 48, 33, 0, 0, 159, 33, 0, 0, 110, 32, 0, 0, 193, 32, 0, 0, 140, 35, 0, 0, 35, 35, 0, 0, 210, 34, 0, 0, 125, 34, 0, 0, 72, 36, 0, 0, 231, 36, 0, 0, 22, 37, 0, 0, 185, 37, 0, 0, 244, 38, 0, 0, 91, 38, 0, 0, 170, 39, 0, 0, 5, 39, 0, 0, 32, 62, 0, 0, 143, 62, 0, 0, 126, 63, 0, 0, 209, 63, 0, 0, 156, 60, 0, 0, 51, 60, 0, 0, 194, 61, 0, 0, 109, 61, 0, 0, 88, 59, 0, 0, 247, 59, 0, 0, 6, 58, 0, 0, 169, 58, 0, 0, 228, 57, 0, 0, 75, 57, 0, 0, 186, 56, 0, 0, 21, 56, 0, 0, 208, 52, 0, 0, 127, 52, 0, 0, 142, 53, 0, 0, 33, 53, 0, 0, 108, 54, 0, 0, 195, 54, 0, 0, 50, 55, 0, 0, 157, 55, 0, 0, 168, 49, 0, 0, 7, 49, 0, 0, 246, 48, 0, 0, 89, 48, 0, 0, 20, 51, 0, 0, 187, 51, 0, 0, 74, 50, 0, 0, 229, 50, 0, 0, 128, 87, 0, 0, 47, 87, 0, 0, 222, 86, 0, 0, 113, 86, 0, 0, 60, 85, 0, 0, 147, 85, 0, 0, 98, 84, 0, 0, 205, 84, 0, 0, 248, 82, 0, 0, 87, 82, 0, 0, 166, 83, 0, 0, 9, 83, 0, 0, 68, 80, 0, 0, 235, 80, 0, 0, 26, 81, 0, 0, 181, 81, 0, 0, 112, 93, 0, 0, 223, 93, 0, 0, 46, 92, 0, 0, 129, 92, 0, 0, 204, 95, 0, 0, 99, 95, 0, 0, 146, 94, 0, 0, 61, 94, 0, 0, 8, 88, 0, 0, 167, 88, 0, 0, 86, 89, 0, 0, 249, 89, 0, 0, 180, 90, 0, 0, 27, 90, 0, 0, 234, 91, 0, 0, 69, 91, 0, 0, 96, 66, 0, 0, 207, 66, 0, 0, 62, 67, 0, 0, 145, 67, 0, 0, 220, 64, 0, 0, 115, 64, 0, 0, 130, 65, 0, 0, 45, 65, 0, 0, 24, 71, 0, 0, 183, 71, 0, 0, 70, 70, 0, 0, 233, 70, 0, 0, 164, 69, 0, 0, 11, 69, 0, 0, 250, 68, 0, 0, 85, 68, 0, 0, 144, 72, 0, 0, 63, 72, 0, 0, 206, 73, 0, 0, 97, 73, 0, 0, 44, 74, 0, 0, 131, 74, 0, 0, 114, 75, 0, 0, 221, 75, 0, 0, 232, 77, 0, 0, 71, 77, 0, 0, 182, 76, 0, 0, 25, 76, 0, 0, 84, 79, 0, 0, 251, 79, 0, 0, 10, 78, 0, 0, 165, 78, 0, 0, 64, 124, 0, 0, 239, 124, 0, 0, 30, 125, 0, 0, 177, 125, 0, 0, 252, 126, 0, 0, 83, 126, 0, 0, 162, 127, 0, 0, 13, 127, 0, 0, 56, 121, 0, 0, 151, 121, 0, 0, 102, 120, 0, 0, 201, 120, 0, 0, 132, 123, 0, 0, 43, 123, 0, 0, 218, 122, 0, 0, 117, 122, 0, 0, 176, 118, 0, 0, 31, 118, 0, 0, 238, 119, 0, 0, 65, 119, 0, 0, 12, 116, 0, 0, 163, 116, 0, 0, 82, 117, 0, 0, 253, 117, 0, 0, 200, 115, 0, 0, 103, 115, 0, 0, 150, 114, 0, 0, 57, 114, 0, 0, 116, 113, 0, 0, 219, 113, 0, 0, 42, 112, 0, 0, 133, 112, 0, 0, 160, 105, 0, 0, 15, 105, 0, 0, 254, 104, 0, 0, 81, 104, 0, 0, 28, 107, 0, 0, 179, 107, 0, 0, 66, 106, 0, 0, 237, 106, 0, 0, 216, 108, 0, 0, 119, 108, 0, 0, 134, 109, 0, 0, 41, 109, 0, 0, 100, 110, 0, 0, 203, 110, 0, 0, 58, 111, 0, 0, 149, 111, 0, 0, 80, 99, 0, 0, 255, 99, 0, 0, 14, 98, 0, 0, 161, 98, 0, 0, 236, 97, 0, 0, 67, 97, 0, 0, 178, 96, 0, 0, 29, 96, 0, 0, 40, 102, 0, 0, 135, 102, 0, 0, 118, 103, 0, 0, 217, 103, 0, 0, 148, 100, 0, 0, 59, 100, 0, 0, 202, 101, 0, 0, 101, 101, 0, 0};
#pragma warning( pop )
unsigned long* specialNums = (unsigned long*)specialNumsBytes;

const char* hexLookup = "0123456789abcdef"; //{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
bool CalcSerialFromId(unsigned long computerId, char*& result) {
    unsigned int out1 = ULONG_MAX;
    char test[33];
    for (int i = 0; i < 32; ++i) {
        test[i] = 'f';
    }
    test[32] = '\0';

    for (int i = 0; i < 20; ++i) {
        out1 = out1 << 8 ^ specialNums[test[i] ^ out1 >> 24];
    }
    

    for (int i1 = 0; i1 < 16; ++i1) {
        unsigned int out2 = out1 << 8 ^ specialNums[hexLookup[i1] ^ out1 >> 24];
        for (int i2 = 0; i2 < 16; ++i2) {
            unsigned int out3 = out2 << 8 ^ specialNums[hexLookup[i2] ^ out2 >> 24];
            for (int i3 = 0; i3 < 16; ++i3) {
                unsigned int out4 = out3 << 8 ^ specialNums[hexLookup[i3] ^ out3 >> 24];
                for (int i4 = 0; i4 < 16; ++i4) {
                    unsigned int out5 = out4 << 8 ^ specialNums[hexLookup[i4] ^ out4 >> 24];
                    for (int i5 = 0; i5 < 16; ++i5) {
                        unsigned int out6 = out5 << 8 ^ specialNums[hexLookup[i5] ^ out5 >> 24];
                        for (int i6 = 0; i6 < 16; ++i6) {
                            unsigned int out7 = out6 << 8 ^ specialNums[hexLookup[i6] ^ out6 >> 24];
                            for (int i7 = 0; i7 < 16; ++i7) {
                                unsigned int out8 = out7 << 8 ^ specialNums[hexLookup[i7] ^ out7 >> 24];
                                for (int i8 = 0; i8 < 16; ++i8) {
                                    unsigned int out9 = out8 << 8 ^ specialNums[hexLookup[i8] ^ out8 >> 24];
                                    for (int i9 = 0; i9 < 16; ++i9) {
                                        unsigned int out10 = out9 << 8 ^ specialNums[hexLookup[i9] ^ out9 >> 24];
                                        for (int i10 = 0; i10 < 16; ++i10) {
                                            unsigned int out11 = out10 << 8 ^ specialNums[hexLookup[i10] ^ out10 >> 24];
                                            for (int i11 = 0; i11 < 16; ++i11) {
                                                unsigned int out12 = out11 << 8 ^ specialNums[hexLookup[i11] ^ out11 >> 24];
                                                unsigned int out12Lo = out12 << 8;
                                                unsigned int out12Hi = out12 >> 24;
                                                for (int i12 = 0; i12 < 16; ++i12) {
                                                    if ((out12Lo ^ specialNums[hexLookup[i12] ^ out12Hi]) == computerId) {
                                                        test[20] = hexLookup[i1];
                                                        test[21] = hexLookup[i2];
                                                        test[22] = hexLookup[i3];
                                                        test[23] = hexLookup[i4];
                                                        test[24] = hexLookup[i5];
                                                        test[25] = hexLookup[i6];
                                                        test[26] = hexLookup[i7];
                                                        test[27] = hexLookup[i8];
                                                        test[28] = hexLookup[i9];
                                                        test[29] = hexLookup[i10];
                                                        test[30] = hexLookup[i11];
                                                        test[31] = hexLookup[i12];
                                                        strncpy_s(result, 33, test, 32);
                                                        return true;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}