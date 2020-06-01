#include<iostream>
using namespace std;

/*Given a positive integer num, write a function which returns True if num is a perfect square else False.
Follow up: Do not use any built-in library function such as sqrt.*/

/*The trick in this question is that we cant use any built in function such as sqrt. So in order to solve this, we can use a rule of squares in mathematics
1*1 = 1
2*2 = 4 = 1 + 3
3*3 = 9 = 1 + 3 +5
4*4 = 16 = 1 + 3 + 5 + 7
*/

bool isPerfectSquare(int n)
{
	int numToForm = 1;	//This is the base case for 1
	int count = 3;		//We add this count every time to numToForm to form the square
	while(numToForm < n)
	{
		numToForm+=count;	//Every time we add count to numToForm it becomes a perfect square
		count+=2;			
	}
	return (numToForm == n);
}

int main()
{
	int num = 16;
	cout<<isPerfectSquare(num)<<endl;
	num = 14;
	cout<<isPerfectSquare(num)<<endl;
}
