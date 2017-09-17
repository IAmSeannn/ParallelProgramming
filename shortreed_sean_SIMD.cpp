/****************************************************************
	MAPS ASSIGNMENT 2 - SIMD (integer to ASCII) conversions
	A.Oram
	Revised: March 2017

	////////////////////////////////////////////////
	// Expected Speedup by Sean : ~70% on most PCs
	////////////////////////////////////////////////

	PLEASE EDIT THE LINES BELOW WITH YOUR NAME AND STUDENT NUMBER:
*/

#define  StudentName "Sean Shortreed"
#define  StudentNumber "26037992"

	
//*****************************************************************

#include <fstream>			//for file output
#include <iostream>			//for console output
#include <string>			//
#include <conio.h>			//for kbhit
#include "hr_time.h"		//for stopwatches
#include <stdio.h>			//for fputs
#include <iomanip>          //for formatted output in 'cout'
#include <omp.h>			//for OpenMP constructs
using namespace std;

#define MAX_NUMS 100000		// keep this small e.g. 10, for testing and checking
							// but use 100,000 (for example) for timing.
							// Avoid updating the output text file "theDataFile.txt" when MAX_NUMS is large.
#define MAX_CHARS 6			// Assume our data has no more than 5 digits plus a NULL string terminator.


#define ten 10
#define ASCII 0x30			// add this to a single digit 0-9 to make an ASCII character

__declspec(align(16)) int theData[MAX_NUMS];		// filled up with random numbers

CStopWatch ITOA,CPP,ASM,SIMD;		// Timers

void createTheData (void);			// Array of randomly generated numbers to convert to ASCII
void outputResults (void);

void outputData_ItoA_Library (void); // Uses library call to do conversion
void outputData_ItoA_CPP (void);	 // Uses C++ to do conversion (using division and modulus)
void outputData_ItoA_ASM (void);	 // Uses handwritten assembly to do conversion (avoiding division)
void outputData_ItoA_SIMD (void);	 // *** To be written using SIMD methods! ***

FILE * theDataFile;					// all converted number strings written here
char numString[MAX_CHARS];			// temp store for converted numbers
string allDataASCII;

//***********************************************************
int main(void)
{
		createTheData();
		fopen_s(&theDataFile, "theDataFile.txt", "w");	// open output File for appending

//ITOA *********************************************
		outputData_ItoA_Library();
//CPP **********************************************
		allDataASCII.clear();
		outputData_ItoA_CPP();
//ASM **********************************************
		allDataASCII.clear();
		outputData_ItoA_ASM();
//SIMD *********************************************
		allDataASCII.clear();
		outputData_ItoA_SIMD();  // *** to be developed!
//**************************************************

		fclose(theDataFile);
		outputResults();
		cout << "\n\n\n Done...\n ";
		system ("PAUSE");
}
//************** END MAIN *********************************************


//SIMD **************************************************

void outputData_ItoA_SIMD(void)
{
	void ItoA_SIMD(int*, char *);
	char SIMD_String[MAX_CHARS*8] = "xxxxx";	// will need to accommodate 4 (or more?) converted numbers
												// "xxxxx" is just for test purposes!
	SIMD.startTimer();

	//for loop set to go in increments of 8, due to simd function handling 8 numbers at a time
	for (int i = 0; i < MAX_NUMS; i+=8)		 // You will need to tailor this FOR loop
	{
		ItoA_SIMD(&theData[i], SIMD_String); // do conversion using SIMD, avoiding division

		//two ints to handle the building of one char array that can be added once, instead of multiple adds.
		int count = 0;
		int index = 0;
		//buffer to store complete char array
		char CharBuffer[1+(MAX_CHARS * 8)];

		//loops over the 8 numbers from SIMD
		for (int j = 0; j < 8; j++)
		{
			//check whether to convert '0's to spaces
			if (SIMD_String[count] == '0')
			{
				SIMD_String[count] = ' ';

				if (SIMD_String[count + 1] == '0')
				{
					SIMD_String[count + 1] = ' ';

					if (SIMD_String[count + 2] == '0')
					{
						SIMD_String[count + 2] = ' ';

						if (SIMD_String[count + 3] == '0')
						{
							SIMD_String[count + 3] = ' ';

							if (SIMD_String[count + 4] == '0')
							{
								SIMD_String[count + 4] = ' ';
							}
						}
					}
				}
			}

			//loops over the numbers individually
			//Adds them to the char array
			for (int i = 0; i < MAX_CHARS-1; i++)
			{
				CharBuffer[index++] = SIMD_String[count++];
			}
			count++;
			CharBuffer[index++] = '\t';
		}

		CharBuffer[index++] = '\0';
		
		allDataASCII += CharBuffer;
	}



	allDataASCII += "\n\n";
	SIMD.stopTimer();

	fputs("SIMD method: ", theDataFile);
	fputs(allDataASCII.c_str(), theDataFile);
}
//***********************************************************

