from equations import *

# Euro-ENAER Eaglet
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
print(f"ground run: {s_tof(v_lof_actual, a_tof_g_actual)} [m]")