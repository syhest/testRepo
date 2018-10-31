#include "type.h"

void main()
{	
	printf("hello world！！！！！");
	fun_1();
	fun_2();
	
}#define total_second 24*60*60

int get_hour_d(uint16_t second)
{
	return second/60
}
int get_minu_d(uint16_t second)
{
	
}
int get_sec_d(uint16_t second)
{
	
}
void main()
{
	int i;
	int test_d；
	int hour_d,minu_d,sec_d;
	uint16_t second;
	uint16_t happy_cnt;
	for(i=0; i<total_second; i++)
	{
		second++;
		hour_d = get_hour_d(second);//时针角度
		minu_d = get_minu_d(second);//分针角度
		sec_d  = get_sec_d(second);//秒针角度
		
		if(abs(hour_d - minu_d) >= test_d && abs(hour_d - sec_d) >= test_d)
		{
			if(abs(sec_d - hour_d) >= test_d && abs(sec_d - minu_d) >= test_d)
			{
				if(abs(minu_d - hour_d) >= test_d && abs(minu_d - sec_d) >= test_d)
				{
					happy_cnt++
				}
			}
		
		}
		printf("happy percent = ")	
	}
}