/********************************************************************
 * Name:        Muhammad Faheem
 * Student ID:  22I-0485
 * Course:      Parallel and Distributed Computing
 * Assignment:  #3
 * Task:        Q1 - Parallel Box Blur Filter
 *
 * Description: This program applies a box blur filter to a grayscale image
 *              using a 3x3 kernel. The computation is parallelized using
 *              OpenMP to improve efficiency. The execution times of serial
 *              and parallel implementations are compared across different
 *              image sizes and thread configurations.
 ********************************************************************/

#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

# define NUM_THREADS 4

// Sequential box blur implementation
void boxBlurSequential(const vector<vector<int>>& input, vector<vector<int>>& output, int rows, int cols) 
{
    for (int i = 0; i < rows; i++) 
    {
        for (int j = 0; j < cols; j++) 
        {
            int sum = 0, count = 0;
        
            // Iterating over the 3x3 neighborhood
            for (int di = -1; di <= 1; di++) 
            {
                for (int dj = -1; dj <= 1; dj++) 
                {
                    int ni = i + di;
                    int nj = j + dj;
                
                    // Checking boundary conditions
                    if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) 
                    {
                        sum += input[ni][nj];
                    
                        count++;
                    }
                }
            }

            output[i][j] = sum / count;  // Replace with average value
        }
    }
}

// Parallel box blur implementation using OpenMP
void boxBlurParallel(const vector<vector<int>>& input, vector<vector<int>>& output, int rows, int cols) 
{
    // Collapsing two loops to allow parallelization across both dimensions
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows; i++) 
    {
        for (int j = 0; j < cols; j++) 
        {
            int sum = 0, count = 0;
        
            // Iterating over the 3x3 neighborhood
            for (int di = -1; di <= 1; di++) 
            {
                for (int dj = -1; dj <= 1; dj++) 
                {
                    int ni = i + di;
                    int nj = j + dj;
                
                    // Checking for valid indices (boundary conditions)
                    if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) 
                    {
                        sum += input[ni][nj];
                    
                        count++;
                    }
                }
            }

            output[i][j] = sum / count;
        }
    }
}

int main() 
{
    // Image sizes to test
    int sizes[] = {1000, 2000, 3000, 5000};

    // Looping over each image size
    for (int s = 0; s < 4; s++) 
    {
        int rows = sizes[s], cols = sizes[s];
    
        // Creating an image with constant pixel intensity (e.g., 128)
        vector<vector<int>> input(rows, vector<int>(cols, 128));
        vector<vector<int>> output(rows, vector<int>(cols, 0));

        // Measuring sequential execution time
        double start = omp_get_wtime();

        boxBlurSequential(input, output, rows, cols);
        
        double end = omp_get_wtime();
        
        cout << "> Sequential Execution:" << endl;
        cout << "-> Time for " << rows << "x" << cols << " image: " << (end - start) << " seconds." << endl << endl;

        // Measure parallel execution time (adjusting thread count as needed via environment variable or omp_set_num_threads)

        cout << "> Parallel Execution:" << endl;
        for (int i = 2; i <= 16; i+=2) 
        {
            double startP = omp_get_wtime();
    
            omp_set_num_threads(i);  
            
            boxBlurParallel(input, output, rows, cols);
            
            double endP = omp_get_wtime();
            
            cout << "-> Time for " << rows << "x" << cols << " image: " << (endP - startP) << " seconds. Using " << i << " threads." << endl;
        }

        cout << "\n--------------------------------------------------------------------------------" << endl << endl;
    }

    return 0;
}
