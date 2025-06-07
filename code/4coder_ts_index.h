/* date = June 7th 2025 7:59 pm */

#ifndef FCODER_TS_INDEX_H
#define FCODER_TS_INDEX_H


typedef uint64_t TS_Index_Note_Kind;
enum
{
	Index_Note_None,
	Index_Note_Function,
	Index_Note_Product_Type,
	Index_Note_Sum_Type,
	Index_Note_Constant,
	
	Index_Note_COUNT,
};

#endif //FCODER_TS_INDEX_H
