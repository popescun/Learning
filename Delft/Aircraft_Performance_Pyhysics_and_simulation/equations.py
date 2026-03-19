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
"""
    Air temperature [K]
        h - altitude in [m]
"""
T = lambda h: T0 - 6.5 * pow(10, -3) * h

"""
    Air pressure [N/m2]
        h - altitude in [m]
"""
p = lambda h: P0 * pow(T(h) / T0, -C_atm)


"""
    Air density [Kg/m3]
        h - altitude in [m]
"""
d = lambda h: D0 * pow(T(h) / T0, -(C_atm + 1))

"""
    Mass air flow
        h - altitude in [m]
        v - aircraft velocity in [m/s]
        fa - front engine area in [m2]
"""
m_flow = lambda h, v, fa: d(h) * v * fa

"""
    The dynamic pressure of air [N/m2]
        h - altitude in [m]
        v - aircraft velocity in [m/s]
"""
p_dynamic = lambda h, v: d(h) * pow(v, 2) / 2

# The lift
"""
    Lift equation [N]
        cl - lift coefficient
        h  - altitude [m]
        v  - aircraft speed [m/s]
        s  - wing surface [m2] 
"""
l = lambda cl, h, v, s: cl * p_dynamic(h, v) * s

# Lift coefficient
cl = lambda l, h, v, s: l / (p_dynamic(h, v) * s)
cl_by_density = lambda l, d, v, s: 2 * l / (d*pow(v, 2)*s)

# The drag
"""
    Drag equation
        cd - drag coefficient
        h  - altitude [m]
        v  - aircraft speed [m/s]
        s  - wing surface [m2] 
"""
dr = lambda cd, h, v, s: cd * p_dynamic(h, v) * s
dr_by_density = lambda cd, d, v, s: cd * d * pow(v, 2) * s / 2

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
id = lambda  cl, b, s, e: pow(cl, 2) / (math.pi * ar(b, s) * e)
cd = lambda cd0, cl, b, s, e: cd0 + id(cl, b, s, e)
cd_with_k = lambda cd0, cl, k: cd0 + k*pow(cl, 2)

# Propulsion
"""
    Thrust of engine
        mf  - mass of air flow through the engine  [Kg]
        vj - exhaust jet speed. [m/s]
        v  - aircraft speed [m/s]
"""
t = lambda mf, vj, v: mf * (vj - v)

"""
    The available power [W]
        m  - mass of air flow through the engine  [Kg]
        vj - exhaust jet speed. [m/s]
        v  - aircraft speed [m/s]
"""
pa = lambda mf, vj, v: t(mf, vj, v) * v

"""
    The required power [W]
        cd - drag coefficient
        h  - altitude [m]
        v  - aircraft speed [m/s]
        s  - wing surface [m2] 
"""
par = lambda cd, h, v, s: dr(cd, h, v, s) * v

"""
    The thermal power [W]
        mf - mass flow [Kg]
        h  - amount of energy per unit of fuel
"""
q = lambda mf, h: mf  * h

"""
    Jet power: 
        m  - mass of air flow through the engine  [Kg]
        vj - exhaust jet speed. [m/s]
        v  - aircraft speed [m/s]
"""
pj = lambda m, vj, v: m * pow(vj, 2) / 2 - m * pow(v, 2) / 2

"""
    Thermal efficiency
        m  - mass of air flow through the engine  [Kg]
        vj - exhaust jet speed. [m/s]
        v  - aircraft speed [m/s]
        mf - mass flow [Kg]
        h  - amount of energy per unit of fuel
"""
et = lambda m, vj, v, mf, h: pj(m, vj, v) / q(mf, h)

"""
    Propulsive efficiency
        vj - exhaust jet speed. [m/s]
        v  - aircraft speed [m/s]
"""
ej = lambda vj, v: 2 / (1 + vj / v)

"""
    Total efficiency
        m  - mass of air flow through the engine  [Kg]
        vj - exhaust jet speed. [m/s]
        v  - aircraft speed [m/s]
        mf - mass flow [Kg]
        h  - amount of energy per unit of fuel
"""
e = lambda m, vj, v, mf, h: et(m, vj, v, mf, h) * ej(vj, v)


"""
    Speed of sound
        h - altitude [m]
"""
MIU = 1.4
a = lambda h: math.sqrt(MIU * R * T(h))

"""
    Aircraft(air) speed
        m - mach speed
        h - altitude [m]
"""
v_air = lambda m, h: m * a(h)

"""
    Thrust of engine as static thrust
        t0     -  static thrust [N]
        v      - aircraft speed [m/s]
        h      - altitude [m]
        k1, k2 - coefficients
        
"""
t_from_static = lambda t0, v, h, k1, k2: t0 * (1 - k1 * v / a(h) + k2 * pow(v / a(h), 2))

