/***********************************************************************
MAPS Assignment 1 - Starting Point for OMP Assignment

SortOut - integer sorting and file output program.

The data is held in 2000 rows of 1000 numbers each. Each row is sorted independently,
using a simple bubble sort.
A moving average for each sorted row should be generated (see assignment spec).
Your version should also include sorting all of the data at once, i.e. all 2,000,000 numbers,
and this can be done after the rows have been sorted.

This version includes basic timing information and uses strings to create the file output.

S.Andrews / A.Oram
Revised: A.Oram Feb 2017
************************************************************************
PLEASE ADD YOUR NAME AND STUDENT NUMBER HERE:
SEAN SHORTREED 26037992

************************************************************************/

#include <fstream>			//for file output
#include <iostream>			//for console output
#include <conio.h>			//for kbhit
#include "hr_time.h"		//for stopwatches
#include <stdio.h>			//for fputs
#include "omp.h"			//omp

using namespace std;

#define MAX_ROWS 2000
#define MAX_COLS 1000
#define MAX_CHARS 6			// numbers are in the range 1- 32,767, so 6 digits is enough.

#define SortedRows "SortedRows.txt"
#define MovingAve  "MovingAve.txt"		// for future use
#define SortedAll  "SortedAll.txt"		// for future use


int **data;		// 2000 rows of 1000 numbers to sort!
int **dataAvg;		// 2000 rows of 1000 numbers to sort!
int **output;

CStopWatch s1, s2, s3, s4, s5, s6;

void getData(void);
void sortEachRow(void);
void displayCheckData(void);
void outputDataAsString(int**, string filename);
void outputTimes(void);
void megaSort(void);
void workoutMovingAverage(void);


int main(void)
{
	getData();

	s1.startTimer();
	sortEachRow();
	s1.stopTimer();

	displayCheckData();

	s2.startTimer();
	outputDataAsString(data, SortedRows);
	s2.stopTimer();

	s3.startTimer();
	workoutMovingAverage();
	s3.stopTimer();

	s4.startTimer();
	outputDataAsString(dataAvg, MovingAve);
	s4.stopTimer();

	s5.startTimer();
	megaSort();
	s5.stopTimer();

	s6.startTimer();
	outputDataAsString(data, SortedAll);
	s6.stopTimer();

	outputTimes();

	while (!_kbhit());  //to hold console
}

//*********************************************************************************
void getData()		// Generate the same sequence of 'random' numbers.
{
	data = new int*[MAX_ROWS];
	dataAvg = new int*[MAX_ROWS];
	output = new int*[MAX_ROWS];

	for (int i = 0; i < MAX_ROWS; i++)
	{
		data[i] = new int[MAX_COLS];
		dataAvg[i] = new int[MAX_COLS];
		output[i] = new int[MAX_COLS];
	}

	srand(123); //random number seed PLEASE DON'T CHANGE!
	int count = 0;
	for (int i = 0; i<MAX_ROWS; i++)
		for (int j = 0; j < MAX_COLS; j++)
		{
			data[i][j] = rand(); //RAND_MAX = 32767
		}

}

//*********************************************************************************

// returns the max number in the array
int getMax(int arr[], const int n)
{
	int mx = arr[0];
#pragma omp parallel for
	for (int i = 1; i < n; i++)
		if (arr[i] > mx)
			mx = arr[i];
	return mx;
}

// Does the main sort of the radix sort
void countSort(int arr[], const int n, int exp)
{
	int output[MAX_COLS]; // output array
	int i, count[10] = { 0 };

	//fills up the buckets
	for (i = 0; i < n; i++)
		count[(arr[i] / exp) % 10]++;

	// Change count[i] so that count[i] now contains actual
	//  position of this digit in output[]
	for (i = 1; i < 10; i++)
		count[i] += count[i - 1];

	// Build the output array
	for (i = n - 1; i >= 0; i--)
	{
		output[count[(arr[i] / exp) % 10] - 1] = arr[i];
		count[(arr[i] / exp) % 10]--;
	}

	//Copy the output array to the initial array, to have sorted numbers
#pragma omp parallel for
	for (i = 0; i < n; i++)
		arr[i] = output[i];
}
 
