#include<iostream>
#include<vector>
using namespace std;

//Given a string, find the first non-repeating character in it and return it's index. If it doesn't exist, return -1.

int firstUniqChar(string s)
{
	vector<int> v(26,0);
	for(int i = 0;i<s.length();i++)
	{
		v[s[i] - 'a']++;
	}
	for(int i = 0;i<s.length();i++)
	{
		if(v[s[i] - 'a']==1)
		{
			return i;
		}
	}
	return -1;
}

int main()
{
	string s = "leetcode";
	cout<<firstUniqChar(s)<<endl;
	s = "loveleetcode";
	cout<<firstUniqChar(s)<<endl;
}
