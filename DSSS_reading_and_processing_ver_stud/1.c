if (final_correlation < THRESHOLD) {
            printf("相關性不足，嘗試微調 (Segment: %d, 起始點 %d, 相關性 %d)\n", segment, idx, final_correlation);
            fprintf(fp_final_result, "相關性不足，嘗試微調 (Segment: %d, 起始點 %d, 相關性 %d)\n", segment, idx, final_correlation);
            int corr_prev = recover_data(I_QPSK_demod, selected_prn, LOADED_DATA_SIZE, 255 , idx - 1); //前移一位
            int corr_next = recover_data(I_QPSK_demod, selected_prn, LOADED_DATA_SIZE, 255 , idx + 1); //後移一位
            if (corr_prev >= THRESHOLD && corr_prev >corr_next){
                idx = idx - 1; 
                final_correlation = corr_prev;
            }else if (corr_next >= THRESHOLD && corr_next >corr_prev){
                idx = idx + 1; 
                final_correlation = corr_next;
            } else {
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
                        if (corr > best_local_corr) {
                            best_local_corr = corr;
                            best_local_idx = cand_idx;
                            best_local_angle = fine_best_angle + ddeg;
                        }
                    }
                }
                if (best_local_corr > final_correlation) {
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
                    if (cp > final_correlation){ final_correlation = cp; best_idx = idx-1; }
                    if (cn > final_correlation){ final_correlation = cn; best_idx = idx+1; }
                    idx = best_idx;

                    if (final_correlation < THRESHOLD) {
                        printf("粗+細調後仍不足（corr=%d），保留 0 並前進。\n", final_correlation);
                    } else {
                        printf("粗+細調成功：angle=%.1f°, dir=%s, idx=%d, corr=%d\n",fine_best_angle, (final_best_direction? "順" : "逆"), idx, final_correlation);
                    }
                }
            }
        }