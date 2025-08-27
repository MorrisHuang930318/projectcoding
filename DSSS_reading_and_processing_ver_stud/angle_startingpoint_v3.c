#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <string.h>
#define PI 3.14159265358979323846
#define LOADED_DATA_SIZE 50000
#define THRESHOLD 30

int calculate_correlation(int *dataArray1, int *dataArray2, int size);
int find_start_point(int *I_QPSK_demod, const int *PRN, int data_size, int prn_length, int *best_correlation_out);
void calculate_all_correlations(int *I_QPSK_demod, const int *PRN, int data_size, int prn_length, int **correlation_table, int angle_index);
void qpsk_demodulation(double *i_data, double *q_data, int *I_QPSK_demod, int *Q_QPSK_demod, int size);
void rotate_data(double *i_data, double *q_data, double *I_rot, double *Q_rot, double angle, int size, int clockwise);
int recover_data(int *I_QPSK_demod, const int *PRN,int data_size, int prn_length,int start_point);

typedef struct {
    const int *prn;     // 指向被選中的 PRN_1 或 PRN_3
    int  prn_id;        // 1 或 3（僅方便印訊息時辨識）
    double angle_deg;   // 最終細調角度（度）
    int  direction;     // 0: 逆時針, 1: 順時針
    int  start_point;   // 最佳起始點
    int  correlation;   // 對應的最大相關性
} TuneResult;

