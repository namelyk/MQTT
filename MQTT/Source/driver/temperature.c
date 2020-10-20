#include "driver.h"
#include "temperature.h"



#ifdef TEMPERATURE_ENABLE


// 通道数
#define TEMPERATURE_MAX_SIZE				(2)


// 采样数
#define SIMPLING_MAX_SIZE						20



// temperature type
#define TEMPERATURE_TYPE_NULL	0
#define TEMPERATURE_TYPE_10K	1	
#define TEMPERATURE_TYPE_12K	2
#define TEMPERATURE_TYPE_100K	3


// temperature 100K
//#define TEMPERATURE_10K_ENABLE
//#define TEMPERATURE_12K_ENABLE
#define TEMPERATURE_100K_ENABLE


//temperature
typedef struct temperature_t
{
  // 
  unsigned char flag;
  unsigned char subnet;
  unsigned char device;
  unsigned char equalize;
  
	// type
	unsigned char type;
	
	// define 
	int min_value;
	int max_value;
	
  // value
  float value;
  
  //simpling event
  unsigned char count;
  unsigned int sumSimpling;
  unsigned short maxSimpling;
  unsigned short maxSimpling_backup;
  unsigned short minSimpling;
  unsigned short minSimpling_backup;
  unsigned short averageValue;
	unsigned short simplingValue;
  
  
}temperature_t;



typedef union
{
  float fv;
  int iv;
  unsigned char cv[4];
}temperatureValueType_t;




static struct temperature_t temperature[TEMPERATURE_MAX_SIZE];


//=======================User define API===============================

/***************************************************************************/




#ifdef TEMPERATURE_10K_ENABLE

//,10K  -30 100
const unsigned short TEMPERATURE_10K_TABLE[]  =
{ 
    13053,//(-30)
    12466,11879,11299,10732,10180,9648,9137,8650,8187,7748,//(-29)~(-20)
    7354,6979,6622,6283,5960,5654,5362,5085,4822,4573, //(-19)-(-10)
    4355,4152,3961,3782,3613,3456,3307,3167,3035,2910, //(-9)-0
    2779,2654,2536,2423,2316,2215,2119,2027,1940,1857, //1-10 
    1779,1704,1632,1565,1500,1439,1380,1324,1271,1220, //11-20
    1172,1126,1082,1040,1000, 961, 925, 890, 857, 825, //21-30
     795, 765, 737, 710, 687, 661, 637, 615, 593, 572, //31-40
     553, 534, 515, 498, 481, 465, 449, 435, 420, 406, //41-50
     393, 381, 368, 357, 345, 334, 324, 314, 304, 295, //51-60
     286, 277, 268, 260, 252, 245, 238, 231, 224, 217, //61-70  
     211, 205, 199, 193, 188, 182, 177, 172, 167, 163, //71-80
     158, 154, 149, 145, 141, 137, 134, 130, 127, 123, //81-90
	 120, 117, 114, 111, 108, 105, 102, 99,  97,  94,   //91-100 
	0
};

#endif


#ifdef TEMPERATURE_12K_ENABLE

//12K   -30 100
const unsigned short TEMPERATURE_12K_TABLE[] =
{  
   32700, //-30
   31000,28500,27000,25600,24300,23100,22000,21000,12000,11225,//(-29-20)
   10589,9994,9437,8915,8425,7966,7535,7130,6750,6393, //(-19)-(-10)
   6057 ,5741,5444,5165,4901,4653,4419,4199,3991,3794, //(-9)-0
   3609 ,3434,3268,3112,2965,2827,2694,2569,2450,2336, //1-10 
   2230 ,2129,2034,1943,1857,1775,1697,1624,1554,1487, //11-20
   1424 ,1364,1306,1252,1200,1151,1104,1059,1016, 975, //21-30
     936, 899, 864, 830, 798, 767, 738, 710, 683, 657, //31-40
     632, 609, 586, 565, 544, 525, 506, 488, 470, 454, //41-50
     438, 422, 408, 394, 380, 367, 355, 343, 331, 320, //51-60
     310, 300, 290, 280, 271, 262, 254, 246, 238, 237, //61-70    
	 230, 223, 216, 210, 203, 197, 191, 185, 179, 174, //71-80
	 169, 164, 159, 155, 146, 142, 138, 134, 130, 126, //81-90
	 123, 119, 116, 129, 110, 107, 104, 101, 984, 958,  //91-100
		0
};

#endif

//100K   0 100
#ifdef TEMPERATURE_100K_ENABLE
const unsigned short TEMPERATURE_100K_TABLE[] =
{
  32710,31090,29560,28110,26740,25450,24220,23060,21960,20920,//0-9
	19940,19010,18120,17280,16490,15730,15020,14340,13690,13080,
	12500,11950,11420,10930,10450,10000,9571,9162,8774,8403,
	8051,7715,7395,7090,6799,6522,6257,6005,5764,5534,
	5314,5105,4904,4713,4530,4355,4188,4028,3875,3728,
	3588,3456,3329,3208,3091,2979,2871,2768,2669,2573,
	2482,2394,2309,2228,2105,2075,2003,1933,1867,1802,
	1741,1681,1624,1569,1516,1465,1416,1369,1324,1280,
	1238,1198,1159,1121,1085,1050,1016,984,953,923,
	894,866,840,814,789,764,741,718,697,676,
	655,
		0
};

#endif
	

//

