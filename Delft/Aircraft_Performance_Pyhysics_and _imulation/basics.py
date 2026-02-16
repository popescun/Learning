from equations import *
from matplotlib import pyplot as plt

print(f"C_atm: {C_atm}")

h = 5000
print(f"air temperature at {h} meters altitude: {T(h)} [K]")
print(f"air pressure at {h} meters altitude: {p(h)} [N/m2]")

h = 5000
v = 200
s = 35.5
l_given = 450000 # equal to aircraft weight
print(f"air density at {h} meters altitude: {d(h)} [kg/m3]")
print(f"lift coefficient at constant altitude: {cl(l_given, h, v, s)}")
l_given -= 50000
print(f"lift coefficient after fuel is burnt: {cl(l_given, h, v, s)}")

# Boeing 747-400
b = 68.5
s = 554
print(f"Boeing 747-400 aspect ratio: {ar(b, s)}")
# Oswald efficiency factor, typically ranging from 0.7 to 0.85
e = 0.8
cl_given = 0.86 # arbitrary
print(f"Boeing 747-400 induced drag coefficient: {id(cl_given, b, s, e)}")
cl_given = 0.4 # from exercise
cd0 = 0.0225 # cd to cl diagram from exercise
print(f"Boeing 747-400 induced drag coefficient for cl={cl_given}: {id(cl_given, b, s, e)}")
print(f"Boeing 747-400 drag coefficient for cd0={cd0}: {cd(cd0, cl_given, b, s, e)}")
l_given = 300000 # equal to aircraft weight
h = 10000
v = math.sqrt(2 * l_given / (cl_given * d(h) * s))
print(f"air density at {h} meters altitude: {d(h)} [kg/m3]")
print(f"velocity: {v}")
print(f"Boeing 747-400 velocity for lift={l}, cl={cl} at altitude {h} meters: {v}")
cd_actual = 0.02
print(f"Boeing 747-400 drag for cl={cl}, cd0={cd0}, v={v}: {dr(cd_actual, h, v, s)} [Kg]")
print(f"Boeing 747-400 drag for cl={cl}, cd0={cd0}, v={v}: {dr(cd_actual, h, v, s) * G} [N]")
print(f"Boeing 747-400 drag for lift={l}, cl={cl_given}, cd={cd_actual}: {(cd_actual * l_given / cl_given) * G} [N]")

# glider
b = 18
s = 11.8
print(f"Glider aspect ratio: {ar(b, s)}")
print(f"Glider induced drag coefficient: {id(0.86, b, s, e)}")

# Propulsion

# Alpha Jet
t = 10000
vj = 407.3
m = 28.1
# t = lambda m, vj, v: m * (vj - v)
v = vj - t / m
print(f"Alpha Jet flight speed: {v} [m/s]")
print(f"Alpha Jet power available: {pa(m, vj, v)} [W]")
print(f"Alpha Jet propulsive efficiency: {ej(vj, v)}")
print(f"Alpha Jet jet power: {pj(m, vj, v)} [W]")
et = 65.3
print(f"Alpha Jet total efficiency: {ej(vj, v) * et}")

