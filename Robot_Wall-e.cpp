#include "RobotBase.h"
#include <vector>
#include <iostream>
#include <algorithm> // For std::find_if

class Robot_Wall_e : public RobotBase 
{
private:
    bool m_moving_down = true; // Tracks vertical movement direction
    int to_shoot_row = -1;   // Tracks the row of the next target to shoot
    int to_shoot_col = -1;   // Tracks the column of the next target to shoot
    int m_move_direction = 3;
    bool run_away;
    int radar_direction = 1; // Radar scanning direction (1-8)
    bool at_edge = false;
    const int max_range = 4; // Maximum range of the flamethrower
    bool go_right = false;
    std::vector<RadarObj> known_obstacles; // Permanent obstacle list

    // Helper function to determine if a cell is an obstacle
   bool is_obstacle(int row, int col) const 
    {
        return std::any_of(known_obstacles.begin(), known_obstacles.end(), 
                           [&](const RadarObj& obj) {
                               return obj.m_row == row && obj.m_col == col;
                           });
    }
    
    // Clears the target when no enemy is found
    void clear_target() 
    {
        to_shoot_row = -1;
        to_shoot_col = -1;
        m_move_direction = 3;
        run_away = false;
    }
    // Helper function to calculate Manhattan distance
    int calculate_distance(int row1, int col1, int row2, int col2) const 
    {
        return std::abs(row1 - row2) + std::abs(col1 - col2);
    }

    // Helper function to add an obstacle to the list if it's not already there
   void add_obstacle(const RadarObj& obj) 
    {
        if ((obj.m_type == 'M' || obj.m_type == 'P' || obj.m_type == 'F' || obj.m_type == 'X') &&
            !is_obstacle(obj.m_row, obj.m_col))
        {
            known_obstacles.push_back(obj);
        }
    }
    void set_move_direction(int direction){
        m_move_direction = direction;
    }


public:
    Robot_Wall_e() : RobotBase(3, 4, flamethrower) {} 

   virtual void get_radar_direction(int& radar_direction_out) override 
    {
       if(at_edge == true){
            if(radar_direction == 1 || radar_direction == 2 || radar_direction == 8){
                radar_direction = 3;
                radar_direction_out = radar_direction;
                return;
            }
            if(radar_direction == 3 ) radar_direction = 7;
            if(radar_direction == 7) radar_direction = 5;
            if(radar_direction == 5) radar_direction = 4;
            if(radar_direction == 4) radar_direction = 3;
            radar_direction_out = radar_direction;
            return;

       }
        radar_direction_out = radar_direction;
        radar_direction = (radar_direction % 4) + 1; 

    }



    // Processes radar results and updates known obstacles and target
    virtual void process_radar_results(const std::vector<RadarObj>& radar_results) override 
    {
        clear_target();

        int rr, current_col;
        get_current_location(rr, current_col);

        for (const auto& obj : radar_results) 
        {
            add_obstacle(obj);
            if (obj.m_type == 'M' || obj.m_type == 'P' || obj.m_type == 'F' || obj.m_type == 'X')
                continue;

            int dist = calculate_distance(obj.m_row, obj.m_col, rr, current_col);
           
                
            if (dist == 1) 
            {
                run_away = true;

                int dr = rr - obj.m_row;
                int dc = current_col - obj.m_col;

                if (dr != 0)
                    m_move_direction = (dr < 0) ? 1 : 5; 
                else if (dc != 0)
                    m_move_direction = (dc < 0) ? 7 : 3; 

                return; 
            }
            if (dist <= 4 || get_move_speed() == 0)
            {
                to_shoot_row = obj.m_row;
                to_shoot_col = obj.m_col;
                return; 
            }
        }
        
    }
    

    // Determines the next shot location
    virtual bool get_shot_location(int& shot_row, int& shot_col) override 
    {
        if (to_shoot_row != -1 && to_shoot_col != -1) 
        {
            shot_row = to_shoot_row;
            shot_col = to_shoot_col;
            clear_target(); // Clear target after shooting
            return true;
        }
        return false;
    }

