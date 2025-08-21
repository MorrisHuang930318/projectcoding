#include<stdio.h>
#include <stdlib.h>  
#include <math.h>
long long c(int n, int r);
long long c(int n, int r) {
    long long sum = 1;
    for (int i = 0; i < r; i++) {
        sum = sum * (n - i) / (i + 1);
    }
    return sum;
}
int main(void) {

    int n,r;
    long long C;
    printf("Enter the total number:");
    scanf("%d",&n);
    printf("Enter the number of elements to arrange without considering the order:");
    scanf("%d",&r);
    C = c(n,r);
    printf("C(%d, %d) = %lld\n", n, r, C);
    
    system("pause");
    return 0;
}
// #include<stdio.h>
// #include <stdlib.h>  
// #include <math.h>
// int c(int n, int r);
// int f(int n);

// int c(int n, int r){
//     if (r > n || n < 0 || r < 0) {
//         printf("錯誤：n 和 r 必須滿足 n >= r 且非負。\n");
//         return -1;  
//     }
//     return f(n)/(f(n-r)*f(r));
// }
// int f(int n) {
//     if (n == 0 || n == 1)  
//         return 1;
//     return n * f(n - 1);  
// }
// int main(void) {

//     int n,r,C;

//     printf("Enter the total number:");
//     scanf("%d",&n);
//     printf("Enter the number of elements to arrange without considering the order:");
//     scanf("%d",&r);

//     C = p(n,r);
//     if (C != -1) {  
//         printf("%d",P);
//     }
//     system("pause");
//     return 0;
// }

// #include <stdio.h>
// #include <stdlib.h>

// // 階乘函數
// int f(int n) {
//     if (n == 0 || n == 1)  // 基本情況
//         return 1;
//     return n * f(n - 1);  // 遞迴計算
// }

// // 計算組合數 C(n, r)
// int c(int n, int r) {
//     if (r > n || n < 0 || r < 0) {  // 檢查非法輸入
//         printf("錯誤：n 和 r 必須滿足 n >= r 且非負。\n");
//         return -1;  // 返回錯誤代碼
//     }
//     return f(n) / (f(r) * f(n - r));  // 使用組合公式
// }

// int main(void) {
//     int n, r, C;

//     // 提示用戶輸入
//     printf("Enter the total number:");
//     scanf("%d", &n);
//     printf("Enter the number of elements to arrange without considering the order:");
//     scanf("%d", &r);

//     // 計算組合數
//     C = c(n, r);
//     if (C != -1) {  // 確保無錯誤後輸出
//         printf("組合數 C(%d, %d) = %d\n", n, r, C);
//     }

//     system("pause");  // 暫停程式以查看輸出
//     return 0;
// }
