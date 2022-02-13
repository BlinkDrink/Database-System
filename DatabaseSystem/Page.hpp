#pragma once
#include "Record.hpp"
#include "FileHelper.hpp"
using fh = FileHelper;

class Page {

private:
	/**
	 * A page is part of a table holding its records.
	 * Tables are stored in many pages in a binary
	 * file format (.bin files)
	 */
	int maxSize;
	string path;
	vector<Record> records;

public:

	Page(ifstream& in)
	{
		/// @brief Read page's max capacity to object
		in.read((char*)&maxSize, sizeof(maxSize));

		/// @brief Read page's path to object
		fh::readString(in, path);

		/// @brief Read number of records 
		size_t num_records = 0;
		in.read((char*)&num_records, sizeof(num_records));

		/// @brief Read records themselves
		for (size_t i = 0; i < num_records; i++)
		{
			records.push_back(Record(in));
		}
	}

	/**
	 * Create a new page specifying the maximum number of records it can hold
	 * and the path at which the page will be stored relative to the executable files
	 *
	 * @param maxSize the maximum number of records that fit in one page
	 * @param path the path at which the page is stored relative to the executable files
	 */
	Page(int maxSize, const string& path)
	{
		this->path = path;
		this->maxSize = maxSize;
		this->save();
	}

	/**
	 * Check whether a page has records with the maximum number of records or not
	 * @return whether a page is full or not
	 */
	bool isFull()
	{
		return records.size() == maxSize;
	}

	/**
	 * Insert a new record at the end of the page
	 * @param record the record to be inserted
	 * @return a boolean to indicate a successful/failed insertion
	 * @throws IOException If an I/O error occurred
	 */
	bool addRecord(const Record& record)
	{
		if (isFull())
			return false;

		records.push_back(record);
		save();

		return true;
	}

	/**
	 * Delete a record from the page at specified index
	 * @param index the index of the record in the page to be deleted
	 */
	void removeRecord(size_t index)
	{
		records[index].invalidateRecord();
		save();
	}

	/**
	 * @brief Save the page on the disk
	*/
	void save()
	{
		ofstream out(path, std::ios::binary);
		if (!out.is_open())
			throw std::logic_error("Couldn't open file to save page " + path);

		/// @brief Save page's max capacity to file
		out.write((char*)&maxSize, sizeof(maxSize));

		/// @brief Save page's path to file
		fh::writeString(out, path);

		/// @brief Save page's number of current records to file
		size_t size = records.size();
		out.write((char*)&size, sizeof(size));

		/// @brief Save the records themseleves to file
		for (size_t i = 0; i < records.size(); i++)
			records[i].write(out);

		out.close();
	}

	/**
	 * Returns the current number of records in the page
	 * @return the number of records in the pag
	 */
	size_t size()
	{
		return records.size();
	}

	/**
	 * Get a record with its position in the page
	 * @param index the position of the record in the page
	 * @return the required record
	 */
	Record get(size_t index)
	{
		if (index >= 0 && index < records.size())
			return records[index];

		throw std::out_of_range(index + " is out of range");
	}
};