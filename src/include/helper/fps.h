#pragma warning(disable:4819)
#include <chrono>
#include <vector>

struct time_point_sampler
{
    using clock=std::chrono::system_clock;
    using T=decltype(clock::now());
    static constexpr int count_of_fps_time_points_samples = 11;
    T time_points[time_point_sampler::count_of_fps_time_points_samples];
    int next=0;//[0,coftps): 缓冲区未采满,代表缓冲区中的数据个数 [coftps,coftps*2]: 缓冲区采集满，此时next代表缓冲区最旧的数据位置+coftps
    void Tick(){
        if(next<time_point_sampler::count_of_fps_time_points_samples)
        {
            time_points[next]=clock::now(); 
        }else
        {
            time_points[next-count_of_fps_time_points_samples]=clock::now();
        }
        ++next;
        if(next==2*count_of_fps_time_points_samples)
            next-=count_of_fps_time_points_samples;
    }
    clock::duration Frame_length(){
        if(next<2)return clock::duration::max();
        if(next<count_of_fps_time_points_samples)
            return (time_points[next-1]-time_points[0])/(next-1);
        else 
            return (time_points[(next-1)%count_of_fps_time_points_samples]-time_points[next%count_of_fps_time_points_samples])/count_of_fps_time_points_samples;   
    }
    int FPS(){
        return 1000/std::chrono::duration_cast<std::chrono::duration<int,std::ratio<1,1000>>>(Frame_length()).count();
    }
};