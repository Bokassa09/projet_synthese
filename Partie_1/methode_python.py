import numpy as np
import pandas as pd
from scipy.integrate import solve_ivp

# Paramètres 
rho, beta, sigma, gamma = 1/365, 0.5, 1/3, 1/7
u0 = [0.999, 0.0, 0.001, 0.0]
t_span = (0, 730)
t_eval = np.linspace(0, 730, 7301) # dt = 0.1 

def seirs_system(t, y):
    S, E, I, R = y
    return [rho*R - beta*I*S, beta*I*S - sigma*E, sigma*E - gamma*I, gamma*I - rho*R]

# Méthode 1 : RK4 (via bibliothèque SciPy) 
sol_rk4 = solve_ivp(seirs_system, t_span, u0, t_eval=t_eval, method='RK45')

# Méthode 2 : Euler (à la main, car absent de SciPy) 
def euler_manual(u0, t_eval):
    dt = t_eval[1] - t_eval[0]
    u = np.zeros((len(t_eval), 4))
    u[0] = u0
    for i in range(len(t_eval)-1):
        deriv = np.array(seirs_system(t_eval[i], u[i]))
        u[i+1] = u[i] + dt * deriv
    return u

sol_euler = euler_manual(u0, t_eval)

# Sauvegarde
pd.DataFrame(sol_rk4.y.T).to_csv("py_rk4.csv", index=False, header=False)
pd.DataFrame(sol_euler).to_csv("py_euler.csv", index=False, header=False)