import math
from equations import *

# Assignment 4.1 optimum cruise condition

# small business jet aircraft

# best(optimal) range
s = 30
cl_max = 1.35
cd0 = 0.022
k = 0.047
cd_max = cd0 + k * pow(cl_max, 2)
t0 = 12000
# t = t0 * (d(h) / d(0))
w = 50000
ct =0.7 # N/Nh

h = 0
cl_opt_jet_actual = cl_max_range_jet(cd0, k)
print(f"optimum range lift coefficient: {cl_opt_jet_actual} ")
v_opt = v_min(w, s, d(h), cl_opt_jet_actual)
print(f"optimum range air speed: {v_opt} [m/s]")
cd_opt = cd0 + k * pow(cl_opt_jet_actual, 2)
print(f"optimum drag coefficient: {cd_opt}")
t_cruise_actual = t_cruise(w, cd_opt, cl_opt_jet_actual)
m_f_actual = m_f_jet(ct, t_cruise_actual)
print(f"fuel flow: {m_f_actual} [Kg/h]")
print(f"fuel flow: {m_f_actual / 3600} [Kg/s]")
range_cruise_actual = range_cruise(v_opt, m_f_actual / 3600)
print(f"range cruise: {range_cruise_actual} [m/Kg]")

# best endurance
cl_max_range_jet_actual = cl_max_endurance_jet(cd0, k)
print(f"\nmaximum range lift coefficient: {cl_max_range_jet_actual} ")
v_max_range = v_min(w, s, d(0), cl_max_range_jet_actual)
print(f"max range air speed: {v_max_range} [m/s]")
cd_max_range = cd0 + k * pow(cl_max_range_jet_actual, 2)
t_cruise_actual = t_cruise(w, cd_max_range, cl_max_range_jet_actual)
m_f_actual = m_f_jet(ct, t_cruise_actual) / 3600
print(f"fuel flow: {m_f_actual} [Kg/s]")
range_cruise_actual = range_cruise(v_max_range, m_f_actual)
print(f"range cruise: {range_cruise_actual} [m/Kg]")


# best range at altitude
h = 5000
cl_opt_jet_actual = cl_max_range_jet(cd0, k)
print(f"\noptimum range lift coefficient: {cl_opt_jet_actual} ")
v_opt = v_min(w, s, d(h), cl_opt_jet_actual)
print(f"optimum range air speed: {v_opt} [m/s]")
cd_opt = cd0 + k * pow(cl_opt_jet_actual, 2)
print(f"optimum drag coefficient: {cd_opt}")
t_cruise_actual = t_cruise(w, cd_opt, cl_opt_jet_actual)
m_f_actual = m_f_jet(ct, t_cruise_actual) / 3600
print(f"fuel flow: {m_f_actual} [Kg/s]")
range_cruise_actual = range_cruise(v_opt, m_f_actual)
print(f"range cruise: {range_cruise_actual} [m/Kg]")


w = 58000
w_fuel = 18000

# max range
cl_opt_jet_actual = cl_max_range_jet(cd0, k)
print(f"\noptimum range lift coefficient: {cl_opt_jet_actual}")
cd_opt = cd0 + k * pow(cl_opt_jet_actual, 2)
print(f"optimum drag coefficient: {cd_opt}")
dr_cruise_actual = dr_cruise(w, cd_opt, cl_opt_jet_actual)
print(f"cruise drag: {dr_cruise_actual} [N]")
# t_altitude = lambda t0, d: t0 * d / d(0)
t_altitude = dr_cruise_actual
d_actual = (t_altitude / t0) * d(0)
print(f"air density: {d_actual} [Kg/m3]")

# d = lambda h: D0 * pow(T(h) / T0, -(C_atm + 1))
# (d / d0) ** (1/-(C_atm +1)) = T(h) / T0
# T(h) = T0 * ((d / d0) ** (1/-(C_atm +1)))
# T = lambda h: T0 - 6.5 * pow(10, -3) * h
# pow(10, -3) * h = (T0 - T) / 6.5
# h = (T0 - T) * 1000 / 6.5
h = (T0 - T0 * ((d_actual / D0) ** (1/-(C_atm +1)))) * 1000 / 6.5
print(f"optimum flight altitude: {h} [m]")
v_opt = v_min(w , s, d(h), cl_opt_jet_actual)
print(f"optimum range air speed: {v_opt} [m/s]")
v_cruise = v_min(w , s, d(0), cl_opt_jet_actual)
print(f"optimum range air speed: {v_cruise} [m/s]")

# best endurance
# Note: ct = TSFC [1/h] (convert to [1/s]: divide by 3600)
range_gradual_climb = range_climb_jets(v_opt, ct,  cl_opt_jet_actual, cd_opt, w , w - w_fuel)
print(f"range gradual climb: {range_gradual_climb / 1000}  [Km]")


# Douglas DC3 - Intersofia - CC0 - Propeller
w = 112130
s = 91.7
cp = 2.0288 * pow(10, -7) # [Kg/Ws]
eta = 0.75
cd0 = 0.0165
k = 0.0478
cl_max_endurance_propeller_actual = cl_max_endurance_propeller(cd0, k)
cl_actual = cl_max_endurance_propeller_actual
cd = 0.0165 - 0.0023  * cl_actual + 0.0478 * pow(cl_actual, 2)
print(f"\ndrag coefficient: {cd}")

h = 5000

# In the quiz they expect max range CL, though the endurance formula gives the
# correct result
print(f"max endurance lift coefficient: {cl_max_endurance_propeller_actual}")
v_max_endurance = v_min(w, s, d(h), cl_actual)
print(f"air speed: {v_max_endurance} [m/s]")
par_actual = par(cd, h, v_max_endurance, s)
m_f_propeller_actual = m_f_propeller(cp , eta, par_actual)
print(f"fuel flow: {m_f_propeller_actual} [Kg/s]")

w_fuel = 1600
w_final = w - w_fuel * G

range_climb_propellers_actual = range_climb_propellers(eta, cp, cl_actual, cd, w, w_final)
print(f"range gradual climb: {range_climb_propellers_actual / 1000}  [Km]")

t_hour = w_fuel / m_f_propeller_actual / 3600
print(f"range time: {t_hour} [h]")


# Transonic lift drag polar

w = 3500000
s = 500
h = 10000

m = [0.5, 0.6, 0.75, 0.8, 0.85, 0.88, 0.9, 0.92]
cd0 = [0.0160, 0.0160, 0.0163, 0.0171, 0.0179, 0.0199, 0.0226, 0.0302]
k = [0.058, 0.058, 0.059, 0.063, 0.069, 0.077, 0.084, 0.110]

p = []
for i, m in enumerate(m):
    v_air_actual = v_air(m, h)
    cl_actual = cl(w, h, v_air_actual, s)
    cd = cd0[i] * m + k[i] * pow(cl_actual, 2)
    p.append(m * cl_actual / cd)

print(p)