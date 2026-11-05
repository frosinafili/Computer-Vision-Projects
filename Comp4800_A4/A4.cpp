#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <cstdlib>
#include <ctime>
#include <algorithm>  

using namespace cv;

using namespace std;
struct point{
    double x;
    double  y;
    int cluster = -1;
};
struct centroid{
    double x;
    double y;
};

struct Bounds{
    double minX, maxX, minY, maxY;
};

double distance(const point& p, const centroid& c){
    double dx = p.x - c.x;
    double dy = p.y - c.y;
    return dx*dx + dy*dy;
}
void assignCluster(vector<point>& points, const vector<centroid>& centroids){
    for (auto& p : points) {
        double best_distance = std::numeric_limits<double>::infinity();
        int best_cluster = 0;

        for (size_t i = 0; i < centroids.size(); i++) {
            double d = distance(p, centroids[i]);
            if (d < best_distance) {
                best_distance = d;
                best_cluster = i;
            }
        }
        p.cluster = best_cluster;
    }
}

void updateCentroids(vector<point>& points, vector<centroid>& centroids) {
    vector<double> sumX(centroids.size(), 0.0);
    vector<double> sumY(centroids.size(), 0.0);
    vector<int> count(centroids.size(), 0);

    for (const auto& p : points) {
        if (p.cluster >= 0 && p.cluster < centroids.size()) {
            sumX[p.cluster] += p.x;
            sumY[p.cluster] += p.y;
            count[p.cluster]++;
        }
    }

    for(size_t i = 0; i < centroids.size(); i++) {
        if(count[i] == 0)
    {
    int randomIndex = rand() % points.size();
    centroids[i].x = points[randomIndex].x;
    centroids[i].y = points[randomIndex].y;
    continue;
    }


        centroids[i].x = sumX[i] / count[i];
        centroids[i].y = sumY[i] / count[i];
     }
}


double computeCentroidShift(const vector<centroid>& oldCentroids, const vector<centroid>& newCentroids) {
    
    double totalShift = 0.0;
    
    for (size_t i = 0; i < oldCentroids.size(); i++) {
        double dx = newCentroids[i].x - oldCentroids[i].x;
        double dy = newCentroids[i].y - oldCentroids[i].y;
        totalShift += dx * dx + dy * dy;
     }
        
    return totalShift;
}


Bounds computeBounds(const vector<point>& points,
                     const vector<centroid>& centroids)
{
    Bounds b;

    b.minX = b.minY = numeric_limits<double>::infinity();
    b.maxX = b.maxY = -numeric_limits<double>::infinity();

    for(const auto& p : points){
        b.minX = min(b.minX, p.x);
        b.maxX = max(b.maxX, p.x);
        b.minY = min(b.minY, p.y);
        b.maxY = max(b.maxY, p.y);
    }
   
    for(const auto& c : centroids){
        b.minX = min(b.minX, c.x);
        b.maxX = max(b.maxX, c.x);
        b.minY = min(b.minY, c.y);
        b.maxY = max(b.maxY, c.y);
    }
 
    double dx = b.maxX - b.minX;
    double dy = b.maxY - b.minY;

    if(dx < 1e-6) dx = 1.0;
    if(dy < 1e-6) dy = 1.0;


    double padX = dx * 0.05;   
    double padY = dy * 0.05;

    b.minX -= padX;
    b.maxX += padX;
    b.minY -= padY;
    b.maxY += padY;

    return b;
}
Point toScreen(const point& p,
               const Bounds& b,
               int width,
               int height)
{
    double dx = b.maxX - b.minX;
    double dy = b.maxY - b.minY;

    if(dx < 1e-9) dx = 1.0;
    if(dy < 1e-9) dy = 1.0;

    double sx = (p.x - b.minX) / dx;
    double sy = (p.y - b.minY) / dy;

    sx = std::clamp(sx, 0.0, 1.0);
    sy = std::clamp(sy, 0.0, 1.0);

    int x = static_cast<int>(std::round(sx * width));
    int y = height - static_cast<int>(std::round(sy * height)); // flip Y

    return Point(x, y);
}

vector<Scalar> generateColors(int k)
{
    vector<Scalar> colors;
    RNG rng(1234567); 

    for(int i=0;i<k;i++)
    {
        colors.push_back(
            Scalar(
                rng.uniform(0,255),
                rng.uniform(0,255),
                rng.uniform(0,255)
            )
        );
    }
    return colors;
}

