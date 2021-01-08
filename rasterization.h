#ifndef RASTERIZATION_H
#define RASTERIZATION_H

#include<cmath>
#include<vector>
#include<array>
#include<type_traits>
#include<condition_variable>
#include<thread>
#include<iostream>


namespace pipeline3D {
	

	
	template<class Target_t>
	class Rasterizer {
	public:

		/*
		 * Synchronizer class used to synchronize the threads for the objects
		 * When a new thread is asked to be created, the synchronizer will check that there is still space available (adder)
		 * When a thread has fullfilled its job, it will be removed from the pool and possibly create space for new ones (remover)
		 */
		class Synchronizer{
			private:
				std::condition_variable cv_;
				unsigned short int nThreads_ = std::thread::hardware_concurrency();
				unsigned short int usedThreads_ = 0;
				// flag for synchronization
				bool canAdd = true;
				
				std::mutex mtx_;

			public:
				Synchronizer() = default;

				Synchronizer(const Synchronizer & synch) : nThreads_(synch.nThreads_) {}

				// Set thread number. returns a warning if high number of thread is given
				inline void setNThreads(unsigned short int nThreads){
					// Check if high number of thread is inserted and eventually warn user
					if (nThreads > std::thread::hardware_concurrency())
						std::cout << "WARNING: You're setting more threads than the maximum hardware supported ("<< std::thread::hardware_concurrency() <<"), this may decrease performance."<<std::endl;
					nThreads_=nThreads;
				}

				// Adding a new thread to the pool
				inline void adder(){
					// Lock the mutex and wait that thread spawn is possible
					std::unique_lock<std::mutex> lock (mtx_);
					cv_.wait(lock, [this](){return canAdd;});
				
					// increase current used threads and update the flag
					usedThreads_++;
					canAdd = (nThreads_ > usedThreads_);

				}

				// Removing a thread from the pool
				inline void remover(){
					std::unique_lock<std::mutex> lock (mtx_);
					
					// decrease current used threads
					usedThreads_--;
					canAdd = (nThreads_ > usedThreads_);
					// notify the removal to the next thread and allow the synchronizer to go on
					cv_.notify_one();
				}

				inline unsigned short int getNThreads(){return nThreads_;}

		};

		// Wrapper for setting the desired threads number
		void set_n_thread(short int n){synchro.setNThreads(n);}
		// Wrapper for getting the setted threads number
		unsigned short int get_n_thread(){return synchro.getNThreads();}
	
    	void set_target(int w, int h, Target_t* t) {
        	width=w;
        	height=h;
        	target=t;
        	z_buffer.clear();
                z_buffer.resize(w*h,1.0f);
    	}
	
    	std::vector<Target_t> get_z_buffer() { return std::move(z_buffer); }
	
    	void set_perspective_projection(float left, float right, float top, float bottom, float near, float far) {
        	const float w=right-left;
        	const float h=bottom-top;
        	const float d=far-near;
	
        	//row-major
        	projection_matrix[0] = 2.0f*near/w;
        	projection_matrix[1] = 0;
        	projection_matrix[2] = -(right+left)/w;
        	projection_matrix[3] = 0;
        	projection_matrix[4*1+0] = 0;
        	projection_matrix[4*1+1] = 2.0f*near/h;
        	projection_matrix[4*1+2] = -(bottom+top)/h;
        	projection_matrix[4*1+3] = 0;
        	projection_matrix[4*2+0] = 0;
        	projection_matrix[4*2+1] = 0;
        	projection_matrix[4*2+2] = (far+near)/d;
        	projection_matrix[4*2+3] = -2.0f*far*near/d;
        	projection_matrix[4*3+0] = 0;
        	projection_matrix[4*3+1] = 0;
        	projection_matrix[4*3+2] = 1;
        	projection_matrix[4*3+3] = 0;
    	}
	
    	void set_orthographic_projection(float left, float right, float top, float bottom, float near, float far) {
        	const float w=right-left;
        	const float h=bottom-top;
        	const float d=far-near;
	
        	//row-major
        	projection_matrix[0] = 2.0f/w;
        	projection_matrix[1] = 0;
        	projection_matrix[2] = 0;
        	projection_matrix[3] = -(right+left)/w;
        	projection_matrix[4*1+0] = 0;
        	projection_matrix[4*1+1] = 2.0f/h;
        	projection_matrix[4*1+2] = 0;
        	projection_matrix[4*1+3] = -(bottom+top)/h;
        	projection_matrix[4*2+0] = 0;
        	projection_matrix[4*2+1] = 0;
        	projection_matrix[4*2+2] = 2.0f/d;
        	projection_matrix[4*2+3] = -(far+near)/d;
        	projection_matrix[4*3+0] = 0;
        	projection_matrix[4*3+1] = 0;
        	projection_matrix[4*3+2] = 0;
        	projection_matrix[4*3+3] = 1;
    	}

