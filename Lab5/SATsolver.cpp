// ENSC 351 - Lab5
// SAT Solver Program
// Description: Solve SAT problem
// Input: DIMACS format text file
// Output: Solution of SAT problem if possible, progress report of number of backtracks every 2 seconds
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
#include <thread>

using namespace std;
// Global Variables
long double backtrack = 0;
bool not_done = true;
chrono::high_resolution_clock::time_point t1;
chrono::high_resolution_clock::time_point t2;
chrono::high_resolution_clock::time_point t3;

vector<int> read_input()
{
	ifstream file("DIMACSTest.txt");
	vector<int> datalist;
	string word;
	int number;
	if (file.is_open())
	{
		file >> word;								// Skip the first two strings
		file >> word;
		while (file >> number)						// Read numbers and put into vector
		{
			datalist.push_back(number);
		}
	}
	else
		cout << "Unable to open file" << endl;
	return datalist;
}

vector<vector<int>> clause_list(vector<int> clause_data)
{
	vector<vector<int>> clauseDataList;
	vector<int> tempList;
	int clause_size = clause_data.size();
	for (int i = 0; i < clause_size; i++)
	{
		if (clause_data[i] != 0)
		{
			tempList.push_back(clause_data[i]);
		}
		if (clause_data[i] == 0)
		{
			clauseDataList.push_back(tempList);
			tempList.clear();
		}
	}
	return clauseDataList;
}
bool OR_clause(vector<bool> checkedClause)
{
	bool OR_results = false;
	for (int i = 0; i < checkedClause.size(); i++)
	{
		OR_results = OR_results || checkedClause[i];
	}
	return OR_results;
}

bool AND_clause(vector<bool> checkedClause)
{
	bool AND_results = true;
	for (int i = 0; i < checkedClause.size(); i++)
	{
		AND_results = AND_results && checkedClause[i];
	}
	return AND_results;
}

void print_data(vector<bool> input_data)
{
	for (std::vector<bool>::iterator it = input_data.begin(); it != input_data.end(); ++it)
		std::cout << ' ' << *it << '\n';
}

bool checkClauses(vector<int> clause, vector<bool> SAT_vars)
{
	vector<bool> VarCheck;
	bool clauseBool;
	bool OR_results = false;
	#pragma omp parallel for num_threads(2)
	for (int k = 0; k < clause.size(); k++)
	{
		int clause_value = clause[k];
		clauseBool = clause_value > 0 ? true : false;
		if (clauseBool == SAT_vars[abs(clause_value) - 1])
		{
			VarCheck.push_back(true);
		}
		else
		{
			VarCheck.push_back(false);
		}
	}
	OR_results = (OR_clause(VarCheck));
	return OR_results;
}

void print_backtrack()
{
	while (not_done == true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		cout << "Backtrack Counter: " << backtrack << endl;
	}
}

bool SolveSAT(int VarNum, int ClauseNum, vector<bool> SATVars, vector<vector<int>> clauseList)
{
	bool SolutionFound = false;
	bool potentialSolution = false;
	vector<bool> SAT_value = SATVars;
	for (int i = VarNum; i >= 0;)
	{
		vector<bool> clauseResults;
		#pragma omp parallel for num_threads(2)
		for (int j = 0; j < ClauseNum; j ++)
		{
			#pragma omp critical	
            clauseResults.push_back(checkClauses(clauseList[j], SAT_value));
		}
		potentialSolution = AND_clause(clauseResults);
		// Solution Found Return True
		if (potentialSolution == true)
		{
			cout << "\nSolution found \nSatisfiable\n";
			for (int k = 0; k < SAT_value.size(); k++)
			{
				cout << "X" << k + 1 << ": " << SAT_value[k] << endl;
			}
			SolutionFound = true;
			not_done = false;
			return SolutionFound;
		}
		// Backtrack
		// 1 Backtrack occurs when a solution is false
		else
		{
			i--;
			backtrack++;
			if (SAT_value[i] == true)
			{
				SAT_value[i] = false;
				i++;
			}
			else
			{
				SAT_value[i] = !SAT_value[i];
				i--;
				//backtrack++;
				if (i < 0) // Solution not found for 1 variable case
				{
					cout << "Solution not found \nUnsatisfiable";
					SolutionFound = false;
					not_done = false;
					return SolutionFound;
				}
				while (SAT_value[i] == false)
				{
					SAT_value[i] = !SAT_value[i];
					i--;
					//backtrack++;
					if (i < 0) // Solution not found
					{
						cout << "Solution not found \nUnsatisfiable";
						SolutionFound = false;
						not_done = false;
						return SolutionFound;
					}
				}
				SAT_value[i] = !SAT_value[i];
				i = VarNum;
				backtrack++;
			}
		}
	}
	return SolutionFound;
}

int main()
{
	t1 = std::chrono::high_resolution_clock::now();
	vector<int> SATinput = read_input();
	int SATsize = SATinput.size();
	vector<int> clauseData;
	int VarNum = SATinput[0];
	int ClauseNum = SATinput[1];
	vector<bool> SAT_vars;
	bool SATresults;
	// Assign list of varibles with true 
	SAT_vars.assign(VarNum, true);
	// Amend input data into a list of clauses
	for (int i = 2; i < SATsize; i++)
	{
		clauseData.push_back(SATinput[i]);
	}
	vector<vector<int>> List_of_Clause = clause_list(clauseData);
	int clauseList = List_of_Clause.size();	
	if (clauseList == 0)
	{
		cout << "No clauses \nUnsatisfiable";
		cin.get();
		return 0;
	}
	// Check if solution is satisfied
	// Create one thread to print backtrack progress, one thread to solve the SAT
	/*// Better performance than using OpenMP in Windows
	std::thread first(print_backtrack);
	std::thread second(SolveSAT, VarNum, ClauseNum, SAT_vars, List_of_Clause);
	second.join();
	t2 = std::chrono::high_resolution_clock::now();
	first.join();
	*/
	///*// Better performance in Linux
	#pragma omp parallel num_threads(2)
	{
		#pragma omp master
		{
		SATresults = SolveSAT(VarNum, ClauseNum, SAT_vars, List_of_Clause);
		t2 = std::chrono::high_resolution_clock::now();
		}
		
		{
			print_backtrack();
		}
	}//*/ 
	//Check program duration for testing
/*
	t3 = std::chrono::high_resolution_clock::now();
	cout << "Duration of Solver:" << chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << endl;
	cout << "Duration of Program:" << chrono::duration_cast<std::chrono::milliseconds>(t3 - t1).count() << endl;
	*/
	cin.get();
	return 0;
}
