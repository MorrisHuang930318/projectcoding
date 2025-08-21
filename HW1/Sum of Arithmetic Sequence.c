#include<stdio.h>
#include <stdlib.h>  // 包含 system 函數所需的標頭
float sum(float a, float b, int c);

float sum(float a, float b, int c){
    return (c*(2*a+(c-1)*b))/2;
}
int main(void) {

    float a,d,Sn;
    int n;

    printf("Please enter the first term:");
    scanf("%f",&a);
    printf("Please enter the common difference:");
    scanf("%f",&d);
    printf("Please enter the number of terms:");
    scanf("%d",&n);

    Sn = sum(a, d, n);
    printf("%f",Sn);
    
    system("pause");  // 這行放在 main 函數內
    return 0;
}