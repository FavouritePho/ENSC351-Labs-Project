// ENSC 351 - Lab3
// Census (w/ mapReduce) Program
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
#include <sstream>
using namespace std;
std::mutex mtx;

struct Key_Value_Pair
{
	string key;
	int value = 0;
};

vector<pair<string,string>> read_input()
{
	ifstream file("CensusTest.txt");
	vector<pair<string,string>> wordlist;
	string word;
	pair<string,string> input_pair;
	string age;
	stringstream ss;
	if (file.is_open())
	{
		while (file >> word >> age)
		{
			input_pair.first = word;
			input_pair.second = age;
			wordlist.push_back(input_pair);
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
		//cout << ordered_pairs[i].value << " people" << endl;
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
	vector<pair<string, string>> Key;
	int step;
	vector<Key_Value_Pair> mapped_KV_pairs;
};
struct thread_reduce_args
{
	int start_age;
	int end_age;
	vector<vector<Key_Value_Pair>> grouped_KV_pairs;
	vector<Key_Value_Pair> reduced_group;
};

void *thread_map_pairs(void *ptr)
{
	struct thread_args *args = (struct thread_args*) ptr;
	int increment = args->step;
	vector<pair<string,string>> word_list = args->Key;
	int age;
	vector<Key_Value_Pair> mapped_pairs;
	int word_size = word_list.size();
	mtx.lock();
	for (int i = increment; i < word_size; i += 2)
	{
		Key_Value_Pair Word = map_pair(word_list[i].first);
		age = stoi(word_list[i].second);
		Word.value = age;
		mapped_pairs.push_back(Word);
		args->mapped_KV_pairs = mapped_pairs;
	}
	mtx.unlock();
	pthread_exit(NULL);
	return NULL;
}

void *thread_reduce_pairs(void *ptr)
{
	
	struct thread_reduce_args *args = (struct thread_reduce_args*) ptr;
	vector<vector<Key_Value_Pair>> All_Grouped_List = args->grouped_KV_pairs;
	vector<Key_Value_Pair> Grouped_List;

	vector<Key_Value_Pair> Reduced_List;
	int All_Group_Size = All_Grouped_List.size();
	int age_start = args->start_age;
	int age_end = args->end_age;
	mtx.lock();
	for (int j = 0; j < All_Group_Size; j++)
	{
		vector<Key_Value_Pair> Age_group;
		Grouped_List = All_Grouped_List[j];
		int Group_Size = Grouped_List.size();
		
		for (int k = 0; k < Group_Size; k++)
		{
			if (Grouped_List[k].value >= age_start && Grouped_List[k].value <= age_end)
			{
				Age_group.push_back(Grouped_List[k]);
			}
		}
		Key_Value_Pair Reduced_KV = reduce_pairs_list(Age_group);
		Reduced_List.push_back(Reduced_KV);
	}
	args->reduced_group = Reduced_List;
	mtx.unlock();
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
	vector<pair<string, string>> list = read_input();
	pthread_t threads[3];
	vector<Key_Value_Pair> mapped_pairs, mapped_pairs2, Reduced_List, Reduced_List2;
	// Create Vector to store KV Pairs
	int word_size = list.size();
	thread1.Key = list;
	thread1.step = 0;	// Step even
	thread2.Key = list;
	thread2.step = 1;	// Step odd
	t3 = std::chrono::high_resolution_clock::now();
	//Create and join threads for map function
	pthread_create(&threads[0], NULL, thread_map_pairs, (void*)&thread1);
	pthread_create(&threads[1], NULL, thread_map_pairs, (void*)&thread2);
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);
	t4 = std::chrono::high_resolution_clock::now();
	mapped_pairs = thread1.mapped_KV_pairs;
	mapped_pairs2 = thread2.mapped_KV_pairs;
	// Combine the 2 KV Pair Lists and Sort KV Pairs
	mapped_pairs.insert(mapped_pairs.end(), mapped_pairs2.begin(), mapped_pairs2.end());
	std::sort(mapped_pairs.begin(), mapped_pairs.end(), &key_sort);
	//Group Pairs
	/**/
	vector<vector<Key_Value_Pair>> All_Groups;
	int mapped_list_size = mapped_pairs.size();
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
	// Creating Thread Arguments for Reduce Function
	thread3.grouped_KV_pairs = All_Groups;
	thread3.start_age = 0;
	thread3.end_age = 50;
	thread4.grouped_KV_pairs = All_Groups;
	thread4.start_age = 51;
	thread4.end_age = 6543;
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
	//Reduced_List.insert(Reduced_List.end(), Reduced_List2.begin(), Reduced_List2.end());
	// Sort by Value
	std::sort(Reduced_List.begin(), Reduced_List.end(), &key_sort);
	std::sort(Reduced_List2.begin(), Reduced_List2.end(), &key_sort);
	// Print Pairs
	cout << "Demographics for the age group " << thread3.start_age << " to " << thread3.end_age << " there are " << endl;
	print_pairs(Reduced_List);
	cout << "Demographics for the age group " << thread4.start_age << " to " << thread4.end_age << " there are " << endl;
	print_pairs(Reduced_List2);
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