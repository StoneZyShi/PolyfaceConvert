#include "sqlliteRW.h"
#include "..\include\easySQLite\easySqlite.h"
#include "..\include\easySQLite\SqlCommon.h"
#include <string>
#include <iostream>

using namespace std;

Field def_tb_obj[] =
{
	Field(FIELD_KEY),
	Field("id", type_int, flag_not_null),
	Field("hostfile_name", type_text, flag_not_null),
	Field("elem_guid_infile", type_text, flag_not_null),
	Field("id_infile", type_int, flag_not_null),
	Field("obj_file_Data", type_bool, flag_not_null),
	Field("guid", type_int, flag_not_null),
	Field(DEFINITION_END),
};




sqlliteRW::sqlliteRW()
{
}


sqlliteRW::~sqlliteRW()
{
}


void sqlliteRW::creatSql()
{
	sql::Database db;
	try
	{
		if (!db.open("d:/test.db"))
		{
			cout << (db.errMsg()) << endl;
		}
		else
		{
			sql::Table tbPerson(db.getHandle(), "person", def_tb_obj);
			//remove table from database if exists
			if (tbPerson.exists())
				tbPerson.remove();
			//create new table
			tbPerson.create();
			//removes all records
			tbPerson.truncate();
			//loads all records to internal recordset
			tbPerson.open();
			//loads one record
			tbPerson.open("_ID == 5");
			//returns loaded records count
			tbPerson.recordCount();

			Record record(tbPerson.fields());

			std::string test = "test";
			record.setInteger("id", 1);
			record.setString("hostfile_name", "Kowalski");
			record.setString("elem_guid_infile", test.c_str());
			record.setInteger("id_infile", 2);
			record.setBool("obj_file_Data", true);
			record.setInteger("guid", 3);

			tbPerson.addRecord(&record);
		}

	}
	catch (Exception e) {
		//...
	}

}