//***********************************************************
// This code should avoid division and use SIMD.

void ItoA_SIMD(int *num, char * numStr)
{
	_declspec(align(16)) short DataToSort[8] = { num[0], num[1],  num[2],  num[3], num[4], num[5], num [6], num[7] };

	//Simd vars
	__m128i* OrigData = (__m128i*)DataToSort;
	__m128i SimdData;
	__m128i opNumber;
	__m128i DataToUse = *OrigData;
	__m128i SavedData;

	//start for loop
	for (int i = 0; i < 5; i++)
	{
		SimdData = DataToUse;
		SavedData = SimdData;
		//setup opNumber with magic number
		opNumber = _mm_set1_epi16(0x6667);
		//times by magic number
		SimdData = _mm_mulhi_epi16(opNumber, SimdData);
		//shift right
		SimdData = _mm_srai_epi16(SimdData, 2);
		//Save these numbers for next round
		DataToUse = SimdData;
		//setup opNumber with 10
		opNumber = _mm_set1_epi16(10);
		//times by 10
		SimdData = _mm_mullo_epi16(opNumber, SimdData);
		//take new num from orig num
		SimdData = _mm_sub_epi16(SavedData, SimdData);
		//SimdData now contains single digits
		//Convert SimdData to char numbers
		//Setup opNumber with char converstion
		opNumber = _mm_set1_epi16(ASCII);
		//do the addition
		SimdData = _mm_add_epi16(opNumber, SimdData);
		//SimdData now contains correct chars
		//sort letters into correct arrays
		numStr[4 - i] = SimdData.m128i_i16[0];
		numStr[6 + 4 - i] = SimdData.m128i_i16[1];
		numStr[12 + 4 - i] = SimdData.m128i_i16[2];
		numStr[18 + 4 - i] = SimdData.m128i_i16[3];
		numStr[24 + 4 - i] = SimdData.m128i_i16[4];
		numStr[30 + 4 - i] = SimdData.m128i_i16[5];
		numStr[36 + 4 - i] = SimdData.m128i_i16[6];
		numStr[42 + 4 - i] = SimdData.m128i_i16[7];
	}

	numStr[5] = '\0';
	numStr[6 + 5] = '\0';
	numStr[12 + 5] = '\0';
	numStr[18 + 5] = '\0';
	numStr[24 + 5] = '\0';
	numStr[30 + 5] = '\0';
	numStr[36 + 5] = '\0';
	numStr[42 + 5] = '\0';
}
//***********************************************************





// ********** The alternatives forms of ItoA **********************

// ITOA ***********************************************************

void outputData_ItoA_Library(void)
{
	ITOA.startTimer();
	for (int i = 0; i < MAX_NUMS; i++)
	{
		//_itoa_s(theData[i], numString, 10);		// do the conversion
		allDataASCII += to_string(theData[i]);	// *** alternative
		//allDataASCII += numString;				//
		allDataASCII += "\t";					// tab separate each number string
	}
	allDataASCII += "\n\n";						// terminate string with newlines
	ITOA.stopTimer();

	fputs  ("ItoA method:  ", theDataFile);
	fputs  (allDataASCII.data(), theDataFile);
}
//****************************************************************


// CPP ***********************************************************

void outputData_ItoA_CPP(void)
{
	void ItoA_CPP(int, char *);
	
	CPP.startTimer();
	for (int i = 0; i < MAX_NUMS; i++)
	{
		ItoA_CPP (theData[i], numString);		// do the conversion in C++
		allDataASCII += numString;				//
		allDataASCII += "\t";					// tab separate each number string
	}
	allDataASCII += "\n\n";						// terminate string with newlines
	CPP.stopTimer();

	fputs("C++ method:  ", theDataFile);
	fputs(allDataASCII.data(), theDataFile);
}

//***********************************************************
void ItoA_CPP (int num, char * numStr)
{
	int nextDig = num;

	for (int i(0); i < MAX_CHARS-1; i++)
	{	*numStr = ' ';			// clear string to spaces to begin with
		numStr++;
	}
	numStr--;
	*numStr = NULL;				// insert string terminator

	while ((nextDig / ten) != 0)
	{
		*numStr = (nextDig % ten) + ASCII;
		nextDig = (nextDig / ten);
		numStr--;				// build string in reverse order
	}
	*numStr = (nextDig % ten) + ASCII;
}


// ASM ***********************************************************

void outputData_ItoA_ASM(void)
{
	void ItoA_ASM(int, char *);
	
	ASM.startTimer();
	for (int i = 0; i < MAX_NUMS; i++)
	{
		ItoA_ASM(theData[i], numString);	// do conversion using assembly
		allDataASCII += numString;
		allDataASCII += "\t";
	}
	allDataASCII += "\n\n";
	ASM.stopTimer();

	fputs("ASM method:  ", theDataFile);
	fputs(allDataASCII.c_str(), theDataFile);
}
//***********************************************************

