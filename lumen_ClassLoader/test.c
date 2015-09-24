#include<stdio.h>
#include<string.h>

char *php_strtr(char *str, int len, char *str_from, char *str_to, int trlen)
{
	int i;
	unsigned char xlat[256];

	if ((trlen < 1) || (len < 1)) {
		return str;
	}

	for (i = 0; i < 256; xlat[i] = i, i++);

	for (i = 0; i < trlen; i++) {
		xlat[(unsigned char) str_from[i]] = str_to[i];
	}

	for (i = 0; i < len; i++) {
		str[i] = xlat[(unsigned char) str[i]];
	}

	return str;
}

int main() {
  char buff[] = "data\\php-5.6.9\\ext\\lumen_ClassLoader";  
  php_strtr(buff, strlen(buff), "\\", "/", 1);
  printf("buff:%s\n", buff);
  return 0;
}