#define Rref        (1000) //10K
#define Vref        (1024) //
/***************************************************************************
    R(temp)           V(temp)
   --------   =  ------------------   
    R(ref)        V(ref) - V(temp)

***************************************************************************/
static unsigned short AD_Change_temperature_Value(unsigned int AD_Value)
{

  unsigned short temp;
  unsigned int AD_Count1;
  unsigned int AD_Count2;
  
  AD_Count1 = AD_Value * Rref;
  AD_Count2 = Vref - AD_Value;

  temp = (unsigned short)(AD_Count1 / AD_Count2);
	
  return temp;
}




void set_temperature_type(struct temperature_t *object,unsigned char type,int min_value,int max_value)
{
	object->type = type;
	object->min_value = min_value;
	object->max_value = max_value;
	
}


static float get_define_temperature(struct temperature_t *object,unsigned short compare_value,const unsigned short *tempTable)
{
  float T1 = 0,T2 = 0;
  int i;
	
	for (i = 0; tempTable[i+1] > 0; i++)
	{
		if (compare_value >= tempTable[0])
		{
			T1 = object->min_value;
			return T1;
		}
		else if (compare_value > tempTable[i+1] 
		&& compare_value <= tempTable[i])
		{
			T1 = tempTable[i] - compare_value;
			T2 = tempTable[i] - tempTable[i+1];
			T1 = T1/T2;
			
			T1 += i + object->min_value + object->equalize - 10;
			return T1;
		}
	}

	T1 = object->max_value;
	return T1;  
}


static float get_temperature(struct temperature_t *object)
{
	unsigned short compare_value;

	compare_value = AD_Change_temperature_Value(object->averageValue);
	switch (object->type)
	{
	#ifdef 	TEMPERATURE_USR_ENABLE
		case TEMPERATURE_TYPE_USR:
			
			object->value = get_user_temperature(object,(float)compare_value);	
			break;
	#endif
		
	#ifdef 	TEMPERATURE_10K_ENABLE		
		case TEMPERATURE_TYPE_10K:
			object->value = get_define_temperature(object,compare_value,TEMPERATURE_10K_TABLE);
			break;
	#endif
	
	#ifdef 	TEMPERATURE_12K_ENABLE
		case TEMPERATURE_TYPE_12K:

			object->value = get_define_temperature(object,compare_value,TEMPERATURE_12K_TABLE);
			break;
	#endif
		
	#ifdef 	TEMPERATURE_100K_ENABLE
		case TEMPERATURE_TYPE_100K:
			
			object->value = get_define_temperature(object,compare_value,TEMPERATURE_100K_TABLE);
			break;
	#endif
			
	}
	return (object->value);
}


static int scan_temperature(unsigned char channel,unsigned short value)
{
	unsigned short simplingValue;
	struct temperature_t *object;
	
	simplingValue = value;
	object = &temperature[channel]; 
	if (object->minSimpling > simplingValue || object->minSimpling == 0)
	{
		object->minSimpling_backup = object->minSimpling;
		object->minSimpling = simplingValue;
	}
	
	if (object->minSimpling_backup == 0)
	{
		object->minSimpling_backup = object->minSimpling;
	}
	if (object->maxSimpling < simplingValue || object->maxSimpling == 0)
	{
		object->maxSimpling_backup = object->maxSimpling;
		object->maxSimpling = simplingValue;
	}
        
	if (object->maxSimpling_backup == 0)
		object->maxSimpling_backup = object->maxSimpling;
        
	object->sumSimpling += simplingValue;
	
	object->count++;
	if (object->count < SIMPLING_MAX_SIZE) return 1;
	object->count = 0;
        
	object->averageValue = (object->sumSimpling - object->maxSimpling - object->minSimpling \
                   - object->maxSimpling_backup - object->minSimpling_backup) / (SIMPLING_MAX_SIZE - 4);
 
	object->sumSimpling = 0;
	object->minSimpling = 0;
	object->maxSimpling = 0;
	object->minSimpling_backup = 0;
	object->maxSimpling_backup = 0;
        
	get_temperature(object);
        
	return 0;
}




//=======================Standard define API===============================
static int _get(unsigned char *config, int len)
{
	return 0;
}

static int _set(unsigned char *config, int len)
{
	return 0;
}



// ========================================================================
static void _init(void)
{
	int i;
	struct temperature_t *object;

	
}

static int _status(unsigned char *buffer,int len)
{
	int index;
	
	return 0;
}

static int _read(unsigned char *buffer,int len)
{
	struct temperature_t *object;
	int index;
	
	if (index >= TEMPERATURE_MAX_SIZE) return 1;
	
	object = &temperature[index];
	
	buffer[0] = (unsigned char)(object->value);
	//buffer[1] = (unsigned char)(object->value >> 8);
	return 0;
}

static int _write(unsigned char *buffer,int len)
{
	struct temperature_t *object;
	int index;
	
	if (index >= TEMPERATURE_MAX_SIZE) return 1;
	
	object = &temperature[index];
	
	return 0;
}


static int _handler(void *event)
{
	int i;
	struct temperature_t *object;
	
	for (i = 0; i < TEMPERATURE_MAX_SIZE; i++)
	{
		object = &temperature[i];
		scan_temperature(i,object->simplingValue);
		
	}
	
	return 0;
}




//====================low interface=====================
const struct driver_interface_t temperature_interface =
{
	_get,_set,_init,_status,_read,_write,_handler
};
#endif
