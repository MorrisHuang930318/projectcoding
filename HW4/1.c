#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    srand(time(NULL));
    int cArray[10] = {0,1,1,0,0,0,1,1,0,0};
    int barkerCode[7] = {1, 1, 1, -1, -1, 1, -1};
    int dataArray[10][7] = {0};
    int dataArray1[70] = {0};
    int XORdataArray[70] = {0};
    int autocorrelationcoeff[70] = {0}; // 增加大小以確保安全

    printf("random 10 bits : ");
    for(int i = 0; i < 10; i++) {
        printf("%d,", cArray[i]);
    }
    printf("\n");

    printf("7 bits of barker code : ");
    for(int i = 0; i < 7; i++) {
        printf("%d,", barkerCode[i]);
    }
    printf("\n");

    // BPSK modulation
    for(int i = 0; i < 10; i++) {
        if(cArray[i] == 0) {
            cArray[i] = -1;
        }
    }
    printf("random 10 bits after BPSK : ");
    for(int i = 0; i < 10; i++) {
        printf("%d,", cArray[i]);
    }
    printf("\n");

    // Generate the DSSS code
    printf("data array : \n");
    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 7; j++) {
            dataArray[i][j] = barkerCode[j] * cArray[i];
            dataArray1[i * 7 + j] = dataArray[i][j]; 
        }
    }

    // 打印二維陣列
    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 7; j++) {
            printf("  %d,", dataArray[i][j]);
        }
        printf("\n");
    }

    // 打印一維陣列dataArray1
    printf("dataArray1 : \n");
    for(int i = 0; i < 70; i++) {
        printf("%d ", dataArray1[i]);
    }
    printf("\n\n");

    // 計算自相關係數 - 修正循環移位和求和操作
    printf("Autocorrelation coefficients:\n");
    for(int shift = 0; shift < 70; shift++) {
        int xorsum = 0; // 每次移位前重置求和變量
        
        for(int i = 0; i < 70; i++) {
            int shifted_idx = (i + shift) % 70; // 正確的循環移位
            XORdataArray[i] = dataArray1[i] * dataArray1[shifted_idx]; // 相關運算
            xorsum += XORdataArray[i];
        }
        
        autocorrelationcoeff[shift] = xorsum;
        
        if (shift < 15) { // 只打印前15個移位的結果，以減少輸出量
            printf("Shift %d: coeff = %d\n", shift, xorsum);
        }
    }
        printf("autocorrelation coeff :\n");
    for(int a= 0 ; a < 70 ; a++){
        printf(" %d, " , autocorrelationcoeff[a]);
        if( a % 7 ==6){
            printf("\n");
        }
    }
    printf("autocorrelation coeff :\n");
    for(int a= 0 ; a < 70 ; a++){
        printf(" %d, " , autocorrelationcoeff[a]);
        }
    // 找出最大相關值及其出現的位置
    int max_coeff = 0;
    int max_position = 0;
    
    for(int i = 1; i < 70; i++) {
        if(autocorrelationcoeff[i] > max_coeff) {
            max_coeff = autocorrelationcoeff[i];
            max_position = i;
        }
    }
    
    printf("\nMaximum correlation coefficient: %d at shift %d\n", max_coeff, max_position);
    
    // 找出次高相關值
    int second_max = 0;
    int second_position = 0;
    
    for(int i = 1; i < 70; i++) {
        if(autocorrelationcoeff[i] > second_max && autocorrelationcoeff[i] < max_coeff) {
            second_max = autocorrelationcoeff[i];
            second_position = i;
        }
    }
    
    printf("Second highest correlation coefficient: %d at shift %d\n", second_max, second_position);
    
    // 如果最大相關值在位置7的倍數，表示已找到碼長
    int code_length = 0;
    if (max_position % 7 == 0) {
        code_length = 7;
        printf("The PRN code length appears to be %d bits\n", code_length);
    } else {
        printf("Could not determine code length from correlation analysis\n");
    }
    system("pause"); // 暫停以查看輸出
    return 0;
}