# CFM56-3 turbofan engine on Boeing 737-300.
bpr = 6 # by-pass ratio
t = 111000 # thrust in [N]
h = 10000
m = 0.8 # mach speed
fa = 3 # frontal area [m2]
fac = 0.55 # frontal area of the core [m2]
print(f"speed of sound at {h} meters altitude: {a(h)} [m/s]")
print(f"Boeing 737-300 speed at {h} meters altitude and mach {m}: {v_air(m, h)} [m/s]")
print(f"air density at {h} meters altitude: {d(h)} [kg/m3]")
print(f"total mass air flow at {h} meters altitude: {m_flow(h, fa, v_air(m, h))} [kg/s]") # is unit correct ?
# vj = v + t / m
print(f"jet velocity: {v_air(m, h) + t / m_flow(h, fa, v_air(m, h))} [m/s]")
# m_fan = m_total - m_core
# bpr = m_fan / m_core = (m_total - m_core) / m_core
# mcore = m_total / (1 + bpr)
print(f"mass flow to the core engine: {m_flow(h, fa, v_air(m, h)) / (1 + bpr)} [Kg/s]")
print(f"mass flow to the core engine: {296.46 / (1 + bpr)} [kg/s]")
print(f"mass flow to the fan: {bpr * m_flow(h, fa, v_air(m, h)) / (1 + bpr)} [kg/s]")
print(f"mass flow to the fan: {bpr * 296.46 / (1 + bpr)} [kg/s]")
# m_flow = lambda h, fa, v: d(h) * fa * v
# v = m_flow / (d(h) * fa) -> this gives different value than deduced from thrust formula?
print(f"the core jet velocity: {(296.46 / (1 + bpr)) / (d(h) * fac)} [m/s]")
# alternative
# t = lambda m, vj, v: m * (vj - v)
# vj = t / m + v
print(f"the core jet velocity: {t / (296.46 / (1 + bpr)) + v_air(m, h)} [m/s]")
print(f"the fan jet velocity: {t / (bpr * 296.46/ (1 + bpr)) + v_air(m, h)} [m/s]")

# MD-11 aircraft
w = 2805000
cl_max = 1.67
s = 338.9
print(f"MD-11 min airspeed at sea level: {v_min(w, s, D0, cl_max)} [m/s]")

# Airbus A320-200
w = 78000 * G # [N]
print(f"Airbus A320-200  weight: {w} [N]")
t0 = 111000
# v_air = lambda m, h: m * a(h)
k1 = 1.1
k2 = 0.8
cd0 = 0.02
e = 0.8
s = 122.6
ar = 9.2
b = math.sqrt(ar * s)
print(f"speed of sound at sea level: {a(0)} [m/s]")
# cl = math.sqrt((cd -cd0) * math.pi * ar * e)
# print(f"Airbus A320-200 lift coefficient at sea level: {cl()} [m/s]")
# print(f"Airbus A320-200 airspeed at sea level: {v_air()} [m/s]")

# cl = lambda l, h, v, s: 2 * l / (d(h) * pow(v, 2) * s)
# l = w
# v_max = lambda w, s, d, cl: math.sqrt(w/s * 2 /d * 1 / cl)

fig, ax = plt.subplots()
fig_drag, ax_drag = plt.subplots()

ax.set_title(f"lift and drag coefficients")
ax.set_xlabel('v')
ax.set_ylabel("cl, cd")

ax_drag.set_title(f"thrust and drag")
ax_drag.set_xlabel('v')
ax_drag.set_ylabel("thrust, drag")

w = 407.906

# v = 308.65
h = 0
data_v = []
data_cl = []
data_cd = []
data_drag = []
data_thrust = []
for v  in range(50, 400, 1):
    print(f"air density at sea level: {d(h)} [Kg/m3]")
    cl_actual = cl(w, h, v, s)
    print(f"Airbus A320-200 lift coefficient cl for speed {v}: {cl_actual}")
    cd_actual = cd(cd0, cl_actual, b, s, e)
    print(f"Airbus A320-200 drag coefficient cd for speed {v}: {cd_actual}")
    t_actual = t_from_static(2 * t0, v, h, k1, k2)
    print(f"Airbus A320-200  thrust: {t_actual} [N]")
    dr_actual = dr(cd(cd0, cl_actual, b, s, e), h, v, s)
    print(f"Airbus A320-200 drag for speed {v}: {dr_actual}")
    data_v.append(v)
    data_cl.append(cl_actual)
    data_cd.append(cd_actual)
    data_drag.append(dr_actual)
    data_thrust.append(t_actual)

ax.plot(  data_v, data_cl, linewidth=1)
ax.plot(  data_v, data_cd, linewidth=1)
ax_drag.plot(  data_v, data_drag, linewidth=1)
ax_drag.plot(  data_v, data_thrust, linewidth=1)

ax.legend(["lift coefficient", "drag coefficient"])
ax_drag.legend(["drag", "thrust"])

# plt.show()

