# constants for International Standard Atmosphere (ISA)
import math
from ctypes.wintypes import LANGID

T0 = 288.15 # [K] air temperature at sea level
d0 = 1.225 # [kg/m3] air density at sea level
p0 = 101325 # [N/m2] air pressure at sea level

# air constants
Lambda = -0.0065 # [K/m] air constant
R = 287.05 # [m2/(s2*K)] specific gas constant of air
M = 29 # [kg/kmol] mean molar mass of air

# gravity
g = 9.80665 # [m/s2] gravitational acceleration

# atmosphere constant
C_atm = g / (Lambda * R)

print(f"C_atm: {C_atm}")

# air equations
T = lambda h: T0 - 6.5 * pow(10, -3) * h # where h is altitude in [m]
p = lambda h: p0 * pow(T(h) / T0, -C_atm) # air pressure at altitude
d = lambda h: d0 * pow(T(h) / T0, -(C_atm + 1)) # air density at altitude

h = 5000
print(f"air temperature at {h} meters altitude: {T(h)} [K]")
print(f"air pressure at {h} meters altitude: {p(h)} [N/m2]")


# The lift
"""
    Lift equation
        cl - lift coefficient
        h  - altitude [m]
        v  - aircraft speed [m/s]
        s  - wing surface [m2] 
"""
l = lambda cl, h, v, s: cl * d(h) * pow(v, 2) * s / 2

# Lift coefficient
cl = lambda l, h, v, s: 2 * l / (d(h) * pow(v, 2) * s)

h = 5000
v = 200
s = 35.5
l = 450000 # equal to aircraft weight
print(f"air density at {h} meters altitude: {d(h)} [kg/m3]")
print(f"lift coefficient at constant altitude: {cl(l, h, v, s)}")
l -= 50000
print(f"lift coefficient after fuel is burnt: {cl(l, h, v, s)}")

# The drag
"""
    Drag equation
        cd - drag coefficient
        h  - altitude [m]
        v  - aircraft speed [m/s]
        s  - wing surface [m2] 
"""
dr = lambda cd, h, v, s: cd * d(h) * pow(v, 2) * s / 2

"""
    Wing aspect ratio
        b - wing span
        s - wing surface [m2] 
"""
ar = lambda b, s: pow(b, 2) / s

"""
    Drag coefficient
        cd0 - zero-lift drag coefficient
        cl  - lift coefficient
        b   - wing span [m]
        s   - wing surface [m2] 
        e   - Oswald efficiency factor
        id  - induced drag coefficient  
"""
id = lambda  cl, a, e: pow(cl, 2) / (math.pi * a * e)
cd = lambda cd0, cl, b, s, e: cd0 + id(cl, ar(b, s), e)

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
print(f"Boeing 747-400 drag for cl={cl}, cd0={cd0}, v={v}: {dr(cd_actual, h, v, s) * g} [N]")
print(f"Boeing 747-400 drag for lift={l}, cl={cl}, cd={cd_actual}: {(cd_actual * l / cl) * g} [N]")

# glider
b = 18
s = 11.8
print(f"Glider aspect ratio: {ar(b, s)}")
print(f"Glider induced drag coefficient: {id(0.86, ar(b, s), e)}")


