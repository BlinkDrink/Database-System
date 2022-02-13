#pragma once
#include<vector>
#include<queue>
#include<stack>
#include<unordered_map>
#include <cstdlib>
#include <stack>
#include <sstream>
#include <algorithm>
#include "Record.hpp"
#include "Operator.h"
#include "StringHelper.hpp"
#include "TypeWrapper.hpp"
#include "QueryType.h"

using std::stack;
using std::queue;
using std::stoi;
using std::to_string;
using std::unordered_map;
using std::vector;
using std::invalid_argument;
using std::stringstream;

using sh = StringHelper;

/**
 *	@brief Descriptor of internal WHERE/REMOVE query expression parser (i.e. ID > 5)
*/
class InternalQuery
{
public:
	InternalQuery() : op(Operator::NONE), isIndexedColumn(false) {}

	/**
	 * @brief Initializes an internal expression with lefthandside ,righthandsite and the operation between them
	 * @param lhs - lefthandside of the equation
	 * @param rhs - righthandside of the equation
	 * @param operation - operation between the operands
	*/
	InternalQuery(const string& lhs, const TypeWrapper& rhs, const string& operation, const string& indexedCol) : lhs(lhs), rhs(rhs)
	{
		if (operation == "=")
			op = Operator::EQUAL;
		else if (operation == ">")
			op = Operator::GREATER_THAN;
		else if (operation == "<")
			op = Operator::LESS_THAN;
		else if (operation == ">=")
			op = Operator::GREATER_THAN_OR_EQUAL;
		else if (operation == "<=")
			op = Operator::LESS_THAN_OR_EQUAL;
		else
			op = Operator::NOT_EQUAL;

		if (lhs == indexedCol)
			isIndexedColumn = true;
		else
			isIndexedColumn = false;
	}

	/**
	 * @brief By given column name indices and record, check whether the record satisfies the current condition
	 * @param colIndex - hashtable containing the name of the column and its corresponding index (from left to right)
	 * @param rec - record to be checked against the condtion
	 * @return True if the record satisfies the condtion, false otherwise
	*/
	bool checkRecordAgainstCondition(const unordered_map<string, size_t>& colIndex, const Record& rec) const
	{
		if (colIndex.find(lhs) == colIndex.end())
			throw invalid_argument("There is no column with name {" + lhs + "} in the table.");

		switch (op)
		{
		case Operator::GREATER_THAN:
			if (rec.get(colIndex.at(lhs)) > rhs)
				return true;
			else
				return false;
		case Operator::LESS_THAN:
			if (rec.get(colIndex.at(lhs)) < rhs)
				return true;
			else
				return false;
		case Operator::EQUAL:
			if (rec.get(colIndex.at(lhs)) == rhs)
				return true;
			else
				return false;
		case Operator::GREATER_THAN_OR_EQUAL:
			if (rec.get(colIndex.at(lhs)) > rhs || rec.get(colIndex.at(lhs)) == rhs)
				return true;
			else
				return false;
		case Operator::LESS_THAN_OR_EQUAL:
			if (rec.get(colIndex.at(lhs)) < rhs || rec.get(colIndex.at(lhs)) == rhs)
				return true;
			else
				return false;
		case Operator::NOT_EQUAL:
			if (rec.get(colIndex.at(lhs)) < rhs || rec.get(colIndex.at(lhs)) > rhs)
				return true;
			else
				return false;
		default:
			return false;
			break;
		}
	}

	bool isPrimaryKeyQuery() const { return isIndexedColumn; }

	string& getColumn() { return lhs; }

	TypeWrapper& getValue() { return rhs; }

	Operator getOperator() const { return op; }

private:
	string lhs;
	TypeWrapper rhs;
	Operator op;
	bool isIndexedColumn;
};

/**
 *	@brief Descriptor of WHERE/REMOVE composite expression parser (i.e. ID > 5 AND (Name > "George" OR Age = 5))
*/
class Query
{
public:
	/**
	 * @brief Constructor, by given expression in string format, parse it so that it can easily fit for shunting yard alg.
	 * Every condtion of format ({column name} {comparison operator} {value of column}) is replaced by an index begining from 0
	 * So an expression of format (ID > 5 AND (Name = "George" OR Age = 21) AND Grade > 4.0) is replaced with
	 *	==> ( 0 AND ( 1 OR 2 ) AND 3 )
	 * @param exp - expression in string format
	 * @param colNameType - hashtable where key is name of colum and value is the type of the given column
	*/
	Query(string exp, const unordered_map<string, string>& colNameType, const string& primaryKey)
	{
		size_t index = 2, pos;
		string result;
		while ((pos = exp.find(" ")) != exp.npos)
		{
			string subStr = exp.substr(0, pos);
			if (colNameType.find(subStr) != colNameType.end())
			{
				string col, op, val;
				col = subStr;// Column name
				exp.erase(0, pos + 1);

				pos = exp.find(" ");
				subStr = exp.substr(0, pos); // operator 
				op = subStr;
				exp.erase(0, pos + 1);

				pos = exp.find(" ");
				subStr = exp.substr(0, pos);
				if (subStr.find("\"") != exp.npos)
				{
					size_t numQuotes = 0;
					size_t i;
					for (i = 0; i < exp.size(); i++)
					{
						if (exp[i] == '"')
							numQuotes++;

						val += exp[i];
						if (numQuotes % 2 == 0)
							break;
					}
					pos = i;
				}
				else
				{
					subStr = exp.substr(0, pos); // Column value
					val = subStr;
				}
				result += " " + to_string(index);

				// Check if query contains primary key, if so then add it to the array of primary key queries
				InternalQuery query(col, decideType(val), op, primaryKey);
				if (col == primaryKey)
					fPrimaryKeyQueries.push_back(query);

				fNumberedQueries[to_string(index++)] = query;
			}
			else if (subStr == "AND" || subStr == "OR" || subStr == "NOT")
			{
				result += " " + subStr;
			}
			else if (subStr == "(" || subStr == ")")
				result += " " + subStr;

			pos != exp.npos ? exp.erase(0, pos + 1) : exp.erase(0, exp.size());
		}

		if (!exp.empty())
			result += " " + exp;

		fQuery = result;
		sh::trim(fQuery);

		fShuntingOutput = shunting_yard(fQuery);
	}

