#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>
#include <iomanip>

// 使用標準函式庫的命名空間
using namespace std;

// ===== 參數設定 =====
const int N_SAMPLES = 10000;              // 接收的總樣本數
const int N_BLOCK = 256;                  // 每個處理區塊的大小 (PN碼長度)
const int PHASE_CHANGE_INTERVAL = 2300;   // 每隔多少樣本，相位就改變一次

const double S_ACQUIRE_THRESHOLD = 250.0; // 採集門檻 (判斷是否成功找到峰值)
const double S_LOCK_LOSS_THRESHOLD = 200.0; // 鎖定遺失門檻 (判斷峰值是否已偏移)

// PI 常數
const double PI = acos(-1.0);

// 系統狀態枚舉
enum class State {
    SEARCHING, // 正在搜索相位 (執行粗調+細調)
    TRACKING   // 已鎖定相位，正在追蹤
};

// 函式：生成固定的PN碼
vector<double> generate_fixed_pn_code(int n) {
    vector<double> code(n);
    // 使用一個固定的種子(例如 1337)，來確保每次生成的序列都相同
    mt19937 gen(1337); 
    uniform_int_distribution<> dis(0, 1);
    for (int i = 0; i < n; ++i) {
        code[i] = (dis(gen) == 0) ? -1.0 : 1.0;
    }
    return code;
}

// 函式：計算相關評分
double calculate_correlation(const vector<complex<double>>& block, const vector<double>& pn, double angle_rad) {
    complex<double> rotation = exp(complex<double>(0, -angle_rad));
    double score = 0.0;
    for (size_t i = 0; i < block.size(); ++i) {
        score += real(block[i] * rotation) * pn[i];
    }
    return abs(score);
}

// 函式：以 (a, b) 格式打印複數向量
void print_complex_vector(const string& title, const vector<complex<double>>& vec, int count) {
    cout << title;
    for (int i = 0; i < count; ++i) {
        cout << "(" << fixed << setprecision(2) << vec[i].real() << ", " << vec[i].imag() << ") ";
    }
    cout << "..." << endl;
}


int main() {
    const vector<double> pn_code = generate_fixed_pn_code(N_BLOCK);

    // --- 1. 模擬信號生成 ---
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> phase_dis(0, 360);
    vector<complex<double>> r(N_SAMPLES);
    double current_actual_phase_deg = phase_dis(gen);
    for (int i = 0; i < N_SAMPLES; ++i) {
        if (i > 0 && i % PHASE_CHANGE_INTERVAL == 0) {
            current_actual_phase_deg = phase_dis(gen);
        }
        double actual_phase_rad = current_actual_phase_deg * PI / 180.0;
        double original_data_bit = (i / N_BLOCK % 2 == 0) ? 1.0 : -1.0;
        double signal = original_data_bit * pn_code[i % N_BLOCK];
        r[i] = signal * exp(complex<double>(0, actual_phase_rad));
    }
    
    // --- 輸出初始資料 ---
    cout << "================== SIMULATION START ==================" << endl;
    print_complex_vector("Original complex data (first 5 samples): ", r, 5);
    cout << "----------------------------------------------------" << endl;

    // --- 2. 狀態機處理流程 ---
    State currentState = State::SEARCHING;
    int tracked_phase_deg = 0;
    int num_blocks = N_SAMPLES / N_BLOCK;
    int tracking_start_block = -1; 

    for (int i = 0; i < num_blocks; ++i) {
        vector<complex<double>> current_block(r.begin() + i * N_BLOCK, r.begin() + (i + 1) * N_BLOCK);

        if (currentState == State::TRACKING) {
            // --- 追蹤模式 ---
            double tracked_angle_rad = (double)tracked_phase_deg * PI / 180.0;
            double current_score = calculate_correlation(current_block, pn_code, tracked_angle_rad);

            if (current_score <= S_LOCK_LOSS_THRESHOLD) {
                // 事件：鎖定遺失 (峰值偏移)
                if (tracking_start_block != -1) {
                    cout << ">> Tracking state maintained for blocks: " << tracking_start_block << " to " << i - 1 << endl;
                }
                cout << ">> Phase Lost! Detected at sample " << i * N_BLOCK << " (Block " << i << "). Re-searching..." << endl;
                cout << "----------------------------------------------------" << endl;

                currentState = State::SEARCHING;
                tracking_start_block = -1;
            }
        } 
        
        if (currentState == State::SEARCHING) {
            // --- 搜索模式 ---
            // 1. 粗調: 找峰值大概在哪
            double max_coarse_score = -1.0;
            int best_coarse_angle_deg = -1;
            for (int angle_deg = 0; angle_deg < 360; angle_deg += 10) {
                double score = calculate_correlation(current_block, pn_code, (double)angle_deg * PI / 180.0);
                if (score > max_coarse_score) {
                    max_coarse_score = score;
                    best_coarse_angle_deg = angle_deg;
                }
            }

            // 2. 細調: 精準定位峰值中心
            double max_fine_score = -1.0;
            int best_fine_angle_deg = -1;
            for (int angle_deg = best_coarse_angle_deg - 10; angle_deg <= best_coarse_angle_deg + 10; ++angle_deg) {
                int normalized_angle = (angle_deg + 360) % 360;
                double score = calculate_correlation(current_block, pn_code, (double)normalized_angle * PI / 180.0);
                if (score > max_fine_score) {
                    max_fine_score = score;
                    best_fine_angle_deg = normalized_angle;
                }
            }
            
            // 3. 判斷是否成功找到峰值
            if (max_fine_score > S_ACQUIRE_THRESHOLD) {
                // 事件：成功找到新峰值
                cout << ">> Phase Lock Successful!" << endl;
                cout << "   - Found at block: " << i << endl;
                cout << "   - Estimated phase angle (Peak): " << best_fine_angle_deg << " degrees" << endl;

                // 計算並顯示校正後的複數資料
                vector<complex<double>> corrected_block(N_BLOCK);
                double best_angle_rad = (double)best_fine_angle_deg * PI / 180.0;
                complex<double> final_rotation = exp(complex<double>(0, -best_angle_rad));
                for(int j=0; j<N_BLOCK; ++j){
                    corrected_block[j] = current_block[j] * final_rotation;
                }
                print_complex_vector("   - Data after phase correction (first 5 samples): ", corrected_block, 5);
                cout << "----------------------------------------------------" << endl;

                // 更新狀態，準備開始追蹤
                currentState = State::TRACKING;
                tracked_phase_deg = best_fine_angle_deg;
                tracking_start_block = i + 1;
            }
        }
    }

    // 迴圈結束後，如果系統仍在追蹤狀態，輸出最後一段追蹤區間
    if (currentState == State::TRACKING && tracking_start_block != -1) {
        cout << ">> Tracking state maintained for blocks: " << tracking_start_block << " to " << num_blocks - 1 << endl;
    }

    cout << "=================== SIMULATION END ===================" << endl;

    system("pause"); // 暫停以便查看輸出
    return 0;
}