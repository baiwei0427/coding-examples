void addvec(int *x, int *y, int *z, int n)
{
    for (int i = 0; i < n; i++) {
        z[i] = x[i] + y[i];
    }
}
