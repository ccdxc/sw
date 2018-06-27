#include "iostream"
#include "pal.h"
#include "planner.h"

int main() {
    planner::plan_and_move("/sw/nic/move_planner/hal_mem.json", "/sw/nic/move_planner/hal_mem_after.json", true);
    return 0;
}
