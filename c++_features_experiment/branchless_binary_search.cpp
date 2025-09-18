#include <bits/stdc++.h>

using namespace std;

int main() {
    int n;
    cin >> n;
    vector<int> a(n);
    for (auto &x : a)
        cin >> x;
    int i = 0; //i indicates first index.
    int len = n;
    // first elem > x
    int x;
    cin >> x;
    while (len > 1) {
        int mid = len >> 1; // bitwise operator takes 1 cycle, while division takes >40.. optimised
        /*
        if (a[i + mid - 1] <= x) i += mid;  
        single conditional binary search, using length and start index
        this can be extended to direct optimisation to branchless 
        binary search via cmov and comparison flags using conditionals / ternary operator like below.
        */
        i += (a[i + mid - 1] <= x) * mid; //no branching, extremely fast
        //this condition can be calculated by arithmetic and shifting operations, or cmov in x86.
        len -= mid;
    }
    cout << a[i] << "\n";
}
