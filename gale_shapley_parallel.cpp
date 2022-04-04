#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

using namespace std;

int main(int argc, char **argv)
{
    // Global variable declaration and initialization
    int rank, size;
    int master = 0; 
    int terminate = -2;
    int reject = -3;
    vector<int>::iterator _old;
    vector<int>::iterator _new;

    // MPI Initialization, getting rank and size
    MPI_Init(&argc, &argv);               
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int temp = 0;
    // if there are n men and women the total number of processes will be 2n+1; n men, n women, 1 master.
    // Work to be done by the master processor    
    if (rank == master)
    {
        double start = MPI_Wtime();
        // Number of unengaged women
        int w = (size - 1) / 2; 
        int count = size - 1;
        int flag = 0;
        // Read input from file 
        ifstream file;
        file.open(argv[1]);
        int total;
        file >> total;
        // Sending the perference lists of each person to the men and women processes
        for (int i = 1; i <= 2 * total; i++)
        {
            for (int j = 0; j < total; j++)
            {
                int person;
                file >> person;
                MPI_Send(&person, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
        // To avoid deadlock, calling MPI_Barrier so that the process waits until all men/women processes have received their preference lists.
        MPI_Barrier(MPI_COMM_WORLD);

        // The while loop runs as long as the flag is set to 0
        while (!flag)
        {
            // Looks for any incoming messages, if found sets flag to 1
            MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
            // Receives incoming message
            if (flag)
            {
                MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // If a woman process has sent out a terminate message, it means that the women is engaged, hence decrement the number of free women                
                if (temp == terminate)
                {
                    w--; 
                }
            }
            flag = 0;
            // If all women have been engaged, terminate all processes and write output results to file
            if (w == 0)
            {
                // Sending out termination signals to all processes
                for (int i = 1; i <= count; i++)
                {
                    MPI_Send(&terminate, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                }

                ofstream opt;
                opt.open("results.txt");
                // Getting engagement results from woman processes
                for (int i = (size + 1) / 2; i < size; i++)
                {
                    MPI_Recv(&temp, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    opt << temp << " is enagaged with " << i - total << endl;
                }
                double end = MPI_Wtime();
                opt<<"Runtime is "<<end-start;
                opt.close();

                MPI_Finalize(); 
                break;
            }
        }
    }
    // Work to be done by men processes
    else if (rank <= (size - 1) / 2)
    {                       
        vector<int> preferences;  
        // Start off by proposing to the first woman in the preference list
        int preferred_woman = 0; 
        int flag = 0;
        // Getting the preference list from master
        int c = (size - 1) / 2;
        for (int i = 0; i < c; i++)
        {
            MPI_Recv(&temp, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            preferences.push_back(temp + (size - 1) / 2);
        }
        // Wait until all the men processes have their preference list
        MPI_Barrier(MPI_COMM_WORLD); 
        // Proposing the first woman in the preference list
        MPI_Send(&rank, 1, MPI_INT, preferences[preferred_woman], 0, MPI_COMM_WORLD);
        //Increment to the next woman to propose again in case of rejection        
        preferred_woman++; 
        // Listening for messages from woman processes
        while (!flag)
        {
            // Listening for incoming messages
            MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
            // If there is incoming message, the flag will be non zero             
            if (flag)
            {
                // Receive incoming message
                MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // In case of termination message, break the loop
                if (temp == terminate)
                { 
                    break;
                }
                // Send proposal to next preferred woman
                else
                { 
                    MPI_Send(&rank, 1, MPI_INT, preferences[preferred_woman], 0, MPI_COMM_WORLD);
                    preferred_woman++;
                }
            }
            flag = 0;
        }

        MPI_Finalize(); 
    }
    // Work to be done by women processes
    else
    { 
        vector<int> preferences; 
        int flag = 0;
        // Free women are not engaged to any man
        int engaged = 0; 
        int c = (size - 1) / 2;
        for (int i = 0; i < c; i++)
        {
            MPI_Recv(&temp, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            preferences.push_back(temp);
        }
        // Wait until all the women processes have their preference list 
        MPI_Barrier(MPI_COMM_WORLD); 

        // The while loop runs as long as the flag is set to 0
        while (!flag)
        {
            // Listening for incoming messages
            MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
            // If there is incoming message, the flag will be non zero
            if (flag)
            {
                // Receive incoming message
                MPI_Recv(&temp, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                flag = 0;
                // In case of termination message, break the loop
                if (temp == terminate)
                { 
                    break;
                }
                else
                {
                    // If the woman is free, accept the propsal received 
                    if (!engaged)
                    {                       
                        engaged = temp; 
                        // Sending termination message to master indicating successful engagment
                        MPI_Send(&terminate, 1, MPI_INT, master, 0, MPI_COMM_WORLD);
                    }
                    else
                    { 
                        // If an engagement already exists and the woman receives a new proposal, compare and select the best proposal
                        _old = find(preferences.begin(), preferences.end(), engaged); 
                        _new = find(preferences.begin(), preferences.end(), temp);    
                        // Retaining old engagement
                        if (_old < _new)                                                 
                            MPI_Send(&reject, 1, MPI_INT, temp, 0, MPI_COMM_WORLD); 
                        // Accepting new proposal
                        else
                        {                                                                   
                            MPI_Send(&reject, 1, MPI_INT, engaged, 0, MPI_COMM_WORLD); 
                            engaged = temp;                                             
                        }
                    }
                }
            }
            flag = 0;
        }
        // Sending final engagement to master process
        MPI_Send(&engaged, 1, MPI_INT, master, 0, MPI_COMM_WORLD);
        MPI_Finalize(); 
    }

    return 0;
}
