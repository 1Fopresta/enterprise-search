
#include <stdio.h>
#include "utf8-filter.h"


int main()
{
    char	*test = "Dette er en test for ��� og æøå";

    char	*ny = utf8_filter(test);

    printf("%s\n", ny);
}
