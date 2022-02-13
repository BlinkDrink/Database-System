#pragma once
enum class CommandType {
	CREATE_TABLE,
	DROP_TABLE,
	LIST_TABLES,
	TABLE_INFO,
	INSERT,
	REMOVE,
	SELECT,
	EXIT,
	NONE
};