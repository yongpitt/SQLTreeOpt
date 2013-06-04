#include "StdAfx.h"
#include "DbTable.h"

DbTable::DbTable(string s) : cardinality(0), bfr(0)
{
    name.assign(s);
}
string DbTable::GetName(void)
{
    return name;
}

string DbTable::GetPk(void)
{
    return pk;
}

long int DbTable::GetCardi(void)
{
  return cardinality;
}

int DbTable::GetBfr(void)
{
	return bfr;
}

double DbTable::GetSel(string s)
{
   list<TabAtr>::iterator str_iter;
   for(str_iter=TableAttributes.begin(); str_iter!=TableAttributes.end(); ++str_iter)
   {
       if (str_iter->GetName() == s)
		   return str_iter->GetSel();
   }
   return 0;

}

int DbTable::GetLen(string s)
{
   list<TabAtr>::iterator str_iter;
   for(str_iter=TableAttributes.begin(); str_iter!=TableAttributes.end(); ++str_iter)
   {
       if (str_iter->GetName() == s)
		   return str_iter->GetLen();
   }
   return 0;
}

Idx_Type DbTable::GetIdx(string s)
{
   list<TabAtr>::iterator str_iter;
   for(str_iter=TableAttributes.begin(); str_iter!=TableAttributes.end(); ++str_iter)
   {
       if (str_iter->GetName() == s)
		   return str_iter->GetIdx();
   }
   return NONE_T;
}

int DbTable::GetIdxBfr(string s)
{
   list<TabAtr>::iterator str_iter;
   for(str_iter=TableAttributes.begin(); str_iter!=TableAttributes.end(); ++str_iter)
   {
       if (str_iter->GetName() == s)
		   return str_iter->GetIdxBfr();
   }
   return 0;
}

void DbTable::SetName(string n)
{
   name.assign(n);
}

void DbTable::SetPk(string s)
{
   pk.assign(s);
}

void DbTable::AddFks(string s)
{
   fks.push_back(s);
}

void DbTable::AddAtr(string n, Atr_Type t,int l)
{
   TabAtr attr(n, t, l);
   TableAttributes.push_back(attr);
}

void DbTable::AddAtr(string n, Atr_Type t)
{
   TabAtr attr(n, t);
   TableAttributes.push_back(attr);
}

int DbTable::SetSel(string s,double d)
{
   list<TabAtr>::iterator str_iter;
   for(str_iter=TableAttributes.begin(); str_iter!=TableAttributes.end(); ++str_iter)
   {
       if (str_iter->GetName() == s)
	   {
		   str_iter->SetSel(d);
		   return 1;
	   }
   }
   return 0; // string does not match any attributes in the list
}

void DbTable::SetLen(string s,int len)
{
  list<TabAtr>::iterator str_iter;
   for(str_iter=TableAttributes.begin(); str_iter!=TableAttributes.end(); ++str_iter)
   {
       if (str_iter->GetName() == s)
	   {
		   str_iter->SetLen(len);
		   return;
	   }
   }
   return;
}

int DbTable::SetIdx(string s, Idx_Type it, int ibfr)
{
   list<TabAtr>::iterator str_iter;
   for(str_iter=TableAttributes.begin(); str_iter!=TableAttributes.end(); ++str_iter)
   {
       if (str_iter->GetName() == s)
	   {
		   str_iter->SetIdx(it);
		   str_iter->SetIdxBfr(ibfr);
		   return 1;
	   }
   }
   return 0; // string does not match any attributes in the list
}

int DbTable::IsFk(string s)
{
   list<string>::iterator str_iter;
   for(str_iter=fks.begin(); str_iter!=fks.end(); ++str_iter)
   {
       if (str_iter->compare(s) == 0)
	   {
		   return 1;
	   }
   }
   return 0; 
}

void DbTable::SetCardi(long int cardi)
{
   cardinality = cardi;
}

void DbTable::SetIdxBfr(string s,int l)
{
   list<TabAtr>::iterator str_iter;
   for(str_iter=TableAttributes.begin(); str_iter!=TableAttributes.end(); ++str_iter)
   {
       if (str_iter->GetName() == s)
		  str_iter->SetIdxBfr(l);
   }
}

void DbTable::SetBfr(int i)
{
   bfr = i;
}

DbTable::~DbTable(void)
{

}
