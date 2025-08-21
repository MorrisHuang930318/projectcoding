#include <stdio.h>
#include <stdlib.h>
#include <math.h>
typedef struct {
    int degree;
    double *coefficients;
} Polynomial;
double def_diff(double x , double h , double (*fx)(double));
double def_fun_cos(double x);
double def_fun_sin(double x);
double def_fun_exp(double x);
double def_fun_ln(double x);
//double def_fun_poly(Polynomial poly, double x);
double exact_diff_poly(Polynomial poly, double x);

double def_diff(double x , double h , double (*fx)(double)){
    return ( fx(x+h) - fx(x) ) / h;
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
// double def_fun_poly(Polynomial poly, double x) {
//     double result = 0.0;
//     for (int i = 1; i <= poly.degree; i++) {
//         result += i * poly.coefficients[i] * pow(x, i);
//     }
//     return result;
//}
double exact_diff_poly(Polynomial poly, double x) {
    double result = 0.0;
    for (int i = 1; i <= poly.degree; i++) {
        result += i * poly.coefficients[i] * pow(x, i -1);
    }
    return result;
}
int main(){
    int i , j;
    char option;
    double h = 0.1;
    double x;
    double def_value , exact_value , error;

    Polynomial poly;
    double coeffs[50]; 
    poly.coefficients = coeffs;

    printf("1 Cos(x)\n2 Sin(x)\n3 Exponential\n4 ln(x)\n5 Polynomial\nWhich function do you want to use to differentiate?\n(if you want to use Cosine function, type 1)");
    scanf(" %c" , &option);

    switch(option){
        case '1':
            printf("Cos(x)\nAt what value of x would you like to calculate the derivative?");
            scanf("%lf" , &x);
            for (int j=0 ; j<=10 ; j++){
                exact_value = -sin(x);
                def_value = def_diff(x , h , def_fun_cos);
                error = fabs(def_value - exact_value) / fabs(exact_value);
                printf("Def_value   : %15.14lf Exact_value : %15.14lf Error : %15.14lf\n"  , def_value , exact_value , error);
                h /= 10;
            }
            break;
        case '2':
            printf("Sin(x)\nAt what value of x would you like to calculate the derivative?");
            scanf("%lf" , &x);
            for (int j=0 ; j<=10 ; j++){
                exact_value = cos(x);
                def_value = def_diff(x , h , def_fun_sin);
                error = fabs(def_value - exact_value) / fabs(exact_value);
                printf("Def_value   : %15.14lf Exact_value : %15.14lf Error : %15.14lf\n"  , def_value , exact_value , error);
                h /= 10;
            }
            break;
        case '3':
            printf("Exponential\nAt what value of x would you like to calculate the derivative?");
            scanf("%lf" , &x);
            for (int j=0 ; j<=10 ; j++){
                exact_value = exp(x);
                def_value = def_diff(x , h , def_fun_exp);
                error = fabs(def_value - exact_value) / fabs(exact_value);
                printf("Def_value   : %15.14lf Exact_value : %15.14lf Error : %15.14lf\n"  , def_value , exact_value , error);
                h /= 10;
            }
            break;
        case '4':
            printf("ln(x)\nAt what value of x would you like to calculate the derivative?");
            scanf("%lf" , &x);
            for (int j=0 ; j<=10 ; j++){
                exact_value = 1/x;
                def_value = def_diff(x , h , def_fun_ln);
                error = fabs(def_value - exact_value) / fabs(exact_value);
                printf("Def_value   : %15.14lf Exact_value : %15.14lf Error : %15.14lf\n"  , def_value , exact_value , error);
                h /= 10;
            }
            break;
        case '5':
            while ((getchar()) != '\n');
            printf("Polynomial\nAt what value of x would you like to calculate the derivative?");
            scanf("%lf" , &x);
            printf("What is the highest degree in this polynomial function?");
            scanf("%d" , &poly.degree);
            //printf("Enter each coefficient increase by degrees");
            for (int i=0 ; i <= poly.degree ; i++){
                printf("Enter each coefficient");
                scanf("%lf" , &coeffs[i]);
            };
            for (int j=0 ; j<=10 ; j++){
                exact_value = exact_diff_poly(poly, x);
                //def_value = def_diff(x , h , (double (*)(double))def_fun_poly);
                double fx_h = 0.0, fx = 0.0;
                for (int i = 0; i <= poly.degree; i++) {
                    fx_h += coeffs[i] * pow(x + h, i);
                    fx += coeffs[i] * pow(x, i);
                }
                def_value = (fx_h - fx) / h;
                error = fabs(def_value - exact_value) / fabs(exact_value);
                printf("Def_value   : %15.14lf Exact_value : %15.14lf Error : %15.14lf\n"  , def_value , exact_value , error);
                h /= 10;
            }
            break;
    }
    system("pause");
    return 0;
}