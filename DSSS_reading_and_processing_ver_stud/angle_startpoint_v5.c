#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <string.h>
#define PI 3.14159265358979323846
#define LOADED_DATA_SIZE 1000
#define THRESHOLD 30
#define PRN_LENGTH 255
#define SEGMENT_SIZE (PRN_LENGTH + 2 * SEARCH_MARGIN)
#define SEARCH_MARGIN 10

int calculate_correlation(int *dataArray1, int *dataArray2, int size);
int find_start_point(int *I_QPSK_demod, const int *PRN, int data_size, int prn_length, int *best_correlation_out);
void calculate_all_correlations(int *I_QPSK_demod, const int *PRN, int data_size, int prn_length, int **correlation_table, int angle_index);
void qpsk_demodulation(double *i_data, double *q_data, int *I_QPSK_demod, int *Q_QPSK_demod, int size);
void rotate_data(double *i_data, double *q_data, double *I_rot, double *Q_rot, double angle, int size, int clockwise);
int recover_data(int *I_QPSK_demod, const int *PRN,int data_size, int prn_length, int start_point);

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

    enum { INITIAL_LEN = 2*PRN_LENGTH, WINDOW_LEN = PRN_LENGTH + 2*SEARCH_MARGIN };
    enum { MAX_BUF = INITIAL_LEN };
    double *i_data = (double *)malloc(MAX_BUF * sizeof(double));
    double *q_data = (double *)malloc(MAX_BUF * sizeof(double));
    double *I_rot = (double *)malloc(MAX_BUF* sizeof(double));
    double *Q_rot = (double *)malloc(MAX_BUF* sizeof(double));
    int *I_QPSK_demod = (int *)malloc(MAX_BUF * sizeof(int));
    int *Q_QPSK_demod = (int *)malloc(MAX_BUF * sizeof(int));
    double *prev_overlap_i = (double *)malloc(SEARCH_MARGIN * sizeof(double));
    double *prev_overlap_q = (double *)malloc(SEARCH_MARGIN * sizeof(double));
    int *final_data = (int *)malloc(SEGMENT_SIZE * sizeof(int));
    if (i_data == NULL || q_data == NULL || I_rot == NULL || Q_rot == NULL || I_QPSK_demod == NULL || Q_QPSK_demod == NULL || final_data == NULL || prev_overlap_i == NULL || prev_overlap_q == NULL) {
        fprintf(stderr, "記憶體配置失敗\n");
        return 1;
    }
    //enum { PRN_LENGTH = PRN_LENGTH, SEARCH_MARGIN = 10 };
   // enum { INITIAL_LEN = 2*PRN_LENGTH, WINDOW_LEN = PRN_LENGTH + 2*SEARCH_MARGIN };
    enum { STEP = WINDOW_LEN - SEARCH_MARGIN }; // 275 - 10 = 265

    int total = LOADED_DATA_SIZE;
    int max_segments = (total >= INITIAL_LEN) ? 1 + (total - INITIAL_LEN) / STEP : 0;

    printf("資料段數: %d\n", max_segments);
    double global_best_angle = 0;
    int global_best_direction = 0;
    int overlap_data_count = 0; 
    int absolute_position = 0;
    int segment_count = 0;
    //int max_segments = LOADED_DATA_SIZE / PRN_LENGTH;
    while(segment_count < max_segments){
        int current_len = (segment_count == 0) ? INITIAL_LEN : WINDOW_LEN;
        int data_read = 0;
        int read_start_idx = 0;
        if (segment_count == 0) {
            printf("第一段：讀取 %d 筆資料進行初始同步...\n", current_len);
            
            for (int i = 0; i < current_len; i++) {
                if (fscanf(file, "%lf %lf", &i_data[i], &q_data[i]) == 2) {
                    data_read++;
                } else {
                    printf("檔案讀取結束，實際讀取 %d 筆資料\n", data_read);
                    break;  // 檔案結束
                }
            }
            if (data_read > 0) {
                printf("第一段讀取完成：%d 筆資料\n", data_read);
                printf("第一筆: I = %.6f, Q = %.6f\n", i_data[0], q_data[0]);
                printf("最後一筆: I = %.6f, Q = %.6f\n", i_data[data_read-1], q_data[data_read-1]);
            }
            
            if (data_read < PRN_LENGTH) {
                printf("資料不足，無法處理 (需要至少 %d 筆)\n", PRN_LENGTH);
                break;
            }
            TuneResult tr = coarse_fine_tuning_stage(i_data, q_data, I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, PRN_1, current_len, PRN_LENGTH);
            printf("angle:%f , direction:%s , 起始點:%d , 相關性:%d \n" , tr.angle_deg , (tr.direction == 0 ? "逆時針":"順時針") , tr.start_point , tr.correlation);
            global_best_angle = tr.angle_deg;
            global_best_direction = tr.direction;
            absolute_position = tr.start_point;

            overlap_data_count = SEARCH_MARGIN;      
            // 第 0 段結尾（coarse+fine 之後）
            int tail = (data_read >= SEARCH_MARGIN) ? SEARCH_MARGIN : data_read;
            overlap_data_count = tail;
            for (int i = 0; i < tail; ++i) {
                int src = data_read - tail + i;   // 用「實際讀到的尾巴」
                prev_overlap_i[i] = i_data[src];
                prev_overlap_q[i] = q_data[src];
            }
            segment_count++;
            continue;
        } else {
            printf("第 %d 段：讀取 %d 筆資料進行微調...\n", segment_count + 1, current_len);
            for (int i = 0; i < overlap_data_count && i < SEARCH_MARGIN; i++) {
                i_data[i] = prev_overlap_i[i];
                q_data[i] = prev_overlap_q[i];
                data_read++;
            }
            //read_start_idx = data_read;
            for (int i = data_read; i < current_len; i++) {
                if (fscanf(file, "%lf %lf", &i_data[i], &q_data[i]) == 2) {
                    data_read++;
                } else {
                    printf("檔案讀取結束，本段實際讀取 %d 筆資料\n", data_read);
                    break;  // 檔案結束
                }
            }
            if (data_read < current_len) {
                printf("本段資料不足，停止處理\n");
                break;
            }
        rotate_data(i_data, q_data, I_rot, Q_rot, global_best_angle, current_len, global_best_direction);
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, current_len);
        int final_correlation = 0;
        int start_idx = SEARCH_MARGIN;
        final_correlation = recover_data(I_QPSK_demod, PRN_1, current_len, PRN_LENGTH, start_idx);
        if(abs(final_correlation) >= THRESHOLD){
            final_data[segment_count] = (final_correlation > 0) ? 1 : 0;
        }else{
            printf("相關性不足，嘗試微調 (Segment: %d, 起始點 %d, 相關性 %d)\n", segment_count, absolute_position + SEARCH_MARGIN, final_correlation);
            int corr_prev = recover_data(I_QPSK_demod, PRN_1, current_len, PRN_LENGTH , start_idx - 1); //前移一位
            int corr_next = recover_data(I_QPSK_demod, PRN_1, current_len, PRN_LENGTH , start_idx + 1); //後移一位
            
            if (abs(corr_prev) >= THRESHOLD && abs(corr_prev) > abs(corr_next)){
                start_idx = start_idx - 1; 
                final_correlation = corr_prev;
                final_data[segment_count] = (corr_prev > 0) ? 1 : 0;
            }else if (abs(corr_next) >= THRESHOLD && abs(corr_next) > abs(corr_prev)){
                start_idx = start_idx + 1; 
                final_correlation = corr_next;
                final_data[segment_count] = (corr_next > 0) ? 1 : 0;
            }else {
                int best_local_corr = final_correlation;
                int best_local_idx  = start_idx;//❎
                double best_local_angle = absolute_position;

                for (int dshift = -8; dshift <= 8; dshift++) {
                    int cand_idx = start_idx + dshift;
                    if (cand_idx < 0) continue;

                    for (double ddeg = -2.0; ddeg <= 2.0; ddeg += 0.5) {
                        rotate_data(i_data, q_data, I_rot, Q_rot, global_best_angle + ddeg, current_len, global_best_direction);
                        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, current_len);
                        int corr = recover_data(I_QPSK_demod, PRN_1, current_len, PRN_LENGTH, cand_idx);
                        if (abs(corr) > abs(best_local_corr)) {
                            best_local_corr = corr;
                            best_local_idx = cand_idx;
                            best_local_angle = global_best_angle + ddeg;
                        }
                    }
                }
                if (abs(best_local_corr) > abs(final_correlation)) {
                    start_idx = best_local_idx;
                    final_correlation = best_local_corr;
                    global_best_angle = best_local_angle; // 視需要更新
                    //printf("局部微調成功：idx=%d, angle=%.1f, corr=%d\n", idx, best_local_angle, final_correlation);

                    if (abs(final_correlation) < THRESHOLD && final_correlation > 0) {
                        printf("局部微調成功：idx=%d, angle=%.1f, corr=%d\n", start_idx, best_local_angle, final_correlation);
                        final_data[segment_count] = 1; // 明確設定為0
                    } else if (abs(final_correlation) <  THRESHOLD && final_correlation < 0){
                        printf("局部微調成功：idx=%d, angle=%.1f, corr=%d\n", start_idx, best_local_angle, final_correlation);
                        final_data[segment_count] = 0;
                    }
                } else {// 粗＋細調一次，嘗試重新定錨角度/方向/PRN/起始點
                    printf("局部微調失敗，嘗試重新粗+細調...\n");
                    TuneResult tr = coarse_fine_tuning_stage(i_data, q_data, I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, PRN_1, current_len, PRN_LENGTH);

                    // 更新「全域」選擇（之後段落用得到）
                    //selected_prn = tr.prn;
                    //selected_prn_id = tr.prn_id;
                    int fine_best_angle = tr.angle_deg;
                    int fine_best_direction = tr.direction;
                    int idx = tr.start_point;
                    rotate_data(i_data, q_data, I_rot, Q_rot, fine_best_angle, current_len, fine_best_direction);
                    qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, current_len);

                    int c0 = recover_data(I_QPSK_demod, PRN_1, current_len, PRN_LENGTH, idx);
                    int cp = recover_data(I_QPSK_demod, PRN_1, current_len, PRN_LENGTH, idx-1);
                    int cn = recover_data(I_QPSK_demod, PRN_1, current_len, PRN_LENGTH, idx+1);

                    final_correlation = c0; int best_idx = idx;
                    if (abs(cp) > abs(final_correlation)){ final_correlation = cp; best_idx = idx-1; }
                    if (abs(cn) > abs(final_correlation)){ final_correlation = cn; best_idx = idx+1; }
                    idx = best_idx;

                    if (abs(final_correlation) < THRESHOLD && final_correlation > 0) {
                        printf("粗+細調後仍不足（corr=%d），保留 1 並前進。\n", final_correlation);
                        final_data[segment_count] = 1; // 明確設定為0
                    } else if (abs(final_correlation) < THRESHOLD && final_correlation < 0){
                        printf("粗+細調後仍不足（corr=%d），保留 0 並前進。\n", final_correlation);
                        final_data[segment_count] = 0;
                    }
                }
            }
        }
        
        printf("段 %d: 起始點 = %d, 相關性 = %d, 還原結果 = %d\n", segment_count, start_idx, final_correlation, final_data[segment_count]);

        // 修正後的overlap邏輯
        int prn_end_pos = start_idx + PRN_LENGTH;  // PRN結束後的第一個位置
        int available_overlap = current_len - prn_end_pos;  // 可用的overlap數量

        // 決定實際要保存的overlap數量
        int actual_overlap_count = (available_overlap >= SEARCH_MARGIN) ? 
                                SEARCH_MARGIN : available_overlap;

        // 邊界檢查：確保不會出現負數或越界
        if (actual_overlap_count <= 0) {
            printf("警告：沒有足夠的資料作為overlap，設為0\n");
            actual_overlap_count = 0;
        }

        // 保存overlap資料
        printf("保存overlap: PRN結束於位置%d, 可用%d筆, 實際保存%d筆\n", 
            prn_end_pos, available_overlap, actual_overlap_count);

        for (int i = 0; i < actual_overlap_count; ++i) {
            int src = prn_end_pos + i;
            if (src < current_len) {  // 額外的邊界檢查
                prev_overlap_i[i] = i_data[src];
                prev_overlap_q[i] = q_data[src];
            }
        }

        // 更新overlap_data_count為實際保存的數量
        overlap_data_count = actual_overlap_count;
        }
        segment_count++;
    }
    printf("還原結果:\n");
    for(int segment = 0; segment < segment_count; segment++) {
        printf("%d", final_data[segment]);
    }
    fclose(file);
    free(i_data);
    free(q_data);
    free(I_rot);
    free(Q_rot);
    free(I_QPSK_demod);
    free(Q_QPSK_demod);
    free(final_data);
    free(prev_overlap_i);
    free(prev_overlap_q);
    system("pause");
    return 0;
}

