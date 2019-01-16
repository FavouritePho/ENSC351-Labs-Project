// ENSC 351 - Lab3
// WordCount (w/o mapReduce) Program
// Steven Luu
// Younghoon Jee

#include <stdio.h>
#include <cstdlib>
#include <process.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;
typedef std::pair<std::string, int> mypair;
map <string, int> word_count;

int main()
{
	// Creating Time point
	chrono::high_resolution_clock::time_point t1;
	chrono::high_resolution_clock::time_point t2;
	t1 = std::chrono::high_resolution_clock::now();
	ifstream file("WordCountTest.txt");
	string word;
	if (file.is_open())
	{
		while (file)
		{
			file >> word;
			//Check for word occurrence
			if (word_count.find(word) == word_count.end()) //	First time encountering word
				word_count[word] = 1;					   //	Set initial value to 1
			else
				word_count[word]++;						   //	Increment value
		}
	}
	else
		cout << "Unable to open file" << endl;
	// Order the Key value pairs
	std::vector<mypair> myvec(word_count.begin(), word_count.end());
	std::sort(myvec.begin(), myvec.end(), [](const mypair& lhs, const mypair& rhs)
		{return lhs.second > rhs.second;});
	// Output key values pairs
	for (auto const &mypair : myvec)
	{
		std::cout << mypair.first << " : " << mypair.second << endl;
	}
	t2 = std::chrono::high_resolution_clock::now();
	int duration = chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	cout << "Execution time: " << duration << " microseconds" << endl;
	cin.get();
	return 0;
}
