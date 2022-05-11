#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

using namespace std;

int find_index(vector<int> vec, int to_find)
{
    auto itr = find(vec.begin(), vec.end(), to_find);
    return distance(vec.begin(), itr);
}

int main(int argc, char **argv)
{
    // Global variable declaration and initialization
    int rank, size;
    int master = 0;
    int terminate = -2;
    int reject = -3;
    int cont = -4;
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
        int men_proc = size - 1;
        int elem_per_proc = total / men_proc;
        // Sending the perference lists of each person to the men processes
        for (int i = 1; i <= men_proc; i++)
        {
            MPI_Send(&elem_per_proc, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&total, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            for (int j = 0; j < total * elem_per_proc; j++)
            {
                int person;
                file >> person;
                MPI_Send(&person, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
        // To avoid deadlock, calling MPI_Barrier so that the process waits until all men have received their preference lists.
        MPI_Barrier(MPI_COMM_WORLD);

        unordered_map<int, vector<int>> women;
        for (int i = 1; i <= total; i++)
        {
            for (int j = 1; j <= total; j++)
            {
                int val;
                file >> val;
                women[i].push_back(val);
            }
        }

        // vector<int> wmn;
        // for (auto wom : women)
        // {
        //     wmn.push_back(wom.first);
        // }
        // for (auto wm : wmn)
        // {
        //     for (auto wi : women[wm])
        //     {
        //         cout << wi << " ";
        //     }
        //     cout << endl;
        // }

        vector<int> engagements(total, 0);

        while (!flag)
        {
            MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
            if (flag)
            {
                int recv_arr[2];
                int proposed_woman, proposed_man;
                MPI_Status stat;
                MPI_Recv(&recv_arr, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);
                proposed_man = recv_arr[0];
                proposed_woman = recv_arr[1];
                cout << proposed_man <<" "<< proposed_woman << " " <<stat.MPI_SOURCE << endl;
                if (engagements[proposed_woman] == 0)
                {
                    engagements[proposed_woman] = proposed_man;
                }
                else
                {
                    int curr_engagement = engagements[proposed_woman];
                    int chosen = find_index(women[proposed_woman], proposed_man);
                    int current = find_index(women[proposed_woman], curr_engagement);
                    if (chosen > current)
                    {
                        engagements[proposed_woman] = proposed_man;
                        int send_reject[2] = {curr_engagement, reject};
                        MPI_Send(&send_reject, 2, MPI_INT, stat.MPI_SOURCE, 0, MPI_COMM_WORLD);
                    }
                    else
                    {
                        int send_cont[2] = {proposed_man, cont};
                        MPI_Send(&send_cont, 2, MPI_INT, stat.MPI_SOURCE, 0, MPI_COMM_WORLD);
                    }
                }
            }
            flag = 0;
        }
        ofstream out;
        out.open("results_modified.txt");
        for (int i = 1; i <= engagements.size(); i++)
        {
            out << i << " - " << engagements[i - 1] << endl;
        }
        MPI_Finalize();
    }
    // Work to be done by men processes
    else if (rank != 0)
    {
        unordered_map<int, vector<int>> men;
        int num_elems;
        int tot;
        int flag = 0;
        MPI_Recv(&num_elems, 1, MPI_INT, master, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&tot, 1, MPI_INT, master, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int start = num_elems * (rank - 1) + 1;
        for (int i = 0; i < num_elems; i++)
        {
            for (int j = 0; j < tot; j++)
            {
                int val;
                MPI_Recv(&val, 1, MPI_INT, master, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                men[start].push_back(val);
            }
            start++;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        vector<int> free_men;
        for (auto m : men)
        {
            free_men.push_back(m.first);
        }

        // for (auto m : free_men)
        // {
        //     for (auto mn : men[m])
        //     {
        //         cout << mn << " ";
        //     }
        //     cout << endl;
        // }

        while (free_men.size() > 0)
        {
            for (auto man : free_men)
            {
                for (auto woman : men[man])
                {
                    int send_arr[] = {man, woman};
                    MPI_Request send_proposal;
                    MPI_Isend(&send_arr, 2, MPI_INT, master, 0, MPI_COMM_WORLD, &send_proposal);
                    MPI_Wait(&send_proposal, MPI_STATUS_IGNORE);
                    MPI_Iprobe(master, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
                    if (flag)
                    {
                        flag = 0;
                        int recv_arr[2];
                        MPI_Recv(&recv_arr, 2, MPI_INT, master, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        int code = recv_arr[1];
                        int recv_man = recv_arr[0];
                        if (code == reject)
                        {
                            free_men.push_back(recv_man);
                            auto remove = find(free_men.begin(), free_men.end(), man);
                            free_men.erase(remove);
                            break;
                        }
                        if (code == cont)
                        {
                            continue;
                        }
                    }
                }
                break;
            }
        }
        MPI_Finalize();
    }

    return 0;
}
