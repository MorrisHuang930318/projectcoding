#include<stdio.h>
#include <stdlib.h>  // 包含 system 函數所需的標頭
#include <math.h>
float sum(float a, float rn, float r, int n);

float sum(float a, float rn, float r, int n){
    if(r==1){
        return a*n;
        }
    return a*(1-rn)/(1-r);
}
int main(void) {

    float a,r,Sn,rn;
    int n;

    printf("Please enter the first term:");
    scanf("%f",&a);
    printf("Please enter the Common ratio:");
    scanf("%f",&r);
    printf("Please enter the number of terms:");
    scanf("%d",&n);

    rn = pow(r, n);
    Sn = sum(a, rn, r, n);
    printf("%f",Sn);
    
    system("pause");  // 這行放在 main 函數內
    return 0;
}