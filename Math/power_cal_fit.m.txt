%% power_cal_fit.m
%% =========================================================================
%%  功率损耗模型离线标定 — 从人工录入 CSV 拟合
%%  输入: power_cal_data.csv (由 gen_power_cal_template.m 生成模板)
%%  输出: 4 个 P_LOSS_K_xxx 系数 + 拟合质量 + 诊断图
%% =========================================================================

clear; clc; close all;

%% ==================== 0. 配置 ====================
%  0x20F 帧改造后 (M4 侧已滤波+Q定点打包):
%    b0..1: Vd [V]  int16 Q5   -> V = int16/32       (±1024V, 31.3mV)
%    b2..3: Vq [V]  int16 Q5   -> V = int16/32
%    b4..5: Id [A]  int16 Q4   -> A = int16/16       (±2048A, 62.5mA)
%    b6..7: Iq [A]  int16 Q4   -> A = int16/16
%  测试员从 CANoe 读取的已是 V/A 物理值, 直接填入 CSV 的 Vd_V/Vq_V/Id_A/Iq_A 列.
DATA_FILE   = 'power_cal_data.csv';
TEMPLATE_FN = 'gen_power_cal_template.m';
BASE_VOLTAGE = 1000;    % V
BASE_CURRENT = 50;      % A
BASE_SPEED   = 8500;    % RPM
BASE_POWER   = 50000;   % W

%% ==================== 1. 读取 CSV ====================
if ~exist(DATA_FILE, 'file')
    fprintf('未找到数据文件: %s\n', DATA_FILE);
    fprintf('请先运行 "%s" 生成模板, 用 Excel 填入实测数据后再运行本脚本。\n', TEMPLATE_FN);
    fprintf('命令: >> %s\n', TEMPLATE_FN(1:end-2));
    error('数据文件缺失');
end

T = readtable(DATA_FILE, 'TextType', 'string');

% 删除 Excel 编辑时可能引入的乱码多余列(保留前 10 列)
if width(T) > 10
    warning('CSV 检测到 %d 列 (>10), 已截取前 10 列, 余列丢弃。请检查模板是否被 Excel 添加多余列。', width(T));
    T = T(:, 1:10);
end

%% ==================== 2. 数据校验 ====================
expected = {'Point_ID','Test_Name','Vdc_V','Idc_A','RPM_rpm', ...
            'Vd_V','Vq_V','Id_A','Iq_A','Notes'};
missing  = setdiff(expected, T.Properties.VariableNames);
if ~isempty(missing)
    error('CSV 缺少必需列: %s\n模板列名必须严格匹配 (运行 %s 重生成)', ...
          strjoin(missing, ', '), TEMPLATE_FN);
end

% 数值列转换
num_cols = {'Vdc_V','Idc_A','RPM_rpm','Vd_V','Vq_V','Id_A','Iq_A'};
for c = num_cols
    col = T.(c{1});
    if ~isnumeric(col)
        col = str2double(string(col));
        T.(c{1}) = col;
    end
end

% 删除含 NaN 的行
T = T(~any(ismissing(T{:,num_cols}), 2), :);
if height(T) < 5
    error('有效数据点仅 %d 个, 至少需要 5 个工作点才能拟合\n请用 Excel 填入模板的空白行', height(T));
end

% 物理合理性检查
if any(T.Vdc_V <= 0) || any(T.Idc_A < 0)
    error('Vdc 必须 > 0, Idc 必须 >= 0 — 检查录入');
end
if any(T.RPM_rpm < 0) || any(T.RPM_rpm > 2*BASE_SPEED)
    warning('RPM 数据超出合理范围 [0, %d] — 复核录入', 2*BASE_SPEED);
end

fprintf('已读取 %d 个有效工作点\n', height(T));

%% ==================== 3. 物理量计算 ====================
P_dc_ref = T.Vdc_V .* T.Idc_A;
P_dq     = 1.5 * (T.Vd_V .* T.Id_A + T.Vq_V .* T.Iq_A);
P_loss_raw = P_dc_ref - P_dq;

% 检测发电工况 (P_dq > P_dc, 物理上不可能为"损耗")
gen_mask = P_loss_raw < 0;
n_gen = sum(gen_mask);
if n_gen > 0
    fprintf('\n*** 检测到 %d 个发电/反拖工况 (P_dq > P_dc): ***\n', n_gen);
    gen_idx = find(gen_mask);
    for ii = 1:numel(gen_idx)
        k = gen_idx(ii);
        fprintf('  Point_ID=%d  P_dc=%.1fW  P_dq=%.1fW  Δ=%.1fW\n', ...
                T.Point_ID(k), P_dc_ref(k), P_dq(k), P_loss_raw(k));
    end
    fprintf('  这些点 P_dq > P_dc 说明 FOC 在反拖/发电状态,\n');
    fprintf('  母线净电流小于电机电磁功率所需, 损耗模型不适用。\n');
    fprintf('  -> 自动剔除这些点, 仅对真正的电动工况拟合损耗。\n\n');
    T    = T(~gen_mask, :);
    P_dc_ref = P_dc_ref(~gen_mask);
    P_dq     = P_dq(~gen_mask);
    P_loss_raw = P_loss_raw(~gen_mask);
