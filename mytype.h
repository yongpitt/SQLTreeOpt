typedef enum 
{
  CHAR_T = 0,
	INT_T = 1,
	NUMBER_T = 2,
	DATE_T = 3,
	UNKNOWN_T = 4
} Atr_Type;

typedef enum 
{
	NONE_T = 0, 
	BTREE_T = 1,
	EHASH_T = 2,
	LHASH_T = 3
} Idx_Type;

typedef enum 
{
	SCAN_T = 0, 
	ISCAN_T = 1,
	HSCAN_T = 2,
	SELECT_T = 3,
	PROJECT_T = 4,
	JOIN_T = 5,
	CPRODUCT_T = 6,
	UNION_T = 7
} Op_Type;
