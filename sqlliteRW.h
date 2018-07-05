#pragma once
#include <string>
#include "..\include\easySQLite\sqlite3.h"


class sqlliteRW
{
public:
	struct obj_struct
	{
		int id;
		std::string hostfile_name;
		std::string elem_guid_infile;
		int id_infile;
		int guid;
	};

public:
	sqlliteRW(std::string fileName);
	~sqlliteRW();
	bool creatObjTable();
	bool addData(std::string str);

	void set_id();
	void set_hostfile_name(std::string hostfile_name);
	void set_elem_guid_infile(std::string elem_guid_infile);
	void set_id_infile(int id_infile);
	void set_guid(int guid);
	
private:
	obj_struct obj;
	std::string fileName; 
	std::string tableName; 
};

