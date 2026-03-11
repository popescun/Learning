from equations import *

v = 100
rc_actual = 15 # climb rate [m/s]
# rc = lambda v, gamma: v & math.sin(gamma)
gamma_actual = math.asin(rc_actual / v)
print(f"climb angle: {gamma_actual * 180 / math.pi} [degree]")
# dc = lambda v, gamma: v * math.cos(gamma)
dc_actual = 1000
print(f"climb distance in 1 second: {dc(v, gamma_actual)} [m]")
climb = dc_actual / dc(v, gamma_actual) * rc_actual
print(f"climb after {dc_actual} meters: {climb} [m]")

gamma_actual = 5 # [degrees]
rc_actual = 5
v_actual = rc_actual / math.sin(gamma_actual * math.pi / 180)
print(f"air speed: {v_actual} [m]")

gamma_actual = 10
v_actual = 200
print(f"rate of climb rc: {rc(v_actual, gamma_actual)} [m/s]")

# Boeing 737-400
h = 3000
w = 68050 * G # [Kg]
v = 220
t = 98000
s = 91.2
b = 28.9
e = 0.8
cd0 = 0.018
d_actual = 0.9046 # Kg/m3
cl_actual = cl(w, h, v, s)
print(f"Boeing 737-400 lift coefficient cl: {cl_actual}")
cd_actual = cd(cd0, cl_actual, b, s, e)
print(f"Boeing 737-400 drag coefficient cd: {cd_actual}")
gamma_by_density_actual = gamma_by_density(t, cd_actual, d_actual, v, s, w) * 180 / math.pi
print(f"Boeing 737-400 climb angle with given density: {gamma_by_density_actual} [degree]")
print(f"Boeing 737-400 climb rate with given density: {rc(v, gamma_by_density_actual)}")
gamma_actual = gamma(t, cd_actual, v, s, w, h) * 180 / math.pi
print(f"Boeing 737-400 climb angle: {gamma_actual} [degree]")
print(f"Boeing 737-400 climb rate: {rc(v, gamma_actual)}")

# Cessna C-172
G_actual = 9.81
MTOM = 1111 * G # maximum take-off mass [N]
v = 20
pa = 119000 # [W]
pr = 25200
rc_actual = (pa - pr) / MTOM
print(f"\nCessna C-172 climb rate for speed {v} [m/s]: {rc_actual} [m/s]")
v = 60
pa = 119000  # [W]
pr = 41000
rc_actual = (pa - pr) / MTOM
print(f"Cessna C-172 climb rate for speed {v} [m/s]: {rc_actual} [m/s]")

# Cessna 208 Caravan
pa_max = 647000 # [W]
cd0 = 0.02
s = 26
ar = 9.55
b = math.sqrt(ar * s)
e = 0.8
MTOM = 3995 * G_actual
d_actual = 1.225 # [Kg/m3]
v = 95
cl_actual = cl(MTOM, 0, v, s)
print(f"\nCessna 208 Caravan lift coefficient cl: {cl_actual}")
cd_actual = cd(cd0, cl_actual, b, s, e)
print(f"Cessna 208 Caravan drag coefficient cd: {cd_actual}")
dr_actual = dr_by_density(cd_actual, d_actual, v, s)
print(f"Cessna 208 Caravan drag D: {dr_actual} [N]")
t_actual = pa_max / v
print(f"Cessna 208 Caravan thrust T: {t_actual} [N]")
rc_actual = rc_by_forces(t_actual, dr_actual, MTOM, v)
print(f"Cessna 208 Caravan climb rate: {rc_actual} [m/s]")