        template<class Vertex> struct default_interpolator {
            Vertex operator()(const Vertex& v1, const Vertex& v2, float w) const {return interpolate(v1,v2,w);}
        };
        template<class Vertex> struct default_corrector {
            void operator()(Vertex& v) const {perspective_correct(v);}
        };
	


        template<class Triangle, class Shader, class Interpolator=default_interpolator<std::remove_reference_t<decltype(Triangle()[0])>>,
                 class PerspCorrector=default_corrector<std::remove_reference_t<decltype(Triangle()[0])>>>
        void render_triangle(const Triangle& triangle, Shader& shader,
                             Interpolator interpolate=Interpolator(), PerspCorrector perspective_correct=PerspCorrector()) {
            render_vertices(triangle[0], triangle[1], triangle[2], shader, interpolate, perspective_correct);
        }

        template<class Vertex, class Shader, class Interpolator=default_interpolator<Vertex>, class PerspCorrector=default_corrector<Vertex>>
        void render_vertices(const Vertex &V1, const Vertex& V2, const Vertex &V3, Shader& shader,
                             Interpolator interpolate=Interpolator(), PerspCorrector perspective_correct=PerspCorrector()) {
        	Vertex v1=V1;
        	Vertex v2=V2;
        	Vertex v3=V3;
	
	
        	//project view coordinates to ndc;
                //assume Vertex has fields x,y,z
                std::array<float,3> ndc1{v1.x, v1.y, v1.z};
                std::array<float,3> ndc2{v2.x, v2.y, v2.z};
                std::array<float,3> ndc3{v3.x, v3.y, v3.z};
                project(ndc1);
                project(ndc2);
                project(ndc3);

	
        	// at first sort the three vertices by y-coordinate ascending so v1 is the topmost vertice
        	if(ndc1[1] > ndc2[1]) {
            	std::swap(v1,v2);
            	std::swap(ndc1,ndc2);
        	}
        	if(ndc1[1] > ndc3[1]) {
            	std::swap(v1,v3);
            	std::swap(ndc1,ndc3);
        	}
        	if(ndc2[1] > ndc3[1]) {
            	std::swap(v2,v3);
            	std::swap(ndc2,ndc3);
        	}
	
	
        	//pre-divide vertex attributes by depth
        	perspective_correct(v1);
        	perspective_correct(v2);
        	perspective_correct(v3);
	
	
	
        	// convert normalized device coordinates into pixel coordinates
        	const float x1f=ndc2idxf(ndc1[0],width);
                const int x1=static_cast<int>(x1f);
        	const float y1f=ndc2idxf(ndc1[1],height);
        	const int y1=static_cast<int>(y1f);
        	const float x2f=ndc2idxf(ndc2[0],width);
        	const int x2=static_cast<int>(x2f);
        	const float y2f=ndc2idxf(ndc2[1],height);
                const int y2=static_cast<int>(y2f);
        	const float x3f=ndc2idxf(ndc3[0],width);
                const int x3=static_cast<int>(x3f);
        	const float y3f=ndc2idxf(ndc3[1],height);
        	const int y3=static_cast<int>(y3f);
	
        	const float idy12 = 1.0f/(y2f-y1f);
        	const float idy13 = 1.0f/(y3f-y1f);
        	const float idy23 = 1.0f/(y3f-y2f);
	
        	const float m12=(x2f-x1f)*idy12;
        	const float m13=(x3f-x1f)*idy13;
        	const float m23=(x3f-x2f)*idy23;
	
        	const float q12=(x1f*y2f - x2f*y1f)*idy12;
        	const float q13=(x1f*y3f - x3f*y1f)*idy13;
        	const float q23=(x2f*y3f - x3f*y2f)*idy23;
	
	
	
        	bool horizontal12=std::abs(m12)>1.0f;
        	bool horizontal13=std::abs(m13)>1.0f;
        	bool horizontal23=std::abs(m23)>1.0f;
	
                if (y1>=height || y3<0) return;
	
	
        	if (m13>m12) { // v2 is on the left of the line v1-v3
                        int y=std::max(y1,0);
                	float w1f = (y2f-y)*idy12;
                	float w1l = (y3f-y)*idy13;
                	float xf = m12*y+q12;
                	float xl = m13*y+q13;
                	while (y!=y2){
                    	const int first = horizontal12?static_cast<int>(xf+0.5*m12):static_cast<int>(xf);
                    	const int last = horizontal13?static_cast<int>(xl+0.5*m13)+1:static_cast<int>(xl)+1;
	
                    	const float step = 1.0f/(xl-xf);
                    	const float w0 = 1.0f + (xf-first)*step;
	
                        render_scanline(y,first,last,interpolate(v1,v2,w1f),interpolate(v1,v3,w1l),
                                        interpolatef(ndc1[2],ndc2[2],w1f),interpolatef(ndc1[2],ndc3[2],w1l), w0, step,
                                shader, interpolate, perspective_correct);
                    	++y;
                        if (y==height) return;
                    	w1f -= idy12;
                    	w1l -= idy13;
                    	xf += m12;
                    	xl += m13;
                	}
                	if (y2==y3) {
                    	const float step = 1.0f/(xl-xf);
                    	const float w0 = 1.0f + (xf-x2)*step;
                        render_scanline(y,x2,x3,v2,v3,ndc2[2],ndc3[2],w0,step,
                                shader, interpolate, perspective_correct);
                    	return;
                	} else {
                    	if (std::abs(m12)>std::abs(m23)) xf=m23*y+q23;
	
                    	const float step = 1.0f/(xl-xf);
                    	const float w0 = 1.0f + (xf-x2)*step;
                    	const int last = horizontal13?static_cast<int>(xl+0.5*m13)+1:static_cast<int>(xl)+1;
	
                        render_scanline(y,x2,last,v2,interpolate(v1,v3,w1l),ndc2[2],interpolatef(ndc1[2],ndc3[2],w1l),w0,step,
                                shader, interpolate, perspective_correct);
                	}
                	++y;
                        if (y==height) return;
                	w1l -= idy13;
                	float w2f = (y3f-y)*idy23;
                	xf = m23*y+q23;
                	xl += m13;
	
                	while (y!=y3){
                    	const int first = horizontal23?static_cast<int>(xf+0.5*m23):static_cast<int>(xf);
                    	const int last = horizontal13?static_cast<int>(xl+0.5*m13)+1:static_cast<int>(xl)+1;
	
                    	const float step = 1.0f/(xl-xf);
                    	const float w0 = 1.0f + (xf-first)*step;
	
                    	render_scanline(y,first,last,interpolate(v2,v3,w2f),interpolate(v1,v3,w1l),
                                        interpolatef(ndc2[2],ndc3[2],w2f),interpolatef(ndc1[2],ndc3[2],w1l),
                                        w0,step,
                                shader, interpolate, perspective_correct);
                    	++y;
                        if (y==height) return;
                    	w1l -= idy13;
                    	w2f -= idy23;
                    	xf += m23;
                    	xl += m13;
                	}
	
                	const int first = horizontal23?static_cast<int>(xf+0.5*m23):static_cast<int>(xf);
                	const float step = 1.0f/(xl-xf);
                	const float w0 = 1.0f + (xf-first)*step;
                        render_scanline(y,first,x3,interpolate(v2,v3,w2f),v3,interpolatef(ndc2[2],ndc3[2],w2f),ndc3[2],w0,step,
                                shader, interpolate, perspective_correct);
	
        	} else { // v2 is on the right of the line v1-v3
                int y=std::max(y1,0);
            	float w1f = (y3f-y)*idy13;
            	float w1l = (y2f-y)*idy12;
            	float xf = m13*y+q13;
            	float xl = m12*y+q12;
            	while (y!=y2){
                	const int first = horizontal13?static_cast<int>(xf+0.5*m13):static_cast<int>(xf);
                	const int last = horizontal12?static_cast<int>(xl+0.5*m12)+1:static_cast<int>(xl)+1;
	
                	const float step = 1.0f/(xl-xf);
                	const float w0 = 1.0f + (xf-first)*step;
	
                	render_scanline(y,first,last,interpolate(v1,v3,w1f),interpolate(v1,v2,w1l),
                                        interpolatef(ndc1[2],ndc3[2],w1f),interpolatef(ndc1[2],ndc2[2],w1l),
                                w0,step,
                                shader, interpolate, perspective_correct);
                	++y;
                        if (y==height) return;
                	w1f -= idy13;
                	w1l -= idy12;
                	xf += m13;
                	xl += m12;
            	}
            	if (y2==y3) {
                	const float step = 1.0f/(xl-xf);
                	const float w0 = 1.0f + (xf-x3)*step;
	
                        render_scanline(y,x3,x2+1,v3,v2,ndc3[2],ndc2[2],w0,step,
                                shader, interpolate, perspective_correct);
                	return;
            	} else {
                	const int first = horizontal13?static_cast<int>(xf+0.5*m13):static_cast<int>(xf);
                	const float step = 1.0f/(x2+1-xf);
                	const float w0 = 1.0f + (xf-first)*step;
	
                        render_scanline(y,first,x2+1,interpolate(v1,v3,w1f),v2,interpolatef(ndc1[2],ndc3[2],w1f),ndc2[2],w0,step,
                                shader, interpolate, perspective_correct);
            	}
            	++y;
                if (y==height) return;
            	w1f -= idy13;
            	float w2l = (y3f-y)*idy23;
            	xl = m23*y+q23;
            	xf += m13;
	
            	while (y!=y3){
                	const int first = horizontal13?static_cast<int>(xf+0.5*m13):static_cast<int>(xf);
                	const int last = horizontal23?static_cast<int>(xl+0.5*m23)+1:static_cast<int>(xl)+1;
	
                	const float step = 1.0f/(xl-xf);
                	const float w0 = 1.0f + (xf-first)*step;
	
                	render_scanline(y,first,last,interpolate(v1,v3,w1f),interpolate(v2,v3,w2l),
                                        interpolatef(ndc1[2],ndc3[2],w1f),interpolatef(ndc2[2],ndc3[2],w2l),
                                        w0,step,
                                shader, interpolate, perspective_correct);
                	++y;
                        if (y==height) return;
                	w1f -= idy13;
                	w2l -= idy23;
                	xl += m23;
                	xf += m13;
            	}
            	const float step = 1.0f/(xl-xf);
            	const float w0 = 1.0f + (xf-x3)*step;
            	const int last = horizontal23?static_cast<int>(xl+0.5*m23)+1:static_cast<int>(xl)+1;
	
                render_scanline(y,x3,last,v3,interpolate(v2,v3,w2l),ndc3[2],interpolatef(ndc2[2],ndc3[2],w2l),w0,step,
                        shader, interpolate, perspective_correct);
        	}
	
	
	
    	}
	
