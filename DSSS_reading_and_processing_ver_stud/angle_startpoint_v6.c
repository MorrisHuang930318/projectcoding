#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <string.h>
#define PI 3.14159265358979323846
#define LOADED_DATA_SIZE 100000
#define THRESHOLD 30
#define PRN_LENGTH 255
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

TuneResult coarse_fine_tuning_stage(
    double *i_data, double *q_data, double *I_rot, double *Q_rot, int *I_QPSK_demod, int *Q_QPSK_demod, const int *PRN_1, int data_size, int prn_len);

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
    FILE *file = fopen("data_RRC_matched.txt", "r");
    if (file == NULL) {
        perror("無法開啟檔案");
        return 1;
    }
    printf("檔案開啟成功\n");

    int row_count = 0;
    int col_count = 0;
    char line[32];

    while (fgets(line, sizeof(line), file)) {//先計算行數與欄數
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

    printf("\n開始旋轉與相關性分析...\n");
    
    fclose(file);
    printf("檔案關閉成功\n");
    printf("\n開始旋轉與相關性分析...\n");
    TuneResult tr = coarse_fine_tuning_stage(i_data, q_data, I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod,PRN_1, LOADED_DATA_SIZE ,PRN_LENGTH);
    printf("angle:%f , direction:%s , 起始點:%d , 相關性:%d " , tr.angle_deg , (tr.direction == 0 ? "逆時針":"順時針") , tr.start_point , tr.correlation);
    printf("\n========== 還原資料 ==========\n");
    int final_correlation = 0;
    printf("呼叫 recover_data，參數: start_point=%d\n", tr.start_point);

    int data_segments =(LOADED_DATA_SIZE - tr.start_point) / PRN_LENGTH;
    printf("THRESHOLD: %d\n", THRESHOLD);
    printf("資料段數: %d\n", data_segments);
    rotate_data(i_data, q_data, I_rot, Q_rot, abs(tr.angle_deg), LOADED_DATA_SIZE, tr.direction);
    qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);

    for(int i = 0; i < data_segments; i++){
        int idx = tr.start_point;
        final_correlation = recover_data(I_QPSK_demod, PRN_1, 255, PRN_LENGTH , idx);
        if(abs(final_correlation) >= THRESHOLD){
            final_data[i] = (final_correlation > 0) ? 1 : 0;
        }

        idx += PRN_LENGTH;
    }
    for(int i = 0; i < data_segments; i++){
        printf("%d ", final_data[i]);
    }

    free(i_data);
    free(q_data);
    free(I_rot);
    free(Q_rot);
    free(I_QPSK_demod);
    free(Q_QPSK_demod);
    free(final_data);
    //free(segment_I_data);
    system("pause");
    return 0;
}
int recover_data(int *I_QPSK_demod, const int *PRN, int data_size, int prn_length, int start_point) {
    int correlation = 0;
    for (int i = 0; i < prn_length; i++) {
        int idx = start_point + i;
        int bi = (I_QPSK_demod[idx] == 0) ? 1 : -1;
        int bp = (PRN[i] == 0) ? 1 : -1;
        correlation += bi * bp;
    }
    return correlation;
}

