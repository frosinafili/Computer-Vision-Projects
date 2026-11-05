#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
using namespace std;

struct Point {
    double x, y;
    int cluster;
};

struct Centroid {
    double x, y;
};

double distanceSquared(const Point& p, const Centroid& c) {
    double dx = p.x - c.x;
    double dy = p.y - c.y;
    return dx*dx + dy*dy;
}

void assignClusters(vector<Point>& points,
                    const vector<Centroid>& centroids) {

    for (auto& p : points) {

        double bestDist = numeric_limits<double>::infinity();
        int bestCluster = -1;

        for (int i = 0; i < centroids.size(); i++) {

            double d = distanceSquared(p, centroids[i]);

            if (d < bestDist) {
                bestDist = d;
                bestCluster = i;
            }
        }
        p.cluster = bestCluster;
    }
}

void updateCentroids(const vector<Point>& points,
                     vector<Centroid>& centroids) {

    int K = centroids.size();

    vector<double> sumX(K, 0.0);
    vector<double> sumY(K, 0.0);
    vector<int> count(K, 0);

    for (const auto& p : points) {
        sumX[p.cluster] += p.x;
        sumY[p.cluster] += p.y;
        count[p.cluster]++;
    }

    for (int i = 0; i < K; i++) {
        if (count[i] > 0) {
            centroids[i].x = sumX[i] / count[i];
            centroids[i].y = sumY[i] / count[i];
        }
    }
}

double computeShift(const vector<Centroid>& oldC,
                    const vector<Centroid>& newC) {

    double total = 0.0;

    for (int i = 0; i < oldC.size(); i++) {
        double dx = newC[i].x - oldC[i].x;
        double dy = newC[i].y - oldC[i].y;
        total += dx*dx + dy*dy;
    }

    return total;
}

void kmeans(vector<Point>& points,
            vector<Centroid>& centroids,
            int maxIterations,
            double epsilon) {

    int iter = 0;

    while (true) {

        vector<Centroid> oldCentroids = centroids;

        // Step 1: Assignment
        assignClusters(points, centroids);

        // Step 2: Update
        updateCentroids(points, centroids);

        // Step 3: Check convergence
        double shift = computeShift(oldCentroids, centroids);

        iter++;

        if (iter >= maxIterations)
            break;

        if (shift < epsilon)
            break;
    }
}