#include <gtest/gtest.h>
#include <helper/gtest_main_proxy.h>



int main(int argc, char **argv)
{
    // if there doesn't want execute gtest env
    if(class_main_proxy::Instance().main){
        return class_main_proxy::Instance().main(argc,argv);
    }
    const char *fake_agv[]={argv[0],"--gtest_catch_exceptions=0","--gtest_break_on_failure"};
    //GTEST_FLAG_GET(break_on_failure) = true;
    //GTEST_FLAG_GET(catch_exceptions) = false;
     testing::InitGoogleTest(&argc,(char**) fake_agv);
    // auto pb = GTEST_FLAG_GET(break_on_failure);
    // auto pc = GTEST_FLAG_GET(catch_exceptions);
    return RUN_ALL_TESTS();
}