MTOM -= 500 * G_actual
cl_actual = cl(MTOM, 0, v, s)
print(f"\nCessna 208 Caravan lift coefficient cl: {cl_actual}")
cd_actual = cd(cd0, cl_actual, b, s, e)
print(f"Cessna 208 Caravan drag coefficient cd: {cd_actual}")
dr_actual = dr_by_density(cd_actual, d_actual, v, s)
print(f"Cessna 208 Caravan drag D: {dr_actual} [N]")
t_actual = pa_max / v
print(f"Cessna 208 Caravan thrust T: {t_actual} [N]")
rc_actual = rc_by_forces(t_actual, dr_actual, MTOM, v)
print(f"Cessna 208 Caravan climb rate: {rc_actual} [m/s]")

MTOM += 500 * G_actual
cl_actual = cl(MTOM, 0, v, s)
print(f"\nCessna 208 Caravan lift coefficient cl: {cl_actual}")
cd_actual = cd(cd0, cl_actual, b, s, e)
print(f"Cessna 208 Caravan drag coefficient cd: {cd_actual}")
dr_actual = dr_by_density(cd_actual, d_actual, v, s)
print(f"Cessna 208 Caravan drag D: {dr_actual} [N]")
pa_max = 680000
t_actual = pa_max / v
print(f"Cessna 208 Caravan thrust T: {t_actual} [N]")
rc_actual = rc_by_forces(t_actual, dr_actual, MTOM, v)
print(f"Cessna 208 Caravan climb rate: {rc_actual} [m/s]")

# Cessna C-172 (part II)
MTOM = 1111 * G_actual
v = 33
pa_sea_level = 119000
pr_sea_level = 21200
rc_actual = rc_by_power(pa_sea_level, pr_sea_level, MTOM)
print(f"\nCessna 208 Caravan climb rate at sea level: {rc_actual} [m/s]")
v = 38
pa_2000 = 105000
pr_2000 = 26000
rc_actual = rc_by_power(pa_2000, pr_2000, MTOM)
print(f"Cessna 208 Caravan climb rate at 2000 meters: {rc_actual} [m/s]")

v_eas = 150
d_actual = 1.0555
print(f"\nv_tas at FL50: {v_tas(v_eas, d_actual)} [m/s]")
d_actual = 0.90464
print(f"v_tas at FL100: {v_tas(v_eas, d_actual)} [m/s]")

h = 9000
a_actual = a(h)
m = 0.7
print(f"speed sound at {h} meters: {a_actual} [m/s]")
v_tas_actual = v_air(m, h)
print(f"true air speed v at {h} meters: {v_tas_actual} [m/s]")
d_actual = d(h)
print(f"air density d at {h} meters: {d_actual} [Kg/m3]")
# v_tas  = lambda v_eas, d: math.sqrt(D0 / d) * v_eas
v_eas = v_tas_actual / math.sqrt(D0 / d_actual)
print(f"estimated air speed v at {h} meters: {v_eas} [m/s]")
h1 = 9100
v_tas1 = v_air(m, h1)
print(f"true air speed v at {h1} meters: {v_tas1} [m/s]")
delta_v_tas_actual = delta_v_tas(v_eas, h, h1)
print(f"change of true air speed from {h} to {h1} meters: {delta_v_tas_actual} [m^-1]")
rc_r_actual = rc_r(v_tas_actual, v_eas, h, h1)
print(f"climb rate ration from {h} to {h1} meters: {rc_r_actual}")

# Cessna Citation II
w = 55000
s = 30
cd0 = 0.02
k = 0.044
d_actual = 1.225
v_actual = 100
gamma = -3 * math.pi / 180 # [rad]
cl_actual = cl_by_density(w, d_actual, v_actual, s)
print(f"\nCessna Citation II lift coefficient cl: {cl_actual}")
cd_actual = cd_with_k(cd0, cl_actual, k)
print(f"Cessna Citation II drag coefficient cd: {cd_actual}")
dr_actual = dr_by_density(cd_actual, d_actual, v_actual, s)
print(f"Cessna Citation drag dr: {dr_actual} [N]")
tr = dr_actual + w * math.sin(gamma)
print(f"Cessna Citation required thrust tr: {tr} [N]")

