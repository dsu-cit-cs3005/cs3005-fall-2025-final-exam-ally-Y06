#include "RobotBase.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>

class Robot_Bomber : public RobotBase 
{
private:
    bool m_moving_down = true;

    int target_row = -1;
    int target_col = -1;

    std::vector<RadarObj> known_obstacles;

    bool is_obstacle(int r, int c) const
    {
        return std::any_of(known_obstacles.begin(), known_obstacles.end(),
            [&](const RadarObj& o){ return o.m_row == r && o.m_col == c; });
    }

    void clear_target()
    {
        target_row = -1;
        target_col = -1;
    }

    void add_obstacle(const RadarObj& obj)
    {
        if ((obj.m_type == 'M' || obj.m_type == 'P' || obj.m_type == 'F') &&
            !is_obstacle(obj.m_row, obj.m_col))
        {
            known_obstacles.push_back(obj);
        }
    }

public:

    // Movement 3, armor 4, weapon = grenade launcher
    Robot_Bomber() : RobotBase(3, 4, grenade) {}

    // Better radar: always scan left, unless already at col 0
    void get_radar_direction(int& radar_direction) override
    {
        int r, c;
        get_current_location(r, c);

        if (c > 0)
            radar_direction = 7;  // Scan Left
        else
            radar_direction = 3;  // Scan Right
    }

    // Smarter processing: pick the closest enemy robot
    void process_radar_results(const std::vector<RadarObj>& radar) override
    {
        clear_target();

        int my_r, my_c;
        get_current_location(my_r, my_c);

        int best_dist = 999999;

        for (const auto& obj : radar)
        {
            add_obstacle(obj);

            if (obj.m_type == 'R')  // Enemy robot
            {
                int dist = abs(obj.m_row - my_r) + abs(obj.m_col - my_c);
                if (dist < best_dist)
                {
                    best_dist = dist;
                    target_row = obj.m_row;
                    target_col = obj.m_col;
                }
            }
        }
    }

    // Fires only if we have grenades left
    bool get_shot_location(int& sr, int& sc) override
    {
        if (target_row != -1 && target_col != -1 && get_grenades() > 0)
        {
            sr = target_row;
            sc = target_col;
            clear_target();
            return true;
        }

        return false;
    }

    // Movement: try to go left, but if blocked, move around
    void get_move_direction(int& dir, int& dist) override
    {
        int r, c;
        get_current_location(r, c);
        int max_move = get_move_speed();

        // If not at the left wall → always move left
        if (c > 0)
        {
            dir = 7; // Left
            dist = std::min(max_move, c);
            return;
        }

        // When at left wall → vertical patrol
        if (m_moving_down)
        {
            if (r + max_move < m_board_row_max)
            {
                dir = 5; // Down
                dist = std::min(max_move, m_board_row_max - r - 1);
            }
            else
            {
                m_moving_down = false;
                dir = 1; // Up
                dist = 1;
            }
        }
        else
        {
            if (r - max_move >= 0)
            {
                dir = 1; // Up
                dist = std::min(max_move, r);
            }
            else
            {
                m_moving_down = true;
                dir = 5; // Down
                dist = 1;
            }
        }
    }
};

// Factory
extern "C" RobotBase* create_robot()
{
    return new Robot_Bomber();
}
