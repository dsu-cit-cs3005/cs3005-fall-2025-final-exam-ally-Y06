#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <dlfcn.h>
#include "RobotBase.h"
#include <ctime>
#include <list>
#include <string_view>
#include "Scans.cpp"
#include <unistd.h>

//variables so typing is easier
typedef RobotBase* (*make_robot)();
std::ostream &out = std::cout;
std::istream &in = std::cin;
char end = '\n';




std::vector<char> create_board(int rows, int cols){
    std::vector<char> board;
    board.resize(rows*cols);
    return board;
}
RobotBase* load_robot(std::string &so){
    void* handle = dlopen(so.c_str(), RTLD_LAZY);
    RobotBase* (*create)() = (make_robot) dlsym(handle, "create_robot");

    return create();
}
std::vector<RobotBase*> load_so() {
    namespace fs = std::filesystem;

    std::vector<std::string> robot_files;

    // --- FIND ALL Robot_*.cpp FILES ---
    for (const auto& entry : fs::directory_iterator(".")) {
        if (!entry.is_regular_file()) continue;

        std::string name = entry.path().filename().string();

        if (name.size() > 10 &&                         // min "Robot_.cpp"
            name.rfind("Robot_", 0) == 0 &&             // starts with Robot_
            name.substr(name.size() - 4) == ".cpp")     // ends with .cpp
        {
            robot_files.push_back(name);
            std::cout << "Found robot source: " << name << "\n";
        }
    }

    std::vector<RobotBase*> robots;  // <-- only declare once here

    // --- COMPILE & LOAD ---
    for (const std::string& filename : robot_files) {

        std::string shared_lib = filename.substr(0, filename.size() - 4) + ".so";

        std::string compile_cmd =
            "g++ -shared -fPIC -o " + shared_lib + " " + filename +
            " RobotBase.o -I. -std=c++20";

        std::cout << "Compiling: " << compile_cmd << "\n";

        if (std::system(compile_cmd.c_str()) != 0) {
            std::cerr << "Compile failed for " << filename << "\n";
            continue;
        }

        void* handle = dlopen(shared_lib.c_str(), RTLD_LAZY);
        if (!handle) {
            std::cerr << "dlopen failed for " << shared_lib 
                      << ": " << dlerror() << "\n";
            continue;
        }

        RobotFactory create_robot =
            (RobotFactory)dlsym(handle, "create_robot");

        if (!create_robot) {
            std::cerr << "Missing create_robot() in " << shared_lib 
                      << ": " << dlerror() << "\n";
            dlclose(handle);
            continue;
        }

        RobotBase* robot = create_robot();

        // Set the robot's name to what comes after "Robot_"
        robot->m_name = filename.substr(6, filename.size() - 6 - 4);

        robots.push_back(robot);
        std::cout << "Loaded robot: " << robot->m_name << "\n";
     
    }

    return robots;
}


