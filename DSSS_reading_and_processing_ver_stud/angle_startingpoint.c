#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <math.h>
#define PI 3.14159265358979323846
#define LOADED_DATA_SIZE 1000

int calculate_correlation(int *dataArray1, int *dataArray2, int size);
int find_start_point(int *I_QPSK_demod, int *PRN, int data_size, int prn_length, int *best_correlation_out);
void qpsk_demodulation(double *i_data, double *q_data, int *I_QPSK_demod, int *Q_QPSK_demod, int size);
void rotate_data(double *i_data, double *q_data, double *I_rot, double *Q_rot, double angle, int size, int clockwise);
int recover_data(int *I_QPSK_demod, int *PRN,int data_size, int prn_length,int start_point);

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    int PRN_1[255] = {
    1,0,1,0,1,0,0,1,1,1,1,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,0,0,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,0,0,0,0,1,0,1,1,0,1,1,1,0,1,0,1,
    0,0,0,0,1,1,1,1,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,0,0,0,1,0,1,0,1,1,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,0,1,0,0,0,1,1,1,0,
    1,0,0,1,1,1,0,0,1,0,1,1,0,1,0,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,1,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,1,1,1,0,0,0,1,0,0,0,
    1,1,0,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,1,1,1,0,1,1,1,0,1,0,1,1,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,0,1,0,0,0,0
    };

    int PRN_3[255] = {
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
    double *I_rot = (double *)malloc(num_rows * sizeof(double));
    double *Q_rot = (double *)malloc(num_rows * sizeof(double));
    int *I_QPSK_demod = (int *)malloc(num_rows * sizeof(int));
    int *Q_QPSK_demod = (int *)malloc(num_rows * sizeof(int));
    int *final_data = (int *)malloc(num_rows * sizeof(int));
    if (i_data == NULL || q_data == NULL || I_rot == NULL || Q_rot == NULL || I_QPSK_demod == NULL || Q_QPSK_demod == NULL ||final_data == NULL) { 
        fprintf(stderr, "記憶體配置失敗\n");
        return 1;
    }else{
        printf("記憶體配置成功\n");
    }
    
    // 一行一行讀取兩個浮點數
    int i = 0;
    while (i < num_rows && fscanf(file, "%lf %lf", &i_data[i], &q_data[i]) == 2) {
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

    // 建立結果檔案
    FILE *fp_result;
    char filename_result[] = "rotation_correlation_result.txt";
    fp_result = fopen(filename_result, "w");
    if (fp_result == NULL) {
        perror("無法開啟結果檔案");
        return 1;
    }

    // 儲存最佳結果的變數
    double best_angle = 0;
    int best_start_point_1 = 0, best_start_point_3 = 0;
    int best_correlation_1 = -1, best_correlation_3 = -1;
    int best_direction = 0; // 0: 逆時針, 1: 順時針

    printf("\n開始旋轉與相關性分析...\n");
    fprintf(fp_result, "旋轉角度\t方向\t\tPRN_1起始點\tPRN_1相關性\tPRN_3起始點\tPRN_3相關性\n");
    fprintf(fp_result, "========================================================================\n");

    // 逆時針旋轉 (0到180度)
    for(double angle = 0; angle <= 180; angle += 10) {
        // 旋轉資料
        rotate_data(i_data, q_data, I_rot, Q_rot, angle, LOADED_DATA_SIZE, 0);
        
        // QPSK解調
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);
        
        // 與PRN_1做相關性分析
        int correlation_1, correlation_3;
        int start_point_1 = find_start_point(I_QPSK_demod, PRN_1, LOADED_DATA_SIZE, 255, &correlation_1);
        int start_point_3 = find_start_point(I_QPSK_demod, PRN_3, LOADED_DATA_SIZE, 255, &correlation_3);
        
        // 記錄結果
        printf("逆時針旋轉 %.0f度: PRN_1(起始點:%d, 相關性:%d), PRN_3(起始點:%d, 相關性:%d)\n", 
               angle, start_point_1, correlation_1, start_point_3, correlation_3);
        fprintf(fp_result, "%.0f\t\t逆時針\t\t%d\t\t%d\t\t%d\t\t%d\n", 
                angle, start_point_1, correlation_1, start_point_3, correlation_3);
        
        // 更新最佳PRN_1結果
        if (correlation_1 > best_correlation_1) {
            best_correlation_1 = correlation_1;
            best_start_point_1 = start_point_1;
            best_angle = angle;
            best_direction = 0;
        }
        
        // 更新最佳PRN_3結果
        if (correlation_3 > best_correlation_3) {
            best_correlation_3 = correlation_3;
            best_start_point_3 = start_point_3;
            if (correlation_3 > best_correlation_1) {
                best_angle = angle;
                best_direction = 0;
            }
        }
    }

    // 順時針旋轉 (0到180度)
    for(double angle = 0; angle <= 180; angle += 10) {
        if (angle == 0) continue; // 避免重複計算0度
        
        // 旋轉資料
        rotate_data(i_data, q_data, I_rot, Q_rot, angle, LOADED_DATA_SIZE, 1);
        
        // QPSK解調
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);
        
        //
        //pos_neg_transform(I_QPSK_demod, PRN_1, LOADED_DATA_SIZE, 255);
        // 與PRN做相關性分析
        int correlation_1, correlation_3;
        int start_point_1 = find_start_point(I_QPSK_demod, PRN_1, LOADED_DATA_SIZE, 255, &correlation_1);
        int start_point_3 = find_start_point(I_QPSK_demod, PRN_3, LOADED_DATA_SIZE, 255, &correlation_3);
        
        // 記錄結果
        printf("順時針旋轉 %.0f度: PRN_1(起始點:%d, 相關性:%d), PRN_3(起始點:%d, 相關性:%d)\n", 
               angle, start_point_1, correlation_1, start_point_3, correlation_3);
        fprintf(fp_result, "-%.0f\t\t順時針\t\t%d\t\t%d\t\t%d\t\t%d\n", 
                angle, start_point_1, correlation_1, start_point_3, correlation_3);
        
        // 更新最佳PRN_1結果
        if (correlation_1 > best_correlation_1) {
            best_correlation_1 = correlation_1;
            best_start_point_1 = start_point_1;
            best_angle = -angle; // 負數表示順時針
            best_direction = 1;
        }
        
        // 更新最佳PRN_3結果
        if (correlation_3 > best_correlation_3) {
            best_correlation_3 = correlation_3;
            best_start_point_3 = start_point_3;
            if (correlation_3 > best_correlation_1) {
                best_angle = -angle;
                best_direction = 1;
            }
        }
    }
    // 重新計算最佳角度時的資料並輸出I軸解調結果
    int final_correlation = 0;
    printf("呼叫 recover_data，參數: start_point=%d\n", best_start_point_1);
    double final_best_angle = best_direction == 0 ? best_angle : -best_angle;
    rotate_data(i_data, q_data, I_rot, Q_rot, abs((int)final_best_angle), LOADED_DATA_SIZE, best_direction);
    qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);
    final_correlation = recover_data(I_QPSK_demod, PRN_1,LOADED_DATA_SIZE, 255 , best_start_point_1);
    printf("final_correlation:%d\n", final_correlation);
    if(final_correlation >= 50){
    final_data[0] = 1;
    } else {
        final_data[0] = 0;
    }

    printf("final_data[0]:%d\n", final_data[0]);
    fflush(stdout);

    // 輸出最佳結果
    printf("\n=========== 最佳結果 ===========\n");
    printf("最佳旋轉角度: %.0f度 (%s)\n", best_direction == 0 ? best_angle : -best_angle,best_direction == 0 ? "逆時針" : "順時針");
    printf("PRN_1 - 最佳起始點: %d, 最佳相關性: %d\n", best_start_point_1, best_correlation_1);
    printf("PRN_3 - 最佳起始點: %d, 最佳相關性: %d\n", best_start_point_3, best_correlation_3);
    
    // 輸出最佳角度時的I軸QPSK解調結果
    printf("\n最佳旋轉角度時的I軸QPSK解調結果:\n");
    for(int i = 0; i < LOADED_DATA_SIZE; i++) {
        if(i % 20 == 0 && i != 0) {
            printf("\n");
        }
        printf("%d", I_QPSK_demod[i]);
        if((i + 1) % 20 != 0 && i != LOADED_DATA_SIZE - 1) {
            printf(" ");
        }
    }
    printf("\n");
    
    // 將I軸解調結果寫入檔案
    fprintf(fp_result, "\n=========== 最佳結果 ===========\n");
    fprintf(fp_result, "最佳旋轉角度: %.0f度 (%s)\n", best_direction == 0 ? best_angle : -best_angle, best_direction == 0 ? "逆時針" : "順時針");
    fprintf(fp_result, "PRN_1 - 最佳起始點: %d, 最佳相關性: %d\n", best_start_point_1, best_correlation_1);
    fprintf(fp_result, "PRN_3 - 最佳起始點: %d, 最佳相關性: %d\n", best_start_point_3, best_correlation_3);

    //將I軸解調結果寫入檔案
    fprintf(fp_result, "\n最佳旋轉角度時的I軸QPSK解調結果:\n");
    for(int i = 0; i < LOADED_DATA_SIZE; i++) {
        if(i % 20 == 0 && i != 0) {
            fprintf(fp_result, "\n");
        }
        fprintf(fp_result, "%d", I_QPSK_demod[i]);
        if((i + 1) % 20 != 0 && i != LOADED_DATA_SIZE - 1) {
            fprintf(fp_result, " ");
        }
    }
    fprintf(fp_result, "\n");

    fclose(fp_result);
    printf("結果已儲存至 '%s'\n", filename_result);

    // 釋放記憶體
    free(i_data);
    free(q_data);
    free(I_rot);
    free(Q_rot);
    free(I_QPSK_demod);
    free(Q_QPSK_demod);
    free(final_data);
    
    system("pause");
    return 0;
}