# Glider
h = 1250
horizontal_distance = 23850
gamma_actual = -math.atan(h / horizontal_distance) * 180 / math.pi
print(f"Glider minimum glide angle: {gamma_actual} [degree]")

# Path performance
h_a = [0, 300, 600, 900, 1200, 1500, 1800]
v_ias = 50
rc_a = [4.6, 4.4, 4.2, 4, 3.8, 3.6, 3.4]
v_tas_actual = [50.00, 50.73, 51.47, 52.23, 53.00, 53.80, 54.61]
ct_total = 0
ground_distance_total = 0
for i, h_current in enumerate(h_a):
    if i == 0:
        continue
    ct_current = ct(h_a[i-1], h_current, rc_a[i-1])
    print(f"total time to climb from {h_a[i-1]} to {h_current}: {ct_current} [s]")
    distance_current = v_tas_actual[i-1] * ct_current
    ct_total += ct_current
    ground_distance_total += distance_current
print(f"total time to climb from {h_a[0]} to {h_a[len(h_a) - 1]}: {ct_total} [s]")
print(f"total ground distance to climb {h_a[len(h_a) - 1]}: {ground_distance_total / 1000} [km]")

# Energy height
m = 0.5
h = 5000
w = 400000
G = 9.81
mass = w / G_actual
v_air_actual = v_air(m, h)
print(f"\ntrue air speed v at {h} meters: {v_air_actual} [m/s]")
eh_actual = eh(h, v_air_actual)
print(f"energy height at {h} meters: {eh_actual} [J/N]")
e_total = mass * G * h + mass * pow(v_air_actual, 2) / 2
print(f"potential energy at {h} meters: {e_total} [J]")
# ek = mass * pow(v, 2) / 2 = ep
v_air_actual = math.sqrt(2  * e_total / mass)
print(f"speed if e_total = ek: {v_air_actual} [m/s]")

m = 1.0
h = 15000
v_air_actual = v_air(m, h)
print(f"\nmaximum speed at {h} meters: {v_air_actual} [m/s]")

v_actual = math.sqrt( h * 2 * G)
print(f"\nmaximum speed: {v_actual} [m/s]")

rc_actual = 50
# rc = lambda v, gamma: v * math.sin(gamma * math.pi / 180)
gamma = 5 # this is a guess that this climb angle is used;
          # somehow it gives a close value 573 to the expected one 575
v_air_actual =rc_actual / math.sin(gamma * math.pi / 180)
print(f"\nmaximum speed at {h} meters: {v_air_actual} [m/s]")

# Assignment

# Cessna 208 Caravan

# Q1
cd0 = 0.02
s = 26
b = 15.86
e = 0.8
MTOM = 3995 * G_actual
v = 95
pa_max = 647000
# rc = 8.46
b += 0.1
cd0 += cd0 / 100
e += 10 * e / 100
MTOM += 50 * G_actual
cl_actual = cl(MTOM, 0, v, s)
print(f"\nCessna 208 Caravan lift coefficient cl: {cl_actual}")
cd_actual = cd(cd0, cl_actual, b, s, e)
print(f"Cessna 208 Caravan drag coefficient cd: {cd_actual}")
dr_actual = dr_by_density(cd_actual, d_actual, v, s)
print(f"Cessna 208 Caravan drag D: {dr_actual} [N]")
t_actual = pa_max / v
print(f"Cessna 208 Caravan thrust T: {t_actual} [N]")
rc_actual = rc_by_forces(t_actual, dr_actual, MTOM, v)
print(f"Cessna 208 Caravan climb rate: {rc_actual} [m/s]")

