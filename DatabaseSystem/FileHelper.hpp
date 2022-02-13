#pragma once
#include<fstream>
#include<string>

using std::ifstream;
using std::string;
using std::ofstream;

class FileHelper
{
public:
	static void readString(ifstream& in, string& dest)
	{
		size_t size = 0;
		char* str = nullptr;
		in.read((char*)&size, sizeof(size));
		str = new char[size + 1];
		in.read(str, size);
		str[size] = '\0';
		dest = str;
		delete[] str;
	}

	static void writeString(ofstream& out, string dest)
	{
		size_t size = dest.size();
		out.write((char*)&size, sizeof(size));
		out.write((char*)dest.c_str(), size);
	}
};
