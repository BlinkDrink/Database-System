#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "CommandType.h"
#include "StringHelper.hpp"

using std::cin;
using std::getline;
using std::invalid_argument;
using std::out_of_range;
using sh = StringHelper;
using std::string;
using std::vector;

class CommandParser
{
private:
	bool fIsDistinct = false;
	string fOrderBy;
	string fRaw;
	vector<string> fTokens;

public:

	///@brief Sets the inner raw string of data and splits it into tokens
	///
	///@param str - string to be set and splited
	void setData(const string& data)
	{
		clearCmd();
		fIsDistinct = false;

		if (getNumberOfSymbol(fRaw, '\"') % 2 != 0)
			throw invalid_argument("Invalid command, check the number of quotes");

		if (getNumberOfSymbol(fRaw, '{') != getNumberOfSymbol(fRaw, '}'))
			throw invalid_argument("Invalid command, check the number of curly braces");

		if (getNumberOfSymbol(fRaw, '(') != getNumberOfSymbol(fRaw, ')'))
			throw invalid_argument("Invalid command, check the number of braces");

		fRaw = data;

		tokenizeInnerString();

		if (std::find(fTokens.begin(), fTokens.end(), "WHERE") != fTokens.end())
		{
			for (size_t i = 0; i < fTokens.size(); i++)
			{
				size_t currInd = i;
				if (fTokens[i] == "WHERE")
				{
					i++;
					while (i < fTokens.size() && (fTokens[i] != "ORDER" && fTokens[i] != "BY" && fTokens[i] != "DISTINCT"))
					{
						fTokens[currInd] += " " + fTokens[i];
						i++;
					}
				}
			}
		}

		if (std::find(fTokens.begin(), fTokens.end(), "ORDER") != fTokens.end())
		{
			for (size_t i = 0; i < fTokens.size() - 1; i++)
			{
				if (i < fTokens.size() - 1 && fTokens[i] == "ORDER")
				{
					if (fTokens[i + 1] == "BY")
					{
						fOrderBy = fTokens[i + 2];
					}
				}
			}
		}

		if (std::find(fTokens.begin(), fTokens.end(), "DISTINCT") != fTokens.end())
			fIsDistinct = true;


		if (fRaw.size() == 0 || fTokens.size() == 0)
			throw invalid_argument("Invalid command, check the number of arguments you've given");
	}

	/// @brief Splits raw into parts(tokens) and pushes them inside tokens private member.
	void tokenizeInnerString()
	{
		size_t tokensWordInd = 0;
		size_t numberOfQuotes = getNumberOfSymbol(fRaw, '\"');
		size_t currentQuotes = 0;
		size_t i = 0;

		while (fRaw[i])
		{
			if (i >= fRaw.size())
				break;

			if (fRaw[i] == '"')
			{
				currentQuotes++;
				tokensWordInd = i;
				i++;
				while (currentQuotes < numberOfQuotes)
				{
					if (fRaw[i] == '"')
						currentQuotes++;

					if (currentQuotes == numberOfQuotes)
						break;

					i++;
				}
				fTokens.push_back(fRaw.substr(tokensWordInd, i - tokensWordInd + 1));
				i++;
			}
			else if (fRaw[i] == ' ')
			{
				while (fRaw[i] == ' ')
					i++;

			}
			else if (fRaw[i] == '{')
			{
				tokensWordInd = i;
				while (fRaw[i] != '}')
					i++;

				i++;
				fTokens.push_back(fRaw.substr(tokensWordInd, i - tokensWordInd));
			}
			else if (fRaw[i] == '(')
			{
				tokensWordInd = i;
				while (fRaw[i] != ')')
					i++;

				i++;
				fTokens.push_back(fRaw.substr(tokensWordInd, i - tokensWordInd));
			}
			else
			{
				tokensWordInd = i;
				while (isalpha(fRaw[i]) || isdigit(fRaw[i]) || fRaw[i] == '*' || fRaw[i] == '>' || fRaw[i] == '<' || fRaw[i] == '='
					|| fRaw[i] == '!' || fRaw[i] == ',' || fRaw[i] == '.')
					i++;

				fTokens.push_back(fRaw.substr(tokensWordInd, i - tokensWordInd));
			}
		}
	}

	/**
	 * @brief Get number of times a symbol repeats itself in a string
	*/
	size_t getNumberOfSymbol(const string& str, char lookFor)
	{
		size_t symbol = 0;
		for (size_t i = 0; i < str.size(); i++)
		{
			if (str[i] == lookFor)
			{
				symbol++;
			}
		}
		return symbol;
	}

	///	@brief Getter. Throws out_of_range if pos is invalid
	///
	///	@param pos - index to be accessed
	///	@return the element at index pos
	const string& atToken(size_t pos) const
	{
		if (pos >= fTokens.size())
			throw out_of_range("There are not enough arguments.");

		return fTokens[pos];
	}

	/// @brief Getter
	/// 
	/// @returns size of tokens
	size_t size() const
	{
		return fTokens.size();
	}

	string getOrderBy() const { return fOrderBy; }

	bool isDistinct() const { return fIsDistinct; }

	/// @brief Getter
	/// @return raw string
	string& getRaw()
	{
		return fRaw;
	}

	/// @return the type of the input command
	CommandType getCommandType() const
	{
		string cmd = sh::toUpper(fTokens[0]);

		if (cmd == "CREATETABLE")
		{
			return CommandType::CREATE_TABLE;
		}
		else if (cmd == "DROPTABLE")
		{
			return CommandType::DROP_TABLE;
		}
		else if (cmd == "LISTTABLES")
		{
			return CommandType::LIST_TABLES;
		}
		else if (cmd == "TABLEINFO")
		{
			return CommandType::TABLE_INFO;
		}
		else if (cmd == "INSERT")
		{
			return CommandType::INSERT;
		}
		else if (cmd == "REMOVE")
		{
			return CommandType::REMOVE;
		}
		else if (cmd == "SELECT")
		{
			return CommandType::SELECT;
		}
		else if (cmd == "EXIT")
			return CommandType::EXIT;

		return CommandType::NONE;
	}

	/// @brief Getter
	/// @return vector of commands
	const vector<string>& getCommands() const { return fTokens; }

	///	@brief Clears raw, leaves it empty. Clears tokens, leaves it empty
	void clearCmd()
	{
		fRaw.clear();
		fTokens.clear();
	}
};