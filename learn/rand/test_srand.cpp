#include<stdlib.h>/*用到了srand函数，所以要有这个头文件*/
#include<stdio.h>
#include<time.h>
#include<string>
#define MAX 10
 
 int main(void)
 {
//	 int number[MAX] = {0};
//	 int i;
//	 unsigned int seed = 10;
//	 srand(time(NULL));
//	 for(i = 0; i < MAX; i++)
//	 {
//		 number[i] = rand() % 100;/*产生100以内的随机整数*/
//		 printf("%d : %d\n",i,number[i]);
//	 }
//	 printf("\n");
     std::string str1 = "36.25";
     double d1 = atof(str1.c_str());
     printf("d1[%lf]\n",d1);
     d1 = int(d1*100)/100.0;
     printf("d1[%lf]\n",d1);

	 return 0;
 }
