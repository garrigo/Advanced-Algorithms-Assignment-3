#include"rasterization.h"
#include"scene.h"
#include"read-obj.h"
using namespace pipeline3D;
#include<iostream>
#include<chrono>


    //Values for testing, indicating number of workers used by the rasterizer, number of iterations of the add object loop, number of iterations of render loop
    constexpr unsigned int NUMBER_OF_WORKERS = 1;
    constexpr unsigned int ADD_OBJECTS_ITERATIONS = 500;
    constexpr unsigned int RENDER_ITERATIONS = 100;


    struct my_shader{
         inline char operator ()(const Vertex &v)  {
            return static_cast<char>((v.z-1)*10.0f+0.5f)%10+'0';
        }
    };


    int main() {

        const int w=150;
        const int h=50;    
        my_shader shader;
        //Rasterizer can be instantiated with the number of worker-threads as argument. If not specified, worker-threads number = std::thread::hardware_concurrency()
        //If argument > std::thread::hardware_concurrency() the latter will be used. User can force the former value by using forceMaxWorkers method of rasterizer.
        Rasterizer<char> rasterizer(NUMBER_OF_WORKERS);
        // rasterizer.forceMaxWorkers(10);
        std::cout << "Number of worker-threads: " << rasterizer.getMaxWorkers() << "\n";
        rasterizer.set_perspective_projection(-1,1,-1,1,1,2);

        std::vector<char> screen(w*h,'.');
        rasterizer.set_target(w,h,&screen[0]);

        // For loop inserting identic (overlapping) objects in the scene to be renderized, ADD_OBJECTS_ITERATIONS times
        std::vector<std::vector<std::array<Vertex,3>> > meshes;
        for (int i=0; i< ADD_OBJECTS_ITERATIONS; i++){
            meshes.push_back(read_obj("cubeMod.obj"));
        }
        //Another object partially overlapping to the previous ones
        // meshes.push_back(read_obj("strange.obj"));
   
        Scene<char> scene;
        scene.view_={0.5f,0.0f,0.0f,0.7f,0.0f,0.5f,0.0f,0.7f,0.0f,0.0f,0.5f,0.9f,0.0f,0.0f,0.0f,1.0f};
        for (auto& m : meshes)
            scene.add_object(Scene<char>::Object(std::move(m),shader));


        
        std::cout << "Rendering ...\n";
        auto start_time = std::chrono::high_resolution_clock::now();

        //For loop executing render method of a scene with a rasterizer, RENDER_ITERATIONS times
        for (int i=0; i!=RENDER_ITERATIONS; ++i) {
            scene.render(rasterizer);
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed_time = std::chrono::duration<double>(end_time-start_time).count();
        std::cout << "ELAPSED TIME: " << elapsed_time << '\n';


        // print out the screen with a frame around it
        std::cout << "\n\n";
        std::cout << '+';
        for (int j=0; j!=w; ++j) std::cout << '-';
        std::cout << "+\n";

        for (int i=0;i!=h;++i) {
            std::cout << '|';
            for (int j=0; j!=w; ++j) std::cout << screen[i*w+j];
            std::cout << "|\n";
        }

        std::cout << '+';
        for (int j=0; j!=w; ++j) std::cout << '-';
        std::cout << "+\n";

        
        return 0;
    }