//***********************************************************
// This code avoids use of division.

void ItoA_ASM (int num, char * numStr)
{	__asm {
			//push	ebx				// These volatile register pushes are done
			//push	esi				// by the calling mechanism so are here as reminders only.
			mov		ebx,numStr		// point EBX to numStr
			mov		esi,num			// store number in ESI
			cmp		esi, 0			// IF number = 0
			jne		nextdigit
			mov		[ebx],ASCII		// THEN simply set numStr to ASCII "0"
			mov		[ebx+1],NULL	//      add terminating null character
			jmp		endItoA			//      and end

// -------- divide num by 10 to get next digit, using a performance trick. ---------------
nextdigit:	mov		eax, 66666667h	// 66666667h = 2^34 / 10
			imul	esi				// EDX:EAX = number * (2^34 / 10)
									// therefore EDX = number * (2^2 / 10)
			sar		edx,2			// EDX = EDX / 2^2
									// therefore EDX = number / 10 (integer division)
// ---------------------------------------------------------------------------------------
			lea		ecx,[edx+edx*4]	// ECX = EDX * 5
			add		ecx,ecx			// ECX = EDX * 10 (could use "sal ecx,1")
									// therefore ECX = (number div 10)*10

			mov		eax,esi			// store original number in EAX
			sub		eax,ecx			// subtract ECX to leave remainder in EAX
			add		eax,ASCII		// add 30h to make EAX the digit's ASCII code
			mov		[ebx],al		// store digit character in "numStr"
			inc		ebx				// move pointer along string

			mov		esi,edx			// move number div 10 back into ESI
			cmp		esi,0			// if (number div 10) = 0, we've finished
			jnz		nextdigit

			mov		[ebx],NULL		// so add terminating null character

//reverse string
			mov		edx,numStr		// the number is in reverse order of digits
nextChar:	dec		ebx				// so we need to reverse the string
			cmp		ebx,edx
			jle		endItoA
			mov		eax,[edx]
			mov		ecx,[ebx]
			mov		[ebx],al
			mov		[edx],cl
			inc		edx
			jmp     nextChar
endItoA:
			//pop esi
			//pop ebx
	} // end of ASM block
}


//***********************************************************

string commarise(long long value)	// insert commas in long integers to make reading easier
{
	string WithCommas = to_string(value);
	int commaPos = WithCommas.length() - 3;
	while (commaPos > 0)
	{
		WithCommas.insert(commaPos, ",");
		commaPos -= 3;
	}
	return WithCommas;
}//*******************************************************************



void outputResults(void)
{
#define milli 1000
#define milli_s " ms"
#define micro 1000000
#define micro_s " us"

#define textLine "\n-----------------------------------------------------"
#define text1 " version took "
#define text2  "\nSIMD speed up over ASM method "

	ofstream ItoAResults;
	ItoAResults.open("ItoAResults.txt", ios::app);
	ItoAResults << textLine;
	// Display results...
	cout << setprecision(7);
	cout << "\n\nItoA" << text1 << ITOA.getElapsedTime()* micro << micro_s;
	cout << "\n\nCPP " << text1 << CPP.getElapsedTime()* micro << micro_s;
	cout << "\n\nASM " << text1 << ASM.getElapsedTime()* micro << micro_s;
	cout << "\n\nSIMD" << text1 << SIMD.getElapsedTime()* micro << micro_s;
	ItoAResults << setprecision(7);
	ItoAResults << "\nItoA" << text1 << ITOA.getElapsedTime()* micro << micro_s;
	ItoAResults << "\nCPP " << text1 << CPP.getElapsedTime()* micro << micro_s;
	ItoAResults << "\nASM " << text1 << ASM.getElapsedTime()* micro << micro_s;
	ItoAResults << "\nSIMD" << text1 << SIMD.getElapsedTime()* micro << micro_s;
	
	cout << setprecision(4) << text2 << "= "
		<< (double) 100.0F*((ASM.getElapsedTime() - SIMD.getElapsedTime()) / ASM.getElapsedTime()) << "%";

	ItoAResults << setprecision(4) << text2 << "for "<< StudentName << ", " << StudentNumber << " =\t"
		<< (double) 100.0F*((ASM.getElapsedTime() - SIMD.getElapsedTime()) / ASM.getElapsedTime()) << "%";
	cout << "\n\nMaxNums =" << commarise (MAX_NUMS);
	ItoAResults << "\nMaxNums =" << commarise (MAX_NUMS);
	ItoAResults.close();
}
//******************************************************************


//******** Generate the Data **************************************

void createTheData()
{
	cout << "Generating data...";
	srand(123); //arbitrary random number seed
	for (int i = 0; i < MAX_NUMS; i++)
		theData[i] = rand();				//RAND_MAX = 32,767
}

//********** End of program *************************