void configure_arena(int &rows, int &cols, int &mounds, int &flames, int &pits, int &rounds, int &live){
    std::ifstream inputFile("config.in");
    if(inputFile >> rows >> cols >> mounds >> flames >> pits >> rounds >> live){
       out << "Configuring arena..." <<std::endl;
       out << rows << "x" << cols << std::endl;
    }
    else{
       out << "Error reading config file, setting default values...\n";
       out << "10x10\n";
        rows = 10;
        cols = 10;
        mounds = 3;
        flames = 3;
        pits = 3;
        rounds = 10;
        live = 1;
    }
}
void make_obstacle(std::vector<char>& board, int obstacles, char letter, int& cols, int& rows){
    for(int i = 0; i < obstacles; i++){
        int x, y;
        while(true){
            x = rand() % rows;
            y = rand() % cols;
            if(board[x * cols + y] == '.'){
                board[x*cols + y] = letter;
                break;
            }
        }
    }
}
void place_robot(std::vector<char>& board, char symbol, int& cols, int& rows, RobotBase* robot){
    int x, y;
    while(true){
        x = rand() % rows;
        y = rand() % cols;
        if(board[x * cols + y] == '.'){
            board[x*cols + y] = symbol;
            robot->move_to(x, y);
            break;
        }
    }
    
}
void print_board(int& rows, int& cols, std::vector<char>& board){
   out<<"  ";
    for (int col = 0; col < cols; col++) {
        if(col < 10)out<<' ' << col << ' ';
        else out<< col << ' ';
    }
    out<<'\n';
    for(int row = 0; row < rows; row++){
       out << row;
       if(row<10) out<<' ';
        for(int col = 0; col < cols; col++){
            char cell = board[row*cols + col];
            if(cell == 'M' || cell == 'P' || cell == 'F' || cell == '.' || cell=='X')out<<' '<< cell  << ' ';
            else out<< 'R' << cell  << ' ';
        }
       out << '\n';
    }
    out<<'\n';
}
void radar_scan(int direction, std::vector<RadarObj> *radar_results, std::vector<char>& board, int rows, int cols, RobotBase* robot){
    int r_row, r_col;
    robot->get_current_location(r_row, r_col);
    if (direction == 1){
        up(radar_results, board, rows, cols, robot);
    }
    else if (direction == 2){
        up_right(radar_results, board, rows, cols, robot);
    }
    else if (direction == 3){
        right(radar_results, board, rows, cols, robot);
    }
    else if (direction == 4){
        down_right(radar_results, board, rows, cols, robot);
    }
    else if (direction == 5){
        down(radar_results, board, rows, cols, robot);
    }
    else if (direction == 6){
        down_left(radar_results, board, rows, cols, robot);
    }
    else if (direction == 7){
        left(radar_results, board, rows, cols, robot);
    }
    else if (direction == 8){
        up_left(radar_results, board, rows, cols, robot);
    }
    else if(direction == 0){
        scan_surrounding(radar_results, board, rows, cols, robot);
    }

}
int apply_damage_with_armor(RobotBase* target, int base_damage)
{
    int armor = target->get_armor();   
    double reduction = armor * 0.10;   

    int final_damage = static_cast<int>(base_damage * (1.0 - reduction));

    if (final_damage < 0)
        final_damage = 0;

    if (armor > 0)
        target->reduce_armor(1);
    target->take_damage(final_damage);
    return final_damage;
}
void move_robot(RobotBase* robot, int rows, int cols, std::vector<char> &board){
    int direction, distance;
    robot->get_move_direction(direction, distance);
    if(distance > robot->get_move_speed()){
        distance = robot->get_move_speed();
    }
    else if(distance<0){
        distance = 0;
    }
    int r, c;
    robot->get_current_location(r, c);
    int r_o = r;
    int c_o = c;
    for(int step = 0; step < distance; ++step) {
        int new_r = r + directions[direction].first;
        int new_c = c + directions[direction].second;

        // Check bounds
        if(new_r < 0 || new_r >= rows || new_c < 0 || new_c >= cols) break;

        char cell = board[new_r * cols + new_c];

        if(cell == '.') {
            // Move robot
            r = new_r;
            c = new_c;
        } 
        else if(cell == 'M'){
            out<<robot->m_name<<" hit a mound.\n";
            break;
        }
        else if(cell == 'P'){
            out<<robot->m_name<<" fell in a pit. \n";
            robot->disable_movement();
            r = new_r;
            c = new_c;
            break;
            

        }
        else if(cell == 'X'){
            out<<robot->m_name<<" hit a dead robot.\n";
            break;
        }
        else if(cell == 'F'){
            out<<robot->m_name<<" moved into burning flames\n";
            int damage = 30 + (std::rand() % 21); 
            int dmg = apply_damage_with_armor(robot, damage);
            out<<"They take "<<dmg<<" damage.\n";
            
            r = new_r;
            c = new_c;
            board[r *cols + c] = '.';//the flames are absorbed and gone
        }
        else{
            out<<robot->m_name<<" hit another robot.\n";
            break;
        }
    }

    // Update robot's location
    out<<robot->m_name<<" is in spot ("<<r<<", "<<c<<") \n";
    if(r != r_o || c != c_o){
        board[r *cols + c] = robot->m_character;
        board[r_o *cols + c_o] = '.';
    }
    
    robot->move_to(r, c);

}