	/**
	 * @brief By given record and hashtable of column names and their corresponding indices,
	 * check whether the record satisfies the where condition
	 * @param r - record to be checked
	 * @param colIndex - hashtable of column names and their corresponding indices (first column - index 0, second col - index 1..)
	 * @return True if the record satisfies all of the conditions, false otherwise
	*/
	bool checkRecordAgainstQuery(const Record& r, const unordered_map<string, size_t>& colIndex) const
	{
		return postfix_equation(fShuntingOutput, r, colIndex);
	}

	/**
	 * @return The array of queries that contain primary key
	*/
	vector<InternalQuery>& getPrimaryKeyQueries() { return fPrimaryKeyQueries; }

	queue<string> getShuntingOutput() { return fShuntingOutput; }

	unordered_map<string, InternalQuery>& getNumberedQueries() { return fNumberedQueries; }

private:
	/**
	 * @brief Given a string decide what type the object will be
	 * @param val - stringified value
	 * @return TypeWrapper of type double, int or string
	*/
	TypeWrapper decideType(const string& val) const
	{
		string cpy(val);
		if (sh::isStringInteger(cpy))
		{
			return TypeWrapper(stoi(cpy));
		}
		else if (sh::isStringValidString(cpy))
		{
			return TypeWrapper(cpy);
		}
		else if (sh::isStringDouble(cpy))
		{
			return TypeWrapper(stod(cpy));
		}
	}

	/**
	 * @brief Check operator precedence, the higher the value, the more priority this operator has
	 * @param op - operator to be checked - {AND, OR, NOT}
	 * @return the precedence of the operator
	*/
	size_t precedence(const string& op) const
	{
		if (op == "OR")
			return 1;
		if (op == "AND")
			return 2;
		if (op == "NOT")
			return 3;

		return 0;
	}

	/**
	 * @brief By given string of conditions (i.e. 0 AND (1 OR 2) AND 3 ) transform it into posftix expression
	 * @param expression - the string to be transformed into postfix expression
	 * @return queue of expression members written in postfix order
	*/
	queue<string> shunting_yard(string expression/*, const Record& r, const unordered_map<string, int> colIndex*/) const
	{
		queue<string> output;
		stack<string> operators;

		while (!expression.empty())
		{
			size_t pos = expression.find(" ");

			string element = pos != expression.npos ? expression.substr(0, pos) : expression;

			if (element == " ")
			{
				continue;
			}
			else if (element == "(")
			{
				operators.push(element);
			}
			else if (sh::isStringInteger(element))
			{
				output.push(element);
			}
			else if (element == ")")
			{
				while (!operators.empty() && operators.top() != "(")
				{
					string op;
					op += operators.top();
					operators.pop();

					output.push(op);
				}

				if (!operators.empty())
					operators.pop();
			}
			else if (element == "AND" || element == "OR")
			{
				while (!operators.empty() && precedence(operators.top()) >= precedence(element))
				{
					string op;
					op += operators.top();
					operators.pop();

					output.push(op);
				}

				operators.push(element);
			}

			pos != expression.npos ? expression.erase(0, pos + 1) : expression.erase(0, expression.size());
		}

		while (!operators.empty())
		{
			string op;
			op += operators.top();
			operators.pop();

			output.push(op);
		}

		return output;
	}

	/**
	 * @brief Calculate the given postfix expression agaisnt a record, to see if the record satisfies all of the
	 * conditions of the expression.
	 * @param output - queue of expression members written in postfix order
	 * @param r - record to be checked against the expression
	 * @param colIndex - hashtable of column names and their appropriate index
	 * @return True if the record satisfies the set of conditions, false otherwise
	*/
	bool postfix_equation(queue<string> output, const Record& r, const unordered_map<string, size_t>& colIndex) const
	{
		stack<bool> result;
		vector<Record> res;

		while (!output.empty())
		{
			if (sh::isStringInteger(output.front()))
			{
				result.push(fNumberedQueries.at(output.front()).checkRecordAgainstCondition(colIndex, r));
				output.pop();
			}
			else
			{
				bool val2 = result.top();
				result.pop();

				bool val1 = result.top();
				result.pop();

				string op = output.front();
				output.pop();

				if (op == "AND")
					result.push(val1 && val2);
				else
					result.push(val1 || val2);
			}
		}

		return result.top();
	}

private:
	unordered_map<string, InternalQuery> fNumberedQueries;
	vector<InternalQuery> fPrimaryKeyQueries;
	queue<string> fShuntingOutput;
	string fQuery;
};
