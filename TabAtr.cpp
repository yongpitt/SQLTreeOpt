#include "StdAfx.h"
#include "TabAtr.h"

TabAtr::TabAtr(string n, Atr_Type t,int l) : selectivity(0), index(NONE_T), idx_bfr(0)
{
  name.assign(n);
	type = t;
	lengh = l;
}

TabAtr::TabAtr(string n, Atr_Type t) : selectivity(0), index(NONE_T), idx_bfr(0)
{
    name.assign(n);
	type = t;
	switch(t)
	{
		case DATE_T:
	      lengh = 8;
		  break;
		case INT_T:
		  lengh = 4;
		  break;
		default:
          lengh = 4;
	}
}

string TabAtr::GetName(void)
{
	return name;
}

Atr_Type TabAtr::GetType(void)
{
	return type;
}

int TabAtr::GetLen(void)
{
	return lengh;
}

void TabAtr::SetLen(int len)
{
	lengh = len;
}

void TabAtr::SetSel(double d)
{
    selectivity = d;
}

void TabAtr::SetIdx(Idx_Type i)
{
    index = i;
}

void TabAtr::SetIdxBfr(int b)
{
    idx_bfr = b;
}

double TabAtr::GetSel(void)
{
	return selectivity;
}

Idx_Type TabAtr::GetIdx(void)
{
	return index;
}

int TabAtr::GetIdxBfr(void)
{
	return idx_bfr;
}

TabAtr::~TabAtr(void)
{
}
