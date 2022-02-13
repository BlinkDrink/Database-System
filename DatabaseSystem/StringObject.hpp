#pragma once 
#include "Object.hpp"
#include"FileHelper.hpp"

using fh = FileHelper;

/**
 * @brief Descriptor of string object
*/
class StringObject : public Object
{
public:
	StringObject() {}

	StringObject(string value) : fValue(value) {}

	virtual Object* clone() const final override
	{
		return new StringObject(*this);
	}

	virtual size_t memsize() const final override
	{
		return fValue.size();
	}

	virtual string toString() const final override
	{
		return fValue;
	}

	virtual size_t size() const final override
	{
		return fValue.size();
	}

	virtual void write(ofstream& out) const final override
	{
		size_t size = 0;

		ObjectType s = ObjectType::STRING;
		out.write((char*)&s, sizeof(s));
		fh::writeString(out, fValue);
	}

private:
	string fValue;

	virtual bool isGreaterThan(const Object& other) const override
	{
		return fValue > static_cast<const StringObject&>(other).fValue;
	}

	virtual bool isEqualTo(const Object& other) const override
	{
		return fValue == static_cast<const StringObject&>(other).fValue;
	}

	virtual bool isLesserThan(const Object& other) const override
	{
		return fValue < static_cast<const StringObject&>(other).fValue;
	}
};