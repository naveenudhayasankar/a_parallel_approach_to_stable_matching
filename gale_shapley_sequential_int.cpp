#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <mpi.h>

using namespace std;

int find_index(vector<int> vec, int to_find)
{
    auto itr = find(vec.begin(), vec.end(), to_find);
    return distance(vec.begin(), itr);
}

int main(int argc, char **argv)
{
    int rank, size;
    int master = 0;
    ifstream file;
    file.open(argv[1]);
    int total;
    file >> total;

    unordered_map<int, vector<int>> men;
    unordered_map<int, vector<int>> women;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == master)
    {
        double start = MPI_Wtime();
        for (int i = 1; i <= total; i++)
        {
            for (int j = 1; j <= total; j++)
            {
                int val;
                file >> val;
                men[i].push_back(val);
            }
        }

        for (int i = 1; i <= total; i++)
        {
            for (int j = 1; j <= total; j++)
            {
                int val;
                file >> val;
                women[i].push_back(val);
            }
        }

        vector<vector<int>> tentative_assignments;

        vector<int> free_men;

        for (auto m : men)
        {
            // cout << m.first <<  endl;
            free_men.push_back(m.first);
        }

        // cout<< free_men.size() <<  endl;
        while (free_men.size() > 0)
        {
            for (auto man : free_men)
            {
                for (auto woman : men[man])
                {
                    vector<vector<int>> taken;
                    for_each(tentative_assignments.begin(), tentative_assignments.end(), [&taken, woman](vector<int> s)
                             {
                    if(s[1] == woman)
                        taken.push_back(s); });
                    if (taken.size() == 0)
                    {
                        cout << "First Matching " << man << " and " << woman << endl;
                        vector<int> add = {man, woman};
                        tentative_assignments.push_back(add);
                        vector<int>::iterator remove = find(free_men.begin(), free_men.end(), man);
                        free_men.erase(remove);
                        break;
                    }
                    else if (taken.size() > 0)
                    {
                        int chosen_man = taken[0][0];
                        int chosen = find_index(women[woman], chosen_man);
                        int current = find_index(women[woman], man);
                        if (chosen < current)
                        {
                            cout << woman << " already has a better match with " << chosen_man << endl;
                        }
                        else
                        {
                            cout << "Matching " << man << " and " << woman << endl;
                            vector<int>::iterator remove = find(free_men.begin(), free_men.end(), man);
                            free_men.erase(remove);
                            free_men.push_back(chosen_man);
                            tentative_assignments.push_back({man, woman});
                            vector<vector<int>> temp;
                            for_each(tentative_assignments.begin(), tentative_assignments.end(), [chosen_man, woman, &temp](vector<int> s)
                                     {
                            if(!(s[0] == chosen_man && s[1] == woman))
                                temp.push_back(s); });
                            tentative_assignments = temp;
                            taken[0][0] = man;
                            break;
                        }
                    }
                }
                break;
            }
        }
        double end = MPI_Wtime();
        cout << "Runtime is " << end - start;
    }
    return 0;
}
