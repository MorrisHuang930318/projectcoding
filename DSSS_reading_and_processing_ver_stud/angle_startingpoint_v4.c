#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <string.h>
#define PI 3.14159265358979323846
#define LOADED_DATA_SIZE 600000
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





    system("pause");
    return 0;
}