// Radix Sort
void RadixSort(int arr[], const int n)
{
	// Find the maximum number to know number of digits
	int m = getMax(arr, n);

	// Do counting sort for every digit. Note that instead
	// of passing digit number, exp is passed. exp is 10^i
	// where i is current digit number
	for (int exp = 1; m / exp > 0; exp *= 10)
		countSort(arr, n, exp);
}

//*********************************************************************************
//RADIX SORT FULL ON 2D ARRAY
//*********************************************************************************

// returns the max number in the array
int getMaxFull(int **arr)
{
	int mx = arr[0][0];
#pragma omp parallel for
	for (int i = 0; i < MAX_ROWS; i++)
	{
		for (int j = 0; j < MAX_COLS; j++)
		{
			if (arr[i][j] > mx)
				mx = arr[i][j];
		}
	}
	return mx;
}

// Does the main sort of the radix sort
void countSortFull(int **arr, int exp)
{
	int count[10] = { 0 };
	//***********************************
	// Section 1 - Using OMP Sections to split load
	//***********************************

	//create 4 sets of buckets
	int count1[10] = { 0 };
	int count2[10] = { 0 };
	int count3[10] = { 0 };
	int count4[10] = { 0 };

	int chunk = MAX_ROWS / 4;

	// Create 4 sections, and fill the 4 sets of buckets
#pragma omp parallel sections
	{
	#pragma omp  section
			for (int i = 0; i < chunk; i++)
			{
				for (int j = 0; j < MAX_COLS; j++)
				{
					count1[(arr[i][j] / exp) % 10]++;
				}
			}

#pragma omp  section
			for (int i = chunk; i < chunk * 2; i++)
			{
				for (int j = 0; j < MAX_COLS; j++)
				{
					count2[(arr[i][j] / exp) % 10]++;
				}
			}

#pragma omp  section
			for (int i = chunk*2; i < chunk*3; i++)
			{
				for (int j = 0; j < MAX_COLS; j++)
				{
					count3[(arr[i][j] / exp) % 10]++;
				}
			}

#pragma omp  section
			for (int i = chunk*3; i < MAX_ROWS; i++)
			{
				for (int j = 0; j < MAX_COLS; j++)
				{
					count4[(arr[i][j] / exp) % 10]++;
				}
			}
	}

	//add the buckets together
	for (int i = 0; i < 10; i++)
	{
		count[i] = count1[i] + count2[i] + count3[i] + count4[i];
	}

	// Change count[i] so that count[i] now contains actual
	//  position of this digit in output[]
	for (int i = 1; i < 10; i++)
		count[i] += count[i - 1];

	//***********************************
	// Section 2 - No possible optimization due to array bring manpiulated
	//***********************************
	// Build the output array

	for (int i = MAX_ROWS - 1; i >= 0; i--)
	{
		for (int j = MAX_COLS - 1; j >= 0; j--)
		{
			int pos = count[(arr[i][j] / exp) % 10] - 1;

			output[pos / MAX_COLS][pos % MAX_COLS] = arr[i][j];
			count[(arr[i][j] / exp) % 10]--;
		}
	}

	//***********************************
	// Section 3 - simple OMP FOR to speed up copying of arrays
	//***********************************
	// Copy output array to initial array
#pragma omp parallel for
	for (int i = 0; i < MAX_ROWS; i++)
	{
		for (int j = 0; j < MAX_COLS; j++)
		{
			arr[i][j] = output[i][j];
		}
	}
}

// Radix Sort Full
void RadixSortFull(int **arr)
{
	// Find the maximum number to know number of digits
	int m = getMaxFull(arr);

	// Do counting sort for every digit. Note that instead
	// of passing digit number, exp is passed. exp is 10^i
	// where i is current digit number
	for (int exp = 1; m / exp > 0; exp *= 10)
	{
		countSortFull(arr, exp);
	}
}



//*********************************************************************************
void sortEachRow()
{
	cout << "Sorting data...";
//Simple OMP for loop, threading is done here to have the overhead of creating threads
// to be as minimal as possible
#pragma omp parallel for
	for (int i = 0; i<MAX_ROWS; i++)
	{
		RadixSort(data[i], MAX_COLS);
	}
}

