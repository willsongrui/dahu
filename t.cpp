#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main()
{
	char temp[500];
	sprintf(temp,"hahaha %s","iiiii");
	printf("%s",temp);
	printf("%d",strlen(temp));
	bool test = (temp[strlen(temp)] == '\0');
	printf("%d",test);
	return 0;
}