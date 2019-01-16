// ENSC 351 - Lab4
// WordCount OMP Program
// Steven Luu
// Younghoon Jee

#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <vector>
#include <algorithm>
#include <omp.h>
using namespace std;

struct Key_Value_Pair
{
	string key;
	int value = 0;
};

vector<string> read_input()
{
	ifstream file("WordCountTest.txt");
	vector<string> wordlist;
	string word;
	if (file.is_open())
	{
		while (file)
		{
			file >> word;
			wordlist.push_back(word);
		}
	}
	else
		cout << "Unable to open file" << endl;
	return wordlist;
}


// Map function, 1 word input, 1 K-V pair output
Key_Value_Pair map_pair(string word)
{
	Key_Value_Pair new_KV_pair;
	new_KV_pair.key = word;
	new_KV_pair.value = 1;
	return new_KV_pair;
}

// Reduce Pairs Functions, multiple K-V pairs input, 1 K-V pair output
Key_Value_Pair reduce_pairs_list(vector <Key_Value_Pair> pair_list)
{
	Key_Value_Pair Reduced_list;
	Reduced_list.key = pair_list[0].key;
	int vector_size = pair_list.size();
	Reduced_list.value = vector_size;
	return Reduced_list;
}

void print_pairs(std::vector<Key_Value_Pair> ordered_pairs)
{
	int vector_length = ordered_pairs.size();
	for (int i = 0; i < vector_length; i++)
	{
		cout << ordered_pairs[i].key << " : " << ordered_pairs[i].value << endl;
	}
}

bool key_sort(const Key_Value_Pair &lhs, const Key_Value_Pair &rhs)
{
	return lhs.key < rhs.key;
}

bool value_sort(const Key_Value_Pair &lhs, const Key_Value_Pair &rhs)
{
	return lhs.value > rhs.value;
}

int main()
{
	// Creating Time point
	chrono::high_resolution_clock::time_point t1, t2, t3, t4, t5, t6;
	t1 = std::chrono::high_resolution_clock::now();
	// Create Structure of thread arguments
	// Create list of words for input file
	vector<string> list = read_input();
	vector<Key_Value_Pair> mapped_pairs, mapped_pairs2, Reduced_List, Reduced_List2;
	// Create Vector to store KV Pairs
	int word_size = list.size();
	// Create threads for map function
	t3 = std::chrono::high_resolution_clock::now();
	#pragma omp parallel num_threads(2)
	{
		int start_step = omp_get_thread_num();
		int increment = omp_get_num_threads();
		for (int i = start_step; i < word_size; i += increment)
		{
			Key_Value_Pair Word = map_pair(list[i]);
			#pragma omp critical
			mapped_pairs.push_back(Word);

		}
	}
	t4 = std::chrono::high_resolution_clock::now();
	// Sort Pairs
	std::sort(mapped_pairs.begin(), mapped_pairs.end(), &key_sort);
	// Group Pairs
	int mapped_list_size = mapped_pairs.size();
	vector<vector<Key_Value_Pair>> All_Groups;
	for (int j = 0; j < mapped_list_size;)
	{
		vector<Key_Value_Pair> groupped_pairs;
		int counter = 0;
		string Key_word = mapped_pairs[j].key;
		for (int k = 0; k < mapped_list_size; k++)
		{
			if (Key_word == mapped_pairs[k].key)
			{
				groupped_pairs.push_back(mapped_pairs[k]);
				counter++;
			}
		}
		j += counter;
		All_Groups.push_back(groupped_pairs);
	}
	// Reduce Groups 
	t5 = std::chrono::high_resolution_clock::now();
	int All_Group_Size = All_Groups.size();
	#pragma omp parallel num_threads(2)
	{
		int start_step = omp_get_thread_num();
		int increment = omp_get_num_threads();
		for (int j = start_step; j < All_Group_Size; j += increment)
		{
			Key_Value_Pair Reduced_KV = reduce_pairs_list(All_Groups[j]);
			#pragma omp critical
			Reduced_List.push_back(Reduced_KV);
		}
	}
	t6 = std::chrono::high_resolution_clock::now();
	// Sort by Value
	std::sort(Reduced_List.begin(), Reduced_List.end(), &value_sort);
	// Print Pairs
	print_pairs(Reduced_List);
	t2 = std::chrono::high_resolution_clock::now();
	int duration = chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	cout << "Execution time: " << duration << " microseconds" << endl;
	int Map_time = chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
	cout << "Map time: " << Map_time << " microseconds" << endl;
	int Reduce_time = chrono::duration_cast<std::chrono::microseconds>(t6 - t5).count();
	cout << "Reduce time: " << Reduce_time << " microseconds" << endl;
	cin.get();
	return 0;
}
