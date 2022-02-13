#pragma once
#include<vector>
#include "TypeWrapper.hpp"
#include "ObjectType.h"

using std::vector;

class Record {

	/**
	 * @brief A record represents a row in a table. It consists of
	 * an array of values.
	 */
private:
	bool fIsInvalidated;
	vector<TypeWrapper> fValues;
	size_t fColumns;

public:
	Record() :fColumns(0), fIsInvalidated(false) {}

	Record(std::ifstream& in)
	{
		in.read((char*)&fIsInvalidated, sizeof(fIsInvalidated));
		in.read((char*)&fColumns, sizeof(fColumns));
		for (size_t i = 0; i < fColumns; i++)
			fValues.push_back(TypeWrapper(in));
	}

	/**
	 * @brief Creates a new record
	 * @param size number of columns of the table holding the record
	 */
	Record(size_t size) : fColumns(size), fIsInvalidated(false)
	{
		this->fValues.reserve(size);
	}

	/**
	 * Update the value for a given column in the record
	 * @param index - the index of the column to be updated
	 * @param value - the new value
	 */
	void addValue(const TypeWrapper& value)
	{
		if (fValues.size() + 1 > fColumns)
			throw std::out_of_range("Record addValue(value) - maximum properties for this record reached");

		fValues.push_back(value);
	}

	/**
	 * Get the value of a given column of this record
	 * @param index - index of the required column
	 * @return the value of that column
	 */
	const TypeWrapper& get(size_t index) const
	{
		if (index >= fValues.size())
			throw std::out_of_range("Record get(index) - index is out of range");

		return fValues[index];
	}

	/**
	 *  @brief Write a record to file
	 *  @param out - output stream, used for writing
	 */
	void write(ofstream& out) const
	{
		out.write((char*)&fIsInvalidated, sizeof(fIsInvalidated));
		out.write((char*)&fColumns, sizeof(fColumns));
		for (size_t i = 0; i < fValues.size(); i++)
			fValues[i].write(out);
	}

	/**
	 * @brief Used when deleting a record, setting its invaldation state to true
	 * Because when remove operation is done in the Page class in real time, simply erasing the element is not enough
	 * because the order and indexation of elements in the vector fValues gets mismatched.
	*/
	void invalidateRecord()
	{
		fIsInvalidated = true;
		fValues.clear();
		fColumns = 0;
	}

	bool isInvalid() const { return fIsInvalidated; }

	size_t getKiloBytesData() const
	{
		size_t KB = 0;
		for (size_t i = 0; i < fValues.size(); i++)
			KB += fValues[i].getContent()->memsize();

		return KB;
	}

	bool operator==(const Record& other) const
	{
		if (other.fColumns != fColumns)
			return false;

		for (size_t i = 0; i < fColumns; i++)
			if (fValues[i] != other.fValues[i])
				return false;

		return true;
	}

	bool operator<(const Record& other) const
	{
		if (other.fColumns != fColumns)
			return false;

		for (size_t i = 0; i < fColumns; i++)
			if (fValues[i] >= other.fValues[i])
				return false;

		return true;
	}

	bool operator>(const Record& other) const
	{
		if (other.fColumns != fColumns)
			return false;

		for (size_t i = 0; i < fColumns; i++)
			if (fValues[i] <= other.fValues[i])
				return false;

		return true;
	}

	/**
	 *	@brief Getter
	 *	@return the number of columns of this record
	 */
	size_t size() const { return this->fColumns; }

	/**
	 * Display the record values
	 */
	std::string toString() const
	{
		std::string res;
		for (size_t i = 0; i < fValues.size(); i++)
			res += fValues[i].getContent()->toString() + "|";

		return res;
	}
};