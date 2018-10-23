#include "gtest/gtest.h"

#include <chrono>
#include <list>
#include <string>
#include <thread>
#include <vector>

#include "watchdog.hpp"

bool exists(std::list<std::string> l, std::string e)
{
    for (auto i: l) {
        if (i == e) {
            return true;
        }
    }
    return false;
}

void assert_expired(std::list<std::string> have, std::vector<std::string> should)
{
    for (auto expect: should) {
        ASSERT_TRUE(exists(have, expect)) << expect << " not found";
    }
}

TEST(Watchdog, BasicFunctionality)
{
    auto w = Watchdog();
    w.refresh("hal");
    w.refresh("nicmgr");
    w.refresh("agent");
    auto sleep = w.next_tick();
    ASSERT_EQ(sleep, 15);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    sleep = w.next_tick();
    ASSERT_EQ(sleep, 13);
    w.refresh("hal");
    std::this_thread::sleep_for(std::chrono::seconds(13));
    sleep = w.next_tick();
    ASSERT_EQ(sleep, 2);
    assert_expired(w.expired(), {"nicmgr", "agent"});
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
