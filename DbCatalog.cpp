#include "StdAfx.h"
#include "DbCatalog.h"


DbCatalog::DbCatalog(string dbSchema, string dbIndexing, string dbConfig):PageSize(4),BuffSize(1024)
{
   
  string delimiters = " \t\n();:>,";
	string token;
	string CurTabName;
	string attr;
	string lengh;
	string index;
	string IdxBfr;
	string pre_token;
	string PageSizeStr;  //is this needed to be stored in the class?
	string BuffSizeStr;  //is this needed to be stored in the class?
	Idx_Type IdxType;
	Atr_Type type;

	/* process the dbSchema string */

	// Skip delimiters at beginning.
	string::size_type lastPos = dbSchema.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	string::size_type pos = dbSchema.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		token = dbSchema.substr(lastPos, pos - lastPos);

		if (token.compare("table") == 0 || token.compare("TABLE") == 0 || token.compare("Table") == 0)
		{
			// read next token
			lastPos = dbSchema.find_first_not_of(delimiters, pos);
			pos = dbSchema.find_first_of(delimiters, lastPos);
			CurTabName = dbSchema.substr(lastPos, pos - lastPos);
			DbTable NewTable(CurTabName); //???
			TableList.push_back(NewTable);

		}

		else if(token.compare("constraints") == 0 || token.compare("CONSTRAINTS") == 0 || token.compare("Constraints") == 0)
		{
			;
		}

		else if(token.compare("PK")==0 || token.compare("pk")==0 )
		{
			lastPos = dbSchema.find_first_not_of(delimiters, pos);
			pos = dbSchema.find_first_of("\n", lastPos);
			token = dbSchema.substr(lastPos, pos - lastPos);
			SetPk(CurTabName,token);
		}

		else if(token.compare("FK")==0 || token.compare("fk")==0 )
		{
			lastPos = dbSchema.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = dbSchema.find_first_of(delimiters, lastPos);
			token = dbSchema.substr(lastPos, pos - lastPos);
			AddFks(CurTabName,token);

			lastPos = dbSchema.find_first_of("\n", pos);
			lastPos = dbSchema.find_first_not_of(delimiters, lastPos);  //read next line
			pos = dbSchema.find_first_of(delimiters, lastPos);
			continue;

		}
		else
		{
			attr = token;
			lastPos = dbSchema.find_first_not_of(" ,\n", pos);
			pos = dbSchema.find_first_of(" ,\n", lastPos);
			token = dbSchema.substr(lastPos, pos - lastPos);
			if(token.compare("char")==0 || token.compare("CHAR")==0)
			{
			    string::size_type lastPos_tmp,pos_tmp;
				type = CHAR_T;
				lastPos_tmp = dbSchema.find_first_not_of(" ,\n", pos);
			    pos_tmp = dbSchema.find_first_of(" ,\n", lastPos);

				lengh = dbSchema.substr(lastPos_tmp, pos_tmp - lastPos_tmp);

			}
			else if(token.compare("int")==0 || token.compare("INT")==0)
			{
				type = INT_T;

				lengh = "4";
			}
			else if(token.compare("date")==0 || token.compare("DATE")==0)
			{
				type = DATE_T;
				lengh = "8";
			}
			else 
			{
				type = UNKNOWN_T;

				lengh = "4";
			}

			// do not implement the lengh field, do we really need it?

			lastPos = dbSchema.find_first_of("\n", pos);
			lastPos = dbSchema.find_first_not_of(delimiters, lastPos);  //read next line
			pos = dbSchema.find_first_of(delimiters, lastPos);

			//AddAtr(attr, type, 4);  //fixed lengh, do we really need to read the lengh information?
			AddAtr(CurTabName, attr, type, 4);
			SetLen(CurTabName,attr,atoi(lengh.c_str()));
			continue;
		}

		// Skip delimiters.  Note the "not_of"
		lastPos = dbSchema.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = dbSchema.find_first_of(delimiters, lastPos);

	}



	/* process the dbIndexing string */
	lastPos = dbIndexing.find_first_not_of(delimiters, 0);
	pos = dbIndexing.find_first_of(delimiters, lastPos);
	while (string::npos != pos || string::npos != lastPos)
	{

		CurTabName = dbIndexing.substr(lastPos, pos - lastPos);

		lastPos = dbIndexing.find_first_not_of(delimiters, pos);
		pos = dbIndexing.find_first_of(delimiters, lastPos);
		attr = dbIndexing.substr(lastPos, pos - lastPos);

		lastPos = dbIndexing.find_first_not_of(delimiters, pos);
		pos = dbIndexing.find_first_of(delimiters, lastPos);
		index = dbIndexing.substr(lastPos, pos - lastPos);

		if(index.compare("B+-tree") == 0)
			IdxType = BTREE_T;
		else if(index.compare("Extensible") == 0)
			IdxType = EHASH_T;
		else if(index.compare("Linear") == 0)
			IdxType = LHASH_T;
		else 
			IdxType = NONE_T;

		lastPos = dbIndexing.find_first_not_of(delimiters, pos);
		pos = dbIndexing.find_first_of(delimiters, lastPos);
		IdxBfr = dbIndexing.substr(lastPos, pos - lastPos);

		if(IdxBfr.compare("Hashing") == 0)
		{
			lastPos = dbIndexing.find_first_not_of(delimiters, pos);
			pos = dbIndexing.find_first_of(delimiters, lastPos);
			IdxBfr = dbIndexing.substr(lastPos, pos - lastPos);
		}

		SetIdx(CurTabName, attr, IdxType, atoi(IdxBfr.c_str()));

		lastPos = dbIndexing.find_first_of("\n", lastPos);
		lastPos = dbIndexing.find_first_not_of(delimiters, lastPos);  //read next line
		pos = dbIndexing.find_first_of(delimiters, lastPos);

	}



	/* process the dbConfig string */
	lastPos = dbConfig.find_first_not_of(delimiters, 0);
	pos = dbConfig.find_first_of(delimiters, lastPos);
	while (string::npos != pos || string::npos != lastPos)
	{
		// Found a token, add it to the vector.

		pre_token = dbConfig.substr(lastPos, pos - lastPos);
		if(pre_token.compare(0,1,"#") == 0 || pre_token.compare("}") == 0)
		{
			lastPos = dbConfig.find_first_of("\n", pos);
			lastPos = dbConfig.find_first_not_of(delimiters, lastPos);  //read next line
			pos = dbConfig.find_first_of(delimiters, lastPos);
			continue;
		}

		lastPos = dbConfig.find_first_not_of(delimiters, pos);
		pos = dbConfig.find_first_of(delimiters, lastPos);
		token = dbConfig.substr(lastPos, pos - lastPos);
		lastPos = dbConfig.find_first_not_of(delimiters, pos);
		pos = dbConfig.find_first_of(delimiters, lastPos);

		if (pre_token.compare("Page") == 0 && token.compare("Size") == 0)
		{
			// read next token(page size)
			string::size_type tmp_pos;
			PageSizeStr = dbConfig.substr(lastPos, pos - lastPos);
			tmp_pos = dbConfig.find_first_of(" Kk", lastPos);
			PageSize = atoi((PageSizeStr.substr(0, tmp_pos - 0)).c_str());

			lastPos = dbConfig.find_first_not_of(delimiters, pos);
			pos = dbConfig.find_first_of(delimiters, lastPos);

			continue;

		}
		else if(pre_token.compare("Memory") == 0 && token.compare("Buffer") == 0)
		{
			string::size_type tmp_pos;
			BuffSizeStr = dbConfig.substr(lastPos, pos - lastPos);
			tmp_pos = dbConfig.find_first_of(" Mm", lastPos);
			BuffSize = 1024*atoi((PageSizeStr.substr(0, tmp_pos - 0)).c_str());


			lastPos = dbConfig.find_first_of("\n", pos);
			lastPos = dbConfig.find_first_not_of(delimiters, lastPos);  //read next line
			pos = dbConfig.find_first_of(delimiters, lastPos);
		}

		else if(token.compare("{")==0 )
		{
			CurTabName = pre_token;
		}

		else if(pre_token.compare("Cardinality") == 0)
		{
			SetCardi(CurTabName,atoi(token.c_str()));
		}

		else if(pre_token.compare("bfr") == 0)
		{
			SetBfr(CurTabName,atoi(token.c_str()));
		}
		else if(token.compare(0,1,"0") == 0)
		{
			SetSel(CurTabName, pre_token, atof(token.c_str()));
		}

	}
	return;

}

