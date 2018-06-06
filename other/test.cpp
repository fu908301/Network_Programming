#include<iostream>
#include<fstream>
#include<vector>
#include<cmath>
using namespace std;
int main(){
	char buf[32468];
	ifstream file1;
	ifstream f1;
	ofstream fileOut1;
	file1.open("Video1.mp4", ios::in | ios::binary | ios::ate);
	f1.open("Video1.mp4", ios::in | ios::binary);
	fileOut1.open("out1.mp4", ios::out | ios::binary);
	cout << "file size : " << file1.tellg() << endl;
	int chunkNum = ceil((float)file1.tellg()/32468);	
	cout << chunkNum << endl;
	int fileSize = file1.tellg();
	/*streampos begin,end;
	begin = file1.tellg();
  	file1.seekg (0, ios::end);
  	end = file1.tellg();
	cout << "file size : " << (end - begin) << endl;*/
	/*while(file1.peek()!=EOF){
		file1.read(buf, sizeof(buf));
		//cout << buf;
		fileOut1 << buf;
	}*/
	
	/*while(file1.read(buf.data(), size)){
		cout << buf.data();
		fileOut1 << buf.data();	
	}*/

	for(int i=0 ; i<chunkNum ; i++){
		if(fileSize > buf)	fileSize -= buf;
		
		f1.read(buf, sizeof(buf));
		cout << buf;
		fileOut1.write(buf, sizeof(buf));
	}
	
	file1.close();
	f1.close();
	fileOut1.close();
}
