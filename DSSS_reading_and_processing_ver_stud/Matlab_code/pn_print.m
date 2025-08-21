% =========================================================================
% MATLAB 腳本：將二元資料視覺化為黑白網格圖 (版本 1.1 - 已修正維度問題)
% =========================================================================
%
% 功能說明：
% 1. 讀取一個僅包含 '0' 和 '1' 的純文字檔 (.txt)。
% 2. 將一維的資料流重新排列成一個二維矩陣。
% 3. 根據指定的每行位元數（寬度）自動換行。
% 4. 將此矩陣顯示為一張圖形，其中 '0' 對應白色，'1' 對應黑色。
%
% 使用方法：
% 1. 將此程式碼儲存為一個 .m 檔案 (例如: visualize_data.m)。
% 2. 將您的資料檔 (例如: data.txt) 與此 .m 檔放在同一個資料夾中。
% 3. 修改下方「使用者設定」區塊中的檔案名稱與每行位元數。
% 4. 在 MATLAB 中執行此腳本。
%
% =========================================================================

clear; clc; close all;

% --- 1. 使用者設定 ---

% 請在此指定您的資料檔案名稱
filename = 'data.txt'; 

% 請設定每一行要顯示多少個位元 (即圖片的寬度)
bits_per_row = 500;


% --- 2. 讀取與處理資料 ---

% 檢查檔案是否存在
if ~exist(filename, 'file')
    error('錯誤：找不到指定的檔案 "%s"。請確認檔案名稱是否正確，以及是否與此腳本在同一個資料夾中。', filename);
end

% 開啟檔案
fileID = fopen(filename, 'r');
if fileID == -1
    error('錯誤：無法開啟檔案 "%s"。請檢查檔案權限。', filename);
end

% 將檔案中的所有字元讀取為一個長字元向量
% 使用 '%c' 可以讀取所有字元，包含換行符與空白，這樣最為穩健
raw_chars = fscanf(fileID, '%c');
fclose(fileID);

% 將字元 '0' 和 '1' 轉換為數值 0 和 1
% '0' 的 ASCII 碼是 48，'1' 是 49。字元向量減去 '0' (即 48) 可得到數值。
% 例如: '1' - '0'  -->  49 - 48 = 1
binary_vector = raw_chars - '0';

% 過濾掉任何非 0 或 1 的值
% (例如換行符、空白等字元在轉換後會變成非 0/1 的數值)
binary_vector = binary_vector(binary_vector == 0 | binary_vector == 1);

% *** 錯誤修正處 ***
% 使用 (:) 運算子確保 binary_vector 是一個單欄的行向量 (column vector)。
% 這是為了與後續用 zeros() 建立的行向量進行垂直串接。
binary_vector = binary_vector(:);

% 取得資料總長度
total_bits = length(binary_vector);
fprintf('成功讀取 %d 個位元。\n', total_bits);

% --- 3. 將資料重塑為矩陣 ---

% 計算需要的總行數 (即圖片的高度)
% 使用 ceil 無條件進位，確保所有資料都能被包含
num_rows = ceil(total_bits / bits_per_row);

% 計算填滿整個矩陣所需的元素總數
total_elements_in_matrix = num_rows * bits_per_row;

% 為了讓資料能被 reshape 函數成功重塑，向量長度必須是可整除的。
% 因此，我們在向量的尾端補上 0，直到其長度符合矩陣總大小。
% 補上的 0 在圖形中會顯示為白色，通常不會影響觀察。
padding_needed = total_elements_in_matrix - total_bits;
padded_vector = [binary_vector; zeros(padding_needed, 1)];

% 將一維向量重新塑形 (reshape) 為二維矩陣
% 注意：MATLAB 的 reshape 是 "column-wise" (優先填滿行)，
% 但我們需要 "row-wise" (優先填滿列) 的效果。
% 所以我們先塑形成 (寬度, 高度) 的矩陣，再用轉置 (') 轉成 (高度, 寬度) 的正確形式。
data_matrix = reshape(padded_vector, bits_per_row, num_rows)';


% --- 4. 產生並美化圖形 ---

% 建立一個新的圖形視窗
figure('Name', '二元資料視覺化結果', 'NumberTitle', 'off');

% 使用 imagesc 顯示矩陣
% imagesc 會自動縮放顏色，非常適合處理 0/1 矩陣
imagesc(data_matrix);

% 定義顏色對應表 (Colormap)
% 建立一個 2x3 的矩陣，第一行是資料最小值(0)的顏色，第二行是最大值(1)的顏色
% [1 1 1] 是 RGB 的白色
% [0 0 0] 是 RGB 的黑色
custom_colormap = [1 1 1;  % 0 -> 白色
                   0 0 0]; % 1 -> 黑色
colormap(custom_colormap);

% 讓圖片的長寬比符合資料比例，這樣每個位元都會顯示為一個正方形
axis image;

% 關閉座標軸的刻度與標籤，讓圖形更純粹、更易於觀察
axis off; 

% 為圖形加上標題
title_str = sprintf('二元資料視覺化 (%d 位元/行)', bits_per_row);
title(title_str, 'FontSize', 14);

% 提示使用者圖形已產生
disp('圖形產生完畢！');