void fire_railgun(RobotBase* robot, int rows, int cols, std::vector<char>& board, int r_shoot, int c_shoot, std::vector<RobotBase*> robots)
{
    int r, c;
    robot->get_current_location(r, c);

    int dr = r_shoot - r;
    int dc = c_shoot - c;

    int step_r;
    if (dr > 0) step_r = 1;
    else if (dr < 0) step_r = -1;
    else step_r = 0;

    int step_c;
    if (dc > 0) step_c = 1;
    else if (dc < 0) step_c = -1;
    else step_c = 0;

    // If both are zero: robot shot itself — do nothing
    if (step_r == 0 && step_c == 0)
        return;

    int damage = 10 + (std::rand() % 11);

    int cur_r = r + step_r;
    int cur_c = c + step_c;

    while (cur_r >= 0 && cur_r < rows && cur_c >= 0 && cur_c < cols)
    {
        char cell = board[cur_r * cols + cur_c];

        if (cell != '.' && cell != 'M' && cell != 'P' && cell != 'F' && cell != 'X')
        {
            
            // Find the robot and damage it
            for (RobotBase* bot : robots)  // you need this global or passed-in list
            {
                int orow, ocol;
                bot->get_current_location(orow, ocol);

                if (orow == cur_r && ocol == cur_c)
                {
                    int dmg = apply_damage_with_armor(bot, damage);
                    out << robot->m_name << " shoots robot "<<bot->m_name<< " for "<< dmg << " damage.\n";

                    if (bot->get_health() <= 0)
                    {
                        out << bot->m_name << " was eliminated.\n";
                        board[cur_r * cols + cur_c] = 'X';
                        bot->m_character = 'X';
                    }
                }
            }
        }

        // Move to next cell along the ray
        cur_r += step_r;
        cur_c += step_c;
    }
}
void fire_launcher(RobotBase* robot, int rows, int cols, std::vector<char>& board, int r_shoot, int c_shoot, std::vector<RobotBase*> robots){
    (void) rows;
    int r, c;
    robot->get_current_location(r, c);
    
    if(robot->get_grenades() > 5)//it's 15 in robot base, but specs say 10
    {
        int base_damage = 10 + (std::rand() % 31);
        for(int dr = -1; dr <= 1; dr++){
            for(int dc = -1; dc <= 1; dc++){
                int rr = r_shoot + dr;
                int cc = c_shoot + dc;

                if(rr < 0 || rr >= rows || cc < 0 || cc >= cols)
                    continue;

                char cell = board[rr * cols + cc];

                if(cell == '.' || cell == 'M' || cell == 'P' || cell == 'F' || cell == 'X')
                    continue;

                for(RobotBase* bot : robots){
                    int br, bc;
                    bot->get_current_location(br, bc);

                    if(br == rr && bc == cc){

                        int dmg = apply_damage_with_armor(bot, base_damage);

                        out << robot->m_name << " hits robot "<<bot->m_name<< " for "<< dmg << " damage.\n";

                        if(bot->get_health() <= 0){
                            out << "     " << bot->m_name << " was eliminated.\n";
                            board[rr * cols + cc] = 'X';
                            bot->m_character = 'X';
                        }

                        break;
                    }
                }
            }
        }
    }
    else{
        out<<robot->m_name<<" is out of grenades. Can't shoot.";
    }
}
void fire_flamethrower(RobotBase* robot, int rows, int cols, std::vector<char>& board,int r_shoot, int c_shoot, std::vector<RobotBase*> robots)
{
    int r, c;
    robot->get_current_location(r, c);

    int dr = r_shoot - r;
    int dc = c_shoot - c;

    if (dr > 1) dr = 1;
    if (dr < -1) dr = -1;
    if (dc > 1) dc = 1;
    if (dc < -1) dc = -1;

    if (dr == 0 && dc == 0)
        return;
    int pr = -dc;
    int pc = dr;

    for (int step = 1; step <= 4; step++)
    {
        int base_r = r + dr * step;
        int base_c = c + dc * step;

        for (int side = -1; side <= 1; side++)
        {
            int fr = base_r + pr * side;
            int fc = base_c + pc * side;

            if (fr < 0 || fr >= rows || fc < 0 || fc >= cols)
                continue;

            char &cell = board[fr * cols + fc];

            if (cell == '.' || cell == 'M' || cell == 'P' || cell == 'F' || cell == 'X')
                continue;

            int damage = 30 + (std::rand() % 21); 

            

            for (RobotBase* bot : robots)
            {
                int br, bc;
                bot->get_current_location(br, bc);

                if (br == fr && bc == fc)
                {
                   int dmg = apply_damage_with_armor(bot, damage);
                   out << robot->m_name << " burns robot "<<bot->m_name<< " for "<< dmg << " damage.\n";

                    if (bot->get_health() <= 0)
                    {
                        out << bot->m_name << " was burned and died.\n";
                        cell = 'X';
                        bot->m_character = 'X';
                    }
                    break;
                }
            }
        }
    }
}

void fire_hammer(RobotBase* robot, int rows, int cols, std::vector<char>& board, int r_shoot, int c_shoot, std::vector<RobotBase*> robots)
{
    (void) rows;
    int r, c;
    robot->get_current_location(r, c);

    // Must be next to the target (8-way adjacency)
    int dr = std::abs(r_shoot - r);
    int dc = std::abs(c_shoot - c);

    // Not adjacent? Hammer can't hit.
    if (!((dr <= 1) && (dc <= 1) && !(dr == 0 && dc == 0)))
        return;

    char cell = board[r_shoot * cols + c_shoot];

    // Must be a robot
    if (cell == '.' || cell == 'M' || cell == 'P' || cell == 'F' || cell == 'X')
        return;

    int damage = 50 + (std::rand() % 11); // 50–60 damage

   

    // Find that robot and apply damage
    for (RobotBase* bot : robots)
    {
        int orow, ocol;
        bot->get_current_location(orow, ocol);

        if (orow == r_shoot && ocol == c_shoot)
        {
            int dmg = apply_damage_with_armor(bot, damage);
           out << robot->m_name << " attacks robot "<<bot->m_name<< " for "<< dmg << " damage.\n";

            if (bot->get_health() <= 0)
            {
                out << bot->m_name << " was smashed to pieces.\n";
                board[r_shoot * cols + c_shoot] = 'X';
                bot->m_character = 'X';
            }
            break;
        }
    }
}


