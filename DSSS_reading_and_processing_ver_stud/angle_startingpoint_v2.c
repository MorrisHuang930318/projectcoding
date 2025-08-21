#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <math.h>
#define PI 3.14159265358979323846
#define LOADED_DATA_SIZE 500

int calculate_correlation(int *dataArray1, int *dataArray2, int size);
int find_start_point(int *I_QPSK_demod, int *PRN, int data_size, int prn_length, int *best_correlation_out);
// 新增函式：計算所有位移的相關性並填入表格
void calculate_all_correlations(int *I_QPSK_demod, int *PRN, int data_size, int prn_length, int **correlation_table, int angle_index);
void qpsk_demodulation(double *i_data, double *q_data, int *I_QPSK_demod, int *Q_QPSK_demod, int size);
void rotate_data(double *i_data, double *q_data, double *I_rot, double *Q_rot, double angle, int size, int clockwise);

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

    //先計算行數與欄數
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

    //分配記憶體
    size_t num_rows = row_count;
    double *i_data = (double *)malloc(num_rows * sizeof(double));
    double *q_data = (double *)malloc(num_rows * sizeof(double));
    double *I_rot = (double *)malloc(num_rows * sizeof(double));
    double *Q_rot = (double *)malloc(num_rows * sizeof(double));
    int *I_QPSK_demod = (int *)malloc(num_rows * sizeof(int));
    int *Q_QPSK_demod = (int *)malloc(num_rows * sizeof(int));
    if (i_data == NULL || q_data == NULL || I_rot == NULL || Q_rot == NULL || I_QPSK_demod == NULL || Q_QPSK_demod == NULL) {
        fprintf(stderr, "記憶體配置失敗\n");
        return 1;
    }
    else
        printf("記憶體配置成功\n");
    
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

    // === 新增：分配二維相關性表格記憶體 ===
    int num_angles = 37;  // 逆時針0-180 + 順時針10-180，共37個點
    int num_shifts = LOADED_DATA_SIZE - 255 + 1;  // 可能的位移數量
    
    // 分配二維相關性表格記憶體 - PRN_1
    int **correlation_table_prn1 = (int **)malloc(num_shifts * sizeof(int *));
    for (int j = 0; j < num_shifts; j++) {
        correlation_table_prn1[j] = (int *)malloc(num_angles * sizeof(int));
    }
    
    // 分配二維相關性表格記憶體 - PRN_3
    int **correlation_table_prn3 = (int **)malloc(num_shifts * sizeof(int *));
    for (int j = 0; j < num_shifts; j++) {
        correlation_table_prn3[j] = (int *)malloc(num_angles * sizeof(int));
    }

    if (!correlation_table_prn1 || !correlation_table_prn3) {
        fprintf(stderr, "相關性表格記憶體配置失敗\n");
        return 1;
    }
    // === 新增結束 ===

    // 建立結果檔案rotation_correlation_result.txt
    FILE *fp_result;
    char filename_result[] = "rotation_correlation_result.txt";
    fp_result = fopen(filename_result, "w");
    if (fp_result == NULL) {
        perror("無法開啟結果檔案");
        return 1;
    }

    fprintf(fp_result, "旋轉角度\t方向\t\tPRN_1起始點\tPRN_1相關性\tPRN_3起始點\tPRN_3相關性\n");
    fprintf(fp_result, "========================================================================\n");

    printf("\n開始旋轉與相關性分析...\n");
    int best_start_point_1 = -1, best_start_point_3 = -1;
    int best_correlation_1 = -1, best_correlation_3 = -1;
    double best_angle_1 = 0    , best_angle_3 = 0;
    int best_direction_1 = 0   , best_direction_3 = 0;    // 0: 逆時針, 1: 順時針

    // === 新增：角度索引 ===
    int angle_index = 0;
    
    // 逆時針旋轉 (0到180度)
    for(double angle = 0; angle <= 180; angle += 10) {
        rotate_data(i_data, q_data, I_rot, Q_rot, angle, LOADED_DATA_SIZE, 0);// 旋轉資料
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);// QPSK解調 
        
        int correlation_1, correlation_3;
        int start_point_1 = find_start_point(I_QPSK_demod, PRN_1, LOADED_DATA_SIZE, 255, &correlation_1); // 與PRN_1做相關性分析
        int start_point_3 = find_start_point(I_QPSK_demod, PRN_3, LOADED_DATA_SIZE, 255, &correlation_3); // 與PRN_3做相關性分析
        
        // === 新增：計算並填入二維表格 ===
        calculate_all_correlations(I_QPSK_demod, PRN_1, LOADED_DATA_SIZE, 255, correlation_table_prn1, angle_index);
        calculate_all_correlations(I_QPSK_demod, PRN_3, LOADED_DATA_SIZE, 255, correlation_table_prn3, angle_index);
        angle_index++;
        // === 新增結束 ===
        
        // 記錄結果
        printf("逆時針旋轉 %.0f度: PRN_1(起始點:%d, 相關性:%d), PRN_3(起始點:%d, 相關性:%d)\n", angle, start_point_1, correlation_1, start_point_3, correlation_3);
        fprintf(fp_result, "%.0f\t\t逆時針\t\t%d\t\t%d\t\t%d\t\t%d\n", angle, start_point_1, correlation_1, start_point_3, correlation_3);
        
        // 更新最佳PRN_1結果
        if (correlation_1 > best_correlation_1) {
            best_correlation_1 = correlation_1;
            best_start_point_1 = start_point_1;
            best_angle_1 = angle;
            best_direction_1 = 0;
        }
        
        // 更新最佳PRN_3結果
        if (correlation_3 > best_correlation_3) {
            best_correlation_3 = correlation_3;
            best_start_point_3 = start_point_3;
            if (correlation_3 > best_correlation_1) {
                best_angle_3 = angle;
                best_direction_3 = 0;
            }
        }
    }

    // === 新增：從二維表格中找出全域最佳結果 ===
    int global_best_correlation_prn1 = -LOADED_DATA_SIZE;
    int global_best_correlation_prn3 = -LOADED_DATA_SIZE;
    int global_best_shift_prn1 = -1, global_best_angle_prn1 = -1;
    int global_best_shift_prn3 = -1, global_best_angle_prn3 = -1;
    
    for (int shift = 0; shift < num_shifts; shift++) {
        for (int angle_idx = 0; angle_idx < num_angles; angle_idx++) {
            if (correlation_table_prn1[shift][angle_idx] > global_best_correlation_prn1) {
                global_best_correlation_prn1 = correlation_table_prn1[shift][angle_idx];
                global_best_shift_prn1 = shift;
                if (angle_idx < 19) {  // 逆時針 0-180度
                    global_best_angle_prn1 = angle_idx * 10;
                } else {  // 順時針 10-180度
                    global_best_angle_prn1 = (angle_idx - 18) * 10;  // 順時針角度
                }
            }
            if (correlation_table_prn3[shift][angle_idx] > global_best_correlation_prn3) {
                global_best_correlation_prn3 = correlation_table_prn3[shift][angle_idx];
                global_best_shift_prn3 = shift;
                if (angle_idx < 19) {  // 逆時針 0-180度
                    global_best_angle_prn3 = angle_idx * 10;
                } else {  // 順時針 10-180度
                    global_best_angle_prn3 = (angle_idx - 18) * 10;  // 順時針角度
                }
            }
        }
    }
    
    // 在控制台印出最佳結果
    printf("\n========== 全域最佳結果 ==========\n");
    printf("PRN_1: 最佳角度 = %d度, 最佳位移 = %d, 最佳相關性 = %d\n", global_best_angle_prn1, global_best_shift_prn1, global_best_correlation_prn1);
    printf("PRN_3: 最佳角度 = %d度, 最佳位移 = %d, 最佳相關性 = %d\n", global_best_angle_prn3, global_best_shift_prn3, global_best_correlation_prn3);
    
    // 將最佳結果寫入 rotation_correlation_result.txt
    fprintf(fp_result, "\n========== 全域最佳結果 ==========\n");
    fprintf(fp_result, "PRN_1: 最佳角度 = %d度, 最佳位移 = %d, 最佳相關性 = %d\n", global_best_angle_prn1, global_best_shift_prn1, global_best_correlation_prn1);
    fprintf(fp_result, "PRN_3: 最佳角度 = %d度, 最佳位移 = %d, 最佳相關性 = %d\n", global_best_angle_prn3, global_best_shift_prn3, global_best_correlation_prn3);

    fclose(fp_result);

    // === 新增：輸出二維相關性表格 ===
    // 輸出 PRN_1 相關性表格到檔案
    FILE *fp_prn1 = fopen("PRN1_correlation_table.txt", "w");
    if (fp_prn1 != NULL) {
        // 寫入表頭 - 逆時針和順時針角度
        fprintf(fp_prn1, "Shift\\Angle");
        for (int angle = 0; angle <= 180; angle += 10) {
            fprintf(fp_prn1, "\tCCW_%d", angle);  // CCW = Counter-Clockwise 逆時針
        }
        for (int angle = 10; angle <= 180; angle += 10) {
            fprintf(fp_prn1, "\tCW_%d", angle);   // CW = Clockwise 順時針
        }
        fprintf(fp_prn1, "\n");
        
        // 寫入數據
        for (int shift = 0; shift < num_shifts; shift++) {
            fprintf(fp_prn1, "%d", shift);
            for (int angle_idx = 0; angle_idx < num_angles; angle_idx++) {
                fprintf(fp_prn1, "\t%d", correlation_table_prn1[shift][angle_idx]);
            }
            fprintf(fp_prn1, "\n");
        }
        fclose(fp_prn1);
        printf("PRN_1 相關性表格已輸出到 PRN1_correlation_table.txt\n");
    }

    // 輸出 PRN_3 相關性表格到檔案
    FILE *fp_prn3 = fopen("PRN3_correlation_table.txt", "w");
    if (fp_prn3 != NULL) {
        // 寫入表頭 - 逆時針和順時針角度
        fprintf(fp_prn3, "Shift\\Angle");
        for (int angle = 0; angle <= 180; angle += 10) {
            fprintf(fp_prn3, "\tCCW_%d", angle);  // CCW = Counter-Clockwise 逆時針
        }
        for (int angle = 10; angle <= 180; angle += 10) {
            fprintf(fp_prn3, "\tCW_%d", angle);   // CW = Clockwise 順時針
        }
        fprintf(fp_prn3, "\n");
        
        // 寫入數據
        for (int shift = 0; shift < num_shifts; shift++) {
            fprintf(fp_prn3, "%d", shift);
            for (int angle_idx = 0; angle_idx < num_angles; angle_idx++) {
                fprintf(fp_prn3, "\t%d", correlation_table_prn3[shift][angle_idx]);
            }
            fprintf(fp_prn3, "\n");
        }
        fclose(fp_prn3);
        printf("PRN_3 相關性表格已輸出到 PRN3_correlation_table.txt\n");
    }

    // 釋放表格記憶體
    for (int j = 0; j < num_shifts; j++) {
        free(correlation_table_prn1[j]);
        free(correlation_table_prn3[j]);
    }
    free(correlation_table_prn1);
    free(correlation_table_prn3);
    // === 新增結束 ===

    // 釋放記憶體
    free(i_data);
    free(q_data);
    free(I_rot);
    free(Q_rot);
    free(I_QPSK_demod);
    free(Q_QPSK_demod);
    
    system("pause");
    return 0;
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
            I_QPSK_demod[i] = 0;
            Q_QPSK_demod[i] = 0;
        } else if (I < 0 && Q >= 0) { // 第二象限或Q軸右側
            I_QPSK_demod[i] = 0;
            Q_QPSK_demod[i] = 1;
        } else if (I <= 0 && Q < 0) { // 第三象限或I軸下方
            I_QPSK_demod[i] = 1;
            Q_QPSK_demod[i] = 1;
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
    int *dot_product_data = (int *)malloc(data_size * sizeof(int));
    int *bipolar_I_data = (int *)malloc(data_size * sizeof(int));
    int *bipolar_prn = (int *)malloc(prn_length * sizeof(int));
    int *correlationcoeff = (int *)malloc(data_size * sizeof(int));

    if (!dot_product_data || !bipolar_I_data || !bipolar_prn || !correlationcoeff) {
    fprintf(stderr, "記憶體配置失敗\n");
    if (dot_product_data) free(dot_product_data);
    if (bipolar_I_data) free(bipolar_I_data);
    if (bipolar_prn) free(bipolar_prn);
    if (correlationcoeff) free(correlationcoeff);
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
    for (int shift = 0; shift <= data_size - prn_length; shift++) {
        int correlation = 0;
        //printf("Shift = %d:\n", shift);
        for (int i = 0; i < prn_length; i++) {
            dot_product_data[i] = bipolar_I_data[shift + i] * bipolar_prn[i];
            correlation += dot_product_data[i];
        }
        //printf("product=%d ",correlation);
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

// === 新增函式：計算所有位移的相關性並填入表格 ===
void calculate_all_correlations(int *I_QPSK_demod, int *PRN, int data_size, int prn_length, int **correlation_table, int angle_index) {
    int *bipolar_I_data = (int *)malloc(data_size * sizeof(int));
    int *bipolar_prn = (int *)malloc(prn_length * sizeof(int));

    if (!bipolar_I_data || !bipolar_prn) {
        fprintf(stderr, "記憶體配置失敗\n");
        if (bipolar_I_data) free(bipolar_I_data);
        if (bipolar_prn) free(bipolar_prn);
        return;
    }
    
    // 轉換為雙極性信號
    for (int i = 0; i < data_size; i++) {
        bipolar_I_data[i] = (I_QPSK_demod[i] == 0) ? 1 : -1;
    }
    for (int i = 0; i < prn_length; i++) {
        bipolar_prn[i] = (PRN[i] == 0) ? 1 : -1;
    }

    // 進行位移並計算相關性，同時填入表格
    for (int shift = 0; shift <= data_size - prn_length; shift++) {
        int correlation = 0;
        for (int i = 0; i < prn_length; i++) {
            correlation += bipolar_I_data[shift + i] * bipolar_prn[i];
        }
        
        // 填入相關性表格
        correlation_table[shift][angle_index] = correlation;
    }
    
    free(bipolar_I_data);
    free(bipolar_prn);
}