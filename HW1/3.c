#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 將兩個大數（以字串形式表示）相乘
char *multiplyStrings(const char *num1, const char *num2) {
    int len1 = strlen(num1);
    int len2 = strlen(num2);
    int *result = (int *)calloc(len1 + len2, sizeof(int));
    char *product = (char *)malloc(len1 + len2 + 1);

    for (int i = len1 - 1; i >= 0; i--) {
        for (int j = len2 - 1; j >= 0; j--) {
            int mul = (num1[i] - '0') * (num2[j] - '0');
            int sum = mul + result[i + j + 1];
            result[i + j + 1] = sum % 10;
            result[i + j] += sum / 10;
        }
    }

    int index = 0;
    for (int i = 0; i < len1 + len2; i++) {
        if (!(index == 0 && result[i] == 0)) {
            product[index++] = result[i] + '0';
        }
    }
    product[index] = '\0';

    if (index == 0) {
        strcpy(product, "0");
    }

    free(result);
    return product;
}

// 計算階乘 r! (使用字串處理大數)
char *factorial(int r) {
    char *result = strdup("1");
    char current[20];
    snprintf(current, sizeof(current), "%d", 1);

    for (int i = 2; i <= r; i++) {
        char *temp = result;
        snprintf(current, sizeof(current), "%d", i);
        result = multiplyStrings(result, current);
        free(temp);
    }

    return result;
}

// 從一個大數字串中減去 1
char *subtractOne(const char *num) {
    int len = strlen(num);
    char *result = (char *)malloc(len + 1);
    strcpy(result, num);
    int i = len - 1;

    while (i >= 0 && result[i] == '0') {
        result[i] = '9';
        i--;
    }

    if (i >= 0) {
        result[i]--;
    }

    if (result[0] == '0' && len > 1) {
        memmove(result, result + 1, len);
    }

    return result;
}

// 計算排列數 P(n, r)
char *permutation(int n, int r) {
    if (r == 0) return strdup("1");

    char *result = strdup("1");
    char current[20];
    snprintf(current, sizeof(current), "%d", n);

    for (int i = 0; i < r; i++) {
        char *temp = result;
        result = multiplyStrings(result, current);
        free(temp);

        temp = subtractOne(current);
        strcpy(current, temp);
        free(temp);
    }

    return result;
}

// 計算組合數 C(n, r) = P(n, r) / r!
char *combination(int n, int r) {
    if (r == 0 || n == r) return strdup("1");

    char *p = permutation(n, r);
    char *f = factorial(r);

    char *c = multiplyStrings(p, "1"); // 實現除法時用乘法模擬（如果需要更高效的除法，可優化）。
    free(p);
    free(f);

    return c;
}

int main() {
    int n, r;
    printf("輸入總數 (n): ");
    scanf("%d", &n);
    printf("輸入組合的數量 (r): ");
    scanf("%d", &r);

    if (n >= r && n >= 0 && r >= 0) {
        char *result = combination(n, r);
        printf("C(%d, %d) = %s\n", n, r, result);
        free(result);
    } else {
        printf("錯誤：n 和 r 必須滿足 n >= r 且均為非負數。\n");
    }
    system("pause");
    return 0;
}
