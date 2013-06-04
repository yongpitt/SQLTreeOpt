#pragma once
/*
 Table attributes class
*/

using namespace std;
class TabAtr
{


private:
  string name;
	double selectivity; //0 means selectivity not specified

	Idx_Type index;// do we allow multiple indice on a same attribute? currently not
	Atr_Type type;
	
	int idx_bfr;
	int lengh;
public:
	
	TabAtr(string n, Atr_Type t,int l);
	TabAtr(string n, Atr_Type t);
	
	string GetName(void);
	Atr_Type GetType(void);
	int GetLen(void);
	
	void SetSel(double d);
	void SetLen(int len);
	void TabAtr::SetIdx(Idx_Type i);
	void TabAtr::SetIdxBfr(int b);

	double GetSel(void);
	Idx_Type TabAtr::GetIdx(void);
	int TabAtr::GetIdxBfr(void);
	
	~TabAtr(void);
};