# Q2
cd0 = 0.02
k = 0.0352
cl_opt = math.sqrt( cd0 / k)
print(f"\noptimum lift coefficient cl: {cl_opt}")
cd_actual = cd_with_k(cd0, cl_opt, k)
print(f"drag coefficient cd: {cd_actual}")
gamma_glide_min_actual = gamma_glide_min(cl_opt, cd_actual)
print(f"gamma glide min: {gamma_glide_min_actual}")
xa = 1000
xb = 715
ha = 120
hb = 50
# tan(gamma) = (ha - hb_glide) / xa
hb_glide = ha  - math.tan(gamma_glide_min_actual) * xa
print(f"height after gliding distance {xa}: {hb_glide} [m]")

# Q3
v_wind = 20
v = 100 - v_wind
rc_actual = 15
# rc = lambda v, gamma: v * math.sin(gamma * math.pi / 180)
gamma_actual = math.asin(rc_actual / v) * 180 / math.pi
print(f"\nclimb angle: {gamma_actual} [degree]")
dc_actual = 1000
print(f"climb distance in 1 second: {dc(v, gamma_actual)} [m]")
climb = dc_actual / dc(v, gamma_actual) * rc_actual
print(f"climb after {dc_actual} meters: {climb} [m]")

# Q4
# Boeing 737-400
h = 3000
G_actual = 9.81
w = 68050 * G_actual
v = 220
t = 2 * 49000
s = 91.2
b = 28.9
e = 0.8
cd0 = 0.018
cl_actual = cl(w, h, v, s)
print(f"\nBoeing 737-400 lift coefficient cl: {cl_actual}")
cd_actual = cd(cd0, cl_actual, b, s, e)
print(f"Boeing 737-400 drag coefficient cd: {cd_actual}")
dr_actual = dr(cd_actual, h, v, s)
print(f"Boeing 737-400 drag D: {dr_actual} [N]")
rc_actual = rc_by_forces(t, dr_actual, w, v)
print(f"Boeing 737-400 climb rate rc: {rc_actual} [m/s]")

# need to find alpha and gamma from full eq of motions:
# t * math.cos(alpha) - dr_actual - w * math.sin(gamma) = 0
# l_actual - w * math.cos(gamma) + t * math.sin(alpha) = 0

# todo: this is not quite correct, we must not approximate alpha
# todo: see chatgpt sol for full eq of motion
l_actual = l(cl_actual, h, v, s)
print(f"Boeing 737-400 lift L: {l_actual} [N]")
gamma_actual = math.atan((t - dr_actual) / l_actual) * 180 / math.pi
print(f"\nBoeing 737-400 climb angle: {gamma_actual} [degree]")
rc_actual_1 = rc(v, gamma_actual)
print(f"Boeing 737-400 rate of climb rc: {rc_actual_1} [m/s]")

# gamma = 4.513
# gamma = 4.473
rc_actual = 17.235 # by guessing
# rc_actual = rc(v, 4.473)
gamma_actual = math.asin(rc_actual / v) * 180 / math.pi
print(f"\nBoeing 737-400 climb angle: {gamma_actual} [degree]")
print(f"Boeing 737-400 percentage: {(rc_actual - 17.257) / rc_actual * 100} [%]")

# from chatgpt

# need to find alpha and gamma from full eq of motions:
# t * math.cos(alpha) - dr_actual - w * math.sin(gamma) = 0
# l_actual - w * math.cos(gamma) + t * math.sin(alpha) = 0

K = (w**2 - t**2 - dr_actual**2 - l_actual**2) / (2*t)
R = math.sqrt(l_actual**2 + dr_actual**2)

alpha = math.atan2(dr_actual, l_actual) + math.asin(K / R)

gamma_actual = math.atan2(
    t * math.cos(alpha) - dr_actual,
    t * math.sin(alpha) + l_actual
) * 180 / math.pi

print(f"\nBoeing 737-400 angle of attack: {alpha } [degree]")
print(f"Boeing 737-400 climb angle: {gamma_actual} [degree]")
rc_actual = rc(v, gamma_actual)
print(f"Boeing 737-400 rate of climb rc: {rc_actual} [m/s]")