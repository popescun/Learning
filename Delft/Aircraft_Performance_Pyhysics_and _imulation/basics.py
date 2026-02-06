# constants for International Standard Atmosphere (ISA)
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
d = lambda h: d0 * pow(T(h), -(C_atm + 1)) # air density at altitude

h = 5000
print(f"air temperature at {h} meters altitude: {T(h)} [K]")
print(f"air pressure at {h} meters altitude: {p(h)} [N/m2]")