int recover_data(int *I_QPSK_demod, int *PRN, int data_size, int prn_length, int start_point) {
    int *dot_product_data = (int *)malloc(prn_length * sizeof(int));
    int *bipolar_I_data   = (int *)malloc(prn_length * sizeof(int));
    int *bipolar_prn      = (int *)malloc(prn_length * sizeof(int));
    int correlation = 0;

    if (!dot_product_data || !bipolar_I_data || !bipolar_prn) {
        fprintf(stderr, "recover_data 記憶體配置失敗\n");
        free(dot_product_data);
        free(bipolar_I_data);
        free(bipolar_prn);
        return 0;
    }

    // 轉換 I_QPSK_demod，只取 prn_length 個，並檢查越界
    for (int i = 0; i < prn_length; i++) {
        int idx = start_point + i;
        if (idx >= data_size) {
            // 如果超出範圍，直接填 0 或適合的預設值
            bipolar_I_data[i] = 0;
        } else {
            bipolar_I_data[i] = (I_QPSK_demod[idx] == 0) ? 1 : -1;
        }
    }

    // 轉換 PRN
    for (int i = 0; i < prn_length; i++) {
        bipolar_prn[i] = (PRN[i] == 0) ? 1 : -1;
    }

    // 計算內積
    for (int i = 0; i < prn_length; i++) {
        dot_product_data[i] = bipolar_I_data[i] * bipolar_prn[i];
        correlation += dot_product_data[i];
    }
    printf("相關性: %d\n", correlation);
    free(dot_product_data);
    free(bipolar_I_data);
    free(bipolar_prn);

    return correlation;
}

