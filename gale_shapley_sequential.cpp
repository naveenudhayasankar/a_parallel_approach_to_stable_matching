#include<iostream>
#include<unordered_map>
#include<string>
#include<vector>
#include<algorithm>
#include<fstream>
#include<sstream>

std::unordered_map<std::string, std::vector<std::string>> readFileToMap(const std::string &filename){
    std::ifstream src;
    std::unordered_map<std::string, std::vector<std::string>> ret_map;
    std::string key;
    std::string value;
    std::string line;
    src.open(filename);
    while(src.good()){
        std::getline(src, line);
        std::istringstream ss(line);
        ss >> key >> value;
        ret_map[key].push_back(value); 
    }
    for(auto i : ret_map){
        std::cout<<i.first<<":"<<std::endl;
        for(auto j : i.second)
            std::cout<<j<<" ";
        std::cout<<" "<<std::endl;
    }
    return ret_map;
}

int find_index(std::vector<std::string> vec, std::string find){
    auto itr = std::find(vec.begin(), vec.end(), find);
    return std::distance(vec.begin(), itr);
}

int main(int argc, char **argv){
    std::unordered_map<std::string, std::vector<std::string>> men = readFileToMap(argv[1]); 
    std::unordered_map<std::string, std::vector<std::string>>::iterator it;
    for(it = men.begin(); it != men.end(); it++){
        std::cout<<it->first<<"-";
        for(auto i : it->second)
            std::cout<<i<<",";
        std::cout << std::endl;
    }
    // = {
    //     {"Ross" , {"Rachel", "Jennifer", "Aniston"}},
    //     {"Barney", {"Jennifer", "Rachel", "Aniston"}},
    //     {"Ted", {"Rachel", "Aniston", "Jennifer"}},
    //     {"Sheldon", {"Aniston", "Patty", "Jennifer", "Rachel"}}
    // };

    std::unordered_map<std::string, std::vector<std::string>> women = readFileToMap(argv[2]); 
    // = {
    //     {"Rachel" , {"Sheldon","Ross", "Ted", "Barney"}},
    //     {"Jennifer", {"Barney", "Ross", "Ted","Sheldon"}},
    //     {"Aniston", {"Ted", "Sheldon","Ross", "Barney"}},
    //     {"Patty",{"Sheldon","Ross","Barney","Ted"}}
    // };

    // std::cout<< women["Aniston"].front();

    std::vector<std::vector<std::string>> tentative_assignments; 

    std::vector<std::string> free_men;

    for(auto m : men){
        //std::cout << m.first << std::endl;
        free_men.push_back(m.first);
    }

    //std::cout<< free_men.size() << std::endl;
    while(free_men.size() > 0){
        for(auto man : free_men){
            for(auto woman : men[man]){
                std::vector<std::vector<std::string>> taken;
                std::for_each(tentative_assignments.begin(), tentative_assignments.end(), [&taken, woman](std::vector<std::string> s)
                {
                    if(s[1] == woman)
                        taken.push_back(s);
                });
                if(taken.size() == 0){
                    std::cout<< "First Matching " << man << " and " << woman << std::endl;
                    std::vector<std::string> add = {man, woman};
                    tentative_assignments.push_back(add);
                    std::vector<std::string>::iterator remove = std::find(free_men.begin(), free_men.end(), man); 
                    free_men.erase(remove);
                    break;
                }
                else if(taken.size() > 0){
                    std::string chosen_man = taken[0][0];
                    int chosen = find_index(women[woman], chosen_man);
                    int current = find_index(women[woman], man);
                    if(chosen < current){
                        std::cout<< woman << " already has a better match with " << chosen_man << std::endl;
                    }
                    else{
                        std::cout<< "Matching " << man << " and " << woman << std::endl;
                        std::vector<std::string>::iterator remove = std::find(free_men.begin(), free_men.end(), man); 
                        free_men.erase(remove);
                        free_men.push_back(chosen_man);
                        tentative_assignments.push_back({man, woman});
                        std::vector<std::vector<std::string>> temp;
                        std::for_each(tentative_assignments.begin(), tentative_assignments.end(),[chosen_man, woman, &temp](std::vector<std::string> s)
                        {
                            if(!(s[0] == chosen_man && s[1] == woman))
                                temp.push_back(s);
                        });
                        tentative_assignments = temp;
                        taken[0][0] = man;
                        break;
                    }
                }
            }
            break;
        }
    }
}