    	std::array<float,16> projection_matrix;

		// A synchronizer is put in the rasterizer so that the user can set the preferred number of threads in an intuitive way
		// This also comes handy, as a rasterizer is passed to the scene render and so can be easily called
		Synchronizer synchro;
	
	private:
	
    	float ndc2idxf(float ndc, int range) { return (ndc+1.0f)*(range-1)/2.0f; }
	
        void project(std::array<float,3> &ndc) {
                const float x = ndc[0]*projection_matrix[4*0+0] + ndc[1]*projection_matrix[4*0+1] + ndc[2]*projection_matrix[4*0+2] + projection_matrix[4*0+3];
                const float y = ndc[0]*projection_matrix[4*1+0] + ndc[1]*projection_matrix[4*1+1] + ndc[2]*projection_matrix[4*1+2] + projection_matrix[4*1+3];
                const float z = ndc[0]*projection_matrix[4*2+0] + ndc[1]*projection_matrix[4*2+1] + ndc[2]*projection_matrix[4*2+2] + projection_matrix[4*2+3];
                const float w = ndc[0]*projection_matrix[4*3+0] + ndc[1]*projection_matrix[4*3+1] + ndc[2]*projection_matrix[4*3+2] + projection_matrix[4*3+3];
                ndc[0]=x/w;
                ndc[1]=y/w;
                ndc[2]=z/w;
    	}
	