    // Determines the next movement direction
    virtual void get_move_direction(int& move_direction, int& move_distance) override 
    {
        int r, c;
        get_current_location(r, c);
        int max_move = get_move_speed();
        if (max_move <= 0) { move_direction = 0; move_distance = 0; return; }

        if (run_away)
        {
            int dir = m_move_direction;
            move_distance = 0;

            for (int step = 1; step <= max_move; ++step)
            {
                int nr = r, nc = c;
                if (dir == 1) nr -= step;
                else if (dir == 5) nr += step;
                else if (dir == 3) nc += step;
                else if (dir == 7) nc -= step;

                if (nr < 0 || nr > m_board_row_max || nc < 0 || nc > m_board_col_max) break;
                if (is_obstacle(nr, nc)) break;

                move_distance = step;
            }

            move_direction = (move_distance > 0) ? dir : 0;
            return;
        }
        if(r == 0 || r == 1){
            at_edge = true;
        }
        else at_edge = false;
        if (r == 0)
        {
            if(c == 0){
                go_right = true;
                if(is_obstacle(r, c+1)){
                    move_direction = 5;
                    move_distance = 1;
                    return;
                }
                if(!is_obstacle(r, c+2) && c+2 < m_board_col_max){
                    move_direction = 3;
                    move_distance = 2;
                    return;
                }
                move_direction = 3;
                move_distance = 1;
                return;
            }
            if(go_right){
                if(is_obstacle(r, c+1)){
                    move_direction = 5;
                    move_distance = 1;
                    go_right = false;
                    return;
                }
                if(!is_obstacle(r, c+2) && c+2 < m_board_col_max){
                    move_direction = 3;
                    move_distance = 2;
                    return;
                }
                move_direction = 3;
                move_distance = 1;
                return;

            }
            if (c < m_board_col_max)
            {
                
                if(is_obstacle(r, c+1)){//both ways obstacles, go down
                    if(is_obstacle(r, c-1)){
                        move_direction = 5;
                        move_distance = 1;
                        go_right = false;
                        return;
                    }
                    
                    move_direction = 7;
                    move_distance = 1;
                    return;
                }
                if(is_obstacle(r, c-1)){
                        move_direction = 7;
                        move_distance = 1;
                        return;
                }
                move_direction = 3; 
                move_distance = std::min(max_move, m_board_col_max - c);
                return;
            }
            else 
            {   
                if(is_obstacle(r, c-1)){
                    if(is_obstacle(r, c+1)){
                        move_direction = 5;
                        move_distance = 1;
                        return;
                    }
                    move_direction = 3;
                    move_distance = 1;
                    return;
                }
                move_direction = 7; 
                move_distance = 1;
                return;
            }
        }
        if (c == m_board_col_max)
        {
            go_right= false;
            if (r < m_board_row_max)
            {
                move_direction = 7; 
                move_distance = std::min(max_move, m_board_row_max - r);
                return;
            }
            else 
            {
                move_direction = 7; 
                move_distance = 1;
                return;
            }
        }

        if (r == m_board_row_max)
        {
            if (c > 0)
            {
                move_direction = 1; 
                move_distance = std::min(max_move, c);
                return;
            }
            else 
            {
                move_direction = 1; 
                move_distance = 1;
                return;
            }
        }

        if (c == 0)
        {
            if (r > 0)
            {
                move_direction = 1; 
                move_distance = std::min(max_move, r);
                return;
            }
            else 
            {
                move_direction = 3;
                move_distance = 1;
                return;
            }
        }
        if(is_obstacle(r+1, c)) {
            if(!is_obstacle(r, c+1) && c+1 < m_board_col_max){
                move_direction = 3; 
                move_distance  = 1;
                return;
            }
            if(!is_obstacle(r, c-1) && c-1> 0){
                move_direction = 7; 
                move_distance  = 1;
                return;
            }
            
        }
        move_direction = 1; 
        move_distance  = std::min(max_move, r);
    }
    

};

// Factory function to create Robot_Wall_e
extern "C" RobotBase* create_robot() 
{
    return new Robot_Wall_e();
}