"""
    Minimum airspeed [m/s]
     w  - weight [Kg]
     s  - surface [m2]
     d  - air density [Kg/m3]
     cl - lift coefficient  
"""
v_min = lambda w, s, d, cl: math.sqrt(2 * w / (s * d * cl))

"""
    Maximum airspeed [m/s]
        w  - weight [Kg]
        s  - surface [m2]
        d  - air density [Kg/m3]
        cl - lift coefficient  
"""
v_max = lambda w, s, d, cl: math.sqrt(w / s * 2 /d * 1 / cl)

# climb and descent
"""
  Climb rate [m/s]
    v - aircraft true speed [m/s] 
    gamma - climb angle [degree]
"""
rc = lambda v, gamma: v * math.sin(gamma * math.pi / 180)

"""
    Climb distance [m]
        v - aircraft true speed [m/s] 
        gamma - climb angle [degree]
"""
dc = lambda v, gamma: v * math.cos(gamma * math.pi / 180)

"""
    Climb angle [degree]
      t - thrust [N]
      cd - drag coefficient
      d - air density [Kg/m3]
      v - aircraft true speed [m/s] 
      s - wing surface [m2]
      w - weight [Kg]
      h - altitude [m]
"""
gamma = lambda t, cd, v, s, w, h: math.asin((t - dr(cd, h, v, s)) / w)
gamma_by_density = lambda t, cd, d, v, s, w: math.asin((t - dr_by_density(cd, d, v, s)) / w)

"""
    Climb rate by power [m/s]
        pa - power available power [W]
        pr - power required [W]
        w - aircraft weight [Kg]
"""
rc_by_power = lambda pa, pr, w: (pa - pr) / w

"""
    Climb rate by forces [m/s] 
        t  - thrust [N]
        dr - drag [N] 
        w  - aircraft weight [Kg]
        v  - aircraft true speed [m/s]
        
    Note: this is using small angle approximation
"""
rc_by_forces = lambda t, dr, w, v: (t - dr) / w * v

""" 
    True air speed [m/s]
        v_eas - estimated airspeed [m/s]
        d     - air density [Kg/m3]
"""
v_tas  = lambda v_eas, d: math.sqrt(D0 / d) * v_eas

"""
    Change of true air speed(rate) with altitude [m^-1]
        v_eas - estimated airspeed [m/s]
        h1    - first altitude [m]
        h2    - second altitude [m]
"""
delta_v_tas = lambda v_eas, h1, h2: v_eas * (math.sqrt(D0 / d(h2)) - math.sqrt(D0 / d(h1))) / (h2 - h1)

""" 
    Climb rate ratio (rc / rc_steady)
        v     - true airspeed [m/s] at altitude h1
        v_eas - estimated airspeed [m/s]
        h1    - first altitude [m]
        h2    - second altitude [m]
"""
rc_r = lambda v, v_eas, h1, h2: 1 / (1 + v / G * delta_v_tas(v_eas, h1, h2))

"""
    Glider minimum glide angle [degree]
        v_eas - estimated airspeed [m/s]
"""
gamma_glide_min = lambda cl, cd: math.asin(cd / cl) * 180 / math.pi

""" 
    Time to climb [s] 
        h1 - first altitude [m]
        h2 - second altitude [m]
        rc - climb rate [m/s] 
"""
ct = lambda h1, h2, rc: (h2 - h1) / rc

"""
    Energy height [J/N]
        h - altitude [m]
        v - aircraft true speed [m/s] 
"""
eh = lambda h, v: h + pow(v, 2) / (2 * G)

# take-off and landing

"""
    Lift-off speed [m/s]
        w      - aircraft weight [Kg]
        s      - wing surface [m2]
        d      - air density [Kg/m3]
        cl_max - max lift coefficient 
"""
v_lof = lambda w, s, d, cl_max: 1.05 * v_min(w, s, d, cl_max)

"""
    Average take-off air speed [m/s] 
        v_lof - lift-off speed [m/s]
"""
v_tof_g =  lambda v_lof: v_lof / math.sqrt(2)

"""
    Average take-off acceleration [m/s2] 
        t_g  - average take-off thrust [N]
        w    - aircraft weight [N]
        l_g  - average take-off lift [N]
        d_g  - average take-off drag [N]
        miu  - tire friction coefficient
"""
a_tof_g = lambda t_g, w, l_g, d_g, miu: G / w * (t_g - d_g - miu * (w - l_g))

"""
    Take-off ground run distance [m]
        v_lof   - lift-off speed [m/s]
        a_tof_g - average take-off acceleration [m/s2]
"""
x_ground = lambda v_lof, a_tof_g: pow(v_lof, 2) / (2 * a_tof_g)

"""
    Transition ground distance [m]
        v_lof   - lift-off speed [m/s]
        gamma   - climb angle [degree]
"""
x_trans = lambda v_lof, gamma: pow(v_lof, 2) / (0.15 * G) * math.sin(gamma * math.pi / 180)

