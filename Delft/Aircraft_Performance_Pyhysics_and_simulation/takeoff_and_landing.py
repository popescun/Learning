import math

from climb_and_descent import v_actual
from equations import *

# Euro-ENAER Eaglet

# take-off ground run
h = 0
w = 8500
s = 9.84
cl_max = 1.4
cl_g = 0.8
cd_g = 0.03 + 0.0637 * pow(cl_g, 2)
print(f"drag coefficient:{cd_g}")
p_br = 115000
miu = 0.05
# d_actual = 1.255
v_min_tof = v_min(w, s, d(0), cl_max) # v_min depends on the max lift coefficient
v_lof_actual = v_lof(w, s, d(0), cl_max)
# niu = p_a / p_br
niu_tof = 0.5
niu_airborne = 0.8
pa = p_br * niu_tof
print(f"min take-off speed: {v_min_tof} [m/s]")
print(f"lift-off speed: {v_lof_actual} [m/s]")
v_tof_g_actual = v_tof_g(v_lof_actual)
print(f"average take off speed: {v_tof_g_actual} [m/s]")
dr_g_actual = dr(cd_g, 0, v_tof_g_actual, s)
print(f"average take off drag: {dr_g_actual} [N]")
l_g_actual = l(cl_g, 0, v_tof_g_actual, s)
print(f"average take off lift: {l_g_actual} [N]")
t_g = pa / v_tof_g_actual
print(f"average take-off thrust: {t_g} [N]")
a_tof_g_actual = a_tof_g(t_g, w, l_g_actual, dr_g_actual, miu)
print(f"average take-off acceleration: {a_tof_g_actual} [m/s2]")
x_ground_actual = x_ground(v_lof_actual, a_tof_g_actual)
print(f"ground run distance: {x_ground_actual} [m]")


# airborne take-off

gamma = 5
# n = l_trans / w
h_scr = 15.2
n = 1.15 * G
x_trans_actual = x_trans(v_lof_actual, gamma)
print(f"\ntransition ground distance: {x_trans_actual} [m]")
x_climb_actual = x_climb(h_scr, v_lof_actual, gamma)
print(f"climb ground distance: {x_climb_actual} [m]")
x_total_airborne_actual =  x_total_airborne(x_trans_actual, x_climb_actual)
print(f"total airborne distance: {x_total_airborne_actual} [m]")
x_total_tof_actual = x_total_tof(x_ground_actual, x_total_airborne_actual)
print(f"total take-off distance: {x_total_tof_actual} [m]")

# take-of at 200 meters altitude
h = 2000
d_actual = 1.0065
pa_2000 = pa * d_actual / D0
print(f"\npower at sea level: {pa_2000} [m/s]")
v_min_tof = v_min(w, s, d_actual, cl_max) # v_min depends on the max lift coefficient
v_lof_actual = v_lof(w, s, d_actual, cl_max)
# niu = p_a / p_br
print(f"min take-off speed: {v_min_tof} [m/s]")
print(f"lift-off speed: {v_lof_actual} [m/s]")
v_tof_g_actual = v_tof_g(v_lof_actual)
print(f"average take off speed: {v_tof_g_actual} [m/s]")
dr_g_actual = dr(cd_g, h, v_tof_g_actual, s)
print(f"average take off drag: {dr_g_actual} [N]")
l_g_actual = l(cl_g, h, v_tof_g_actual, s)
print(f"average take off lift: {l_g_actual} [N]")
t_g = pa_2000 / v_tof_g_actual
print(f"average take-off thrust: {t_g} [N]")
a_tof_g_actual = a_tof_g(t_g, w, l_g_actual, dr_g_actual, miu)
print(f"average take-off acceleration: {a_tof_g_actual} [m/s2]")
x_ground_actual = x_ground(v_lof_actual, a_tof_g_actual)

x_trans_actual = x_trans(v_lof_actual, gamma)
print(f"\ntransition ground distance: {x_trans_actual} [m]")
x_climb_actual = x_climb(h_scr, v_lof_actual, gamma)
print(f"climb ground distance: {x_climb_actual} [m]")
x_total_airborne_actual =  x_total_airborne(x_trans_actual, x_climb_actual)
print(f"total airborne distance: {x_total_airborne_actual} [m]")
x_total_tof_actual_prev = x_total_tof_actual
x_total_tof_actual = x_total_tof(x_ground_actual, x_total_airborne_actual)
print(f"total take-off distance: {x_total_tof_actual} [m]")
distance_change = (x_total_tof_actual - x_total_tof_actual_prev) / x_total_tof_actual_prev * 100
print(f"\ndistance change: {distance_change} [%]")

