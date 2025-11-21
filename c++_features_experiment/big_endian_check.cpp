#include <bits/stdc++.h>

using namespace std;

bool big_endian() {

    union {
        uint32_t data;
        char byte[4];
    } value_{0x01020304};
    return value_.byte[0] == 01;
}

int main() { cout << big_endian() << "\n"; }