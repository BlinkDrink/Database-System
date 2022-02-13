#pragma once
#include<fstream>

using std::ifstream;
using std::ofstream;

/**
 * @brief Descriptor of BPTree pointers to the records.
*/
class RecordPtr
{
public:
	RecordPtr(ifstream& in)
	{
		in.read((char*)&pageNumber, sizeof(pageNumber));
		in.read((char*)&indexInPage, sizeof(indexInPage));
	}

	RecordPtr() : pageNumber(-1), indexInPage(-1) {}

	RecordPtr(int pageNumber, int indexInPage) : pageNumber(pageNumber), indexInPage(indexInPage) {}

	/**
	 * @return the number of the page
	*/
	int getPage() const
	{
		return pageNumber;
	}

	/**
	 * @return the index of the record in the page
	*/
	int getIndexInPage() const
	{
		return indexInPage;
	}

	/**
	 * @brief Write metadata to file
	*/
	void write(ofstream& out) const
	{
		out.write((char*)&pageNumber, sizeof(pageNumber));
		out.write((char*)&indexInPage, sizeof(indexInPage));
	}

	bool operator<(const RecordPtr& other)
	{
		if (pageNumber < other.pageNumber)
			return true;
		else if (pageNumber == other.pageNumber)
			if (indexInPage < other.indexInPage)
				return true;

		return false;
	}

	bool operator>(const RecordPtr& other)
	{
		if (pageNumber > other.pageNumber)
			return true;
		else if (pageNumber == other.pageNumber)
			if (indexInPage > other.indexInPage)
				return true;

		return false;
	}

	bool operator==(const RecordPtr& other)
	{
		if (pageNumber == other.pageNumber && indexInPage == other.indexInPage)
			return true;

		return false;
	}
private:
	int pageNumber, indexInPage;
};