#include<iostream>
#include<fstream>
#include<vector>
#include<cmath>
using namespace std;
int main(){
	ifstream file1;
	ifstream file2;
	ofstream fileOut1;
	file1.open("Video1.mp4", ios::in | ios::binary | ios::ate);
	if(!file1)
		cout << "Open error 1" << endl;
	fileOut1.open("out1.mp4", ios::out | ios::binary);
	if(!fileOut1)
		cout << "Open error 2" << endl;
	int fileSize = file1.tellg();
	char *buffer = new char[fileSize];
	char ch;
	int i = 0;
	cout << "File size : " << fileSize << endl;
	file1.seekg(0, ios::beg);
	file1.read(buffer,fileSize);
	fileOut1.write(buffer, fileSize);
	file1.close();
	fileOut1.close();
	delete [] buffer;
	return 0;
}
