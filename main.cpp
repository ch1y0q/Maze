#include "gamelogic.h"
#include "macro.h"

#include <cstdlib>






/*
double circleSDF(double x, double y, double cx, double cy, double r)
{
    double ux = x - cx, uy = y - cy;
    return sqrtf(ux * ux + uy * uy) - r;
}

double trace(double ox, double oy, double dx, double dy)
{
    double t = 0.0;
    for (int i = 0; i < MAX_STEP && t < MAX_DISTANCE; i++) {
        double sd = circleSDF(ox + dx * t, oy + dy * t, 0.5f, 0.5f, 0.1f);
        if (sd < EPSILON)
            return 2.0;
        t += sd;
    }
    return 0.0;
}

double sample(double x, double y)
{
    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        // double a = TWO_PI * rand() / RAND_MAX;
        // double a = TWO_PI * i / N;
        double a = TWO_PI * (i + (double)rand() / RAND_MAX) / N;
        sum += trace(x, y, cosf(a), sinf(a));
    }
    return sum / N;
}

*/

int main()
{

    main_loop(0, 1, 0, 0);
    return 0;
}
