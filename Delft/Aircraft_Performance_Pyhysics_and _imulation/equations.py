import math

# constants for International Standard Atmosphere (ISA)
T0 = 288.15 # [K] air temperature at sea level
D0 = 1.225 # [kg/m3] air density at sea level
P0 = 101325 # [N/m2] air pressure at sea level

# air constants
LAMBDA = -0.0065 # [K/m] air constant
R = 287.05 # [m2/(s2*K)] specific gas constant of air
M = 29 # [kg/Kmol] mean molar mass of air

# gravity
G = 9.80665 # [m/s2] gravitational acceleration

# atmosphere constant
C_atm = G / (LAMBDA * R)

# air equations
T = lambda h: T0 - 6.5 * pow(10, -3) * h # where h is altitude in [m]
p = lambda h: P0 * pow(T(h) / T0, -C_atm) # air pressure at altitude
d = lambda h: D0 * pow(T(h) / T0, -(C_atm + 1)) # air density at altitude

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

# Propulsion
"""
    Trust of engine
        m  - mass of air flow through the engine  [Kg]
        vj - air speed behind engine [m/s]
        v  - aircraft speed [m/s]
"""
t = lambda m, vj, v: m * (vj - v)

"""
    The available power [W]
        m  - mass of air flow through the engine  [Kg]
        vj - air speed behind engine [m/s]
        v  - aircraft speed [m/s]
"""
pa = lambda m, vj, v: t(m, vj, v) * v

"""
    The thermal power [W]
        mf - mass flow [Kg]
        h  - amount of energy per unit of fuel
"""
q = lambda mf, h: mf  * h

"""
    Jet power: 
        m  - mass of air flow through the engine  [Kg]
        vj - air speed behind engine [m/s]
        v  - aircraft speed [m/s]
"""
pj = lambda m, vj, v: m * pow(vj, 2) / 2 - m * pow(v, 2) / 2

"""
    Thermal efficiency
        m  - mass of air flow through the engine  [Kg]
        vj - air speed behind engine [m/s]
        v  - aircraft speed [m/s]
        mf - mass flow [Kg]
        h  - amount of energy per unit of fuel
"""
et = lambda m, vj, v, mf, h: pj(m, vj, v) / q(mf, h)

"""
    Propulsive efficiency
        vj - air speed behind engine [m/s]
        v  - aircraft speed [m/s]
"""
ej = lambda vj, v: 2 / (1 + vj / v)

"""
    Total efficiency
        m  - mass of air flow through the engine  [Kg]
        vj - air speed behind engine [m/s]
        v  - aircraft speed [m/s]
        mf - mass flow [Kg]
        h  - amount of energy per unit of fuel
"""
e = lambda m, vj, v, mf, h: et(m, vj, v, mf, h) * ej(vj, v)