// 旋轉資料函式
void rotate_data(double *i_data, double *q_data, double *I_rot, double *Q_rot, double angle, int size, int clockwise) {
    double theta = angle * PI / 180.0;
    
    for(int i = 0; i < size; i++) {
        if (clockwise == 0) { // 逆時針旋轉
            I_rot[i] = i_data[i] * cos(theta) - q_data[i] * sin(theta);
            Q_rot[i] = q_data[i] * cos(theta) + i_data[i] * sin(theta);
        } else { // 順時針旋轉
            I_rot[i] = i_data[i] * cos(theta) + q_data[i] * sin(theta);
            Q_rot[i] = q_data[i] * cos(theta) - i_data[i] * sin(theta);
        }
    }
}

// QPSK解調函式
void qpsk_demodulation(double *i_data, double *q_data, int *I_QPSK_demod, int *Q_QPSK_demod, int size) {
    for(int i = 0; i < size; i++) {
        double I = i_data[i];
        double Q = q_data[i];
        if (I >= 0 && Q > 0) {        // 第一象限或I軸上方
            I_QPSK_demod[i] = 1;
            Q_QPSK_demod[i] = 1;
        } else if (I < 0 && Q >= 0) { // 第二象限或Q軸右側
            I_QPSK_demod[i] = 0;
            Q_QPSK_demod[i] = 1;
        } else if (I <= 0 && Q < 0) { // 第三象限或I軸下方
            I_QPSK_demod[i] = 0;
            Q_QPSK_demod[i] = 0;
        } else if (I > 0 && Q <= 0) { // 第四象限或Q軸左側
            I_QPSK_demod[i] = 1;
            Q_QPSK_demod[i] = 0;
        }
    }
}
// 找出最佳起始點的函式，並返回最佳相關性
int find_start_point(int *I_QPSK_demod, int *PRN, int data_size, int prn_length, int *best_correlation_out) {
    int best_correlation = -data_size;  // 初始化為最小可能值
    int best_shift = -1;
    //int correlation = 0;
    int *dot_product_data = (int *)malloc(data_size * sizeof(int));
    int *bipolar_I_data = (int *)malloc(data_size * sizeof(int));
    int *bipolar_prn = (int *)malloc(prn_length * sizeof(int));
    int *correlationcoeff = (int *)malloc(data_size * sizeof(int));

    if (dot_product_data == NULL || bipolar_I_data == NULL || bipolar_prn == NULL || correlationcoeff == NULL) {
    fprintf(stderr, "記憶體配置失敗\n");
    free(dot_product_data);
    free(bipolar_I_data);
    free(bipolar_prn);
    free(correlationcoeff);
    *best_correlation_out = -1;
    return -1;
}
    
    // 複製並轉換資料 (避免修改原始陣列)
    for (int i = 0; i < data_size; i++) {
        bipolar_I_data[i] = (I_QPSK_demod[i] == 0) ? 1 : -1;
    }
    
    for (int i = 0; i < prn_length; i++) {
        bipolar_prn[i] = (PRN[i] == 0) ? 1 : -1;
    }

    // 進行位移並計算相關性
    for (int shift = 0; shift <= data_size - prn_length ; shift++) {
        int correlation = 0;
        // 建立位移後的PRN序列
        for (int i = 0; i < prn_length; i++) {
        dot_product_data[i] = bipolar_I_data[shift + i] * bipolar_prn[i];
            // if(shift + i >= prn_length){
            //     dot_product_data[i] = bipolar_I_data[(shift + i) % prn_length] * bipolar_prn[i];
            // }
        }
        for (int j = 0 ; j < prn_length; j++) {
            correlation += dot_product_data[j];
            correlationcoeff[j] = correlation; // 儲存相關性結果
        }
        
        // 更新最佳相關性和起始點
        if (correlation > best_correlation) {
            best_correlation = correlation;
            best_shift = shift;
        }
    }
    
    free(dot_product_data);
    *best_correlation_out = best_correlation;
    return best_shift;
}