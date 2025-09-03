#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <math.h>
#define PI 3.14159265358979323846
#define LOADED_DATA_SIZE 500



int main() {
    
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    int PRN_1[PRN_LENGTH] = {
    1,0,1,0,1,0,0,1,1,1,1,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,0,0,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,0,0,0,0,1,0,1,1,0,1,1,1,0,1,0,1,
    0,0,0,0,1,1,1,1,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,0,0,0,1,0,1,0,1,1,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,0,1,0,0,0,1,1,1,0,
    1,0,0,1,1,1,0,0,1,0,1,1,0,1,0,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,1,1,1,0,0,0,1,0,0,0,
    1,1,0,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,1,1,1,0,1,1,1,0,1,0,1,1,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,0,1,0,0,0,0
    };

    int PRN_3[PRN_LENGTH] = {
    1,0,1,0,1,1,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,0,1,0,0,0,1,1,1,0,1,0,0,1,1,1,0,0,1,0,1,1,0,1,0,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,
    1,0,1,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,1,1,1,0,0,0,1,0,0,0,1,1,0,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,1,
    1,1,0,1,1,1,0,1,0,1,1,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,0,1,1,0,0,0,0,1,0,1,0,1,0,0,1,1,1,1,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,0,0,0,0,1,
    1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,0,0,0,0,1,0,1,1,0,1,1,1,0,1,0,1,0,0,0,0,1,1,1,1,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,0,0,0
    };

    //讀取資料
    FILE *file = fopen("data_coarse_synced.txt", "r");
    if (file == NULL) {
        perror("無法開啟檔案");
        return 1;
    }
    printf("檔案開啟成功\n");

    int row_count = 0;
    int col_count = 0;
    char line[32];

    // Step 1：先計算行數與欄數
   while (fgets(line, sizeof(line), file)) {
        row_count++;
        if (col_count == 0) {
            char *p = line;
            double temp;
            int offset;
            while (sscanf(p, "%lf%n", &temp, &offset) == 1) {
                col_count++;
                p += offset;  // 往後移動到下一個數字
            }
        }
    }

    printf("行數: %d, 欄數: %d\n", row_count, col_count);
    rewind(file); 

    size_t num_rows = row_count;
    // 分配記憶體
    double *i_data = (double *)malloc(num_rows * sizeof(double));
    double *q_data = (double *)malloc(num_rows * sizeof(double));
    double *phase_data = (double *)malloc(num_rows * sizeof(double));
    double *I_rot = (double *)malloc(num_rows * sizeof(double));
    double *Q_rot = (double *)malloc(num_rows * sizeof(double));
    int *I_QPSK_demod = (int *)malloc(num_rows * sizeof(int));  // 改為 int 型態
    int *Q_QPSK_demod = (int *)malloc(num_rows * sizeof(int));  // 改為 int 型態

    if (i_data == NULL || q_data == NULL || phase_data == NULL || I_rot == NULL || Q_rot == NULL) {
        fprintf(stderr, "記憶體配置失敗\n");
        return 1;
    }
    else
        printf("記憶體配置成功\n");
    
    int i = 0;
    // 一行一行讀取兩個浮點數
    while (i < num_rows && fscanf(file, "%lf %lf", &i_data[i], &q_data[i]) == 2) {
       //printf("第 %d 筆資料：x = %.6f, y = %.6f\n", i+1, i_data[i], q_data[i]);
        i++;
        if(i > LOADED_DATA_SIZE){
            printf("已讀取 %d 筆資料，停止讀取。\n", LOADED_DATA_SIZE);
            break;
          }
    }
    printf("共成功讀入%d 筆資料\n", i);
    printf("第一筆: I = %.6f, Q = %.6f\n", i_data[0], q_data[0]);
    printf("最後一筆: I = %.6f, Q = %.6f\n", i_data[i - 2], q_data[i - 2]);

    fclose(file);
    printf("檔案關閉成功\n");


    //建立旋轉過後的結果檔案
    FILE *fp1;
    char filename1[] = "rotate_result.txt";
    // 開啟檔案，模式為 "w" (寫入)
    fp1 = fopen(filename1, "w");
    // 檢查檔案是否成功開啟
    if (fp1 == NULL) {
        perror("無法開啟檔案");
        return 1;
    }

    //建立QPSK Demodulation後結果的檔案
    FILE *fp2;
    char filename2[] = "QPSK_result.txt";
    // 開啟檔案，模式為 "w" (寫入)
    fp2 = fopen(filename2, "w");
    // 檢查檔案是否成功開啟
    if (fp2 == NULL) {
        perror("無法開啟檔案");
        return 1;
    }

    //逆時針旋轉
    for(int i = 0 ; i < LOADED_DATA_SIZE ; i++){
        for(double angle = 0 ; angle <= 180 ; angle += 10){
            double theta;
            theta = angle * PI / 180.0;
            I_rot[i] = i_data[i]*cos(theta) - q_data[i]*sin(theta);
            Q_rot[i] = q_data[i]*cos(theta) + i_data[i]*sin(theta);  
            //printf("旋轉%06.2f°(%9.6f)後 , 第%d筆資料為 : %9.6f + j%9.6f\n", angle , theta , i + 1 , I_rot[i], Q_rot[i]); 
            fprintf(fp1 , "旋轉%06.2f°(%9.6f)後 , 第%d筆資料為 : %9.6f + j%9.6f\n", angle , theta , i + 1 , I_rot[i], Q_rot[i]); 
        }
    }
    printf("\n");
    //順時針旋轉
    for(int i = 0 ; i < LOADED_DATA_SIZE ; i++){
        for(double angle = 0 ; angle <= 180 ; angle += 10){
            double theta;
            theta = angle * PI / 180.0;
            I_rot[i] = i_data[i]*cos(theta) + q_data[i]*sin(theta);
            Q_rot[i] = q_data[i]*cos(theta) - i_data[i]*sin(theta);  
            //printf("旋轉%7.2f°(%9.6f)後 , 第%d筆資料為 : %9.6f + j%9.6f\n", -angle , theta , i + 1 , I_rot[i], Q_rot[i]); 
            fprintf(fp1 , "旋轉%06.2f°(%9.6f)後 , 第%d筆資料為 : %9.6f + j%9.6f\n", angle , theta , i + 1 , I_rot[i], Q_rot[i]); 
        }
    }

    fclose(fp1);
    printf("檔案 '%s' 建立並寫入成功！\n", filename1);

    // for(int i = 0; i < 100; i++) {
    //     printf("第 %3d 筆資料：I = %9.6f, Q = %9.6f\n", i + 1, i_data[i], q_data[i]);
    // }

    //QPSK Demodulation
    printf("QPSK Demodulation:\n");
    for(int i = 0; i < LOADED_DATA_SIZE; i++){
        double I = i_data[i];
        double Q = q_data[i];
        if (I >= 0 && Q > 0) {        //第一象限或I軸上方
            //printf("第 %03d 筆資料：( %9.6f + j%9.6f ) 第一象限 \n", i + 1 , i_data[i], q_data[i] );
            I_QPSK_demod[i] = 0;  // 存儲解調結果
            Q_QPSK_demod[i] = 0; 
            fprintf(fp2, "解調結果: 0 (i軸) 0 (q軸)\n");
        } else if (I < 0 && Q >= 0) { //第二象限或Q軸右側
            //printf("第 %03d 筆資料：( %9.6f + j%9.6f ) 第二象限 \n", i + 1 , i_data[i], q_data[i] );
            I_QPSK_demod[i] = 0;  // 存儲解調結果
            Q_QPSK_demod[i] = 1;
            fprintf(fp2, "解調結果: 0 (i軸) 1 (q軸)\n");
        } else if (I <= 0 && Q < 0) { //第三象限或I軸下方
            //printf("第 %03d 筆資料：( %9.6f + j%9.6f ) 第三象限 \n", i + 1 , i_data[i], q_data[i] );
            I_QPSK_demod[i] = 1;  // 存儲解調結果
            Q_QPSK_demod[i] = 1;
            fprintf(fp2, "解調結果: 1 (i軸) 1 (q軸)\n");
        } else if (I > 0 && Q <= 0) { //第四象限或Q軸左側
            //printf("第 %03d 筆資料：( %9.6f + j%9.6f ) 第四象限 \n", i + 1 , i_data[i], q_data[i] );
            I_QPSK_demod[i] = 1;  // 存儲解調結果
            Q_QPSK_demod[i] = 0;
            fprintf(fp2, "解調結果: 1 (i軸) 0 (q軸)\n");
        } else
            printf("第 %03d 筆資料：( %9.6f + j%9.6f ) 無法分類 \n", i + 1 , i_data[i], q_data[i] );
    }


    fclose(fp2);
    free(i_data);
    free(q_data);
    free(phase_data);
    free(I_rot);
    free(Q_rot);
    free(I_QPSK_demod);
    free(Q_QPSK_demod);
    
    system("pause");
    return 0;
}


