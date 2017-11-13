#include <iostream>
#include <map>
#include <vector>

using namespace std;

int main()
{
    map<int, vector<int>> result;
    int key_start = 1, key_end = 5;
    
    for (int key = key_start; key <= key_end; key++) {
        vector<int> vals;
        for (int val = key; val <= key_end + key - key_start; val++) {
            vals.push_back(val);
        }
        
        result.insert(pair<int, vector<int>>(key, vals));
    }
    
    map<int, vector<int>>::iterator itr;
    for (itr = result.begin(); itr != result.end(); itr++) {
        cout << itr->first << " : ";
        for (int i = 0; i < itr->second.size(); i++) {
            cout << itr->second[i] << " ";
        }
        cout << endl;
    }
    
    return 0;
}