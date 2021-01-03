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
        a = 1;
        const int w=150;
        const int h=50;    
        my_shader shader;
        Rasterizer<char> rasterizer;
        std::cout << "Number of worker-threads: " << rasterizer.workers.getMaxWorkers() << "\n";
        rasterizer.set_perspective_projection(-1,1,-1,1,1,2);

        std::vector<char> screen(w*h,'.');
        rasterizer.set_target(w,h,&screen[0]);


        std::vector<std::array<Vertex,3>> mesh = read_obj("cubeMod.obj");
        std::vector<std::array<Vertex,3>> mesh2 = read_obj("cubeMod.obj");
        std::vector<std::array<Vertex,3>> mesh3 = read_obj("cubeMod.obj");
        std::vector<std::array<Vertex,3>> mesh4 = read_obj("cubeMod.obj");
        std::vector<std::array<Vertex,3>> mesh5 = read_obj("cubeMod.obj");
        std::vector<std::array<Vertex,3>> mesh6 = read_obj("cubeMod.obj");


   
        Scene<char> scene;
        scene.view_={0.5f,0.0f,0.0f,0.7f,0.0f,0.5f,0.0f,0.7f,0.0f,0.0f,0.5f,0.9f,0.0f,0.0f,0.0f,1.0f};
        scene.add_object(Scene<char>::Object(std::move(mesh),shader));
        scene.add_object(Scene<char>::Object(std::move(mesh2),shader));
        scene.add_object(Scene<char>::Object(std::move(mesh3),shader));
        scene.add_object(Scene<char>::Object(std::move(mesh4),shader));
        scene.add_object(Scene<char>::Object(std::move(mesh5),shader));
        scene.add_object(Scene<char>::Object(std::move(mesh6),shader));


        

        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i=0; i!=1; ++i) {
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

