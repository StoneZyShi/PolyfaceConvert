#include "sqlliteRW.h"
#include "..\include\easySQLite\easySqlite.h"
#include "..\include\easySQLite\SqlCommon.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "getData.h"

using namespace std;


sqlliteRW::sqlliteRW(std::string fileName)
{
	obj.id = 0;
	this->fileName = fileName;
}


sqlliteRW::~sqlliteRW()
{
}

//static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
//	int i;
//	for (i = 0; i<argc; i++) {
//		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//	}
//	printf("\n");
//	return 0;
//}

bool sqlliteRW::creatObjTable()
{
	sqlite3 *db;
	char *zErrMsg = 0;

	int result = sqlite3_open(fileName.c_str(), &db);
	if (!result)
	{
		//pri(L"Open the database sqlite.db sucessfully\n");
	}

	/* Create SQL statement */
	char* sql = "CREATE TABLE obj_table("  \
		"id                    INT PRIMARY KEY  NOT NULL," \
		"hostfile_name         CHAR(255)," \
		"elem_guid_infile      CHAR(100)," \
		"id_infile             INT," \
		"obj_file_Data         BLOB," \
		"gudi                  INT );";
	result = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if (result != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		//pri(L"Table created successfully\n");
	}

	sqlite3_close(db);

	return false;
}

bool sqlliteRW::addData(std::string str)
{
	sqlite3 *db;
	sqlite3_stmt *stat;

	int result = sqlite3_open(fileName.c_str(), &db);
	if (!result)
	{
		//pri(L"addData");
		sqlite3_prepare(db, "insert into obj_table (id,hostfile_name,id_infile,obj_file_Data) values(?,?,?,?);", (int)-1, &stat, (int)0);
		//WString msg;
		//msg.Sprintf(L"ID=%d   obj.hostfile_name.c_str()=%s   obj.id_infile=%d\n", obj.id, obj.hostfile_name.c_str(), obj.id_infile);
		//mdlDialog_dmsgsPrint(msg.GetWCharCP());

		//pri(L"开始插入");
		sqlite3_bind_int(stat, 1, obj.id);
		sqlite3_bind_text(stat, 2, obj.hostfile_name.c_str(),(int)(obj.hostfile_name.length() + 1),NULL);
		sqlite3_bind_int(stat, 3, obj.id_infile);
		sqlite3_bind_blob(stat, 4, str.c_str(), (int)(str.length() + 1), NULL);
		
		sqlite3_step(stat);
		sqlite3_finalize(stat);
		sqlite3_close(db);

		//pri(L"插入完成");
		//msg.Sprintf(L"str= %s\n", str.c_str());
		//mdlDialog_dmsgsPrint(msg.GetWCharCP());
	}

	return true;
}

void sqlliteRW::set_id()
{
	this->obj.id++;
}

void sqlliteRW::set_hostfile_name(std::string hostfile_name)
{
	this->obj.hostfile_name = hostfile_name;
}

void sqlliteRW::set_elem_guid_infile(std::string elem_guid_infile)
{
	this->obj.elem_guid_infile = elem_guid_infile;
}

void sqlliteRW::set_id_infile(int id_infile)
{
	this->obj.id_infile = id_infile;
}

void sqlliteRW::set_guid(int guid)
{
	this->obj.guid = guid;
}