void drawClusters(
    const vector<point>& points,
    const vector<centroid>& centroids,
    const vector<Scalar>& colors,
    int iteration)
{
    int width = 900;
    int height = 700;

    Mat canvas(height, width, CV_8UC3, Scalar(255,255,255));

    Bounds bounds = computeBounds(points, centroids); 

    for(int y=0;y<height;y++)
    {
        for(int x=0;x<width;x++)
        {
            
            double wx = bounds.minX + (double)x/width * (bounds.maxX-bounds.minX);
            double wy = bounds.maxY - (double)y/height * (bounds.maxY-bounds.minY);

            double bestDist = DBL_MAX;
            int bestCluster = 0;

            for(int c=0;c<centroids.size();c++)
            {
                double dx = wx-centroids[c].x;
                double dy = wy-centroids[c].y;
                double dist = dx*dx + dy*dy;

                if(dist < bestDist)
                {
                    bestDist = dist;
                    bestCluster = c;
                }
            }

            Scalar shaded;
            addWeighted(colors[bestCluster], 0.35,
            Scalar(255,255,255), 0.65,
            0, shaded);

            canvas.at<Vec3b>(y,x) = Vec3b(
                 (uchar)shaded[0],
                 (uchar)shaded[1],
                     (uchar)shaded[2]
                        );
        }
    }

  
    for(const auto& p : points)
    {
        Point pt = toScreen(p, bounds, width, height);

        circle(canvas, pt, 4,
               colors[p.cluster],
               FILLED);
        circle(canvas, pt, 4,
               Scalar(0,0,0), 1);
    }

   
    for(int i=0;i<centroids.size();i++)
    {
        point fake;
        fake.x = centroids[i].x;
        fake.y = centroids[i].y;

        Point c = toScreen(fake, bounds, width, height);

        circle(canvas, c, 12,
               Scalar(0,0,0), FILLED);

        circle(canvas, c, 8,
               colors[i], FILLED);
    }

    putText(canvas,
            "Iteration: " + to_string(iteration),
            Point(20,40),
            FONT_HERSHEY_SIMPLEX,
            1,
            Scalar(0,0,0),
            2);

    imshow("K-Means Simulation", canvas);
    
}
bool animateCentroidMove(
    const vector<point>& points,
    const vector<centroid>& oldC,
    const vector<centroid>& newC,
    const vector<Scalar>& colors,
    int iteration,
    int delay)
{
    int frames = 12;

    for(int f=1; f<=frames; f++)
    {
        float t = (float)f / frames;
        vector<centroid> temp = oldC;

        for(size_t i=0;i<temp.size();i++)
        {
            temp[i].x = oldC[i].x*(1-t) + newC[i].x*t;
            temp[i].y = oldC[i].y*(1-t) + newC[i].y*t;
        }

        drawClusters(points, temp, colors, iteration);

        int k = waitKey(std::max(1, delay));
        if(k == 27) return true; // ESC pressed
    }
    return false;
}

void kmeans(vector<point>& points,
            vector<centroid>& centroids,
            int delay,
            bool useMaxIterations,
            int maxIterations,
            double epsilon)
{
    vector<Scalar> colors = generateColors(centroids.size());

int screenW = 1920;  
int screenH = 1080;

int winW = 900;
int winH = 800;

namedWindow("K-Means Simulation", WINDOW_NORMAL);
resizeWindow("K-Means Simulation", winW, winH);

moveWindow(
    "K-Means Simulation",
    (screenW - winW) / 2,
    (screenH - winH) / 2
);
    int iter = 0;

        while(true)
        {
          int key = waitKey(delay);
          
            if(key == 27) // ESC key
        return;
        vector<centroid> oldCentroids = centroids;

        assignCluster(points, centroids);
        drawClusters(points, centroids, colors, iter);
       
        vector<centroid> newCentroids = centroids;

        updateCentroids(points, newCentroids);
       
        if(animateCentroidMove(points, centroids, newCentroids, colors, iter, delay))
    return; 
        centroids = newCentroids;

        double shift =
            computeCentroidShift(oldCentroids, centroids);
        
            iter++;

        if(useMaxIterations)
            {
                    if(iter >= maxIterations)
                          break;
            }
        else
        {
            if(shift < epsilon)
            break;
            }

    }

    waitKey(0);
}



int main(int argc, char* argv[])
{
    
    if(argc < 3)
    {
        cout << "Usage:\n";
        cout << "./kmeans <input_file> <delay_ms> [max_iterations]\n";
        cout << "If max_iterations is omitted, algorithm stops using epsilon.\n";
        return 1;
    }

    string filename = argv[1];
    int delay = stoi(argv[2]);

    bool useMaxIterations = false;
    int maxIterations = 0;

    if(argc >= 4)
    {
        useMaxIterations = true;
        maxIterations = stoi(argv[3]);
    }

    srand(time(0));

    vector<point> points;
    vector<centroid> centroids;

    ifstream file(filename);
    if(!file)
    {
        cout << "Error: Could not open input file: " << filename << endl;
        return 1;
    }

    int N;
    file >> N;

    cout << "Loading " << N << " points...\n";

    for(int i = 0; i < N; i++)
    {
        point p;
        file >> p.x >> p.y;
        p.cluster = -1;
        points.push_back(p);
    }

    int K;
    file >> K;

    cout << "Loading " << K << " initial centroids...\n";

    for(int i = 0; i < K; i++)
    {
        centroid c;
        file >> c.x >> c.y;
        centroids.push_back(c);
    }

    file.close();

    cout << "\n========== RUN MODE ==========\n";
    cout << "Input file       : " << filename << endl;
    cout << "Delay (ms)       : " << delay << endl;

    if(useMaxIterations)
        cout << "Stopping after   : " << maxIterations << " iterations\n";
    else
        cout << "Stopping via     : epsilon convergence\n";

    cout << "==============================\n\n";

    double epsilon = 0.001;

    
    kmeans(points,
           centroids,
           delay,
           useMaxIterations,
           maxIterations,
           epsilon);
    return 0;
}
