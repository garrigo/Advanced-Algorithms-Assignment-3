#include"rasterization.h"
#include"scene.h"
#include"read-obj.h"
using namespace pipeline3D;

#include<iostream>
#include<chrono>







    struct my_shader{
         char operator ()(const Vertex &v)  {
            return static_cast<char>((v.z-1)*10.0f+0.5f)%10+'0';
        }
    };


    int main() {
        const int w=150;
        const int h=50;    
        my_shader shader;
        Rasterizer<char> rasterizer(10);
        std::cout << "Number of worker-threads: " << rasterizer.workers.getMaxWorkers() << "\n";
        rasterizer.set_perspective_projection(-1,1,-1,1,1,2);

        std::vector<char> screen(w*h,'.');
        rasterizer.set_target(w,h,&screen[0]);

        std::vector<std::vector<std::array<Vertex,3>> > meshes;
        for (int i=0; i< 250; i++){
            meshes.push_back(read_obj("cubeMod.obj"));
            meshes.push_back(read_obj("cube2.obj"));
        }
   
        Scene<char> scene;
        scene.view_={0.5f,0.0f,0.0f,0.7f,0.0f,0.5f,0.0f,0.7f,0.0f,0.0f,0.5f,0.9f,0.0f,0.0f,0.0f,1.0f};
        for (auto& m : meshes)
            scene.add_object(Scene<char>::Object(std::move(m),shader));


        
        std::cout << "BEGINNING RENDER ...\n";
        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i=0; i!=100; ++i) {
            scene.render(rasterizer);
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed_time = std::chrono::duration<double>(end_time-start_time).count();
        std::cout << "Total elapsed time: " << elapsed_time << '\n';

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

