#include <stdio.h>
#include <gsl/gsl_odeiv2.h>
#include <gsl/gsl_errno.h>

// Paramètres du modèle SEIRS 
const double rho = 1.0/365.0;
const double beta = 0.5;
const double sigma = 1.0/3.0;
const double gamma_seirs = 1.0/7.0;  

// Système d'équations différentielles 
int func(double t, const double y[], double f[], void *params) {
    f[0] = rho * y[3] - beta * y[2] * y[0];          // S
    f[1] = beta * y[2] * y[0] - sigma * y[1];       // E
    f[2] = sigma * y[1] - gamma_seirs * y[2];       // I
    f[3] = gamma_seirs * y[2] - rho * y[3];         // R
    return GSL_SUCCESS;
}

// Méthode d'Euler implémentée manuellement 
void solve_euler_manual(double y[], double dt, int steps, const char *filename) {
    FILE *f = fopen(filename, "w");
    double f_eval[4];
    for (int i = 0; i <= steps; i++) {
        fprintf(f, "%f,%f,%f,%f\n", y[0], y[1], y[2], y[3]);
        func(0, y, f_eval, NULL);
        for (int j = 0; j < 4; j++) {
            y[j] = y[j] + dt * f_eval[j];
        }
    }
    fclose(f);
}

// Méthode RK4 via la bibliothèque GSL
void solve_rk4_gsl(double y[], double dt, int steps, const char *filename) {
    gsl_odeiv2_system sys = {func, NULL, 4, NULL};
    
    gsl_odeiv2_driver *d = gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rk4, dt, 1e-6, 0.0);
    double t = 0.0;
    FILE *f = fopen(filename, "w");
    for (int i = 0; i <= steps; i++) {
        fprintf(f, "%f,%f,%f,%f\n", y[0], y[1], y[2], y[3]);
        gsl_odeiv2_driver_apply(d, &t, (i + 1) * dt, y);
    }
    gsl_odeiv2_driver_free(d);
    fclose(f);
}

int main() {
    double y_init_euler[4] = {0.999, 0.0, 0.001, 0.0}; 
    double y_init_rk4[4] = {0.999, 0.0, 0.001, 0.0};
    double dt = 0.1;
    int steps = 7300; // 730 jours 

    solve_euler_manual(y_init_euler, dt, steps, "c_euler_manual.csv");
    solve_rk4_gsl(y_init_rk4, dt, steps, "c_rk4_library.csv");

    return 0;
}
