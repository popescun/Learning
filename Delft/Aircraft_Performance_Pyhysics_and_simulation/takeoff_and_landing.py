from equations import *

# Euro-ENAER Eaglet

# take-off ground run
w = 8500
s = 9.84
cl_max = 1.4
cl_g = 0.8
cd_g = 0.03 + 0.0637 * pow(cl_g, 2)
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