"""
    Climb ground distance [m]
        h_scr   - screen height [m] 
        v_lof   - lift-off speed [m/s]
        gamma   - climb angle [degree]
"""
x_climb = lambda h_scr, v_lof, gamma: (h_scr - (1 - math.cos(gamma * math.pi / 180)) * pow(v_lof, 2) / (0.15 * G)) / (math.tan(gamma * math.pi / 180))

"""
    Total airborne distance [m]
        x_trans - transition ground distance [m]
        x_climb - climb ground distance [m]
"""
x_total_airborne = lambda x_trans, x_climb: x_trans + x_climb

"""
    Total take-off distance [m]
        x_ground         - ground run distance [m]
        x_total_airborne - total airborne distance [m]
"""
x_total_tof = lambda x_ground, x_total_airborne: x_ground + x_total_airborne

"""
    Take-off field length [m]
        bfl - balanced field length [m]
"""
field_length_tof = lambda bfl: 1.15 * bfl

"""
    Landing air speed [m/s]
        l_landing - landing lift force [N] 
        s         - wing surface [m2]
        d         - air density [Kg/m3]
        cl_max    - max lift coefficient 
"""
v_landing = lambda l_landing, s, d, cl_max: 1.3 * v_min(l_landing, s, d, cl_max)

"""
    Landing radius [m]
        w       - aircraft weight [Kg]
        s       - wing surface [m2]
        d       - air density [Kg/m3]
        cl_max  - max lift coefficient 
        delta_n - load factor 
"""
r_landing = lambda w, s, d, cl_max, delta_n: (pow(1.3, 2) * w / s * 2 / d * 1 / cl_max) / (delta_n * G)

"""
    Landing flare height [m]
        r - lading radius [m]
"""
h_flare = lambda r: (1  - math.cos(3 * math.pi / 180)) * r

"""
    Landing approach distance [m]
        h_scr   - screen height [m]
        h_flare - flare height [m]
"""
x_ap = lambda h_scr, h_flare: (h_scr - h_flare) / math.tan(3 * math.pi / 180)

"""
    Landing flare distance [m]
        r - lading radius [m]
"""
x_flare = lambda r:  r * math.sin(3 * math.pi / 180)

"""
    Landing airborne distance [m]
        x_ap - landing approach distance [m]
        x_flare - landing flare distance [m]
"""
x_landing_airborne_distance = lambda x_ap, x_flare: x_ap + x_flare

"""
    Landing airborne distance [m]
        r - landing radius [m]
        h_scr - screen height [m]
        gamma - landing angle [degree]
"""
x_landing_airborne_distance_by_radius = lambda r, h_scr, gamma: r * math.sin(gamma * math.pi / 180) + (h_scr - (1 - math.cos(gamma * math.pi / 180)) * r) / math.tan(gamma * math.pi / 180)


"""
    Landing approach speed [m/s]
        v_min - landing minimum speed [m/s]
"""
v_ap = lambda v_min: 1.3 * v_min

"""
    Landing transition ground-run distance [m]
        v_t - touchdown air speed [m/s]
        t_trans - landing transition time [s]
"""
x_trans_ground_run = lambda v_t, t_trans: t_trans * v_t

"""
    Touchdown airspeed [m/s]
        v_min - min airspeed before touchdown [m/s]
        k     - correction coefficient [m/s]
"""
v_t = lambda v_min, k: k * v_min

"""
    Ground run airspeed [m/s]
        v_t - touchdown airspeed [m/s]
"""
v_ground_run = lambda v_t: v_t / math.sqrt(2)

"""
    Friction drag of the wheels [N]
        w     - aircraft weight [N]
        l     - ground run lift force [N]
        miu_r - friction coefficient 
"""
dr_friction = lambda w, l, miu_r: miu_r * (w -l)

"""
    Ground run deceleration [m/s2]
    w     - aircraft weight [N]
    t     - thrust [N]
    dr    - drag [N]
    l     - lift [N]
    miu_r - friction coefficient    
"""
a_ground_run = lambda w, t, dr, l, miu_r: G / w * (t - dr - miu_r * (w - l))

"""
    Landing ground-run brake distance[m]
    w      - aircraft weight [N]
    t      - thrust [N]
    dr     - drag [N]
    l      - lift [N]
    miu_r  - friction coefficient    
    s      - wing surface [m2]
    cl_max - max lift coefficient  
    k      - correction coefficient [m/s]
"""
x_brake_ground_run = lambda w, t, dr, l, miu_r, s, cl_max, k: -(w * pow(k, 2)) / (s * d(0) * cl_max) * 1 / a_ground_run(w, t, dr, l, miu_r)

"""
    Total ground run distance [m]
    x_trans_ground_run - ground run transition distance [m]
    x_brake_ground_run - ground run brake distance [m]
"""
x_ground_run_total = lambda x_trans_ground_run, x_brake_ground_run: x_trans_ground_run + x_brake_ground_run
