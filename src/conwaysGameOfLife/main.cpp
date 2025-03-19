/********************************************************************
 * Name:        Muhammad Faheem
 * Student ID:  22I-0485
 * Course:      Parallel and Distributed Computing
 * Assignment:  #3
 * Task:        Q3 - Conway's Game of Life with OpenMP
 *
 * Description: This program implements Conway's Game of Life in both serial
 *              and parallel versions using OpenMP. The grid size is fixed at
 *              100x100, with an initial configuration of live and dead cells.
 *              The simulation runs for 100 generations, utilizing toroidal
 *              boundary conditions. The program includes static and guided
 *              scheduling for performance comparison.
 ********************************************************************/

#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

using namespace std;

const int SIZE = 100;
const int GENERATIONS = 100;

// Initializing grid with all dead cells and center 10x10 as live cells.
void initializeGrid(vector<vector<char>> &grid) 
{
    grid.assign(SIZE, vector<char>(SIZE, '.'));

    int start = SIZE / 2 - 5;
    int end = start + 10;

    for (int i = start; i < end; i++) 
    {
        for (int j = start; j < end; j++) 
            grid[i][j] = '*';
    }
}

// Counting live neighbors with toroidal (wrap-around) boundary conditions.
int countLiveNeighbors(const vector<vector<char>> &grid, int i, int j) 
{
    int liveCount = 0;

    for (int di = -1; di <= 1; di++) 
    {
        for (int dj = -1; dj <= 1; dj++) 
        {
            if (di == 0 && dj == 0)
                continue;
        
                int ni = (i + di + SIZE) % SIZE;
            int nj = (j + dj + SIZE) % SIZE;
        
            if (grid[ni][nj] == '*')
                liveCount++;
        }
    }

    return liveCount;
}

void printGrid(const vector<vector<char>> &grid) 
{
    for (int i = 0; i < SIZE; i++) 
    {
        for (int j = 0; j < SIZE; j++)
            cout << grid[i][j];
        cout << "\n";
    }
}

// Serial implementation of Conway's Game of Life.
void gameOfLifeSerial() 
{
    vector<vector<char>> current(SIZE, vector<char>(SIZE, '.'));
    vector<vector<char>> next(SIZE, vector<char>(SIZE, '.'));

    initializeGrid(current);

    for (int gen = 0; gen < GENERATIONS; gen++) 
    {
        for (int i = 0; i < SIZE; i++) 
        {
            for (int j = 0; j < SIZE; j++) 
            {
                int liveNeighbors = countLiveNeighbors(current, i, j);
            
                if (current[i][j] == '*') 
                { // Live cell
                    if (liveNeighbors < 2 || liveNeighbors > 3)
                        next[i][j] = '.'; // Underpopulation or overpopulation
                    else
                        next[i][j] = '*'; // Survival
                } 
                else 
                { // Dead cell
                    if (liveNeighbors == 3)
                        next[i][j] = '*'; // Reproduction
                    else
                        next[i][j] = '.';
                }
            }
        }

        current.swap(next);
    }

    // printGrid(current);
}

// Parallel implementation with static scheduling (chunk size 1).
void gameOfLifeParallelStatic() 
{
    vector<vector<char>> current(SIZE, vector<char>(SIZE, '.'));
    vector<vector<char>> next(SIZE, vector<char>(SIZE, '.'));

    initializeGrid(current);

    for (int gen = 0; gen < GENERATIONS; gen++) 
    {
        #pragma omp parallel for schedule(static,1)
    
        for (int i = 0; i < SIZE; i++) 
        {
            for (int j = 0; j < SIZE; j++) 
            {
                int liveNeighbors = countLiveNeighbors(current, i, j);
            
                if (current[i][j] == '*') 
                { // Live cell
                    if (liveNeighbors < 2 || liveNeighbors > 3)
                        next[i][j] = '.';
                    else
                        next[i][j] = '*';
                } 
                else 
                { // Dead cell
                    if (liveNeighbors == 3)
                        next[i][j] = '*';
                    else
                        next[i][j] = '.';
                }
            }
        }
        
        current.swap(next);
    }

    // printGrid(current);
}

// Parallel implementation with guided scheduling (chunk size 1).
void gameOfLifeParallelGuided() 
{
    vector<vector<char>> current(SIZE, vector<char>(SIZE, '.'));
    vector<vector<char>> next(SIZE, vector<char>(SIZE, '.'));

    initializeGrid(current);

    for (int gen = 0; gen < GENERATIONS; gen++) 
    {
        #pragma omp parallel for schedule(guided,1)
        for (int i = 0; i < SIZE; i++) 
        {
            for (int j = 0; j < SIZE; j++) 
            {
                int liveNeighbors = countLiveNeighbors(current, i, j);
                
                if (current[i][j] == '*') { // Live cell
                    if (liveNeighbors < 2 || liveNeighbors > 3)
                        next[i][j] = '.';
                    else
                        next[i][j] = '*';
                } 
                else 
                { // Dead cell
                    if (liveNeighbors == 3)
                        next[i][j] = '*';
                    else
                        next[i][j] = '.';
                }
            }
        }

        current.swap(next);
    }

    // printGrid(current);
}

int main() 
{
    // int version;

    // cout << "Please, select version to run:\n"
    //      << "1. Serial\n"
    //      << "2. Parallel (Static Scheduling, chunk=1)\n"
    //      << "3. Parallel (Guided Scheduling, chunk=1)\n"
    //      << "Choice: ";
    // cin >> version;

    // cout << endl;

    // Timer variables
    double totalTime = 0;
    const int repetitions = 1;

    // for (int rep = 0; rep < repetitions; rep++) 
    // {
    //     auto start = chrono::high_resolution_clock::now();
        
    //     switch (version) 
    //     {
    //         case 1:
    //             gameOfLifeSerial();
        
    //             break;
    //         case 2:
    //             gameOfLifeParallelStatic();
        
    //             break;
    //         case 3:
    //             gameOfLifeParallelGuided();
        
    //             break;
    //         default:
    //             cout << "Invalid choice.\n";
        
    //             return 1;
    //     }
        
    //     auto end = chrono::high_resolution_clock::now();
        
    //     chrono::duration<double> diff = end - start;
        
    //     totalTime += diff.count();
    // }
    
    // cout << "\nAverage execution time over " << repetitions << " runs: "
    //      << (totalTime / repetitions) << " seconds." << endl;
    
    for (int version = 1; version <= 3; version++) 
    {
        cout << "> Version " << version << ":\n";
        
        for (int rep = 0; rep < repetitions; rep++) 
        {
            auto start = chrono::high_resolution_clock::now();
            
            switch (version) 
            {
                case 1:
                    gameOfLifeSerial();
            
                    break;
                case 2:
                    gameOfLifeParallelStatic();
            
                    break;
                case 3:
                    gameOfLifeParallelGuided();
            
                    break;
                default:
                    cout << "Invalid choice.\n";
            
                    return 1;
            }
            
            auto end = chrono::high_resolution_clock::now();
            
            chrono::duration<double> diff = end - start;
            
            totalTime += diff.count();
        }
        
        cout << "-> Average execution time over " << repetitions << " runs: "
             << (totalTime / repetitions) << " seconds.\n" << endl;
    }

    return 0;
}
