#include <stdio.h>

#define total_second		60
#define total_minune		60 
#define total_hour			24
#define degree_per_hour		30
#define degree_per_minu		6
#define degree_per_seco		6

static int get_hour_d(uint16_t hour, uint16_t minune)
{

	int hour_degree;

	hour_degree = hour%12 *degree_per_hour + (minune/12)*degree_per_minu ;
	
	return hour_degree;
}
static int get_minu_d(uint16_t minune)
{
	return minune*degree_per_minu;
}
static int get_sec_d(uint16_t second)
{
	return second*degree_per_seco;
}
/**
 * main - 打印出一天中时针，分针，秒针角度两两大于某值次数百分
 * 
 * 返回值:void
 */
void main(void)
{
	int i;
	int j;
	int k;
	int test_d;
	int hour_d,minu_d,sec_d;
	uint16_t second;
	uint16_t happy_cnt;
	for(i=0; i<total_hour; i++)
	{
		for(j=0; j<total_minune; j++)
		{
			for(k=0; k<total_second; k++)
			{
				hour_d = get_hour_d(i,j);
				minu_d = get_minu_d(j);
				sec_d = get_sec_d(k);
				if(abs(hour_d - minu_d) >= test_d && abs(hour_d - sec_d) >= test_d)
				{
					if(abs(sec_d - hour_d) >= test_d && abs(sec_d - minu_d) >= test_d)
					{
						if(abs(minu_d - hour_d) >= test_d && abs(minu_d - sec_d) >= test_d)
						{
							happy_cnt++;
							printf("hour = %d,minune = %d,second = %d\r\n",i,j,k);	
						}
					}
				}
			}
		}
	}
	printf("happy percent count = %d\r\n",happy_cnt);
	printf("happy percent = %d\r\n",(happy_cnt/total_second)*100);	
}
