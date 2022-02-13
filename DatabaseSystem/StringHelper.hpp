#pragma once
#include <string>
#include <vector>

using std::string;
using std::vector;

class StringHelper
{
private:
	StringHelper();
public:
	static bool isStringDouble(const string& source)
	{
		if (source.empty())
		{
			return false;
		}

		size_t numberOfDots = 0;

		for (size_t i = 0; i < source.size(); i++)
		{
			if (source[i] == '.')
			{
				numberOfDots++;
			}
		}

		if (numberOfDots != 1)
		{
			return false;
		}

		vector<string> parts = splitBy(source, ".");
		removeEmptyStringsInVector(parts);

		if (parts.size() != 2)
		{
			return false;
		}

		for (size_t i = 0; i < parts.size(); i++)
		{
			if (!isStringInteger(parts[i]))
			{
				return false;
			}

			if (i == 1 && (parts[i].front() == '-' || parts[i].front() == '+'))
			{
				return false;
			}
		}

		return true;
	}

	static bool isStringInteger(const string& source)
	{
		if (source.empty())
		{
			return false;
		}

		bool hasDigits = false;

		for (size_t i = 0; i < source.size(); i++)
		{
			if (i == 0 && source[0] == '-')
			{
				continue;
			}

			if (!isdigit(source[i]))
				return false;
			else
				hasDigits = true;
		}

		if (!hasDigits)
			return false;

		return true;
	}

	static bool isStringValidString(const string& source)
	{
		if (source.size() < 2 || source.empty() || source[0] != '"' || source[source.size() - 1] != '"')
		{
			return false;
		}

		return true;
	}

	static vector<string> splitBy(string source, const string& delimeter)
	{
		vector<string> words;
		size_t pos = 0;

		if (source.find(delimeter) == string::npos && !source.empty())
		{
			words.push_back(source);
			return words;
		}

		while ((pos = source.find(delimeter)) != string::npos)
		{
			words.push_back(source.substr(0, pos));
			source.erase(0, pos + delimeter.length());

			if (pos = source.find(delimeter) == string::npos)
			{
				words.push_back(source);
				return words;
			}
		}

		return words;
	}

	static string& trim(string& source)
	{
		if (source.empty())
		{
			return source;
		}

		size_t start = 0;

		while (source[start] == ' ')
		{
			source.erase(source.begin());
		}

		size_t end = source.size() > 0 ? source.size() - 1 : 0;

		while (source[end] == ' ')
		{
			source.erase(source.begin() + end);
			end--;
		}

		return source;
	}

	static const string toUpper(const string& str)
	{
		string item;
		item = str;
		for (size_t i = 0; i < item.size(); i++)
		{
			if (item[i] >= 'a' && item[i] <= 'z')
			{
				item[i] -= 'a' - 'A';
			}
		}

		return item;
	}

	static void removeEmptyStringsInVector(vector<string>& parts)
	{
		for (size_t i = 0; i < parts.size(); i++)
		{
			if (parts[i].empty())
			{
				parts.erase(parts.begin() + i);
				i--;
			}
		}
	}

	static void removeQuotations(string& source)
	{
		if (!isStringValidString(source))
		{
			return;
		}

		source.erase(source.begin());
		source.erase(source.end() - 1);
	}

	static bool isCorrectColumnType(const string& colType, const string& actualInput)
	{
		if (colType == "Integer" && isStringInteger(actualInput))
			return true;
		else if (colType == "Double" && isStringDouble(actualInput))
			return true;
		else if (colType == "Double" && isStringInteger(actualInput))
			return true;
		else if (colType == "String" && isStringValidString(actualInput))
			return true;

		return false;
	}
};