# take-off at sea level with higher weight
h = 0
w = 9000
d_actual = 1.225 # sea level
pa = p_br * niu_tof
print(f"\npower at sea level: {pa} [m/s]")
v_min_tof = v_min(w, s, d_actual, cl_max) # v_min depends on the max lift coefficient
v_lof_actual = v_lof(w, s, d_actual, cl_max)
# niu = p_a / p_br
print(f"min take-off speed: {v_min_tof} [m/s]")
print(f"lift-off speed: {v_lof_actual} [m/s]")
v_tof_g_actual = v_tof_g(v_lof_actual)
print(f"average take off speed: {v_tof_g_actual} [m/s]")
dr_g_actual = dr(cd_g, h, v_tof_g_actual, s)
print(f"average take off drag: {dr_g_actual} [N]")
l_g_actual = l(cl_g, h, v_tof_g_actual, s)
print(f"average take off lift: {l_g_actual} [N]")
t_g = pa / v_tof_g_actual
print(f"average take-off thrust: {t_g} [N]")
a_tof_g_actual = a_tof_g(t_g, w, l_g_actual, dr_g_actual, miu)
print(f"average take-off acceleration: {a_tof_g_actual} [m/s2]")
x_ground_actual = x_ground(v_lof_actual, a_tof_g_actual)

x_trans_actual = x_trans(v_lof_actual, gamma)
print(f"transition ground distance: {x_trans_actual} [m]")
x_climb_actual = x_climb(h_scr, v_lof_actual, gamma)
print(f"climb ground distance: {x_climb_actual} [m]")
x_total_airborne_actual =  x_total_airborne(x_trans_actual, x_climb_actual)
print(f"total airborne distance: {x_total_airborne_actual} [m]")
x_total_tof_actual_prev = x_total_tof_actual_prev # previous at sea level
x_total_tof_actual = x_total_tof(x_ground_actual, x_total_airborne_actual)
print(f"total take-off distance: {x_total_tof_actual} [m]")
distance_change = (x_total_tof_actual - x_total_tof_actual_prev) / x_total_tof_actual_prev * 100
print(f"distance change: {distance_change} [%]")

# airborne landing
s = 93.5
b = 28.1
cl_max = 2.4
cd0 = 0.025
ae = 6
cd = cd0 + pow(cl_max, 2) / (math.pi * ae)
print(f"drag coefficient:{cd}")
w = 375889
t0 = 65000
h = 0
t = t0 * d(h) / D0
h_scr = 15.2
gamma_ap = 3
delta_n = 0.1
# l_landing = delta_n * w
# v_landing_actual = v_landing(l_landing, s, d(h_scr), cl_max)
# print(f"\nv_landing_actual : {v_landing_actual} [m/s]")
# l = cl* d / 2 * pow(v, 2) * s
# v = math.sqrt(2 * l / (cl * d * s)?
# v_landing_actual = math.sqrt(2 * l_landing / (cl_max * d(h_scr) * s))
# print(f"v_landing_actual : {v_landing_actual} [m/s]")
print(f"delta_n: {delta_n} [N]")
r_landing_actual = r_landing(w, s, d(h_scr), cl_max, delta_n)
print(f"landing radius: {r_landing_actual} [m]")
h_flare_actual = h_flare(r_landing_actual)
print(f"flare height: {h_flare_actual} [m]")
x_ap_actual = x_ap(h_scr, h_flare_actual)
print(f"approach distance: {x_ap_actual} [m]")
x_flare_actual = x_flare(r_landing_actual)
print(f"flare distance: {x_flare_actual} [m]")
x_landing_airborne_distance_actual = x_landing_airborne_distance(x_ap_actual, x_flare_actual)
print(f"landing airborne distance: {x_landing_airborne_distance_actual} [m]")
x_landing_airborne_distance_by_radius_actual = x_landing_airborne_distance_by_radius(r_landing_actual, h_scr, gamma_ap)
print(f"landing airborne distance by radius: {x_landing_airborne_distance_by_radius_actual} [m]")
x_landing_airborne_distance_baseline = x_landing_airborne_distance_by_radius_actual