DbCatalog::DbCatalog(void)
{

}

int DbCatalog::AddTable(string s)
{
	DbTable NewTable(s);
	TableList.push_back(NewTable);
	return 0;
}

/*
DbTable* DbCatalog::SearchTable(string tab_name, )
{
list<DbTable>::iterator tab_iter;
for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
{
if (tab_iter->GetName() == tab_name)
return tab_iter;
}
return NULL;
}
*/


long int DbCatalog::GetCardi(string tab_name)
{
	list<DbTable>::iterator tab_iter;
	int cardi;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
			cardi = tab_iter->GetCardi();
	}
	if(cardi > 0)
		return cardi;
	else 
	{
		if(tab_name == "PHARMACEUTICAL-CO{")
			return 50;
		else
			return 10000; //return default
	}
}

int DbCatalog::GetBfr(string tab_name)
{
	list<DbTable>::iterator tab_iter;
	int bfr;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
			bfr = tab_iter->GetBfr();
	}
	if(!bfr>0)
	   bfr = (int) (1024*GetPageSize())/GetAllLen(tab_name);
	return bfr;
}

string DbCatalog::GetPk(string tab_name)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
			return tab_iter->GetPk();
	}
	return " ";
}

int DbCatalog::IsPk(string tab_name, string attr_name)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
			return (attr_name == tab_iter->GetPk());
	}
	return -1;
}

double DbCatalog::GetSel(string tab_name, string attr_name)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
			return tab_iter->GetSel(attr_name);
	}
	return -1;
}