TuneResult coarse_fine_tuning_stage(double *i_data, double *q_data, double *I_rot, double *Q_rot, int *I_QPSK_demod, int *Q_QPSK_demod, const int *PRN_1, int data_size, int prn_len) {
    
    int best_start_point_1 = -1, best_start_point_3 = -1;
    int best_correlation_1 = -data_size, best_correlation_3 = -data_size;  // 修正：初始化為最小可能值
    double best_angle_1 = 0    , best_angle_3 = 0;
    int best_direction_1 = 0   , best_direction_3 = 0;    // 0: 逆時針, 1: 順時針
    int angle_index = 0;
    
    printf("\n========== 開始粗調 ==========\n");
    for(double angle = 10; angle <= 180; angle += 10) {
        rotate_data(i_data, q_data, I_rot, Q_rot, angle, data_size, 0);// 旋轉資料
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, data_size);// QPSK解調 
        int correlation_1, correlation_3;
        int start_point_1 = find_start_point(I_QPSK_demod, PRN_1, data_size, PRN_LENGTH, &correlation_1); // 與PRN_1做相關性分析
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
        rotate_data(i_data, q_data, I_rot, Q_rot, angle, data_size, 1);// 旋轉資料
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, data_size);// QPSK解調 
        int correlation_1, correlation_3;
        int start_point_1 = find_start_point(I_QPSK_demod, PRN_1, data_size, PRN_LENGTH, &correlation_1); // 與PRN_1做相關性分析
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
    printf("\n========== 全域最佳結果 ==========\n");
    printf("PRN_1: 最佳角度 = %.0f度(%s), 最佳位移 = %d, 最佳相關性 = %d\n", best_angle_1, (best_direction_1==0)?"逆時針":"順時針", best_start_point_1, best_correlation_1);
    printf("\n========== 開始細調 ==========\n");
    int fine_best_correlation = -data_size;
    int fine_best_start_point = -1;
    int fine_best_direction = best_direction_1;
    double fine_best_angle = best_angle_1;
    printf("在 %.0f度(%s) 的 ±10度範圍內進行細調...\n", best_angle_1, (best_direction_1==0)?"逆時針":"順時針");
    for(double angle = fine_best_angle - 10; angle <= fine_best_angle + 10; angle += 1.0) {
        if(angle < 0 || angle > 180) continue;
        rotate_data(i_data, q_data, I_rot, Q_rot, angle, PRN_LENGTH, fine_best_direction);// 旋轉資料
        qpsk_demodulation(I_rot, Q_rot, I_QPSK_demod, Q_QPSK_demod, PRN_LENGTH);// QPSK解調 
        int correlation, start_point;
        start_point = find_start_point(I_QPSK_demod, PRN_1, data_size, PRN_LENGTH, &correlation);
        printf("細調角度 %.1f度(%s): 起始點:%d, 相關性:%d\n", angle, (fine_best_direction==0?"逆時針":"順時針"), start_point, correlation);
        if(correlation > fine_best_correlation) {
        fine_best_correlation = correlation;
        fine_best_start_point = start_point;
        fine_best_angle = angle;
        }
    }
    printf("\n========== 細調後最佳結果 ==========\n");
    printf("最佳細調角度: %.1f度(%s)\n", fine_best_angle, (best_direction_1==0)?"逆時針":"順時針");
    printf("起始點 = %d, 相關性 = %d\n", fine_best_start_point, fine_best_correlation);
    
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
int recover_data(int *I_QPSK_demod, const int *PRN, int data_size, int prn_length, int start_point) {
    int correlation = 0;
    for (int i = 0; i < prn_length && (start_point + i) < data_size; ++i) {
        int bi = (I_QPSK_demod[start_point + i] == 0) ? 1 : -1;
        int bp = (PRN[i] == 0) ? 1 : -1;
        correlation += bi * bp;
    }
    return correlation;
}
