#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 將兩個大數（以字串形式表示）相乘
char *multiplyStrings(const char *num1, const char *num2) {
    int len1 = strlen(num1);
    int len2 = strlen(num2);
    int *result = (int *)calloc(len1 + len2, sizeof(int)); // 用於存放計算結果
    char *product = (char *)malloc(len1 + len2 + 1);       // 最後的乘積結果

    // 逐位模擬大數直式乘法
    for (int i = len1 - 1; i >= 0; i--) {
        for (int j = len2 - 1; j >= 0; j--) {
            int mul = (num1[i] - '0') * (num2[j] - '0');
            int sum = mul + result[i + j + 1];
            result[i + j + 1] = sum % 10;
            result[i + j] += sum / 10;
        }
    }

    // 將計算結果轉為字串，去除前導 0
    int index = 0;
    for (int i = 0; i < len1 + len2; i++) {
        if (!(index == 0 && result[i] == 0)) { // 跳過前導 0
            product[index++] = result[i] + '0';
        }
    }
    product[index] = '\0';

    // 如果結果為空，則返回 "0"
    if (index == 0) {
        strcpy(product, "0");
    }

    free(result);
    return product;
}

// 從一個大數字串中減去 1
char *subtractOne(const char *num) {
    int len = strlen(num);
    char *result = (char *)malloc(len + 1);
    strcpy(result, num);
    int i = len - 1;

    // 從最低位開始減，處理進位
    while (i >= 0 && result[i] == '0') {
        result[i] = '9';
        i--;
    }

    if (i >= 0) {
        result[i]--;
    }

    // 如果最高位是 '0'，去除它
    if (result[0] == '0' && len > 1) {
        memmove(result, result + 1, len); // 向左移動字串
    }

    return result;
}

// 計算排列數 P(n, r)
char *permutation(int n, int r) {
    if (r == 0) return strdup("1"); // 如果 r == 0，返回 1

    char *result = strdup("1");     // 初始結果為 "1"
    char current[20];
    snprintf(current, sizeof(current), "%d", n); // 將 n 轉為字串

    for (int i = 0; i < r; i++) {
        char *temp = result;
        result = multiplyStrings(result, current); // 計算結果 * current
        free(temp);

        temp = subtractOne(current); // 將 current 減 1
        strcpy(current, temp);
        free(temp);
    }

    return result;
}

int main() {
    int n, r;
    printf("輸入總數 (n): ");
    scanf("%d", &n);
    printf("輸入排列的數量 (r): ");
    scanf("%d", &r);

    // 檢查 n 和 r 是否滿足條件
    if (n >= r && n >= 0 && r >= 0) {
        char *result = permutation(n, r);
        printf("P(%d, %d) = %s\n", n, r, result);
        free(result);
    } else {
        printf("錯誤：n 和 r 必須滿足 n >= r 且均為非負數。\n");
    }
    system("pause");
    return 0;
}
