#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <math.h>

#define PI 3.14159265358979323846
int main() {
    
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    FILE *file = fopen("data_coarse_synced.txt", "r");
    if (file == NULL) {
        perror("無法開啟檔案");
        //free(i_data);
        //free(q_data);
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
    
    double *i_data = (double *)malloc(num_rows * sizeof(double));
    double *q_data = (double *)malloc(num_rows * sizeof(double));
    double *phase_data = (double *)malloc(num_rows * sizeof(double));
    double *I_rot = (double *)malloc(num_rows * sizeof(double));
    double *Q_rot = (double *)malloc(num_rows * sizeof(double));
    if (i_data == NULL || q_data == NULL && phase_data == NULL || I_rot == NULL || Q_rot == NULL) {
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
        if(i > 99){
            printf("已讀取 100 筆資料，停止讀取。\n");
            break;
          }
    }
    //逆時針旋轉
    for(double angle = 0 ; angle <= 180 ; angle += 10){
        double theta;
        theta = angle * PI / 180.0;
        for(int i = 0 ; i < 1 ; i++){
            I_rot[i] = i_data[i]*cos(theta) - q_data[i]*sin(theta);
            Q_rot[i] = q_data[i]*cos(theta) + i_data[i]*sin(theta);  
            printf("旋轉%06.2f°(%9.6f)後 , 第%d筆資料為 : %9.6f + j%9.6f\n", angle , theta , i + 1 , I_rot[i], Q_rot[i]); 
        }
    }
    printf("\n");
    //順時針旋轉
    for(double angle = 0 ; angle <= 180 ; angle += 10){
        double theta;
        theta = angle * PI / 180.0;
        for(int i = 0 ; i < 1 ; i++){
            I_rot[i] = i_data[i]*cos(theta) + q_data[i]*sin(theta);
            Q_rot[i] = q_data[i]*cos(theta) - i_data[i]*sin(theta);  
            printf("旋轉%7.2f°(%9.6f)後 , 第%d筆資料為 : %9.6f + j%9.6f\n", -angle , theta , i + 1 , I_rot[i], Q_rot[i]); 
        }
    }
    fclose(file);
    free(i_data);
    free(q_data);
    system("pause");
    return 0;
}