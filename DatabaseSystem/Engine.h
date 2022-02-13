#pragma once
#include<iostream>
#include "termcolor.hpp"
#include "Database.h"
#include "CommandParser.hpp"

using std::cout;
using std::endl;
using std::cin;
using termcolor::red;
using termcolor::reset;
using termcolor::green;
using termcolor::yellow;

/**
 * @brief Descriptor of engine singleton class
*/
class Engine
{
private:
	Engine() = default;

	void menu();

	/**
	 * @brief By given string of form ({columnName}:{columnType}) convert it into hashtable
	 * @param scheme - stringified table scheme
	 * @return hashtable where each key is column name and value is column data type
	*/
	unordered_map<string, string> getColNameType(string scheme, vector<string>& colNames);

	/**
	 * @brief By given record in form {({DataType}, {DataType}, {DataType})} where each datatype corresponds to a column
	 * check if it satisfies the table's scheme, after which insert it into the desired table
	 * @param values - stringified values to be inserted
	 * @return vector of hashtables, each hashtable corresponding to one record that the user will be inserting
	*/
	vector<unordered_map<string, TypeWrapper>> getColNameValues(string values, unordered_map<string, string>& scheme, unordered_map<size_t, string>& indexColumn);

	void printSelectedRecords(vector<Record>& records, vector<string>& selectedColumns, unordered_map<string, size_t> colIndex) const;

	void printHeader(vector<string>& selectedColumns, unordered_map<string, size_t>& longestWordsPerCol) const;

	size_t getLongestContentAtCol(size_t col, vector<Record>& records) const;

	void printCellInformation(TypeWrapper& cell, size_t longestWordOfCol, size_t colSize) const;

	unordered_map<string, size_t> getLongestWordPerCol(vector<Record>& records, vector<string>& selectedColumns, unordered_map<string, size_t> colIndex) const;

public:
	static Engine& getInstance();

	Engine(const Engine& other) = delete;
	Engine& operator=(const Engine& other) = delete;
	Engine(Engine&& other) = delete;
	Engine& operator=(Engine&& other) = delete;

	void run();
};