w_reduced = w - w / 100
r_landing_actual = r_landing(w_reduced, s, d(h_scr), cl_max, delta_n)
print(f"\nlanding radius: {r_landing_actual} [m]")
x_landing_airborne_distance_by_radius_actual_prev = x_landing_airborne_distance_by_radius_actual
x_landing_airborne_distance_by_radius_actual = x_landing_airborne_distance_by_radius(r_landing_actual, h_scr, gamma_ap)
print(f"landing airborne distance by radius: {x_landing_airborne_distance_by_radius_actual} [m]")
percentage = (x_landing_airborne_distance_by_radius_actual_prev - x_landing_airborne_distance_by_radius_actual) / x_landing_airborne_distance_by_radius_actual_prev * 100
print(f"percentage: {percentage} [%]")

d_reduced = d(h_scr) - d(h_scr) / 100
r_landing_actual = r_landing(w, s, d_reduced, cl_max, delta_n)
print(f"\nlanding radius: {r_landing_actual} [m]")
x_landing_airborne_distance_by_radius_actual_prev = x_landing_airborne_distance_by_radius_actual_prev
x_landing_airborne_distance_by_radius_actual = x_landing_airborne_distance_by_radius(r_landing_actual, h_scr, gamma_ap)
print(f"landing airborne distance by radius: {x_landing_airborne_distance_by_radius_actual} [m]")
percentage = (x_landing_airborne_distance_by_radius_actual_prev - x_landing_airborne_distance_by_radius_actual) / x_landing_airborne_distance_by_radius_actual_prev * 100
print(f"percentage: {percentage} [%]")

gamma_increased = gamma_ap + 1
r_landing_actual = r_landing(w, s, d(h_scr), cl_max, delta_n)
print(f"\nlanding radius: {r_landing_actual} [m]")
x_landing_airborne_distance_by_radius_actual = x_landing_airborne_distance_by_radius(r_landing_actual, h_scr, gamma_increased)
print(f"landing airborne distance by radius: {x_landing_airborne_distance_by_radius_actual} [m]")
percentage = (x_landing_airborne_distance_by_radius_actual - x_landing_airborne_distance_baseline) / x_landing_airborne_distance_baseline * 100
print(f"percentage: {percentage} [%]")

h_scr_increased = h_scr + 1
r_landing_actual = r_landing(w, s, d(h_scr_increased), cl_max, delta_n)
print(f"\nlanding radius: {r_landing_actual} [m]")
x_landing_airborne_distance_by_radius_actual = x_landing_airborne_distance_by_radius(r_landing_actual, h_scr_increased, gamma_ap)
print(f"landing airborne distance by radius: {x_landing_airborne_distance_by_radius_actual} [m]")
percentage = (x_landing_airborne_distance_by_radius_actual - x_landing_airborne_distance_baseline) / x_landing_airborne_distance_baseline * 100
print(f"percentage: {percentage} [%]")

delta_n_increased = 0.15
r_landing_actual = r_landing(w, s, d(h_scr), cl_max, delta_n_increased)
print(f"\nlanding radius: {r_landing_actual} [m]")
x_landing_airborne_distance_by_radius_actual = x_landing_airborne_distance_by_radius(r_landing_actual, h_scr, gamma_ap)
print(f"landing airborne distance by radius: {x_landing_airborne_distance_by_radius_actual} [m]")
percentage = (x_landing_airborne_distance_by_radius_actual - x_landing_airborne_distance_baseline) / x_landing_airborne_distance_baseline * 100
print(f"percentage: {percentage} [%]")

cl_max_increased = 2.5
w_increased = w + 100 * G
r_landing_actual = r_landing(w_increased, s, d(h_scr), cl_max_increased, delta_n)
print(f"\nlanding radius: {r_landing_actual} [m]")
x_landing_airborne_distance_by_radius_actual = x_landing_airborne_distance_by_radius(r_landing_actual, h_scr, gamma_ap)
print(f"landing airborne distance by radius: {x_landing_airborne_distance_by_radius_actual} [m]")
percentage = (x_landing_airborne_distance_by_radius_actual - x_landing_airborne_distance_baseline) / x_landing_airborne_distance_baseline * 100
print(f"percentage: {percentage} [%]")

# increasing thrust does not affect the landing distance, so percentage change is 0
t_increased = t0 + 5 / 100 * t0

