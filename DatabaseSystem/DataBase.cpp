#include "DataBase.h"

DataBase::DataBase(ifstream& in)
{
	fh::readString(in, fDBName);
	fh::readString(in, fDBPath);

	size_t size = 0;
	in.read((char*)&size, sizeof(size));
	for (size_t i = 0; i < size; i++)
	{
		string first, second;
		fh::readString(in, first);
		fh::readString(in, second);

		ifstream tableReader(second + first + ".bin", std::ios::binary);
		if (!tableReader.is_open())
			throw invalid_argument("Couldn't open " + second + " path for reading. Check for file corruption!");

		fTables.insert({ first, Table(tableReader) });
	}
}

DataBase::DataBase(const string& name, const string& path)
{
	fDBName = name;
	fDBPath = path;
	createDirectory();
	save();
}

void DataBase::createTable(const string& path, const string& tableName, unordered_map<string, string>& colNameType, vector<string>& colNames, const string primaryKey, int maxRecordsPerPage)
{
	if (fTables.find(tableName) != fTables.end())
		throw invalid_argument("There is already a table with this name in the system");

	Table t(fDBPath, tableName, colNameType, colNames, primaryKey, maxRecordsPerPage);
	fTables[tableName] = t;
	save();
}

void DataBase::dropTable(const string& tableName)
{
	string pathToDelete = getTable(tableName).getTablePath();
	std::error_code errorCode;
	if (!fs::remove_all(pathToDelete, errorCode))
		throw logic_error(errorCode.message());

	fTables.erase(tableName);
	save();
}

void DataBase::insert(const string& tableName, vector<unordered_map<string, TypeWrapper>> colNameValueList)
{
	for (size_t i = 0; i < colNameValueList.size(); i++)
		getTable(tableName).insert(colNameValueList[i]);

	save();
}

int DataBase::remove(const string& tableName, Query& query)
{
	int deletedRecords = getTable(tableName).deleteRecord(query);

	save();
	return deletedRecords;
}


void DataBase::listTables() const
{
	for (const pair<string, Table>& entry : fTables)
		std::cout << "\t-" << entry.first << "\n";
}

Table& DataBase::getTable(const string& name)
{
	if (fTables.find(name) == fTables.end())
		throw invalid_argument("There is no such table!");

	return fTables[name];
}

void DataBase::save() const
{
	ofstream out(fDBPath + fDBName + ".bin", std::ios::binary);
	if (!out.is_open())
		throw exception("Couldn't open file to save Database");

	fh::writeString(out, fDBName);
	fh::writeString(out, fDBPath);

	size_t tablePathsSize = fTables.size();
	out.write((char*)&tablePathsSize, sizeof(tablePathsSize));
	for (const pair<string, Table>& entry : fTables)
	{
		fh::writeString(out, entry.first);
		fh::writeString(out, entry.second.getTablePath());
	}

	out.close();
}

void DataBase::createDirectory() const
{
	fs::create_directories(fDBPath);
}
