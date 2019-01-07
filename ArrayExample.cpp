#include <iostream>
#include <algorithm>
using namespace std;

int main()
{
	//lint arr[] = { 1,2,3, 0,4,5, 0,6,7,8,9,0,0 };
	int arr[] = {65,66,67,0,68,69,0,70,71,72,0,0};
	int n = sizeof(arr)/sizeof(arr[0]);
	int elem = 0;


 // counting elements in array:
  int myints[] = {10,20,30,30,20,10,10,20};   // 8 elements
  int candidateCount = std::count (arr,arr+n, 0);
    std::cout << "0 appears " << candidateCount << " times.\n";

	auto itr = find(arr, arr + n, elem);
std::string a[candidateCount];
	if (itr != end(arr)) {
		cout << "Element " << elem << " is present at index " 
			 << distance(arr, itr) << " in the given array";
			 
			 int k=0;
			 for(unsigned int i=0;(i<n)&&(k<candidateCount) ;i++)
			 {
			 a[k].push_back(arr[i]);
			 	if(arr[i] ==0){
			   	k++;
			 	}
			 }
			 //a.push_back(0);
			 for(int i=0;i<k;i++)
			cout<<"\na is "<<a[i]<<"\n";
	}
	else {
		cout << "Element is not present in the given array";
	}

	return 0;
}