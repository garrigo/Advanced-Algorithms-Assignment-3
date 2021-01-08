#include"rasterization.h"
#include"scene.h"
#include"read-obj.h"
using namespace pipeline3D;

#include<iostream>
#include<chrono>


using namespace std;




    struct my_shader{
         char operator ()(const Vertex &v)  {
            return static_cast<char>((v.z-1)*10.0f+0.5f)%10+'0';
        }
    };

    // number of objectes and renders to perform
    constexpr int NOBJECT = 1000;
    constexpr int NRENDER = 100;

    int main() {
        const int w=150;
        const int h=50;

        my_shader shader;
        Rasterizer<char> rasterizer;
        // User sets the number of threads that he wants. If no number is specified, the max hardware number is used
        rasterizer.set_n_thread(5);
        rasterizer.set_perspective_projection(-1,1,-1,1,1,2);

        std::vector<char> screen(w*h,'.');
        rasterizer.set_target(w,h,&screen[0]);


        std::vector<std::array<Vertex,3>> mesh = read_obj("cubeMod.obj");

   
        Scene<char> scene;
        scene.view_={0.5f,0.0f,0.0f,0.7f,0.0f,0.5f,0.0f,0.7f,0.0f,0.0f,0.5f,0.9f,0.0f,0.0f,0.0f,1.0f};

        // Set number of objects that you want to add to the scene (benchmark)
        // for simplicity, we'll just load the same object NOBJECT times in the scene
        
        for (int i = 0; i < NOBJECT; i++)
            scene.add_object(Scene<char>::Object(std::move(mesh),shader));

        std::cout << "Rendering with "<<  rasterizer.get_n_thread() <<" threads.\n";
        auto start = std::chrono::high_resolution_clock::now();

        // Set number of renders to perform (benchmark)
        // again, this is done just to measure the impact of the parallelization
        for (int i= 0; i < NRENDER; i++)
            scene.render(rasterizer);

        auto end = std::chrono::high_resolution_clock::now();
        float total_time = std::chrono::duration<float>(end-start).count();
        std::cout << "Printed "<< NOBJECT << " objects " << NRENDER <<" times, in: " << total_time << " seconds.\n";


        // print out the screen with a frame around it
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

