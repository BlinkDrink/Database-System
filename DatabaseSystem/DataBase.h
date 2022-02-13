#pragma once
#include "Table.hpp"

class DataBase
{
public:
	DataBase(ifstream& in);

	DataBase(const string& name, const string& path);

	/**
	 * @brief Attempt to create a table with given name, column types and primary key
	 * @param path - path of table on disk
	 * @param tableName - name of table
	 * @param colNameType - hashtable where against each column name we have a column type (Integer, String, Double)
	 * @param primaryKey - the name of the indexed column
	 * @param maxRecordsPerPage - how many records we can keep in a page
	*/
	void createTable(const string& path, const string& tableName, unordered_map<string, string>& colNameType, vector<string>& colNames, const string primaryKey = "", int maxRecordsPerPage = 1024);

	/**
	 * @brief Attempts to drop a table with given name, removing it from fTables and deleting the binary file of the table on the disk
	 * @param tableName - name of table
	*/
	void dropTable(const string& tableName);

	/**
	 * @brief Attempts to insert an array of records in the table with name {tableName}
	 * @param tableName - name of table
	 * @param colNameValueList - array of hashtables where against each column we have the value that is corresponding to the column
	*/
	void insert(const string& tableName, vector<unordered_map<string, TypeWrapper>> colNameValueList);

	int remove(const string& tableName, Query& query);

	/**
	 * @return the number of tables in the database
	*/
	size_t getNumTables() const { return fTables.size(); }

	/**
	 * @brief used to display the names of all tables in the database
	*/
	void listTables() const;

	/**
	 * @return desired table by it's name
	*/
	Table& getTable(const string& name);

	/**
	 * @brief Saves the metadata of Database object to binary file
	*/
	void save() const;
private:

	void createDirectory() const;

	string fDBName, fDBPath;
	unordered_map<string, Table> fTables;
};