#pragma once
#include "Object.hpp"
#include "ObjectType.h"

/**
 * @brief Descriptor of int object
*/
class IntegerObject : public Object
{
public:
	IntegerObject() :fValue(0) {}

	IntegerObject(int value) : fValue(value) {}

	virtual Object* clone() const final override
	{
		return new IntegerObject(*this);
	}

	virtual size_t memsize() const final override
	{
		return sizeof(fValue);
	}

	virtual std::string toString() const final override
	{
		return std::to_string(fValue);
	}

	virtual size_t size() const final override
	{
		return to_string(fValue).size();
	}

	virtual void write(ofstream& out) const final override
	{
		ObjectType i = ObjectType::INT;
		out.write((char*)&i, sizeof(i));
		out.write((char*)&fValue, sizeof(fValue));
	}

private:
	int fValue;

	virtual bool isGreaterThan(const Object& other) const final override
	{
		return fValue > static_cast<const IntegerObject&>(other).fValue;
	}

	virtual bool isEqualTo(const Object& other) const final override
	{
		return fValue == static_cast<const IntegerObject&>(other).fValue;
	}

	virtual bool isLesserThan(const Object& other) const final override
	{
		return fValue < static_cast<const IntegerObject&>(other).fValue;
	}
};