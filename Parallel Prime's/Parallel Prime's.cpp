#include "pch.h"
// CPP program to demonstrate multithreading 
// using three different callables. 
#include <iostream> 
#include <thread> 
#include <chrono>
#include <array>
#include <mutex>
#include <omp.h>

using namespace std;

// Size of the array to work on
const int NumberCount = 100000;


// The Prime Detector Function
bool IsPrime(int num) {
	// Check  if 0 or 1
	if (num <= 1)
	{
		return false;
	}

	// Check remainders for 0's
	for (int j = 2; j <= num / 2; j++) {
		if ((num % j) == 0) {
			return false;
		}
	}
	return true;
}

void ThreadArrayIter(int *targetRef, mutex *mutexRef, array<int, NumberCount> *data, array<bool, NumberCount> *isPrime) {
	int tmp;
	while (true) {
		//grab current target in array
		(*mutexRef).lock();
		tmp = (*targetRef);

		if (tmp == NumberCount) {
			//Exit without mutex
			(*mutexRef).unlock();
			return;
		}
		else {
			// Increment target in array for next thread that grabs the mutex, then run on found value.
			(*targetRef)++;
			(*mutexRef).unlock();
			(*isPrime).at(tmp) = IsPrime((*data).at(tmp));
		}
	}
}

int main()
{
	// The worker threads
	thread threads[2];

	// The data
	array<int, NumberCount> data;
	for (int i = 1; i < NumberCount; i++) {
		data.at(i) = i;
	}

	// The output bools for each test
	array<bool, NumberCount> isPrime1;
	array<bool, NumberCount> isPrime2;
	array<bool, NumberCount> isPrime3;

	// ---------------------------------------------------------------------
	// Part 1: running without parallel threads
	// ---------------------------------------------------------------------

	auto start1 = chrono::steady_clock::now();

	for (int i = 1; i < NumberCount; i++) {
		isPrime1.at(i) = IsPrime(i);
	}

	auto finish1 = chrono::steady_clock::now();
	double elapsed_seconds1 = chrono::duration_cast<
		chrono::duration<double>>(finish1 - start1).count();

	cout << "Runtime of linear primality check: " << elapsed_seconds1 << " seconds." << endl;

	// ---------------------------------------------------------------------
	// Part 2: running with 2 manually implemented parallel threads
	// ---------------------------------------------------------------------

	// A shared variable for where the threads should work.
	int curTarget = 0;
	// A shared mutex to protect the variable
	mutex targetMutex;

	auto start2 = chrono::steady_clock::now();

	//launch threads with shared variables
	thread th1(ThreadArrayIter, &curTarget, &targetMutex, &data, &isPrime2);
	thread th2(ThreadArrayIter, &curTarget, &targetMutex, &data, &isPrime2);

	// Wait for the threads to finish 
	th1.join();
	th2.join();

	auto finish2 = chrono::steady_clock::now();
	double elapsed_seconds2 = chrono::duration_cast<
		chrono::duration<double>>(finish2 - start2).count();

	cout << "Runtime of dual thread primality check: " << elapsed_seconds2 << " seconds." << endl;

	// ---------------------------------------------------------------------
	// Part 3: Use OpenMP to run in parallel
	// ---------------------------------------------------------------------

	auto start3 = chrono::steady_clock::now();

#pragma omp parallel for
	for (int i = 1; i < NumberCount; i++) {
		isPrime3.at(i) = IsPrime(i);
	}

	auto finish3 = chrono::steady_clock::now();
	double elapsed_seconds3 = chrono::duration_cast<
		chrono::duration<double>>(finish3 - start3).count();

	cout << "Runtime of OpenMP primality check: " << elapsed_seconds3 << " seconds." << endl;

	// ---------------------------------------------------------------------
	// Part 4: Test results for consistancy
	// ---------------------------------------------------------------------

	// Test that results are consistant
	for (int i = 1; i < NumberCount; i++) {
		if ((isPrime1.at(i) != isPrime2.at(i)) || isPrime1.at(i) != isPrime3.at(i)) {
			cout << "Output values are inconsistant between the runs." << i << ", " << isPrime1.at(i) << ", " << isPrime2.at(i) << ", " << isPrime3.at(i);
			return -1;
		}
	}

	return 0;
}