end

P_loss = abs(P_loss_raw);    % 残余微量负值取绝对值(测量噪声)

if height(T) < 4
    error('剔除发电工况后仅剩 %d 个有效点, 不足 4 个, 无法拟合 4 系数模型。\n请补充电动工况测试点。', height(T));
end

%% ==================== 4. 特征构造 (与 m4_rte.c 编码对齐) ====================
I_sq      = T.Id_A.^2 + T.Iq_A.^2;
I_rms     = sqrt(I_sq / 2);
omega_pu  = T.RPM_rpm / BASE_SPEED;
Vdc_pu    = T.Vdc_V / BASE_VOLTAGE;

x1 = I_sq * 20;
x2 = omega_pu.^2 * BASE_POWER;
x3 = (I_rms / BASE_CURRENT) * BASE_POWER;
x4 = (I_rms / BASE_CURRENT) .* Vdc_pu * BASE_POWER;
X  = [x1, x2, x3, x4];

%% ==================== 4b. 共线性诊断 ====================
% 显示 Vdc × RPM 平面覆盖, 提示缺失单元格
Vdc_edges = [300 550 800];   % 2 档: 低压/高压
RPM_edges = [1000 3500 6500 9500];  % 3 档: 低/中/高转速

% 仅对电动工况点统计
Vdc_bin = discretize(T.Vdc_V, Vdc_edges);
RPM_bin = discretize(T.RPM_rpm, RPM_edges);
cov_grid = accumarray([Vdc_bin, RPM_bin], 1, [2, 3]);

fprintf('\nVdc × RPM 工况覆盖度 (单元格内为样本数):\n');
fprintf('              1-3500rpm  3500-6500rpm  6500-9500rpm\n');
for i = 1:2
    fprintf('%4.0f-%4.0fV    %8d    %8d    %8d\n', ...
            Vdc_edges(i), Vdc_edges(i+1), ...
            cov_grid(i,1), cov_grid(i,2), cov_grid(i,3));
end
empty_cells = sum(cov_grid(:) == 0);
fprintf('缺失单元格数: %d / 6\n', empty_cells);
if empty_cells >= 2
    fprintf('\n*** 警告: Vdc×RPM 平面覆盖率不足 ***\n');
    fprintf('  4 项损耗模型需要 Vdc 和 RPM 两个维度都有变化,\n');
    fprintf('  否则铁损(K_FE)与开关损耗(K_INV_SW)无法分离。\n');
    fprintf('  请补测缺失单元格, 当前数据无法独立识别全部 4 个系数。\n');
end

% 特征共线性
if size(X,1) >= 4
    c = corrcoef(X);
    c(isnan(c)) = 0;
    fprintf('\n特征相关系数矩阵 (|r|>0.95 提示共线性高):\n');
    disp(array2table(c, 'VariableNames', {'Cu','Fe','Cond','Sw'}, ...
                        'RowNames',       {'Cu','Fe','Cond','Sw'}));
end

%% ==================== 5. 非负最小二乘拟合 ====================
beta = lsqnonneg(X, P_loss);
y_hat = X * beta;
resid = P_loss - y_hat;
SS_res = sum(resid.^2);
SS_tot = sum((P_loss - mean(P_loss)).^2);
R2   = 1 - SS_res / SS_tot;
RMSE = sqrt(mean(resid.^2));
max_err = max(abs(resid));

K_CU       = beta(1);
K_FE       = beta(2);
K_INV_COND = beta(3);
K_INV_SW   = beta(4);

%% ==================== 6. 输出结果 ====================
fprintf('\n===================== 拟合结果 =====================\n');
fprintf('工作点数   : %d (发电工况已剔除)\n', height(T));
fprintf('R²         : %.4f\n', R2);
fprintf('RMSE       : %.1f W\n', RMSE);
fprintf('最大残差   : %.1f W\n', max_err);
fprintf('----------------------------------------------------\n');
fprintf('  P_LOSS_K_CU       = %.6ff   (铜损  ∝ I²)        [%s]\n', K_CU, ...
        ternary(K_CU > 0      && K_CU < 1,    '合理', '⚠ 异常'));
fprintf('  P_LOSS_K_FE       = %.6ff   (铁损  ∝ ω²)        [%s]\n', K_FE, ...
        ternary(K_FE > 0      && K_FE < 1,    '合理', '⚠ 异常'));
fprintf('  P_LOSS_K_INV_COND = %.6ff   (导通  ∝ I)         [%s]\n', K_INV_COND, ...
        ternary(K_INV_COND > 0 && K_INV_COND < 1, '合理', '⚠ 异常'));
fprintf('  P_LOSS_K_INV_SW   = %.6ff   (开关  ∝ I×Vdc)     [%s]\n', K_INV_SW, ...
        ternary(K_INV_SW > 0   && K_INV_SW < 1,   '合理', '⚠ 异常'));
fprintf('====================================================\n');

