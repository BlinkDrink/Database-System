#pragma once
#include<climits>
#include "Object.hpp"
#include "ObjectType.h"
using std::fabs;

/**
 * @brief Descriptor of double object
*/
class DoubleObject : public Object
{
public:
	DoubleObject() :fValue(0) {}

	DoubleObject(double value) : fValue(value) {}

	virtual Object* clone() const final override
	{
		return new DoubleObject(*this);
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
		ObjectType i = ObjectType::DOUBLE;
		out.write((char*)&i, sizeof(i));
		out.write((char*)&fValue, sizeof(fValue));
	}

private:
	double fValue;

	virtual bool isGreaterThan(const Object& other) const final override
	{
		double otherfValue = static_cast<const DoubleObject&>(other).fValue;
		return (fValue - otherfValue) > ((fabs(fValue) < abs(otherfValue) ? abs(otherfValue) : abs(fValue)) * std::numeric_limits<double>::epsilon());
	}

	virtual bool isEqualTo(const Object& other) const final override
	{
		return fabs(fValue - static_cast<const DoubleObject&>(other).fValue) < std::numeric_limits<double>::epsilon();
	}

	virtual bool isLesserThan(const Object& other) const final override
	{
		double otherfValue = static_cast<const DoubleObject&>(other).fValue;
		return (otherfValue - fValue) > ((fabs(fValue) < abs(otherfValue) ? abs(otherfValue) : abs(fValue)) * std::numeric_limits<double>::epsilon());
	}
};