int main(){
    bool playing = true;
    
    out<< "---Welcome to---\n";
    out<<"*******************\n";
    out<<" R O B O T W A R Z \n";
    out<<"*******************\n";

    sleep(1);
    //create arena with config file
    int rows, cols, mounds, flames, pits, rounds, live;
    configure_arena(rows, cols, mounds, flames, pits, rounds, live);
    std::vector<char> board(rows*cols, '.');
    srand(time(nullptr));

    make_obstacle(board, mounds, 'M', cols, rows);
    make_obstacle(board, flames, 'F', cols, rows);
    make_obstacle(board, pits, 'P', cols, rows);
    int current_round = 0;


    //make robots
    std::vector<RobotBase*> robots = load_so();
 
    int i = 0;
    std::vector<char> symbols = {'@', '!', '*', '#', '%', '&', '^', '(', ')', '[', ']', '<', '>'};
    for (RobotBase* robot : robots){
        robot->m_character = symbols[i];
        place_robot(board, robot->m_character, cols, rows, robot);
        out<<"Placed robot "<<robot->m_name << " with character "<< robot->m_character << std::endl;
        i++;
    }
    std::cout << "Want to watch live? (0:no, 1:yes) ";
    std::cin >> live;
    std::string winner;
    int living;

    
    //start
    while(playing && current_round < 3000){
        current_round++;
        if(live == 1) sleep(1);
        out << "\n =========== Starting Round "<< current_round<<" ===========\n\n";
        print_board(rows, cols, board);
        for (size_t i = 0; i < robots.size(); i++) {
            RobotBase* robot = robots[i];
            if(live == 1) sleep(1);
            
            living = 0;
            for (RobotBase* r : robots){
                if(r->get_health() > 0) {
                    winner = r->m_name;
                    living ++;
                }
            }

            //dead checks
            if(living <= 1) playing = false;

            int r, c;
            robot->get_current_location(r, c);

            if (robot->get_health() <= 0 && robot->m_character != 'X')
            {
                robot->m_character = 'X';
                board[r*cols + c] = 'X';   
                out<<"Robot: "<<robot->m_name << " is dead. \n\n";
                continue;                 
            }
            if(robot->get_health() <= 0) {
                 out<<"Robot: "<<robot->m_name << " is dead. \n\n";
                continue;
            }



            out<<"Robot: "<<robot->m_name << " (R" << robot->m_character<<") \n";
            out<<"Health: "<< robot->get_health()<<end;
            out<<"Armor: "<< robot->get_armor()<<end;
            out<<"Move Speed: "<< robot->get_move_speed()<<end;


            int direction;
            robot->get_radar_direction(direction);
            std::vector<RadarObj> radar_results;
            radar_scan(direction, &radar_results, board, rows, cols, robot);
            robot->process_radar_results(radar_results);
            int row_r, col_r;
            bool shoot = robot->get_shot_location(row_r, col_r);
            if(shoot == true){
                out<<robot->m_name<<" decided to shoot..\n";
                if(robot->get_weapon() == railgun) fire_railgun(robot, rows, cols, board, row_r, col_r, robots);
                else if(robot->get_weapon() == grenade) fire_launcher(robot, rows, cols, board, row_r, col_r, robots);
                else if(robot->get_weapon() == flamethrower) fire_flamethrower(robot, rows, cols, board, row_r, col_r, robots);
                else if(robot->get_weapon() == hammer) fire_hammer(robot, rows, cols, board, row_r, col_r, robots);
                else out<<"This robot has an invalid weapon type..\n";
            }
            else{
                out<<robot->m_name<<" decided to move..\n";
                move_robot(robot, rows, cols, board);
            }
            
            
            if(robot->get_move_speed() == 0) out<<"This robot is in a pit.\n";
            out<<end;
        }
    }
    out<<"========== Match Ended ========== \n";
    print_board(rows, cols, board);
    if(living > 1){
        out<<"Match ended in a tie\n";
    }
    else{
        out<<winner<<" is the winner!\n";
    }
}


//(2,2) -> (4,1)
//2, 1
//(2,2), (3,1.something)->rounds to 2, (4,1)