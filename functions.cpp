typedef double (*unary_function)(double);

double Identity(double x)
{
    return x;
}

double Linear(double x)
{
    return 5*x;
}

double Quadratic(double x)
{
    return x*x;
}


