#ifndef SCENE_H
#define SCENE_H
#pragma once
#include<memory>
#include<utility>
#include"rasterization.h"



namespace pipeline3D {


const std::array<float,16> Identity{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};


template<class target_t>
class Scene
{
public:


    
    Scene(): view_(Identity) {};
    std::array<float,16> view_;
    // std::atomic<unsigned int> object_counter{0};
    class Object{
    public:



        Object(const Object&) = delete;
        Object(Object&&) = default;

        template<class Mesh,  class Shader, class... Textures>
        Object(Mesh &&mesh, Shader&& shader, Textures&&... textures) :
            pimpl(std::make_unique<concrete_Object_impl<Mesh,Shader,Textures...>>(std::forward<Mesh>(mesh), std::forward<Shader>(shader), std::forward<Textures>(textures)...)), world_(Identity) {}

        void render(Rasterizer<target_t>& rasterizer, const std::array<float,16>& view) {pimpl->render(rasterizer,view,world_);}
        void parallel_render(Rasterizer<target_t>& rasterizer, const std::array<float,16>& view) {
            pimpl->render(rasterizer,view,world_);
            // object_counter++;
            rasterizer.workers.removeWorker();
        }
        std::array<float,16> world_;

    private:

        struct Object_impl {
          virtual ~Object_impl() {}
          virtual void render(Rasterizer<target_t>& rasterizer, const std::array<float,16>& view, const std::array<float,16>& world)=0;
        };

        template<class Mesh, class Shader, class... Textures>
        class concrete_Object_impl : public Object_impl {
        public:
            concrete_Object_impl(Mesh &&mesh, Shader &&shader, Textures&&... textures ) :
                mesh_(std::forward<Mesh>(mesh)), shader_(std::forward<Shader>(shader)), textures_(std::forward<Textures>(textures)...) {}

            
            void render_triangle(int count, Rasterizer<target_t>& rasterizer, const std::array<float,16>& view, const std::array<float,16>& world){
                auto v1=mesh_[count][0];
                auto v2=mesh_[count][1];
                auto v3=mesh_[count][2];
                transform(world,v1);
                transform(view,v1);
                transform(world,v2);
                transform(view,v2);
                transform(world,v3);
                transform(view,v3);

                rasterizer.render_vertices(v1,v2,v3, shader_);
                
                rasterizer.workers.removeWorker();
            }

            void render(Rasterizer<target_t>& rasterizer, const std::array<float,16>& view, const std::array<float,16>& world) override {
                if (a==2) {                  
                    std::vector<std::thread> threads;
                    int count = 0;

                        //POSSIBLE THREAD
                        //Triangle level
                    for (const auto& t : mesh_){
                        rasterizer.workers.addWorker();

                        std::thread t_object (&concrete_Object_impl<Mesh, Shader, Textures...>::render_triangle, this, count, std::ref(rasterizer), std::ref(view), std::ref(world));
                        // threads.push_back(std::move(t_object));
                        t_object.detach();
                    
                        count++;
                        // rasterizer.render_vertices(v1,v2,v3, shader_);
                    }

                    // for(auto& t: threads){
                    //     t.join();
                    // }

                }
                else {
                    for(const auto& t : mesh_) {
                        auto v1=t[0];
                        auto v2=t[1];
                        auto v3=t[2];
                        transform(world,v1);
                        transform(view,v1);
                        transform(world,v2);
                        transform(view,v2);
                        transform(world,v3);
                        transform(view,v3);
                        rasterizer.render_vertices(v1,v2,v3, shader_);
                    }
                }

            }



        private:
            Mesh mesh_;
            Shader shader_;
            std::tuple<Textures...> textures_;
        };

        std::unique_ptr<Object_impl> pimpl;
    };

    void add_object(Object&& o) {objects.emplace_back(std::forward<Object>(o));}

    Object& operator[] (size_t i) {return objects[1];}
    size_t size() const {return objects.size();}
    auto begin() {return objects.begin();}
    auto end() {return objects.end();}

    void render(Rasterizer<target_t>& rasterizer) {
        if (a==1){
            std::vector<std::thread> threads;
            for (auto& o : objects) {
                
                //POSSIBLE THREAD
                //Object level
                rasterizer.workers.addWorker();
                std::thread t_object (&Object::parallel_render, &o, std::ref(rasterizer), std::ref(view_));  
                t_object.detach();
                // threads.push_back(std::move(t_object));
            }
            // for (auto& t : threads){
            //     t.join();
            // }
        }
        else{
            for (auto& o : objects){
                o.render(rasterizer, view_);
            }
        }
    }

private:
    std::vector<Object> objects;
    
};



};
#endif // SCENE_H
