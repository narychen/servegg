#include <vector>
#include <map>
#include <iostream>
using namespace std;

static void stlprint(vector<int> &v)
{
    for (auto e : v)
        cout << e << " " << endl;    
}

static void stlprint(vector<int> &v, int n)
{
    cout << v[n] << endl;    
}

static void stlprint(map<string, string> &m, char* s)
{
    string ss(s);
    cout << m[ss] << endl;
}

int main()
{
    vector<int> v = {1, 3, 5};
    map<string, string> m = { {"xx", "oo"}, {"oo", "xx"} };
    return 1;
}