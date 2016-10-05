//NOTE:  CAN I USE BREAK; IN THE WHILE LOOPS TO AVOID WITH EMPTY LINES
//Arshya Sharifian, CPSC 121, Program#6
/*
This program completes the same action in two separate and distinguishable ways;
first, it stores values from a file into parallel arrays and outputs the
information to the user; second, it stores values into an array of structures, 
and again, output the information to the user.  Here, we are performing the 
same tasks in two unique ways.
*/
//Pre-Processing 
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

///////////////////////////////////////////////////////////////////////////////
//Structure Declaration
struct profile {
	int age;
	string name;
};
///////////////////////////////////////////////////////////////////////////////
//Function Prototypes
void openFile(ifstream&, string);
void getParallelData(ifstream&, string, int*, string*, int &);
int searchParallel(int*, string *, int);
void outputParallel(int *, string *, int);
void getStructData(ifstream&, string, profile *);
int searchStruct(profile *, int);
void outputStruct(profile *, int);
///////////////////////////////////////////////////////////////////////////////
int main() {
///////////////////////////////////////////////////////////////////////////////	
	//Variables Declaration and Initialization
	string file_name, read_file;
	ifstream fin;
	profile person[20];
	int age[20], index = 0, index_max_age = 0, arrsize = 0;
	string name[20];
///////////////////////////////////////////////////////////////////////////////
	//FUNCTION CALL
	openFile(fin, file_name);
	getParallelData(fin, read_file, age, name, arrsize);
	index_max_age = searchParallel(age, name, arrsize);
	outputParallel(age, name, index_max_age);
	fin.close();
	openFile(fin, file_name);
	getStructData(fin, read_file, person);
	index_max_age = searchStruct(person, arrsize);
	outputStruct(person, index_max_age);
	fin.close();
///////////////////////////////////////////////////////////////////////////////	
return 0;
//END MAIN
}
///////////////////////////////////////////////////////////////////////////////
/*
The openFile function asks the user for the name of the file they wish to open
and attempts to open the file.  If the file name is not found, then the program
is terminated.
*/
void openFile(ifstream & fin, string file_name) {
	cout << "Please enter the name of the file you wish to open:  " << endl;
	cin >> file_name;
	fin.open(file_name.c_str());
	//if the file is not found, exit the program
	if (!fin) {
		cout << "file not found, exiting program" << endl;
		exit(0);
	//end if not fin
	}
//End function openFile
}
///////////////////////////////////////////////////////////////////////////////
/*
The getParallelData function simply stores information from the opened file
into two parallel arrays for later use.  We also determine the number of 
profiles in the file using arrsize.
*/
void getParallelData(ifstream & fin, string read_file, int * age, string * name, int & arrsize) {
	//Local Variable
	int index = 0;
	
	while (getline(fin, read_file) && read_file.length() > 0) {
		if (isalpha(read_file[0])) {
			name[index] = read_file;
		//end if isalpha
		}
		else if (isdigit(read_file[0])) {
			age[index] = stoi(read_file);
			index++;
			arrsize++;
		//end else if isdigit
		}
	//end while
	}
//end function getParallelData
}
///////////////////////////////////////////////////////////////////////////////
/*
Here, we are searching through the various values held in the parallel array
and determining which person is the oldest.  The index value of the oldest
person is then sent back to main for future use.
*/
int searchParallel(int * age, string * name, int arrsize) {
	//local variables
	int max_age = 0, index = 0;
	//searching the array for largest age
	for (int i = 0; i < arrsize; i++) {
		if (age[i] > max_age) {
			max_age = age[i];
			index = i;
		//end if age
		}
	//end for loop
	}
	return index;
//end function searchParallel
}
///////////////////////////////////////////////////////////////////////////////
/*
The function outputParallel prints the name and age of the oldest person.
*/
void outputParallel(int * age, string * name, int index_max_age) {
	cout << name[index_max_age] << " is the oldest at "
		<< age[index_max_age] << endl;
}
///////////////////////////////////////////////////////////////////////////////
/*
The getStructData function reads through the user inputed file name and stores
values inside the structure array.  
*/
void getStructData(ifstream & fin, string read_file, profile * person) {
	//local variable
	int index = 0;
	//while there are more values to read
	while (getline(fin, read_file) && read_file.length() > 0) {
		if (isalpha(read_file[0])) {
			person[index].name = read_file;
		//end if isalpha
		}
		else if (isdigit(read_file[0])) {
			person[index].age = stoi(read_file);
			index++;
		//end else if isdigit
		}
	//end while
	}
//end function getStructData
}
///////////////////////////////////////////////////////////////////////////////
/*
The searchStruct function searches through the structure array and determines
who is the oldest person in the file.
*/
int searchStruct(profile * person, int arrsize) {
	//local variables
	int max_age = 0, index = 0;
	//searching for the oldeest person
	for (int i = 0; i < arrsize; i++) {
		if (person[i].age > max_age) {
			max_age = person[i].age;
			index = i;
		//end if person
		}
	//end for loop
	}
	//returnining the position of the oldest person in the array
	return index;
//end searchStruct function	
}
///////////////////////////////////////////////////////////////////////////////
/*
The outputStruct functions prints the name and age of the oldest person in the
file.
*/
void outputStruct(profile * person, int index_max_age) {
	cout << person[index_max_age].name << " is the oldest at "
		<< person[index_max_age].age << endl;
//end outputStruct function
}
///////////////////////////////////////////////////////////////////////////////
//END FUNCTIONS