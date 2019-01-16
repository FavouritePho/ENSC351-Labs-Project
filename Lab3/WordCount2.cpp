// ENSC 351 - Lab3
// WordCount (w/ mapReduce) Program
// Steven Luu
// Younghoon Jee

#include <stdio.h>
#include <cstdlib>
#include <process.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <mutex>
using namespace std;
//std::mutex mtx;

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


// Alternate Map function, 1 word input, 1 K-V pair output
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
struct thread_args
{
	vector<string> Key;
	int step;
	vector<Key_Value_Pair> mapped_KV_pairs;
};
struct thread_reduce_args
{
	int step;
	vector<vector<Key_Value_Pair>> grouped_KV_pairs;
	vector<Key_Value_Pair> reduced_group;
};

void *thread_map_pairs(void *ptr)
{
	struct thread_args *args = (struct thread_args*) ptr;
	int increment = args->step;
	vector<string> word_list = args->Key;
	vector<Key_Value_Pair> mapped_pairs;
	int word_size = word_list.size();
	//mtx.lock();
	for (int i = increment; i < word_size; i+=2)
	{
		Key_Value_Pair Word = map_pair(word_list[i]);
		mapped_pairs.push_back(Word);
		args->mapped_KV_pairs = mapped_pairs; 
	}
	//mtx.unlock();
	pthread_exit(NULL);
	return NULL;
}

void *thread_reduce_pairs(void *ptr)
{
	struct thread_reduce_args *args = (struct thread_reduce_args*) ptr;
	vector<vector<Key_Value_Pair>> All_Grouped_List = args->grouped_KV_pairs;
	vector<Key_Value_Pair> Grouped_List;
	int All_Group_Size = All_Grouped_List.size();
	int increment = args->step;
	//mtx.lock();
	for (int j = increment; j < All_Group_Size; j+=2)
	{
		Key_Value_Pair Reduced_KV = reduce_pairs_list(All_Grouped_List[j]);
		Grouped_List.push_back(Reduced_KV);
	}
	args->reduced_group = Grouped_List;
	//mtx.unlock();
	pthread_exit(NULL);
	return NULL;
}

int main()
{
	// Creating Time point
	chrono::high_resolution_clock::time_point t1, t2, t3, t4, t5, t6;
	t1 = std::chrono::high_resolution_clock::now();
	// Create Structure of thread arguments
	struct thread_args thread1, thread2;
	struct thread_reduce_args thread3, thread4;
	// Create list of words for input file
	vector<string> list = read_input();
	pthread_t threads[3];
	vector<Key_Value_Pair> mapped_pairs, mapped_pairs2, Reduced_List, Reduced_List2;
	// Create Vector to store KV Pairs
	int word_size = list.size();
	thread1.Key = list;
	thread1.step = 0;	// Step even
	thread2.Key = list;
	thread2.step = 1;	// Step odd
	//Create and join threads for map function
	t3 = std::chrono::high_resolution_clock::now();
	pthread_create(&threads[0], NULL, thread_map_pairs, (void*) &thread1);
	pthread_create(&threads[1], NULL, thread_map_pairs, (void*) &thread2);
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);
	t4 = std::chrono::high_resolution_clock::now();
	mapped_pairs = thread1.mapped_KV_pairs;
	mapped_pairs2 = thread2.mapped_KV_pairs;
	// Combine the 2 KV Pair Lists and Sort KV Pairs
	mapped_pairs.insert(mapped_pairs.end(), mapped_pairs2.begin(), mapped_pairs2.end());
	std::sort(mapped_pairs.begin(), mapped_pairs.end(), &key_sort);
	//Group Pairs
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
		j+=counter;
		All_Groups.push_back(groupped_pairs);
	}
	// Creating Thread Arguments for Reduce Function
	thread3.grouped_KV_pairs = All_Groups;
	thread4.grouped_KV_pairs = All_Groups;
	thread3.step = 0;
	thread4.step = 1;
	// Create and Combine threads
	t5 = std::chrono::high_resolution_clock::now();
	pthread_create(&threads[2], NULL, thread_reduce_pairs, (void*)&thread3);
	pthread_create(&threads[3], NULL, thread_reduce_pairs, (void*)&thread4);
	pthread_join(threads[2], NULL);
	pthread_join(threads[3], NULL);
	t6 = std::chrono::high_resolution_clock::now();
	//Get and combine reduced KV lists
	Reduced_List = thread3.reduced_group;
	Reduced_List2 = thread4.reduced_group;
	Reduced_List.insert(Reduced_List.end(), Reduced_List2.begin(), Reduced_List2.end());
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
	pthread_exit(NULL);
	return 0;
}