int DbCatalog::GetLen(string tab_name, string attr_name)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
			return tab_iter->GetLen(attr_name);
	}
	return -1;
}

int DbCatalog::GetAllLen(string tab_name)
{
	list<DbTable>::iterator tab_iter;
	list<TabAtr>::iterator attr_iter;
	int allLen = 0;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
		{
			for(attr_iter=tab_iter->TableAttributes.begin(); attr_iter!=tab_iter->TableAttributes.end(); ++attr_iter)
			{
				
				allLen += attr_iter->GetLen();
			
			}
			break;
		}

	}
	return allLen;
}

Idx_Type DbCatalog::GetIdx(string tab_name, string attr_name)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
			return tab_iter->GetIdx(attr_name);
	}
	return NONE_T;
}

int DbCatalog::GetIdxBfr(string tab_name, string attr_name)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
			return tab_iter->GetIdxBfr(attr_name);
	}
	return -1;
}


void DbCatalog::SetCardi(string tab_name, long int cardi)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
		{
			tab_iter->SetCardi(cardi);
			return;
		}
	}
	return;
}

void DbCatalog::SetBfr(string tab_name, int bfr)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
		{
			tab_iter->SetBfr(bfr);
			return;
		}
	}
	return;
}

void DbCatalog::SetPk(string tab_name, string pk)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
		{
			tab_iter->SetPk(pk);
			return;
		}
	}
	return;
}

void DbCatalog::SetIdxBfr(string tab_name, string attr_name, int ibfr)
{
	list<DbTable>::iterator tab_iter;
	list<TabAtr>::iterator attr_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
		{
			for(attr_iter=tab_iter->TableAttributes.begin(); attr_iter!=tab_iter->TableAttributes.end(); ++attr_iter)
			{
				if (attr_iter->GetName() == attr_name)
				{
					attr_iter->SetIdxBfr(ibfr);
					return;
				}

			}
			break;
		}

	}
	return;
}

void DbCatalog::AddFks(string tab_name, string fk)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
		{
			tab_iter->AddFks(fk);
			return;
		}
	}
	return;
}

void DbCatalog::AddAtr(string tab_name, string n, Atr_Type t,int l)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
		{
			tab_iter->AddAtr(n,t,l);
			return;
		}
	}
	return;
}

void DbCatalog::AddAtr(string tab_name, string n, Atr_Type t)
{
	list<DbTable>::iterator tab_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
		{
			tab_iter->AddAtr(n,t);
			return;
		}
	}
	return;
}

void DbCatalog::SetSel(string tab_name, string attr_name,double d)
{
	list<DbTable>::iterator tab_iter;
	list<TabAtr>::iterator attr_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
		{
			for(attr_iter=tab_iter->TableAttributes.begin(); attr_iter!=tab_iter->TableAttributes.end(); ++attr_iter)
			{
				if (attr_iter->GetName() == attr_name)
				{
					attr_iter->SetSel(d);
					return;
				}
			}
			break;
		}

	}
	return;
}

void DbCatalog::SetLen(string tab_name, string attr_name,int len)
{
    list<DbTable>::iterator tab_iter;
	list<TabAtr>::iterator attr_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
		{
			for(attr_iter=tab_iter->TableAttributes.begin(); attr_iter!=tab_iter->TableAttributes.end(); ++attr_iter)
			{
				if (attr_iter->GetName() == attr_name)
				{
					attr_iter->SetLen(len);
					return;
				}
			}
			break;
		}

	}
	return;
}

void DbCatalog::SetIdx(string tab_name, string attr_name, Idx_Type it, int ibfr)
{
	list<DbTable>::iterator tab_iter;
	list<TabAtr>::iterator attr_iter;
	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
		if (tab_iter->GetName() == tab_name)
		{
			for(attr_iter=tab_iter->TableAttributes.begin(); attr_iter!=tab_iter->TableAttributes.end(); ++attr_iter)
			{
				if (attr_iter->GetName() == attr_name)
			 {
				 attr_iter->SetIdx(it);
				 attr_iter->SetIdxBfr(ibfr);
				 return;
			 }
			}
			break;
		}
	}
	return;
}


DbCatalog::~DbCatalog(void)
{
	return;
}

std::list<std::string> DbCatalog:: GetTables(string attr_name)
{
  	list<DbTable>::iterator tab_iter;
	list<TabAtr>::iterator attr_iter;
	list<std::string> tab_list;

	for(tab_iter=TableList.begin(); tab_iter!=TableList.end(); ++tab_iter)
	{
			for(attr_iter=tab_iter->TableAttributes.begin(); attr_iter!=tab_iter->TableAttributes.end(); ++attr_iter)
			{
				if (attr_iter->GetName() == attr_name)
			 {
				 tab_list.push_back(tab_iter->GetName());
				 break;
			 }
			}	
		
	}
	return tab_list;
}

int DbCatalog::GetPageSize()
{
    return PageSize;
}

int DbCatalog::GetBuffSize()
{
   return BuffSize;
}