        float interpolatef(float v1, float v2, float w) {
        	return v1*w + v2*(1.0f-w);
    	}

		// render_scanline is called by render_verticies, which is called by the render method in the scene Object
		// this means that we have to manage a critical zone in here (buffers can be overwritten)
        template<class Vertex, class Shader, class Interpolator, class PerspCorrector>
        void render_scanline(int y, int xl, int xr, const Vertex& vl, const Vertex& vr, float ndczl, float ndczr, float w, float step,
                             Shader shader, Interpolator interpolate, PerspCorrector perspective_correct) {
        	constexpr float epsilon = 1.0e-8f;
                if (y<0 || y>=height || xl>xr) return;

        	Vertex p;
        	int x=std::max(xl,0);
        	w += (xl-x)*step;
        	for (; x!=std::min(width,xr+1); ++x) {
				// Lock to avoid the wrong overriding of buffers
				std::unique_lock<std::mutex> lock(mtx_);
                const float ndcz=interpolatef(ndczl,ndczr,w);
            	if ((z_buffer[y*width+x]+epsilon)>=ndcz){
					Vertex p=interpolate(vl,vr,w);
					perspective_correct(p);
					target[y*width+x] = shader(p);
					z_buffer[y*width+x]=ndcz;
					w -= step;
				}         	
        	}
	
    	}
	
    	int width;
    	int height;
	
	
	
    	Target_t* target;
        std::vector<float> z_buffer;

		// mutex used to manage the shared critical resources of the z_buffer and the target
		// if not properly locked, multiple threads could be entering the if at the same time and edit values
		// this could cause some data inconsistency and result in a wrong print
		std::mutex mtx_;

		
	};
	
}//pipeline3D

#endif // RASTERIZATION_H
