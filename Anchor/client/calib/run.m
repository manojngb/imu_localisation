%% Housekeeping
clc; close all; clear all;

%% Anchor locations
id_anc = [0 1 2];
xyz_anc = [
    7.111, 2.884, 2.504
    7.094, 7.708, 2.504
    1.606, 7.674, 2.504
    ];

%% Load files

xyz1 = [3.727 0.692 1.344];
d1 = csvread('logs/log_01.txt');

xyz2 = [3.746 8.322 0.763];
d2 = csvread('logs/log_02.txt');

xyz3 = [8.45 6.689 0.792];
d3 = csvread('logs/log_03.txt');

xyz4 = [1.597 3.674 0.783];
d4 = csvread('logs/log_04.txt');

xyz5 = [8.736 3.908 1.288];
d5 = csvread('logs/log_05.txt');

xyz6 = [5.077 3.940 1.227];
d6 = csvread('logs/log_06.txt');

xyz7 = [4.469 7.391 0.02];
d7 = csvread('logs/log_07.txt');

xyz_tot = [xyz1; xyz2; xyz3; xyz4; xyz5; xyz6; xyz7];
d_tot = {d1; d2; d3; d4; d5; d6; d7};

%% Analyze information
figure();
symbols = {'s', 'o', '^'};
colors = {'b', 'r', 'k'};

for i=1:length(id_anc)
    anc = id_anc(i);
    pa = xyz_anc(i,:);
    
    distances = [];
    means = [];
    stds = [];
    
    % loop through all test points
    for j=1:size(xyz_tot,1)
        pt = xyz_tot(j,:);
        dist = norm(pa-pt);
        
        % get all measurements from this anchor to this testpoint
        d = d_tot{j};
        idxs = find(d(:,1) == anc);
        meas = d(idxs,2);
        if isempty(meas)
            continue
        end
        distances = [distances; dist];
        delta = -0.55;
        means = [means; (mean(meas)+delta)];
        stds = [stds; sqrt(var(meas))];
    end
    
    % plot results
    
    errorbar(distances, means, stds, stds, symbols{i}, 'Color', colors{i}, 'MarkerFaceColor', colors{i});
    hold on;
end

% plot ideal
plot(0:8, 0:8, '--k', 'LineWidth',2);

xlabel('True Distance (m)', 'FontSize', 14);
ylabel('Estimated Distance (m)', 'FontSize', 14);
grid on;






