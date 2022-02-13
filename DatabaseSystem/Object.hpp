#pragma once
#include <string>
#include<fstream>
using std::ofstream;
using std::ifstream;
using std::to_string;

class Object
{
public:
	virtual Object* clone() const = 0;
	virtual size_t memsize() const = 0;
	virtual std::string toString() const = 0;
	virtual void write(ofstream& out) const = 0;
	virtual size_t size() const = 0;

	bool operator>(const Object& other) const
	{
		return typeid(*this) == typeid(other) && isGreaterThan(other);
	}

	bool operator==(const Object& other) const
	{
		return typeid(*this) == typeid(other) && isEqualTo(other);
	}

	bool operator<(const Object& other) const
	{
		return typeid(*this) == typeid(other) && isLesserThan(other);
	}

private:
	virtual bool isGreaterThan(const Object& other) const = 0;

	virtual bool isEqualTo(const Object& other) const = 0;

	virtual bool isLesserThan(const Object& other) const = 0;

};