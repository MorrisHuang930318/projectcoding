#include <stdio.h>
#include <stdlib.h> /* 亂數相關函數 */
#include <time.h>   /* 時間相關函數 */
#include <stdlib.h> 

int main(void){

    srand( time(NULL) );
    int min = 0 ;
    int max = 1 ;
    int cArray[10] = {0} ;
    int barkerCode[7] = {1, 1, 1, -1, -1, 1, -1};     //DSSS Code (7-bit Barker Code): [+1, +1, +1, -1, -1, +1, -1]
    int dataArray[10][7] = {0} ;
    int dataArray1[70] = {0};
    int dataArray2[70] = {0};
    int XORdataArray[70] = {0};
    int autocorrelationcoeff[71] = {0} ;
    int max_coeff = 0;
    int max_position = 0;
    int second_max = 0;
    int third_max = 0;
    int second_position = 0;
    int third_position = 0;
    

    //print 7 bits of barker code
    printf("7 bits of barker code     : ");
    for(int i = 0 ; i < 7 ; i++){
        printf("%2d," , barkerCode[i] ); 
        if( i == 6 ){
            printf("\n");
        }
    }
    // generate 10 bits of random data and print it
    for(int i = 0 ; i < 10 ; i++){
        cArray[i] = (rand() % (max - min + 1)) + min; 
    } 
    printf("random 10 bits            : ");
    for(int i = 0 ; i < 10 ; i++){
        printf("%2d," , cArray[i] ); 
        if( i == 9 ){
            printf("\n");
        }
    }
    //BPSK modulation and print it
    for(int i = 0 ; i < 10 ; i++){
        if(cArray[i] == 0){
            cArray[i] = -1;
        }
    } 
    printf("random 10 bits after BPSK : ");
    for(int i = 0 ; i < 10 ; i++){
        printf("%2d," , cArray[i] ); 
        if( i == 9 ){
            printf("\n");
        }
    } 
    //generate the DSSS code and print it
    printf("data array : \n");
    for(int i = 0 ; i < 10 ; i++){
        for(int j = 0 ; j < 7 ; j++){
            dataArray[i][j] = barkerCode[j] * cArray[i];
            dataArray1[i * 7 + j] = dataArray[i][j]; // 將二維陣列的值存入一維陣列
        }
    }
    //print dataArray in 2d array format
    for(int i = 0 ; i < 10 ; i++){
        for(int j = 0 ; j < 7 ; j++){
            printf("%2d," , dataArray[i][j]);
        }
        printf("\n");
    }
    //print dataArray1
    printf("dataArray1 : \n");
    for(int i = 0 ; i < 70 ; i++){
        printf("%d " , dataArray1[i]);
        if( i == 69 ){
            printf("\n");
        }
    }
    //copy dataArray1 to dataArray2
    for(int i = 0 ; i < 70 ; i++){
        dataArray2[i] = dataArray1[i] ;
    }
    //print dataArray2
    printf("dataArray2 : \n");
    for(int i = 0 ; i < 70 ; i++){
        printf("%d " , dataArray2[i]);
        if( i == 69 ){
            printf("\n");
        }
    }
    //dot product operation
    for(int rshift = 0 ; rshift <= 70 ; rshift++){
        //printf("right shift %d bit : \n" , rshift);
        for(int i = 0 ; i < 70 ; i++){
            XORdataArray[i] = dataArray1[i] * dataArray2[i + rshift] ; // XOR operation
            if (i + rshift >= 70){
                XORdataArray[i] = dataArray1[i] * dataArray2[(i + rshift) % 70] ;
            }
        }
        for(int j = 0 , xorsum = 0; j < 70 ; j++){
                //printf("%2d " ,  XORdataArray[j]);
                xorsum += XORdataArray[j] ;
                autocorrelationcoeff[rshift] = xorsum ;
                // if( j == 69 ){
                //     printf("\n");
                // }
            }
        //printf("autocorrelation coeff : %d\n" , autocorrelationcoeff[rshift]);
        //printf("\n");
    }
    for(int i = 0 ; i < 70 ; i++){
        if(autocorrelationcoeff[i] < 0){
            autocorrelationcoeff[i] = -autocorrelationcoeff[i] ;
    }
    }
    printf("autocorrelation coeff :\n");
    for(int a= 0 ; a < 70 ; a++){
        printf(" %2d, " , autocorrelationcoeff[a]);
        if( a % 7 == 6 ){
            printf("\n");
        }
    }
    //guess the PRN code length by finding the maximum correlation value and its position
    // find the maximum correlation value and its position
    for(int i = 0 ; i < 70; i++) {
        if(autocorrelationcoeff[i] > max_coeff) {
            max_coeff = autocorrelationcoeff[i];
            max_position = i;
        }
    }
    printf("\nMaximum correlation coefficient: %d at shift %d\n", max_coeff, max_position);
    // find the second maximum correlation value and its position
    for(int i = 0; i < 70; i++) {
        if(autocorrelationcoeff[i] > second_max && autocorrelationcoeff[i] < max_coeff) {
            second_max = autocorrelationcoeff[i];
            second_position = i;
        }
    }
    printf("Second highest correlation coefficient: %d at shift %d\n", second_max, second_position);
    // find the third maximum correlation value and its position
    for(int i = 0 ; i < 70; i++) {
        if(autocorrelationcoeff[i] > third_max && autocorrelationcoeff[i] < second_max) {
            third_max = autocorrelationcoeff[i];
            third_position = i;
        }
    }
    printf("Third highest correlation coefficient: %d at shift %d\n", third_max, third_position);
    int divisor = 0 ;
    for (int d = 2; d <= max_coeff && d <= second_max; d++) {
        // Check if divisor divides both numbers
        if (max_coeff % d == 0 && second_max % d == 0 && d >= 5 && d <= 10) {
            divisor = d;  
            printf("First common divisor of %d and %d is: %d\n", max_coeff, second_max, divisor);
            break; 
        }
    }
    // int code_length = 0;
    // int guesscodelength = 0;
    // int diff1 = abs(max_position - second_position);
    // int diff2 = abs(max_position - third_position);
    // int diff3 = abs(second_position - third_position);

    // if (diff1 % 7 == 0 && diff2 % 7 == 0 && diff3 % 7 == 0) {
    // guesscodelength = 7;
    // printf("Guessed PRN code length is %d based on periodic correlation peaks.\n", guesscodelength);
    // } else {
    // printf("Unable to determine PRN code length from peak correlation spacing.\n\n");
    // }
    //照理說這邊會猜出週期是7，但是有時候會被雜訊2,10干擾

    // printf("autocorrelation coeff :\n");
    // for(int a= 0 ; a < 70 ; a++){
    //     printf(" %d, " , autocorrelationcoeff[a]);
    //     if( a % 7 == 6 ){
    //         printf("\n");
    //     }
    // }
    // printf("autocorrelation coeff :\n");
    // for(int a = 0 ; a < 70 ; a++){
    //     printf(" %d, " , autocorrelationcoeff[a]);
    //     }
    int rows = 70 / divisor;
    int guesseddataArray[10][7] = {0} ;
    int index = 0 ;
    for(int j = 0 ; j < rows ; j++){
        for(int k = 0 ; k < divisor ; k++){
                guesseddataArray[j][k] = dataArray1[index] ;
                index++;
        }
    }
    printf("guessed data array :\n");
    for(int i = 0 ; i < rows ; i++){
        for(int j = 0 ; j < divisor ; j++){
            printf("%2d," , guesseddataArray[i][j]);
        }
        printf("\n");
    }
    printf("%d\n" , divisor);

    //do dot product to every row with the first row and normalize it
    int rowdotproduct[10] = {0} ;
    int normalizerowdotproduct1[10] = {0} ;
    int normalizerowdotproduct2[10] = {0} ;
    for(int r = 0 ; r < 10 ; r++){
        int rowdotproductsum = 0;
        int normalizerdp = 0 ;
        for(int c = 0 ; c < 7 ; c++){
            rowdotproductsum += dataArray[r][c] * dataArray[0][c] ;
            normalizerdp = rowdotproductsum / 7 ;
        }
        rowdotproduct[r] = rowdotproductsum ;
        normalizerowdotproduct1[r] = normalizerdp ;
        
    }
    // 你是把 每列的 DSSS 編碼 拿去跟 第 0 列（第一個 bit）做比對
    // 👉 若內積是正的 → 相同 → 判為 1
    // 👉 若內積是負的 → 相反 → 判為 0
    // 👉 就能猜測原始資料 bit 是 0 還是 1


    for(int i = 0 ; i < 10 ; i++){
        printf("row %d dot product : %d\n" , i , rowdotproduct[i]);
    }
    printf("after normalization :\n");
    for(int i = 0 ; i < 10 ; i++){
        printf("row %d dot product : %d\n" , i , normalizerowdotproduct1[i]);
    }
    printf("we can guess the data is :\n");
    for(int i = 0 ; i < 10 ; i++){
        printf("%d, " , normalizerowdotproduct1[i]);
    }
    printf("\n");
    printf("we can have original data after BPSK Demodulation :\n");
    for(int i = 0 ; i < 10 ; i++){
        if(normalizerowdotproduct1[i] > 0){
            normalizerowdotproduct1[i] = 1;
        }else{
            normalizerowdotproduct1[i] = 0;
        }
    }
    for(int i = 0 ; i < 10 ; i++){
        if(normalizerowdotproduct1[i] == 0){
            normalizerowdotproduct2[i] = 1;
        }else{
            normalizerowdotproduct2[i] = 0;
        }
    }
    for(int i = 0 ; i < 10 ; i++){
        printf("%d, " , normalizerowdotproduct1[i]);
        if(i == 9){
            printf("\n");
            printf("or :");
        }
    }
    for(int i = 0 ; i < 10 ; i++){
        printf("%d, " , normalizerowdotproduct2[i]);
        if(i == 9){
            printf("\n");
        }
    }
    //guess the PRN code by doing dot product with the first column and normalize it
    printf("PRN code :\n");
    int estimatePRNcode[7] = {0} ;
    for(int j = 0 ; j < 10 ; j++){
        int estimatePRN = 0 ;
        for(int i = 0; i < 10 ; i++){
            estimatePRN = dataArray[i][j] * normalizerowdotproduct1[i] ;
            estimatePRNcode[j] += estimatePRN ;
        }
    }
    
    int estimatePRNcode0 ;
    estimatePRNcode0 = estimatePRNcode[0];
    for(int i = 0 ; i < 7 ; i++){
        estimatePRNcode[i] = estimatePRNcode[i] / estimatePRNcode0 ;
    }
    for(int i = 0 ; i < 7 ; i++){
        printf("%d, " , estimatePRNcode[i]);
        if( i == 6 ){
            printf("\n");
        }
    }
    system("pause");
    return 0 ;
}