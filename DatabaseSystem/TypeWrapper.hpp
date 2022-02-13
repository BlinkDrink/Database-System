#pragma once
#include<string>
#include "IntegerObject.hpp"
#include "StringObject.hpp"
#include "DoubleObject.hpp"

using std::string;

class TypeWrapper
{
public:
	/// Object lifetime
	TypeWrapper() :fContent(nullptr) {}

	TypeWrapper(ifstream& in)
	{
		ObjectType t = ObjectType::INT;
		in.read((char*)&t, sizeof(t));

		if (t == ObjectType::INT) {
			int value = 0;
			in.read((char*)&value, sizeof(value));
			fContent = new IntegerObject(value);
		}
		else if (t == ObjectType::STRING) {
			string value;
			fh::readString(in, value);
			fContent = new StringObject(value);
		}
		else if (t == ObjectType::DOUBLE)
		{
			double value = 0;
			in.read((char*)&value, sizeof(value));
			fContent = new DoubleObject(value);
		}
		else
		{
			fContent = nullptr;
		}
	}

	TypeWrapper(const std::string& content) :fContent(new StringObject(content)) {}

	TypeWrapper(int content) :fContent(new IntegerObject(content)) {}

	TypeWrapper(double content) :fContent(new DoubleObject(content)) {}

	/**
	*	@brief Copy constructor
	*
	*	@param other - TypeWrapper object from which copying will be made
	*/
	TypeWrapper(const TypeWrapper& other)
	{
		copyFrom(other);
	}

	/**
	*	@brief Copy assignment operator
	*
	*	@param other - TypeWrapper object from which copying will be made
	*	@returns *this
	*/
	TypeWrapper& operator=(const TypeWrapper& other)
	{
		if (this != &other)
		{
			delete fContent;
			copyFrom(other);
		}
		return *this;
	}

	/**
	*	@brief Move constructor
	*
	*	@param other - Cell object from which moving will be made
	*/
	TypeWrapper(TypeWrapper&& other) noexcept : TypeWrapper()
	{
		moveFrom(other);
	}

	/**
	*	@brief Move assignment operator
	*
	*	@param other - Cell object from which moving will be made
	*/
	TypeWrapper& operator=(TypeWrapper&& other) noexcept
	{
		if (this != &other)
		{
			moveFrom(other);
		}

		return *this;
	}

	/**
	 * @brief Getter
	 *
	 * @return the content behind the pointer(IntegerType/DoubleType/FormulaType/StringType)
	*/
	Object* getContent() const
	{
		return fContent;
	}

	~TypeWrapper()
	{
		delete fContent;
	}

public:

	string toString() const { return fContent->toString(); }

	/**
	 * @brief Used for writing information of fContent to a file
	 * @param out - output stream
	*/
	void write(ofstream& out) const
	{
		fContent->write(out);
	}

	bool operator>(const TypeWrapper& other) const { return fContent->operator>(*other.fContent); }
	bool operator==(const TypeWrapper& other) const { return fContent->operator==(*other.fContent); }
	bool operator<(const TypeWrapper& other) const { return fContent->operator<(*other.fContent); }
	bool operator<=(const TypeWrapper& other) const { return (fContent->operator<(*other.fContent) || fContent->operator==(*other.fContent)); }
	bool operator>=(const TypeWrapper& other) const { return (fContent->operator<(*other.fContent) || fContent->operator==(*other.fContent)); }
	bool operator!=(const TypeWrapper& other) const { return (fContent->operator<(*other.fContent) || fContent->operator>(*other.fContent)); }


private:
	Object* fContent;

	void copyFrom(const TypeWrapper& other)
	{
		if (other.fContent != nullptr)
			fContent = other.fContent->clone();
		else
			fContent = nullptr;
	}

	void moveFrom(TypeWrapper& other) {
		std::swap(fContent, other.fContent);
	}
};