TuneResult coarse_fine_tuning_stage(double *i_data, double *q_data, double *I_rot, double *Q_rot, int *I_QPSK_demod, int *Q_QPSK_demod, const int *PRN_1, int data_size, int prn_len) {
    
    int best_start_point_1 = -1, best_start_point_3 = -1;
    int best_correlation_1 = -LOADED_DATA_SIZE, best_correlation_3 = -LOADED_DATA_SIZE;  // 修正：初始化為最小可能值
    double best_angle_1 = 0    , best_angle_3 = 0;
    int best_direction_1 = 0   , best_direction_3 = 0;    // 0: 逆時針, 1: 順時針
    int angle_index = 0;
    
    printf("\n========== 開始粗調 ==========\n");
    for(double angle = 10; angle <= 180; angle += 10) {
        rotate_data(i_data, q_data, I_rot, Q_rot, angle, LOADED_DATA_SIZE, 0);// 旋轉資料
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);// QPSK解調 
        int correlation_1, correlation_3;
        int start_point_1 = find_start_point(I_QPSK_demod, PRN_1, LOADED_DATA_SIZE, PRN_LENGTH, &correlation_1); // 與PRN_1做相關性分析
        printf("逆時針旋轉 %.0f度: PRN_1(起始點:%d, 相關性:%d)\n", angle, start_point_1, correlation_1);
        //int start_point_3 = find_start_point(I_QPSK_demod, PRN_3, LOADED_DATA_SIZE, PRN_LENGTH, &correlation_3); // 與PRN_3做相關性分析
        //printf("逆時針旋轉 %.0f度: PRN_3(起始點:%d, 相關性:%d)\n", angle, start_point_3, correlation_3);
        if (correlation_1 > best_correlation_1) {
            best_correlation_1 = correlation_1;
            best_start_point_1 = start_point_1;
            best_angle_1 = angle;
            best_direction_1 = 0;
        }
        
        // if (correlation_3 > best_correlation_3) {
        //     best_correlation_3 = correlation_3;
        //     best_start_point_3 = start_point_3;
        //     best_angle_3 = angle;
        //     best_direction_3 = 0;
        // }
    }
    printf("\n");
    for(double angle = 10; angle <= 180; angle += 10) {
        rotate_data(i_data, q_data, I_rot, Q_rot, angle, LOADED_DATA_SIZE, 1);// 旋轉資料
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);// QPSK解調 
        int correlation_1, correlation_3;
        int start_point_1 = find_start_point(I_QPSK_demod, PRN_1, LOADED_DATA_SIZE, PRN_LENGTH, &correlation_1); // 與PRN_1做相關性分析
        printf("順時針旋轉 %.0f度: PRN_1(起始點:%d, 相關性:%d)\n", angle, start_point_1, correlation_1);
        // int start_point_3 = find_start_point(I_QPSK_demod, PRN_3, LOADED_DATA_SIZE, PRN_LENGTH, &correlation_3); // 與PRN_3做相關性分析
        // printf("逆時針旋轉 %.0f度: PRN_3(起始點:%d, 相關性:%d)\n", angle, start_point_3, correlation_3);
        if (correlation_1 > best_correlation_1) {
            best_correlation_1 = correlation_1;
            best_start_point_1 = start_point_1;
            best_angle_1 = angle;
            best_direction_1 = 1;
        }
        // if (correlation_3 > best_correlation_3) {
        //     best_correlation_3 = correlation_3;
        //     best_start_point_3 = start_point_3;
        //     best_angle_3 = angle;
        //     best_direction_3 = 0;
        // }
    }
    printf("\n========== 全域最佳結果 ==========\n");
    printf("PRN_1: 最佳角度 = %.0f度(%s), 最佳位移 = %d, 最佳相關性 = %d\n", best_angle_1, (best_direction_1==0)?"逆時針":"順時針", best_start_point_1, best_correlation_1);
    printf("\n========== 開始細調 ==========\n");
    int fine_best_correlation = best_correlation_1;
    int fine_best_start_point = best_start_point_1;
    int fine_best_direction = best_direction_1;
    double fine_best_angle = best_angle_1;
    printf("在 %.0f度(%s) 的 ±10度範圍內進行細調...\n", best_angle_1, (best_direction_1==0)?"逆時針":"順時針");
    for(double angle = fine_best_angle - 10; angle <= fine_best_angle + 10; angle += 1.0) {
        if(angle < 0 || angle > 180) continue;
        rotate_data(i_data, q_data, I_rot, Q_rot, angle, LOADED_DATA_SIZE, fine_best_direction);// 旋轉資料
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, LOADED_DATA_SIZE);// QPSK解調 
        int correlation, start_point;
        start_point = find_start_point(I_QPSK_demod, PRN_1, LOADED_DATA_SIZE, PRN_LENGTH, &correlation);
        printf("細調角度 %.1f度(%s): 起始點:%d, 相關性:%d\n", angle, (fine_best_direction == 0)?"逆時針":"順時針", start_point, correlation);
        if(correlation > fine_best_correlation) {
        fine_best_correlation = correlation;
        fine_best_start_point = start_point;
        fine_best_angle = angle;
        }
    }
    printf("\n========== 細調後最佳結果 ==========\n");
    printf("最佳細調角度: %.1f度(%s)起始點 = %d, 相關性 = %d\n", fine_best_angle, (best_direction_1==0)?"逆時針":"順時針",fine_best_start_point, fine_best_correlation);
    
    TuneResult tr;
    tr.angle_deg = fine_best_angle;
    tr.direction = best_direction_1;          // 細調只在 base_dir 內做
    tr.start_point = fine_best_start_point;
    tr.correlation = fine_best_correlation;
    return tr;
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
    
    for (int i = 0; i < data_size; i++) {
        bipolar_I_data[i] = (I_QPSK_demod[i] == 0) ? 1 : -1;
    }
    for (int i = 0; i < prn_length; i++) {
        bipolar_prn[i] = (PRN[i] == 0) ? 1 : -1;
    }

    // 進行位移並計算相關性 - 修正：只搜尋前500個位置
    for (int shift = 0; shift <= 510 - prn_length; shift++) {
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