/********************************************************************
 * Name:        Muhammad Faheem
 * Student ID:  22I-0485
 * Course:      Parallel and Distributed Computing
 * Assignment:  #3
 * Task:        Q2 - Parallel Treasure Hunt Simulation
 *
 * Description: This program simulates a parallel treasure hunt where
 *              adventurers (threads) explore a grid containing treasures
 *              and traps. Each thread accumulates scores dynamically while
 *              navigating the grid. The simulation incorporates barriers,
 *              synchronization, and dynamic thread management to enhance
 *              realism and efficiency.
 ********************************************************************/

#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>
#include <atomic>
#include <set>

using namespace std;

// Enumeration for cell types
enum CellType { EMPTY, TREASURE, TRAP, RESURRECTION, DEADLY_TRAP };

// Structure for a grid cell
struct Cell 
{
    CellType type;

    int value; // For treasure: positive score; for trap: negative penalty; others: 0.
};

// Structure for positions in the grid
struct Position 
{
    int x, y;
};

// Structure representing an adventurer
struct Adventurer 
{
    int id;
    int score;
    int moves;

    bool active;

    Position pos;

    // Keep track of visited cells to avoid revisiting: stored as (x,y) pairs.
    set<pair<int,int>> visited;
};

// Global variables to track highest score and remaining treasures
atomic<int> globalHighestScore(0);
atomic<int> winnerId(-1);
atomic<int> remainingTreasures(0);

const int DYNAMIC_THRESHOLD = 50;  // Threshold score for passing dynamic barrier

// Global grid and its dimension
vector<vector<Cell>> grid;

int gridSize;

// Utility: Check if (x, y) is within grid bounds.
bool inBounds(int x, int y, int N) 
{
    return (x >= 0 && x < N && y >= 0 && y < N);
}

// Initializing grid with random placements
// - TREASURE: 15% probability, value between 10 and 100.
// - TRAP: 15% probability, penalty between -5 and -50.
// - RESURRECTION: 5% probability.
// - DEADLY_TRAP: 5% probability.
// - EMPTY: remaining cells.
void initializeGrid(int N) 
{
    gridSize = N;
    grid.resize(N, vector<Cell>(N, {EMPTY, 0}));

    srand(time(NULL));

    for (int i = 0; i < N; i++) 
    {
        for (int j = 0; j < N; j++) 
        {
            int r = rand() % 100;
        
            if (r < 15) 
            { // 15% chance for treasure
                grid[i][j].type = TREASURE;
                grid[i][j].value = 10 + rand() % 91; // Value between 10 and 100
            
                remainingTreasures++;
            } 
            else if (r < 30) 
            { // 15% chance for trap
                grid[i][j].type = TRAP;
                grid[i][j].value = -(5 + rand() % 46); // Penalty between -5 and -50
            } 
            else if (r < 35) 
            { // 5% chance for resurrection stone
                grid[i][j].type = RESURRECTION;
                grid[i][j].value = 0;
            } 
            else if (r < 40) 
            { // 5% chance for deadly trap
                grid[i][j].type = DEADLY_TRAP;
                grid[i][j].value = 0;
            } 
            else 
            {
                grid[i][j].type = EMPTY;
                grid[i][j].value = 0;
            }
        }
    }
}

// The simulation function for an adventurer
void adventurerSimulation(int init_id, int N) 
{
    Adventurer adv;

    adv.id = init_id;
    adv.score = 0;
    adv.moves = 0;
    adv.active = true;

    // Random starting position
    adv.pos.x = rand() % N;
    adv.pos.y = rand() % N;
    adv.visited.insert({adv.pos.x, adv.pos.y});
    
    while (adv.active) 
    {
        // End simulation if all treasures are collected
        if (remainingTreasures <= 0)
            break;
        
        // Randomly choose a direction: 0-up, 1-down, 2-left, 3-right
        int direction = rand() % 4;
        int newX = adv.pos.x, newY = adv.pos.y;
    
        if (direction == 0) newX--;       // Up
        else if (direction == 1) newX++;  // Down
        else if (direction == 2) newY--;  // Left
        else if (direction == 3) newY++;  // Right
        
        // Validating move: within bounds and not previously visited
        if (!inBounds(newX, newY, N)) continue;
        if (adv.visited.find({newX, newY}) != adv.visited.end()) continue;
        
        // Updating position and mark as visited
        adv.pos.x = newX;
        adv.pos.y = newY;
        adv.visited.insert({newX, newY});
        adv.moves++;
        
        // Processing cell in a critical section to safely update shared grid state.
        #pragma omp critical
        {
            Cell &cell = grid[newX][newY];
            
            switch(cell.type) 
            {
                case TREASURE:
                    adv.score += cell.value;
            
                    cell.type = EMPTY;  // Treasure collected
            
                    remainingTreasures--;
            
                    cout << "-> Adventurer " << adv.id << " collected treasure at (" 
                         << newX << "," << newY << ") for +" << cell.value 
                         << " points. New score: " << adv.score << endl;
            
                         break;
                case TRAP:
                    adv.score += cell.value; // Penalty (value is negative)
            
                    cout << "-> Adventurer " << adv.id << " hit a trap at (" 
                         << newX << "," << newY << ") for " << cell.value 
                         << " points. New score: " << adv.score << endl;
            
                         break;
                case RESURRECTION:
                    cout << "-> Adventurer " << adv.id << " found a Resurrection Stone at (" 
                         << newX << "," << newY << "). Spawning new adventurer." << endl;
                    
                         // Spawning a new adventurer task with a new ID.
                    #pragma omp task firstprivate(N)
                    
                    adventurerSimulation(rand() % 1000 + 1000, N);
                    
                    cell.type = EMPTY;
                    
                    break;
                case DEADLY_TRAP:
                    cout << "-> Adventurer " << adv.id << " encountered a Deadly Trap at (" 
                         << newX << "," << newY << "). Terminating." << endl;
                    
                         adv.active = false;
                    
                         break;
                default:
                    // Empty cell: no effect.
                    break;
            }
        }  // End critical section

        // Updating global highest score if current adventurer’s score exceeds it.
        #pragma omp critical
        {
            if (adv.score > globalHighestScore) 
            {
                globalHighestScore = adv.score;
            
                winnerId = adv.id;
            }
        }
        
        // Every 5 moves, synchronize adventurers at a checkpoint.
        if (adv.moves % 5 == 0) 
        {
            #pragma omp taskwait
            if (adv.score < DYNAMIC_THRESHOLD) 
            {
                cout << "-> Adventurer " << adv.id << " is waiting at checkpoint with low score (" 
                     << adv.score << ")." << endl;
                
                // Waiting for other adventurers to reach the checkpoint.
                #pragma omp taskwait

            }
        }

        // sleep(1);  // Simulating movement delay
    }
}

int main() 
{
    int N, T;
    
    cout << "Please, input grid size (N): ";
    cin >> N;
    
    cout << "Enter number of initial adventurers (T): ";
    cin >> T;

    cout << endl;
    
    initializeGrid(N);

    cout << "> Simulation:" << endl;
    
    // Starting the parallel region and spawn initial adventurer tasks.
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < T; i++) 
            {
                #pragma omp task firstprivate(N)
            
                adventurerSimulation(i, N);
            }
        }
    }
    
    cout << "\n> Treasure hunt completed." << endl;
    cout << "-> Winner: Adventurer " << winnerId 
         << " with score " << globalHighestScore << endl << endl;
    
    return 0;
}
