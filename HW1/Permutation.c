#include<stdio.h>
#include <stdlib.h>  
#include <math.h>
long long p(int n, int r);
long long p(int n, int r){

    long long sum=1;
    int i=0;
    for(i=0;i<r;i++){
        sum=sum*(n-i);
    }
    return sum;
}

int main(void) {
    int n,r;
    long long P=0;
    printf("Enter the total number:");
    scanf("%d\n",&n);
    printf("Enter the number of elements to arrange and consider the order:");
    scanf("%d\n",&r);
    P = p(n,r);
    printf("P(%d, %d) = %lld\n", n, r, P);
    
    system("pause");  
    return 0;
}





// #include<stdio.h>
// #include <stdlib.h>  // 包含 system 函數所需的標頭
// #include <math.h>

// int p(int n, int r);
// int f(int n);

// int p(int n, int r){
//     if (r > n || n < 0 || r < 0) {
//         printf("錯誤：n 和 r 必須滿足 n >= r 且非負。\n");
//         return -1;  // 返回錯誤代碼
//     }
//     return f(n)/f(n-r);
// }

// int f(int n){
//     if(n==0 || n==1){
//         return 1;
//     }
//     return n*f(n-1);
// }
// int main(void) {
//     int n,r,P;

//     printf("Enter the total number");
//     scanf("%d",&n);
//     printf("Enter the number of elements to arrange and consider the order");
//     scanf("%d",&r);

//     P = p(n,r);
//     if (P != -1) {  // 檢查是否發生錯誤
//         printf("排列數 P(%d, %d) = %d\n", n, r, P);
//     }
    
//     system("pause");  
//     return 0;
// }