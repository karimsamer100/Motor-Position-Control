clc;
clear;
close all;

% Motor parameters from estimation
J  = 5.5744e-06;
K  = 0.020117;
Ra = 6.2432;
b  = 2.2381e-06;
L  = 0;

s = tf('s');

% DC motor position transfer function: theta(s) / V(s)
G = K / (s * ((L*s + Ra)*(J*s + b) + K^2));

disp('Motor Position Transfer Function:');
G

figure;
step(G);
grid on;
title('Open-loop Motor Position Response');

C = pidtune(G, 'PID');

disp('Initial PID Controller from pidtune:');
C

Kp = C.Kp;
Ki = C.Ki;
Kd = C.Kd;

fprintf('Kp = %.10f\n', Kp);
fprintf('Ki = %.10f\n', Ki);
fprintf('Kd = %.10f\n', Kd);



T = feedback(C*G, 1);

figure;
step(T);
grid on;
title('Closed-loop Position Response with PID');

info = stepinfo(T);
disp('Step Response Info:');
disp(info);