#include <stdio.h>
#include <stdlib.h>
#include <math.h>
double integr(double a , double b , int n , double (*fx)(double));
double def_fun_cos(double x);
double def_fun_sin(double x);
double def_fun_exp(double x);
double def_fun_ln(double x);

double integr(double a , double b , int n , double (*fx)(double)){
    double h = (b - a) / n;
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += fx(a + i * h);
    }
    return sum * h;
}
double def_fun_cos(double x){
    return cos(x);
}
double def_fun_sin(double x){
    return sin(x);
}
double def_fun_exp(double x){
    return exp(x);
}
double def_fun_ln(double x){
    return log(x);
}
double def_fun_polynomial(double x, const double *coeffs, int degree) {
    double result = 0.0;
    for (int i = 0; i <= degree; i++) {
        result += coeffs[i] * pow(x, i); 
    return result;
    }
}
double exact_integr_poly(double a, double b, const double *coeffs, int degree) {
    double integral_b = 0.0;
    double integral_a = 0.0;

    for (int i = 0; i <= degree; i++) {
        double term_integral = coeffs[i] / (i + 1); 
        integral_b += term_integral * pow(b, i + 1); 
        integral_a += term_integral * pow(a, i + 1); 
    }
    return integral_b - integral_a;
}
double integr_def_poly(double a, double b, int n, double (*fx)(double, const double *, int), const double *coeffs, int degree) {
    double h = (b - a) / n;
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += fx(a + i * h, coeffs, degree); 
    }
    return sum * h;
}
typedef struct {
    int degree;
    double *coefficients;
} Polynomial;

int main(){
    int i , j , n;
    double a , b , def_value , exact_value , error;
    char option;

    Polynomial poly;
    double coeffs[50]; 
    poly.coefficients = coeffs;

    printf("1 Cos(x)\n2 Sin(x)\n3 Exponential\n4 ln(x)\n5 Polynomial\nWhich function do you want to use to differentiate?\n(if you want to use Cosine function, type 1)");
    scanf(" %c" , &option);
    printf("Enter your upper limit:");
    scanf("%lf" , &b);
    printf("Enter your lower limit:");
    scanf("%lf" , &a);
    printf("Enter  the number of intervals n:");
    scanf("%d" , &n);

    switch(option){
        case '1':
            for (int j=0 ; j<=10 ; j++){
                exact_value = sin(b) - sin(a);
                def_value = integr(a , b , n , def_fun_cos);
                error = fabs(def_value - exact_value) / fabs(exact_value);
                printf("Def_value   : %15.14lf Exact_value : %15.14lf Error : %15.14lf\n"  , def_value , exact_value , error);
                n *= 10;
            }    
            break;
        case '2':
            for (int j=0 ; j<=10 ; j++){
                exact_value = -cos(b) + cos(a);
                def_value = integr(a , b , n , def_fun_sin);
                error = fabs(def_value - exact_value) / fabs(exact_value);
                printf("Def_value   : %15.14lf Exact_value : %15.14lf Error : %15.14lf\n"  , def_value , exact_value , error);
                n *= 10;
            }
            break;
        case '3':
            for (int j=0 ; j<=10 ; j++){
                exact_value = exp(b) - exp(a);
                def_value = integr(a , b , n , def_fun_exp);
                error = fabs(def_value - exact_value) / fabs(exact_value);
                printf("Def_value   : %15.14lf Exact_value : %15.14lf Error : %15.14lf\n"  , def_value , exact_value , error);
                n *= 10;
            }
            break;
        case '4':
            for (int j=0 ; j<=10 ; j++){
                exact_value = (b*log(b)-b) - (a*log(a)-a);
                def_value = integr(a , b , n , def_fun_ln);
                error = fabs(def_value - exact_value) / fabs(exact_value);
                printf("Def_value   : %15.14lf Exact_value : %15.14lf Error : %15.14lf\n"  , def_value , exact_value , error);
                n *= 10;
            }
            break;
        case '5':
            while ((getchar()) != '\n');
            printf("What is the highest degree in this polynomial function?");
            scanf("%d" , &poly.degree);
            //printf("Enter each coefficient increase by degrees");
            for (int i=0 ; i <= poly.degree ; i++){
                printf("Enter each coefficient");
                scanf("%lf" , &coeffs[i]);
            };
            for (int j=0 ; j<=10 ; j++){
                exact_value = exact_integr_poly(a, b, coeffs, poly.degree);
                def_value = integr_def_poly(a, b, n, def_fun_polynomial, coeffs, poly.degree);
                 
                error = fabs(def_value - exact_value) / fabs(exact_value);
                printf("Def_value   : %15.14lf Exact_value : %15.14lf Error : %15.14lf\n"  , def_value , exact_value , error);
                n *= 10;
            }
            break;
    }
    system("pause");
    return 0;
}