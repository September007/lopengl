#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    const char *fake_agv[]={argv[0],"--gtest_catch_exceptions=0","--gtest_break_on_failure"};
    //GTEST_FLAG_GET(break_on_failure) = true;
    //GTEST_FLAG_GET(catch_exceptions) = false;
     testing::InitGoogleTest(&argc,(char**) fake_agv);
    // auto pb = GTEST_FLAG_GET(break_on_failure);
    // auto pc = GTEST_FLAG_GET(catch_exceptions);
    return RUN_ALL_TESTS();
}