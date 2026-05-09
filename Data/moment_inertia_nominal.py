import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

# ==========================================
# 1. MOTOR CONSTANTS (From derivations)
# ==========================================
Ra = 6.67          
K = 0.0181         
b = 0.00000227    
gear_ratio = 46.8 

# ==========================================
# 2. LOAD & PREP THE DATA
# ==========================================
df = pd.read_csv('StepResponse.csv') 

time_ms = df.iloc[:, 0].values
output_rpm = df.iloc[:, 1].values

time_sec = time_ms / 1000.0
time_sec = time_sec - time_sec[0] 

internal_rad_s = output_rpm * gear_ratio * (np.pi / 30.0)

# ==========================================
# 3. DEFINE THE PHYSICS MODEL
# ==========================================
# The standard first-order step response equation: w(t) = W_max * (1 - e^(-t/tau))
def first_order_step(t, tau, w_max):
    return w_max * (1 - np.exp(-t / tau))

# ==========================================
# 4. FIT THE CURVE & EXTRACT TIME CONSTANT
# ==========================================
# Provide an initial guess for the algorithm: [tau = 0.1s, w_max = actual max speed]
initial_guess = [0.1, np.max(internal_rad_s)]

# Run the SciPy Curve Fitter
popt, pcov = curve_fit(first_order_step, time_sec, internal_rad_s, p0=initial_guess)

tau_m_calculated = popt[0]
w_max_calculated = popt[1]

# ==========================================
# 5. CALCULATE ROTOR INERTIA (J)
# ==========================================
# Using the derived formula: tau_m = (J * Ra) / (K^2 + b * Ra)
# Rearranged to solve for J:
J_calculated = (tau_m_calculated * ((K**2) + (b * Ra))) / Ra

print("-" * 40)
print(f"Extraction Complete!")
print(f"Max Internal Speed: {w_max_calculated:.2f} rad/s")
print(f"Mechanical Time Constant (tau_m): {tau_m_calculated:.4f} seconds")
print(f"Calculated Rotor Inertia (J): {J_calculated:.8f} kg*m^2")
print("-" * 40)

# ==========================================
# 6. PLOT THE RESULTS FOR VERIFICATION
# ==========================================
# Generate a smooth line using our calculated tau and w_max to see how well it fits
fitted_curve = first_order_step(time_sec, tau_m_calculated, w_max_calculated)

plt.figure(figsize=(10, 6))
plt.plot(time_sec, internal_rad_s, 'b.', label='Raw Sensor Data (Internal rad/s)', alpha=0.5)
plt.plot(time_sec, fitted_curve, 'r-', linewidth=2.5, label=f'SciPy Fit (tau={tau_m_calculated:.3f}s)')

# Draw the 63.2% reference lines
target_63 = w_max_calculated * 0.632
plt.axhline(target_63, color='g', linestyle='--', alpha=0.7, label='63.2% Speed')
plt.axvline(tau_m_calculated, color='g', linestyle='--', alpha=0.7)

plt.title('Motor Step Response: Extracting Inertia (J)', fontsize=14, fontweight='bold')
plt.xlabel('Time (Seconds)', fontsize=12)
plt.ylabel('Internal Motor Speed (rad/s)', fontsize=12)
plt.legend(loc='lower right')
plt.grid(True, linestyle='--', alpha=0.6)
plt.show()