% 系数被压到 0 的诊断
zero_names = {};
if K_CU == 0,             zero_names{end+1} = 'K_CU (铜损)';       end
if K_FE == 0,             zero_names{end+1} = 'K_FE (铁损)';       end
if K_INV_COND == 0,       zero_names{end+1} = 'K_INV_COND (导通)'; end
if K_INV_SW   == 0,       zero_names{end+1} = 'K_INV_SW (开关)';   end
if ~isempty(zero_names)
    fprintf('\n被压到 0 的系数: %s\n', strjoin(zero_names, ', '));
    fprintf('可能原因: 对应特征在样本中变化不足, 或与其它项高度共线性。\n');
end

% Id_A 数据情况说明 (Id=0 是合法的 FOC 控制策略)
if all(T.Id_A == 0)
    fprintf('\n*** 提示: Id_A 全为 0 ***\n');
    fprintf('  FOC 工作在 Id=0 控制(纯 Iq 扭矩控制, 无弱磁)。\n');
    fprintf('  此控制策略下铜损项(I^2)与导通项(I_rms)数学上完全共线 (corr≈1),\n');
    fprintf('  K_CU 与 K_INV_COND 不可分离 — 4 项模型退化为 3 项可识别:\n');
    fprintf('    P_loss = (K_CU·20 + K_INV_COND·50000/50/√2)·Iq  +  K_FE·ω²·50000  +  K_INV_SW·Iq·Vdc·...\n');
    fprintf('  即使 K_INV_COND 单值被压到 0, 它的物理贡献已合并到 K_CU 中,\n');
    fprintf('  功率估算总效果仍正确 (K_CU 实际承载了 铜损+导通 两项)。\n');
    fprintf('  改善方法: 若 FOC 允许, 补一组 Id 注入工况(~-3A)可独立识别两项。\n');
end

fprintf('\n复制下面 4 行到 m4_rte.c 顶部对应 #define:\n');
fprintf('#define P_LOSS_K_CU        %.6ff\n', K_CU);
fprintf('#define P_LOSS_K_FE        %.6ff\n', K_FE);
fprintf('#define P_LOSS_K_INV_COND  %.6ff\n', K_INV_COND);
fprintf('#define P_LOSS_K_INV_SW    %.6ff\n', K_INV_SW);

if RMSE > 200
    warning('RMSE=%.0fW 偏高, 建议:\n  - 增加测试点\n  - 检查异常工作点 Notes', RMSE);
end

%% ==================== 7. 诊断可视化 ====================
figure('Name','功率损耗标定','Position',[80 80 1200 750]);

subplot(2,2,1);
scatter(P_loss, y_hat, 60, T.RPM_rpm, 'filled');
hold on; lim = [0 max(P_loss)*1.1]; plot(lim, lim, 'r--', 'LineWidth',1.5);
xlabel('实测 P_{loss} [W]'); ylabel('拟合 P_{loss} [W]');
title(sprintf('估算 vs 实测 (R^2 = %.3f, RMSE = %.0fW)', R2, RMSE));
grid on; colorbar;

subplot(2,2,2);
stem(T.Point_ID, resid, 'filled', 'MarkerSize', 5);
hold on; yline(0, 'r', 'LineWidth', 1);
xlabel('工作点 ID'); ylabel('残差 [W]');
title('各点拟合残差'); grid on;

subplot(2,2,3);
[~, idx] = sort(I_rms);
loss_cu = beta(1)*x1(idx); loss_fe = beta(2)*x2(idx);
loss_cd = beta(3)*x3(idx); loss_sw = beta(4)*x4(idx);
hh = bar([loss_cu, loss_fe, loss_cd, loss_sw], 'stacked');
hh(1).FaceColor = [0.2 0.6 0.2]; hh(2).FaceColor = [0.8 0.4 0.1];
hh(3).FaceColor = [0.2 0.4 0.8]; hh(4).FaceColor = [0.7 0.1 0.7];
hold on; scatter(1:numel(idx), P_loss(idx), 30, 'k', 'filled');
legend({'Cu','Fe','Cond','Sw','实测'}, 'Location','best');
xlabel('工作点 (按 I_{rms} 排序)'); ylabel('损耗 [W]');
title('各损耗分量贡献'); grid on;

subplot(2,2,4);
hold on;
for spd_grp = [2000 5000 8000]
    mask = abs(T.RPM_rpm - spd_grp) < 500;
    if any(mask), scatter(T.Vdc_V(mask), P_loss(mask), 80, 'filled'); end
end
xlabel('V_{dc} [V]'); ylabel('P_{loss} [W]');
title('损耗 vs 母线电压'); grid on;
legend('2000','5000','8000','Location','best');

sgtitle('功率损耗模型离线标定', 'FontSize', 13);

save('power_cal_result.mat', 'T', 'beta', 'K_CU', 'K_FE', ...
     'K_INV_COND', 'K_INV_SW', 'R2', 'RMSE', 'P_loss', 'y_hat');
fprintf('\n结果已保存到 power_cal_result.mat\n');

%% ---- helper ----
function s = ternary(cond, a, b)
    if cond, s = a; else, s = b; end
end