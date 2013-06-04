#pragma once

#include "TabAtr.h"

using namespace std;

class DbTable
{
private:
  string name;
	string pk;  //primary key(in case of compound key, I store all keys as a whole string)
	list<string> fks;  //foreign keys
	
	long int cardinality;
	int bfr;

public:

	list<TabAtr> TableAttributes;  //attributes list

	DbTable(string s);

	string GetName(void);
	string GetPk(void);
	long int GetCardi(void);
	int GetBfr(void);
	double GetSel(string s);
	Idx_Type GetIdx(string s);
	int GetIdxBfr(string s);
	int IsFk(string s);  // 1 mean fk, 0 mean not
	//int IsPk(string s);
	int GetLen(string s);

	void SetCardi(long int cardi);
	void SetBfr(int i);
	void SetIdxBfr(string s,int l);
	void SetName(string n);
	void SetPk(string s);
	void AddFks(string s);
	void AddAtr(string n, Atr_Type t,int l);
    void AddAtr(string n, Atr_Type t);
	int SetSel(string s,double d);
	int SetIdx(string s, Idx_Type it, int ibfr);
	
	void SetLen(string s,int len);

	~DbTable(void);
};