TuneResult coarse_fine_tune(
    double *i_data, double *q_data, double *I_rot, double *Q_rot,
    int *I_QPSK_demod, int *Q_QPSK_demod,
    const int *PRN_1, const int *PRN_3,
    int data_size, int prn_len,
    double coarse_step_deg, double fine_span_deg, double fine_step_deg
);
// 新增：細調函式
//void fine_tune_angle(double *i_data, double *q_data, double *I_rot, double *Q_rot, int *I_QPSK_demod, int *Q_QPSK_demod, int *PRN_1, int *PRN_3,double best_angle, int best_direction, FILE *fp_result);

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
    FILE *file = fopen("data_complex_output.txt", "r");
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
    double *I_rot =  (double *)malloc(num_rows * sizeof(double));
    double *Q_rot =  (double *)malloc(num_rows * sizeof(double));
    int *I_QPSK_demod = (int *)malloc(num_rows * sizeof(int));
    int *Q_QPSK_demod = (int *)malloc(num_rows * sizeof(int));
    int *final_data  =  (int *)malloc(num_rows * sizeof(int));
    if (i_data == NULL || q_data == NULL || I_rot == NULL || Q_rot == NULL || I_QPSK_demod == NULL || Q_QPSK_demod == NULL || final_data == NULL) {
        fprintf(stderr, "記憶體配置失敗\n");
        return 1;
    }
    else{
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

    // === 新增：分配二維相關性表格記憶體 (包括逆時針和順時針) ===
    int num_angles = 37;  // 逆時針0-180度(19個) + 順時針10-180度(18個) = 37個角度
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
    int best_correlation_1 = -LOADED_DATA_SIZE, best_correlation_3 = -LOADED_DATA_SIZE;  // 修正：初始化為最小可能值
    double best_angle_1 = 0    , best_angle_3 = 0;
    int best_direction_1 = 0   , best_direction_3 = 0;    // 0: 逆時針, 1: 順時針
    int angle_index = 0;
    //coarse_tuning_stage:
    // === 粗調：角度索引 === // 逆時針旋轉 (0到180度)
    printf("\n=== 逆時針旋轉 ===\n");
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
            best_angle_3 = angle;
            best_direction_3 = 0;
        }
    }
    
    // === 粗調：角度索引 === // 順時針旋轉 (10到180度，避免重複0度)
    printf("\n=== 順時針旋轉 ===\n");
    for(double angle = 10; angle <= 180; angle += 10) {
        rotate_data(i_data, q_data, I_rot, Q_rot, angle, LOADED_DATA_SIZE, 1);// 順時針旋轉資料
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);// QPSK解調 
        
        int correlation_1, correlation_3;
        int start_point_1 = find_start_point(I_QPSK_demod, PRN_1, LOADED_DATA_SIZE, 255, &correlation_1); // 與PRN_1做相關性分析
        int start_point_3 = find_start_point(I_QPSK_demod, PRN_3, LOADED_DATA_SIZE, 255, &correlation_3); // 與PRN_3做相關性分析
        
        // === 計算並填入二維表格 ===
        calculate_all_correlations(I_QPSK_demod, PRN_1, LOADED_DATA_SIZE, 255, correlation_table_prn1, angle_index);
        calculate_all_correlations(I_QPSK_demod, PRN_3, LOADED_DATA_SIZE, 255, correlation_table_prn3, angle_index);
        angle_index++;
        // === 結束 ===
        
        // 記錄結果
        printf("順時針旋轉 %.0f度: PRN_1(起始點:%d, 相關性:%d), PRN_3(起始點:%d, 相關性:%d)\n", angle, start_point_1, correlation_1, start_point_3, correlation_3);
        fprintf(fp_result, "%.0f\t\t順時針\t\t%d\t\t%d\t\t%d\t\t%d\n", angle, start_point_1, correlation_1, start_point_3, correlation_3);
        
        // 更新最佳PRN_1結果
        if (correlation_1 > best_correlation_1) {
            best_correlation_1 = correlation_1;
            best_start_point_1 = start_point_1;
            best_angle_1 = angle;
            best_direction_1 = 1;  // 順時針
        }
        
        // 更新最佳PRN_3結果
        if (correlation_3 > best_correlation_3) {
            best_correlation_3 = correlation_3;
            best_start_point_3 = start_point_3;
            best_angle_3 = angle;
            best_direction_3 = 1;  // 順時針
        }
    }

    // === 新增：從二維表格中找出全域最佳結果 ===
    int global_best_correlation_prn1 = -LOADED_DATA_SIZE;
    int global_best_correlation_prn3 = -LOADED_DATA_SIZE;
    int global_best_shift_prn1 = -1, global_best_angle_idx_prn1 = -1;
    int global_best_shift_prn3 = -1, global_best_angle_idx_prn3 = -1;
    
    for (int shift = 0; shift < num_shifts; shift++) {
        for (int angle_idx = 0; angle_idx < num_angles; angle_idx++) {
            if (correlation_table_prn1[shift][angle_idx] > global_best_correlation_prn1) {
                global_best_correlation_prn1 = correlation_table_prn1[shift][angle_idx];
                global_best_shift_prn1 = shift;
                global_best_angle_idx_prn1 = angle_idx;
            }
            if (correlation_table_prn3[shift][angle_idx] > global_best_correlation_prn3) {
                global_best_correlation_prn3 = correlation_table_prn3[shift][angle_idx];
                global_best_shift_prn3 = shift;
                global_best_angle_idx_prn3 = angle_idx;
            }
        }
    }
    
    // 將角度索引轉換為實際角度和方向
    double global_best_angle_prn1;
    char global_best_direction_prn1[10];
    double global_best_angle_prn3;
    char global_best_direction_prn3[10];
    int global_best_direction_flag_prn1, global_best_direction_flag_prn3;
    
    if (global_best_angle_idx_prn1 < 19) {  // 逆時針 0-180度
        global_best_angle_prn1 = global_best_angle_idx_prn1 * 10;
        strcpy(global_best_direction_prn1, "逆時針");
        global_best_direction_flag_prn1 = 0;
    } else {  // 順時針 10-180度
        global_best_angle_prn1 = (global_best_angle_idx_prn1 - 18) * 10;
        strcpy(global_best_direction_prn1, "順時針");
        global_best_direction_flag_prn1 = 1;
    }
    
    if (global_best_angle_idx_prn3 < 19) {  // 逆時針 0-180度
        global_best_angle_prn3 = global_best_angle_idx_prn3 * 10;
        strcpy(global_best_direction_prn3, "逆時針");
        global_best_direction_flag_prn3 = 0;
    } else {  // 順時針 10-180度
        global_best_angle_prn3 = (global_best_angle_idx_prn3 - 18) * 10;
        strcpy(global_best_direction_prn3, "順時針");
        global_best_direction_flag_prn3 = 1;
    }
    
    // 在控制台印出最佳結果
    printf("\n========== 全域最佳結果 ==========\n");
    printf("PRN_1: 最佳角度 = %.0f度(%s), 最佳位移 = %d, 最佳相關性 = %d\n", global_best_angle_prn1, global_best_direction_prn1, global_best_shift_prn1, global_best_correlation_prn1);
    printf("PRN_3: 最佳角度 = %.0f度(%s), 最佳位移 = %d, 最佳相關性 = %d\n", global_best_angle_prn3, global_best_direction_prn3, global_best_shift_prn3, global_best_correlation_prn3);
    
    // 將最佳結果寫入 rotation_correlation_result.txt
    fprintf(fp_result, "\n========== 全域最佳結果 ==========\n");
    fprintf(fp_result, "PRN_1: 最佳角度 = %.0f度(%s), 最佳位移 = %d, 最佳相關性 = %d\n",  global_best_angle_prn1, global_best_direction_prn1, global_best_shift_prn1, global_best_correlation_prn1);
    fprintf(fp_result, "PRN_3: 最佳角度 = %.0f度(%s), 最佳位移 = %d, 最佳相關性 = %d\n", global_best_angle_prn3, global_best_direction_prn3, global_best_shift_prn3, global_best_correlation_prn3);

    // === 新增：選擇最佳PRN進行細調 ===
    double final_best_angle;
    int final_best_direction;
    const int *selected_prn = NULL;
    int selected_prn_id = 0;
    
    if (global_best_correlation_prn1 >= global_best_correlation_prn3) {
        printf("\n選擇 PRN_1 進行細調 (相關性: %d)\n", global_best_correlation_prn1);
        fprintf(fp_result, "\n選擇 PRN_1 進行細調 (相關性: %d)\n", global_best_correlation_prn1);
        selected_prn = PRN_1;
        selected_prn_id = 1;
        final_best_angle = global_best_angle_prn1;
        final_best_direction = global_best_direction_flag_prn1;
    } else {
        printf("\n選擇 PRN_3 進行細調 (相關性: %d)\n", global_best_correlation_prn3);
        fprintf(fp_result, "\n選擇 PRN_3 進行細調 (相關性: %d)\n", global_best_correlation_prn3);
        selected_prn = PRN_3;
        selected_prn_id = 3;
        final_best_angle = global_best_angle_prn3;
        final_best_direction = global_best_direction_flag_prn3; 
    }
    //fine_tuning_stage:
    // === 進行細調 ===
    printf("\n========== 開始細調 ==========\n");
    int fine_best_correlation = -LOADED_DATA_SIZE;
    int fine_best_start_point = -1;
    const char* direction_str = (final_best_direction == 0) ? "逆時針" : "順時針";
    double fine_best_angle = final_best_angle;
    printf("在 %.0f度(%s) 的 ±10度範圍內進行細調...\n", final_best_angle, direction_str);
    //fprintf(fp_result, "\n========== 開始細調 ==========\n");
    for(double angle = final_best_angle - 10; angle <= final_best_angle + 10; angle += 1.0) {
        if(angle < 0 || angle > 180) continue;
        rotate_data(i_data, q_data, I_rot, Q_rot, angle, LOADED_DATA_SIZE, final_best_direction);// 旋轉資料
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);// QPSK解調 
        
        int correlation, start_point;
        // if(final_best_direction == global_best_direction_flag_prn1) {
        //     start_point = find_start_point(I_QPSK_demod, PRN_1, LOADED_DATA_SIZE, 255, &correlation);
        // } else {
        //     start_point = find_start_point(I_QPSK_demod, PRN_3, LOADED_DATA_SIZE, 255, &correlation);
        // }
        start_point = find_start_point(I_QPSK_demod, selected_prn, LOADED_DATA_SIZE, 255, &correlation);
        printf("細調角度 %.1f度(%s): 起始點:%d, 相關性:%d\n", angle, direction_str, start_point, correlation);
        if(correlation > fine_best_correlation) {
        fine_best_correlation = correlation;
        fine_best_start_point = start_point;
        fine_best_angle = angle;
        }
    }
    printf("\n========== 細調後最佳結果 ==========\n");
    printf("最佳細調角度: %.1f度(%s)\n", fine_best_angle, direction_str);
    printf("起始點 = %d, 相關性 = %d\n", fine_best_start_point, fine_best_correlation);

    // === 新增：輸出二維相關性表格 ===
    // 輸出 PRN_1 相關性表格到檔案
    FILE *fp_prn1 = fopen("PRN1_correlation_table.txt", "w");
    if (fp_prn1 != NULL) {
        // 寫入表頭 - 包括逆時針和順時針角度
        fprintf(fp_prn1, "Shift\\Angle");
        // 逆時針 0-180度
        for (int angle = 0; angle <= 180; angle += 10) {
            fprintf(fp_prn1, "\tCCW%d", angle);  // CCW = Counter-Clockwise
        }
        // 順時針 10-180度
        for (int angle = 10; angle <= 180; angle += 10) {
            fprintf(fp_prn1, "\tCW%d", angle);   // CW = Clockwise
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
        // 寫入表頭 - 包括逆時針和順時針角度
        fprintf(fp_prn3, "Shift\\Angle");
        // 逆時針 0-180度
        for (int angle = 0; angle <= 180; angle += 10) {
            fprintf(fp_prn3, "\tCCW%d", angle);  // CCW = Counter-Clockwise
        }
        // 順時針 10-180度
        for (int angle = 10; angle <= 180; angle += 10) {
            fprintf(fp_prn3, "\tCW%d", angle);   // CW = Clockwise
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
    
    //255-sliding window  // 還原資料  
    FILE *fp_final_result = fopen("final_result.txt", "w");
    printf("\n========== 還原資料 ==========\n");
    int final_correlation = 0;
    printf("呼叫 recover_data，參數: start_point=%d\n", fine_best_start_point);
    
    int data_segments =(LOADED_DATA_SIZE - fine_best_start_point) / 255;
    int idx = fine_best_start_point;
    printf("THRESHOLD: %d\n", THRESHOLD);
    fprintf(fp_final_result, "THRESHOLD: %d\n", THRESHOLD);
    printf("資料段數: %d\n", data_segments); 
    fprintf(fp_final_result, "資料段數: %d\n", data_segments);
    rotate_data(i_data, q_data, I_rot, Q_rot, abs((int)fine_best_angle), LOADED_DATA_SIZE, final_best_direction);
    qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);
    
    for(int segment = 0; segment < data_segments; segment++) {
        final_correlation = recover_data(I_QPSK_demod, selected_prn, LOADED_DATA_SIZE, 255 , idx);
        
        // 修正：檢查絕對值是否大於閾值，然後用正負號判斷1/0
        if(abs(final_correlation) >= THRESHOLD){
            final_data[segment] = (final_correlation > 0) ? 1 : 0;
        }else{
            printf("相關性不足，嘗試微調 (Segment: %d, 起始點 %d, 相關性 %d)\n", segment, idx, final_correlation);
            fprintf(fp_final_result, "相關性不足，嘗試微調 (Segment: %d, 起始點 %d, 相關性 %d)\n", segment, idx, final_correlation);
            int corr_prev = recover_data(I_QPSK_demod, selected_prn, LOADED_DATA_SIZE, 255 , idx - 1); //前移一位
            int corr_next = recover_data(I_QPSK_demod, selected_prn, LOADED_DATA_SIZE, 255 , idx + 1); //後移一位
            
            // 修正：檢查前一位和後一位的相關性
            if (abs(corr_prev) >= THRESHOLD && abs(corr_prev) > abs(corr_next)){
                idx = idx - 1; 
                final_correlation = corr_prev;
                final_data[segment] = (corr_prev > 0) ? 1 : 0;
            }else if (abs(corr_next) >= THRESHOLD && abs(corr_next) > abs(corr_prev)){
                idx = idx + 1; 
                final_correlation = corr_next;
                final_data[segment] = (corr_next > 0) ? 1 : 0;
            }else {
                int best_local_corr = final_correlation;
                int best_local_idx  = idx;
                double best_local_angle = fine_best_angle;

                for (int dshift = -8; dshift <= 8; dshift++) {
                    int cand_idx = idx + dshift;
                    if (cand_idx < 0) continue;

                    for (double ddeg = -2.0; ddeg <= 2.0; ddeg += 0.5) {
                        rotate_data(i_data, q_data, I_rot, Q_rot,fine_best_angle + ddeg, LOADED_DATA_SIZE, final_best_direction);
                        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);
                        int corr = recover_data(I_QPSK_demod, selected_prn, LOADED_DATA_SIZE, 255, cand_idx);
                        if (abs(corr) > abs(best_local_corr)) {
                            best_local_corr = corr;
                            best_local_idx = cand_idx;
                            best_local_angle = fine_best_angle + ddeg;
                        }
                    }
                }
                if (abs(best_local_corr) > abs(final_correlation)) {
                    idx = best_local_idx;
                    final_correlation = best_local_corr;
                    fine_best_angle = best_local_angle; // 視需要更新
                    printf("局部微調成功：idx=%d, angle=%.1f, corr=%d\n", idx, best_local_angle, final_correlation);
                    fprintf(fp_final_result, "局部微調成功：idx=%d, angle=%.1f, corr=%d\n", idx, best_local_angle, final_correlation);
                } else {
                    // 粗＋細調一次，嘗試重新定錨角度/方向/PRN/起始點
                    TuneResult tr = coarse_fine_tune(i_data, q_data, I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod,PRN_1, PRN_3, LOADED_DATA_SIZE, 255,
                        10.0,   // coarse_step_deg
                        10.0,   // fine_span_deg
                        1.0     // fine_step_deg
                    );

                    // 更新「全域」選擇（之後段落用得到）
                    selected_prn = tr.prn;
                    selected_prn_id = tr.prn_id;
                    fine_best_angle = tr.angle_deg;
                    final_best_direction = tr.direction;

                    // 立刻用新角度重做一次 rotate+demod，並在「當前 idx、idx±1」嘗試 recover
                    rotate_data(i_data, q_data, I_rot, Q_rot, fine_best_angle, LOADED_DATA_SIZE, final_best_direction);
                    qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);

                    int c0 = recover_data(I_QPSK_demod, selected_prn, LOADED_DATA_SIZE, 255, idx);
                    int cp = recover_data(I_QPSK_demod, selected_prn, LOADED_DATA_SIZE, 255, idx-1);
                    int cn = recover_data(I_QPSK_demod, selected_prn, LOADED_DATA_SIZE, 255, idx+1);

                    final_correlation = c0; int best_idx = idx;
                    if (abs(cp) > abs(final_correlation)){ final_correlation = cp; best_idx = idx-1; }
                    if (abs(cn) > abs(final_correlation)){ final_correlation = cn; best_idx = idx+1; }
                    idx = best_idx;

                    if (abs(final_correlation) < THRESHOLD) {
                        printf("粗+細調後仍不足（corr=%d），保留 0 並前進。\n", final_correlation);
                        final_data[segment] = 0; // 明確設定為0
                    } else {
                        printf("粗+細調成功：angle=%.1f°, dir=%s, idx=%d, corr=%d\n",fine_best_angle, (final_best_direction? "順" : "逆"), idx, final_correlation);
                    }
                }
                
                // 最終根據相關性的正負號和閾值來決定資料值
                if(abs(final_correlation) >= THRESHOLD){
                    final_data[segment] = (final_correlation > 0) ? 1 : 0;
                }else{
                    final_data[segment] = 0; // 相關性不足時預設為0
                }
            }
        }
        
        printf("段 %d: 起始點 = %d, 相關性 = %d, 還原結果 = %d\n", segment, idx, final_correlation, final_data[segment]);
        fprintf(fp_final_result, "段 %d: 起始點 = %d, 相關性 = %d, 還原結果 = %d\n", segment, idx, final_correlation, final_data[segment]);
        idx += 255; 
    }
    printf("還原結果:\n");
    fprintf(fp_final_result, "還原結果:\n");
    for(int segment = 0; segment < data_segments; segment++) {
        printf("%d", final_data[segment]);
        fprintf(fp_final_result, "%d", final_data[segment]);
    }

    fclose(fp_final_result);
    // 釋放表格記憶體
    for (int j = 0; j < num_shifts; j++) {
        free(correlation_table_prn1[j]);
        free(correlation_table_prn3[j]);
    }
    //cleanup:
    // 釋放記憶體
    free(correlation_table_prn1);
    free(correlation_table_prn3);
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

