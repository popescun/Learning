import math
from equations import *

print(f"C_atm: {C_atm}")

h = 5000
print(f"air temperature at {h} meters altitude: {T(h)} [K]")
print(f"air pressure at {h} meters altitude: {p(h)} [N/m2]")

h = 5000
v = 200
s = 35.5
l = 450000 # equal to aircraft weight
print(f"air density at {h} meters altitude: {d(h)} [kg/m3]")
print(f"lift coefficient at constant altitude: {cl(l, h, v, s)}")
l -= 50000
print(f"lift coefficient after fuel is burnt: {cl(l, h, v, s)}")

# Boeing 747-400
b = 68.5
s = 554
print(f"Boeing 747-400 aspect ratio: {ar(b, s)}")
# Oswald efficiency factor, typically ranging from 0.7 to 0.85
e = 0.8
cl = 0.86 # arbitrary
print(f"Boeing 747-400 induced drag coefficient: {id(cl, ar(b, s), e)}")
cl = 0.4 # from exercise
cd0 = 0.0225 # cd to cl diagram from exercise
print(f"Boeing 747-400 induced drag coefficient for cl={cl}: {id(cl, ar(b, s), e)}")
print(f"Boeing 747-400 drag coefficient for cd0={cd0}: {cd(cd0, cl, b, s, e)}")
l = 300000 # equal to aircraft weight
h = 10000
v = math.sqrt(2 * l / (cl * d(h) * s))
print(f"air density at {h} meters altitude: {d(h)} [kg/m3]")
print(f"velocity: {v}")
print(f"Boeing 747-400 velocity for lift={l}, cl={cl} at altitude {h} meters: {v}")
cd_actual = 0.02
print(f"Boeing 747-400 drag for cl={cl}, cd0={cd0}, v={v}: {dr(cd_actual, h, v, s)} [Kg]")
print(f"Boeing 747-400 drag for cl={cl}, cd0={cd0}, v={v}: {dr(cd_actual, h, v, s) * G} [N]")
print(f"Boeing 747-400 drag for lift={l}, cl={cl}, cd={cd_actual}: {(cd_actual * l / cl) * G} [N]")

# glider
b = 18
s = 11.8
print(f"Glider aspect ratio: {ar(b, s)}")
print(f"Glider induced drag coefficient: {id(0.86, ar(b, s), e)}")

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