//*********************************************************************************
void workoutMovingAverage()
{
//OMP FOR used here. Used at the highest level possible to reduce thread creation overhead.
#pragma omp parallel for
	for (int i = 0; i < MAX_ROWS; i++)
	{
		int m = 0;
		for (int j = 0; j < MAX_COLS; j++)
		{
			if (j <= MAX_COLS - 100)
			{
				m = 100;
			}
			else
			{
				m = MAX_COLS - j;
			}

			int sum = 0;
			for (int k = j; k < j + m; k++)
			{
				sum = sum + data[i][k];
			}

			dataAvg[i][j] = sum / m;
		}
	}
}

//*********************************************************************************
void megaSort()
{
	RadixSortFull(data);
}

//*********************************************************************************
void displayCheckData()
{																														// Should be:
	cout << "\n\ndata[0][0]                   = " << data[0][0] << '\t'
		<< (data[0][0] - 87 ? " - FAILED!" : " - OK!");								//= 87 for srand(123)
	cout << "\ndata[MAX_ROWS/2][MAX_COLS/2] = " << data[MAX_ROWS / 2][MAX_COLS / 2] << '\t'
		<< (data[MAX_ROWS / 2][MAX_COLS / 2] - 16440 ? " - FAILED!" : " - OK!");	//= 16440 for srand(123)
	cout << "\ndata[MAX_ROWS-1][MAX_COLS-1] = " << data[MAX_ROWS - 1][MAX_COLS - 1] << '\t'
		<< (data[MAX_ROWS - 1][MAX_COLS - 1] - 32760 ? " - FAILED!" : " - OK!");	//= 32760 for srand(123)
}

//*********************************************************************************
void outputTimes()
{
	cout << "\n\nTime for sorting all rows		(s)		: " << s1.getElapsedTime();
	cout << "\nTime for outputting to file		(s)		: " << s2.getElapsedTime();
	cout << "\nTime for moving average			(s)		: " << s3.getElapsedTime();
	cout << "\nTime for outputting moving average	(s)		: " << s4.getElapsedTime();
	cout << "\nTime for megasort			(s)		: " << s5.getElapsedTime();
	cout << "\nTime for outputting megasort		(s)		: " << s6.getElapsedTime();

	cout << "\n\nCombined time				(s)		: " 
		 << s1.getElapsedTime() + s2.getElapsedTime() + s3.getElapsedTime() + s4.getElapsedTime() + s5.getElapsedTime() + s6.getElapsedTime() 
		 << "\n\n\nPress a key to terminate.";
}

//*********************************************************************************
//Builds a sorted number list as a long string then outputs the whole thing in one big fputs!

void outputDataAsString(int **outputData, string filename)
{
	cout << "\n\nOutputting data to " << filename.c_str() << "...";

	char numString[MAX_CHARS];
	string odata;
	string odata1;
	string odata2;
	string odata3;
	string odata4;
	int chunk = MAX_ROWS / 4;

//Using OMP Sections. numString is set to firstprivate due to being edited by all threads. 
//Split the data ranges into 4, create 4 strings and add them together at the end.

#pragma omp parallel sections firstprivate (numString)
	{
#pragma omp section
		for (int i = 0; i<chunk; i++)
		{
			for (int j = 0; j<MAX_COLS; j++)
			{
				odata1 += "\t";
				_itoa_s<6>(outputData[i][j], numString, 10);
				odata1 += numString;

			}
			odata1 += "\n";
		}

#pragma omp section
		for (int i = chunk; i<chunk * 2; i++)
		{
			for (int j = 0; j<MAX_COLS; j++)
			{
				odata2 += "\t";
				_itoa_s<6>(outputData[i][j], numString, 10);
				odata2 += numString;

			}
			odata2 += "\n";
		}

#pragma omp section
		for (int i = chunk * 2; i<chunk * 3; i++)
		{
			for (int j = 0; j<MAX_COLS; j++)
			{
				odata3 += "\t";
				_itoa_s<6>(outputData[i][j], numString, 10);
				odata3 += numString;

			}
			odata3 += "\n";
		}

#pragma omp section
		for (int i = chunk * 3; i<MAX_ROWS; i++)
		{
			for (int j = 0; j<MAX_COLS; j++)
			{
				odata4 += "\t";
				_itoa_s<6>(outputData[i][j], numString, 10);
				odata4 += numString;

			}
			odata4 += "\n";
		}
	}

	odata = odata1 + odata2 + odata3 + odata4;

	FILE * sodata;
	fopen_s(&sodata, filename.c_str(), "w");
	std::fputs(odata.c_str(), sodata);
	std::fclose(sodata);
}
//*********************************************************************************