# Cessna Citation
w = 60000
s = 30
cl_max = 2
cd0 = 0.07
ae = 5.8
cl_g = 1.1 # lift coefficient ground-run altitude
cd = cd0 + pow(cl_g, 2) / (math.pi * ae)
print(f"drag coefficient:{cd}")
gamma_ap = 3
v_min_actual = v_min(w, s, d(0), cl_max)
print(f"\nlanding min speed: {v_min_actual} [m/s]")
miu_r = 0.4 # friction coefficient
t_trans = 2
k = 1.2 # is 1.3 in general
v_t_actual = v_t(v_min_actual, k)
print(f"touch-down speed: {v_t_actual} [m/s]")
x_trans_ground_run_actual = x_trans_ground_run(v_t_actual, t_trans)
print(f"landing transition ground-run distance: {x_trans_ground_run_actual} [m]")
v_ground_run_actual = v_ground_run(v_t_actual)
print(f"ground-run speed: {v_ground_run_actual} [m/s]")
l_actual = l(cl_g, 0, v_ground_run_actual, s)
print(f"lift l: {l_actual} [N]")
dr_actual = dr(cd, 0, v_ground_run_actual, s)
print(f"drag dr: {dr_actual} [N]")
# Dg = µ (Nn + Nm )= µ(W− L)
dr_friction_actual = dr_friction(w, l_actual, miu_r)
print(f"friction drag: {dr_friction_actual} [N]")
a_ground_run_actual = a_ground_run(w, 0, dr_actual, l_actual, miu_r)
print(f"ground run deceleration: {a_ground_run_actual} [m/s2]")
x_brake_ground_run_actual = x_brake_ground_run(w, 0, dr_actual, l_actual, miu_r, s, cl_max, k)
print(f"ground run brake distance:{x_brake_ground_run_actual} [m]")
x_ground_run_total_actual = x_ground_run_total(x_trans_ground_run_actual, x_brake_ground_run_actual)
print(f"ground run total distance: {x_ground_run_total_actual} [m]")

# Assignment
x_total_tof_actual = 1800
field_length_tof_actual = field_length_tof(x_total_tof_actual)
print(f"\nfield length: {field_length_tof_actual} [m]")

v_forward = 40
head_wind_speed = 10
alpha = 0.5 # [degree]
w = 100000
miu = 0.03
s = 45
t0 = 2 * 12000
cl = 0.25 # correspond to alpha in polar
cd = 0.026 # correspond to cl in polar

v_actual = v_forward -  head_wind_speed * math.cos(math.degrees(alpha))
print(f"\nfor speed: {v_actual} [m/s]")
m_actual = v_actual / a(0)
print(f"mach number: {m_actual} [m/s]")
t_g = t0 * (1 - 1.2 * m_actual + 0.8 * pow(m_actual, 2))
dr_actual = dr(cd, 0, v_actual, s)
print(f"drag dr: {dr_actual} [N]")
l_actual = l(cl, 0, v_actual, s)
print(f"lift l: {l_actual} [N]")
a_tof_g_actual = a_tof_g(t_g, w, l_actual, dr_actual, miu)
print(f"Average take-off acceleration: {a_tof_g_actual} [m/s]")

v_lof_actual = 84
# v_lof_actual = v_actual * a_tof_g_actual * t
t = (v_lof_actual - v_actual) / a_tof_g_actual
print(f"time to reach loft-off speed: {t} [m/s]")

tail_wind_speed = 3
v_actual = v_forward + tail_wind_speed * math.cos(math.degrees(alpha))
print(f"\nfor speed: {v_actual} [m/s]")
m_actual = v_actual / a(0)
print(f"mach number: {m_actual} [m/s]")
t_g = t0 * (1 - 1.2 * m_actual + 0.8 * pow(m_actual, 2))
dr_actual = dr(cd, 0, v_actual, s)
print(f"drag dr: {dr_actual} [N]")
l_actual = l(cl, 0, v_actual, s)
print(f"lift l: {l_actual} [N]")
a_tof_g_actual = a_tof_g(t_g, w, l_actual, dr_actual, miu)
print(f"Average take-off acceleration: {a_tof_g_actual} [m/s]")
t = (v_lof_actual - v_actual) / a_tof_g_actual
print(f"time to reach loft-off speed: {t} [m/s]")

v_min_actual = 40
v_min_touchdown_required = 1.1 * v_min_actual
print(f"\nmin touchdown speed required: {v_min_touchdown_required} [m/s]")
water_current_speed_to_east = 5
wind_speed_to_east = 10
carrier_speed = 12
# the carrier should turn and goes against water current at max speed
# this will allow the aircraft to land at min air speed, considering
# there is also a wind from the tail of the aircraft
print(f"carrier speed to west: {carrier_speed} [m/s]")
v_actual = v_min_touchdown_required - (carrier_speed - water_current_speed_to_east) - wind_speed_to_east
print(f"touchdown speed relative to the deck: {v_actual} [m/s]")