int recover_data(int *I_QPSK_demod, const int *PRN, int data_size, int prn_length, int start_point) {
    int correlation = 0;
    for (int i = 0; i < prn_length; i++) {
        int idx = start_point + i;
        int bi = (idx >= data_size) ? 0 : ((I_QPSK_demod[idx] == 0) ? 1 : -1);
        int bp = (PRN[i] == 0) ? 1 : -1;
        correlation += bi * bp;
    }
    return correlation;
}

// 旋轉資料函式
void rotate_data(double *i_data, double *q_data, double *I_rot, double *Q_rot, double angle, int size, int clockwise) {
    double theta = angle * PI / 180.0;   //轉徑度
    
    for(int i = 0; i < size; i++) {
        if (clockwise == 0) {    // counterclockwise 逆時針旋轉
            I_rot[i] = i_data[i] * cos(theta) - q_data[i] * sin(theta);
            Q_rot[i] = q_data[i] * cos(theta) + i_data[i] * sin(theta);
        } else {                 // clockwise 順時針旋轉
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
        if (I >= 0 && Q >= 0) {       // 第一象限或I軸上方
            I_QPSK_demod[i] = 1;
            Q_QPSK_demod[i] = 1;
        } else if (I < 0 && Q >= 0) { // 第二象限或Q軸右側
            I_QPSK_demod[i] = 0;
            Q_QPSK_demod[i] = 1;
        } else if (I < 0 && Q < 0) {  // 第三象限或I軸下方
            I_QPSK_demod[i] = 0;
            Q_QPSK_demod[i] = 0;
        } else {//if (I > 0 && Q <= 0) { // 第四象限或Q軸左側
            I_QPSK_demod[i] = 1;
            Q_QPSK_demod[i] = 0;
        }
    }
}

int find_start_point(int *I_QPSK_demod, const int *PRN, int data_size, int prn_length, int *best_correlation_out) {
    int best_correlation = -data_size;  // 初始化為最小可能值
    int best_shift = -1;
    int *bipolar_I_data = (int *)malloc(data_size * sizeof(int));
    int *bipolar_prn = (int *)malloc(prn_length * sizeof(int));

    if (!bipolar_I_data || !bipolar_prn) {
        fprintf(stderr, "記憶體配置失敗\n");
        if (bipolar_I_data) free(bipolar_I_data);
        if (bipolar_prn) free(bipolar_prn);
        *best_correlation_out = -1;
        return -1;
    }
    
    // 轉換為雙極性信號 (避免修改原始陣列)
    for (int i = 0; i < data_size; i++) {
        bipolar_I_data[i] = (I_QPSK_demod[i] == 0) ? 1 : -1;
    }
    for (int i = 0; i < prn_length; i++) {
        bipolar_prn[i] = (PRN[i] == 0) ? 1 : -1;
    }

    // 修正：限制搜尋範圍在前500以內
    int max_shift = 500;  // 限制搜尋範圍
    if (max_shift > data_size - prn_length) {
        max_shift = data_size - prn_length;  // 確保不超出資料範圍
    }

    // 進行位移並計算相關性 - 修正：只搜尋前500個位置
    for (int shift = 0; shift <= max_shift; shift++) {
        int correlation = 0;
        for (int i = 0; i < prn_length; i++) {
            correlation += bipolar_I_data[shift + i] * bipolar_prn[i];
        }
        
        // 更新最佳相關性和起始點
        if (correlation > best_correlation) {
            best_correlation = correlation;
            best_shift = shift;
        }
    }
    
    free(bipolar_I_data);
    free(bipolar_prn);
    *best_correlation_out = best_correlation;
    return best_shift;
}

// === 新增函式：計算所有位移的相關性並填入表格 ===
void calculate_all_correlations(int *I_QPSK_demod, const int *PRN, int data_size, int prn_length, int **correlation_table, int angle_index) {
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
// 粗調步階、細調範圍與步階都可傳參數調整
TuneResult coarse_fine_tune(double *i_data, double *q_data , double *I_rot,  double *Q_rot , int *I_QPSK_demod, int *Q_QPSK_demod , const int *PRN_1, const int *PRN_3,
    int data_size, int prn_len , double coarse_step_deg , double fine_span_deg ,  double fine_step_deg){

    // --- 粗調：同時找 PRN_1 / PRN_3 的最佳角度、方向、起始點 ---
    int best_corr_1 = -data_size, best_corr_3 = -data_size;
    int best_sp_1 = -1, best_sp_3 = -1;
    double best_ang_1 = 0.0, best_ang_3 = 0.0;
    int best_dir_1 = 0, best_dir_3 = 0; // 0:CCW, 1:CW

    // CCW: 0..180
    for(double ang = 0; ang <= 180.0; ang += coarse_step_deg){
        rotate_data(i_data, q_data, I_rot, Q_rot, ang, data_size, 0);
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, data_size);

        int c1, c3, sp1, sp3;
        sp1 = find_start_point(I_QPSK_demod, PRN_1, data_size, prn_len, &c1);
        sp3 = find_start_point(I_QPSK_demod, PRN_3, data_size, prn_len, &c3);

        if (c1 > best_corr_1){ best_corr_1 = c1; best_sp_1 = sp1; best_ang_1 = ang; best_dir_1 = 0; }
        if (c3 > best_corr_3){ best_corr_3 = c3; best_sp_3 = sp3; best_ang_3 = ang; best_dir_3 = 0; }
    }
    // CW: 10..180（避免 0 度重複）
    for(double ang = coarse_step_deg; ang <= 180.0; ang += coarse_step_deg){
        rotate_data(i_data, q_data, I_rot, Q_rot, ang, data_size, 1);
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, data_size);

        int c1, c3, sp1, sp3;
        sp1 = find_start_point(I_QPSK_demod, PRN_1, data_size, prn_len, &c1);
        sp3 = find_start_point(I_QPSK_demod, PRN_3, data_size, prn_len, &c3);

        if (c1 > best_corr_1){ best_corr_1 = c1; best_sp_1 = sp1; best_ang_1 = ang; best_dir_1 = 1; }
        if (c3 > best_corr_3){ best_corr_3 = c3; best_sp_3 = sp3; best_ang_3 = ang; best_dir_3 = 1; }
    }

    // 選 PRN
    const int *sel_prn; int sel_id; double base_ang; int base_dir; int base_sp; int base_corr;
    if (best_corr_1 >= best_corr_3){
        sel_prn = PRN_1; sel_id = 1; base_ang = best_ang_1; base_dir = best_dir_1; base_sp = best_sp_1; base_corr = best_corr_1;
    } else {
        sel_prn = PRN_3; sel_id = 3; base_ang = best_ang_3; base_dir = best_dir_3; base_sp = best_sp_3; base_corr = best_corr_3;
    }

    // --- 細調：以選到的 PRN，圍繞 base_ang 做 ±fine_span_deg 的掃描 ---
    int fine_best_corr = -data_size, fine_best_sp = -1;
    double fine_best_ang = base_ang;

    double a0 = base_ang - fine_span_deg;
    if (a0 < 0.0) a0 = 0.0;
    double a1 = base_ang + fine_span_deg;
    if (a1 > 180.0) a1 = 180.0;

    for(double ang = a0; ang <= a1 + 1e-9; ang += fine_step_deg){
        rotate_data(i_data, q_data, I_rot, Q_rot, ang, data_size, base_dir);
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, data_size);

        int c, sp;
        sp = find_start_point(I_QPSK_demod, sel_prn, data_size, prn_len, &c);

        if (c > fine_best_corr){ fine_best_corr = c; fine_best_sp = sp; fine_best_ang = ang; }
    }

    TuneResult ret;
    ret.prn = sel_prn;
    ret.prn_id = sel_id;
    ret.angle_deg = fine_best_ang;
    ret.direction = base_dir;          // 細調只在 base_dir 內做
    ret.start_point = fine_best_sp;
    ret.correlation = fine_best_corr;
    return ret;
}
