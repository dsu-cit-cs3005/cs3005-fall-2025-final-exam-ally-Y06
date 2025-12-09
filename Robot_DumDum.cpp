#include "RobotBase.h"
#include <vector>
#include <iostream>
#include <algorithm>

class Robot_DumDum : public RobotBase 
{
private:
    int target_row = -1;
    int target_col = -1;

public:
    Robot_DumDum() : RobotBase(3, 4, hammer) {} // Move 3, Armor 4, Hammer

    // -----------------------------
    // RADAR: Look toward where target might be
    // -----------------------------
    void get_radar_direction(int& radar_direction) override
    {
        int r, c;
        get_current_location(r, c);

        if (target_row == -1)
        {
            // No target yet — scan right first, then left next turn
            radar_direction = (c % 2 == 0) ? 3 : 7;
            return;
        }

        // Aim radar toward target
        int dr = target_row - r;
        int dc = target_col - c;

        if (abs(dr) > abs(dc))
            radar_direction = (dr > 0) ? 5 : 1;   // Down or up
        else
            radar_direction = (dc > 0) ? 3 : 7;   // Right or left
    }

    // -----------------------------
    // RADAR RESULTS
    // -----------------------------
    void process_radar_results(const std::vector<RadarObj>& radar_results) override
    {
        target_row = -1;
        target_col = -1;

        for (auto& obj : radar_results)
        {
            if (obj.m_type == 'R')     // enemy robot
            {
                target_row = obj.m_row;
                target_col = obj.m_col;
                return;
            }
        }
    }

    // -----------------------------
    // HAMMER: Only fire if adjacent
    // -----------------------------
    bool get_shot_location(int& shot_row, int& shot_col) override
    {
        if (target_row == -1)
            return false; // nothing to hit

        int r, c;
        get_current_location(r, c);

        int dist_r = abs(target_row - r);
        int dist_c = abs(target_col - c);

        if (dist_r + dist_c == 1)
        {
            // Target is adjacent — hammer can hit
            shot_row = target_row;
            shot_col = target_col;
            target_row = target_col = -1;
            return true;
        }

        // Not adjacent → do NOT waste shot
        return false;
    }

    // -----------------------------
    // MOVEMENT: Move toward target
    // -----------------------------
    void get_move_direction(int& move_direction, int& move_distance) override
    {
        int r, c;
        get_current_location(r, c);

        // No target: wander left/right
        if (target_row == -1)
        {
            move_direction = (c % 2 == 0) ? 3 : 7; // right / left alternating
            move_distance = 1;
            return;
        }

        int dr = target_row - r;
        int dc = target_col - c;

        move_distance = 1; // Hammer bot doesn’t need long-distance sprinting

        if (abs(dr) > abs(dc))
        {
            move_direction = (dr > 0) ? 5 : 1; // Down / Up
        }
        else
        {
            move_direction = (dc > 0) ? 3 : 7; // Right / Left
        }
    }
};

// Factory function
extern "C" RobotBase* create_robot()
{
    return new Robot_DumDum();
}
