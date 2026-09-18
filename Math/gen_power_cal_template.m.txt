%% gen_power_cal_template.m
%% 生成 power_cal_data.csv 模板文件 (含示例数据 + 空白行)
%% 运行后在当前目录生成可直接用 Excel 打开编辑的 CSV
%% ----------------------------------------------------------------
%
%  关于添加工况点:
%    本脚本生成的 CSV 可任意添加工况行, 不影响 power_cal_fit.m 的拟合.
%    拟合脚本通过 readtable 自动读取全部行, 仅过滤掉含 NaN 的空白行.
%    增加工况点只会:
%      - 提升拟合自由度 (样本数 ↑)
%      - 改善共线性 (尤其补全 Vdc-RPM 平面空格时, 单系数可信度 ↑)
%      - 改变 R² 与 RMSE 数值
%    不会改变模型结构或使脚本失效.
%
%    例外: 不要新增同 Point_ID 的行 (Point_ID 应唯一, 否则残差图标签重叠).

clear; clc;

% 9 列模板定义
header = {'Point_ID','Test_Name','Vdc_V','Idc_A','RPM_rpm', ...
         'Vd_V','Vq_V','Id_A','Iq_A','Notes'};

% 4 行示例数据 (让测试员看到正确的填写格式)
sample = {
    1, 'HV_2000_30pct', 720.5,  4.20, 2005, 12.3, 45.6, 0.8, 15.1, '稳态5s平均';
    2, 'HV_5000_60pct', 716.5, 10.30, 4998, 11.2, 82.4, 0.9, 33.7, '稳态5s平均';
    3, 'HV_8000_90pct', 709.4, 18.90, 8002, 10.5, 138.7, 1.0, 54.2, '稳态5s平均';
    4, 'LV_5000_60pct', 448.9, 11.80, 4998, 15.2, 97.8, 1.2, 34.1, '低压重载';
};

% 12 个空白工况点供填写 (按推荐的测试矩阵)
test_matrix = {
    5,  'HV_2000_60pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
    6,  'HV_2000_90pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
    7,  'HV_5000_30pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
    8,  'HV_5000_90pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
    9,  'HV_8000_30pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
    10, 'HV_8000_60pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
    11, 'LV_2000_60pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
    12, 'LV_5000_30pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
    13, 'LV_5000_90pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
    14, 'LV_8000_30pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
    15, 'LV_8000_60pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
    16, 'LV_8000_90pct', NaN, NaN, NaN, NaN, NaN, NaN, NaN, '';
};

all_rows = [sample; test_matrix];
T = cell2table(all_rows, 'VariableNames', header);

% 写 CSV (空单元格保留为空, 便于 Excel 编辑)
writetable(T, 'power_cal_data.csv', 'QuoteStrings', true);

fprintf('已生成模板文件: power_cal_data.csv\n');
fprintf('  - 4 行示例数据 (可删除)\n');
fprintf('  - 12 行空白工况点 (按测试矩阵预填 Point_ID 和 Test_Name)\n');
fprintf('用法: 用 Excel 打开 → 填入实测数据 → 保存 → 运行 power_cal_fit.m\n');
fprintf('\n添加工况点: 在 CSV 末尾追加新行, Point_ID 唯一即可, 拟合脚本会自动纳入.\n');