#include <iostream>
#include <complex>
#include <SFML/Graphics.hpp>
#include <omp.h>

const int width = 800;
const int height = 800;
const std::complex<double> I(0., 1.);
const std::complex<float> If(0.f, 1.f);

struct rgb{
    int r=0, g=0, b=0;

    void set(int r_, int g_, int b_){
        r = r_; g = g_; b = b_;
    }
};

template <class T>
class ColorMap{
    private:
        double max, min;

    public:
        ColorMap() : max(255), min(0){};

        ColorMap(T max_, T min_){
            set_range(max, min);
        }
        void set_range(T max_, T min_){
            max = double(max_);
            min = double(min_);
        }

        rgb get_color(T value){
            rgb color;
            double x = 32.*(double(value)-min)/(max-min);
            double m = 1./12., c = 2./3.;
            if(x<=4){
                color.set(0, 0, int(255.*(m*x+c)));
            }
            else if(x<=12){
                color.set(0, int(255.*m*(x-4.)), 255);
            }
            else if(x<=20){
                color.set(int(255.*m*(x-12.)), 255, int(255.*(1.-m*(x-12))));
            }
            else if(x<=28){
                color.set(255, int(255.*(1.-m*(x-20))), 0);
            }
            else if(x<=32){
                color.set(int(255.*(1.-m*(x-28))), 0, 0);
            }
            else{
                std::cout << "ERROR: value out of range" << std::endl;
                exit(EXIT_FAILURE);
            }
            return color;
        }
};

class DragBox : public sf::Drawable, public sf::Transformable{
    private:
        sf::Color color;
        sf::VertexArray rect;
    
    public:
        DragBox(sf::Color color_) : color(color_)
        {
            rect.setPrimitiveType(sf::LineStrip);
            rect.resize(5);
        }

        void set(const float *a, const float *b){
            rect[0].position = sf::Vector2f(a[0], a[1]);
            rect[1].position = sf::Vector2f(a[0], b[1]);
            rect[2].position = sf::Vector2f(b[0], b[1]);
            rect[3].position = sf::Vector2f(b[0], a[1]);
            rect[4].position = sf::Vector2f(a[0], a[1]);

            for(int i=0; i<5; i++){
                rect[i].color = sf::Color::White;
            }
        }

    private:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const{
            states.transform *= getTransform();
            states.texture = NULL;
            target.draw(rect, states);
        }
};

class Mandelbrot : public sf::Drawable, public sf::Transformable{
    private:
        sf::VertexArray pixelmap;
        ColorMap<int> cmap;
        int max_iter;
    
    protected:
        int get_convergence(std::complex<double> c){
            int iter=0;
            std::complex<double> z(0., 0.);
            while (iter < max_iter && std::abs(z) < 2.){
                z = std::pow(z, 2.) + c;
                iter++;
            }
            return iter;
        }
    
    public:
        Mandelbrot(int max_iter_) : max_iter(max_iter_){
            pixelmap.setPrimitiveType(sf::Points);
            pixelmap.resize(width*height);
            cmap.set_range(max_iter, 0);
        }

        void set(std::complex<double> max, std::complex<double> min){
            #ifdef _OPENMP
                #pragma omp parallel for collapse(2) schedule(dynamic)
                for(int row=0; row<height; row++){
                    for(int col=0; col<width; col++){
                        int index = col + row*width;
                        double cr = min.real() + col*(max.real()-min.real())/(width-1);
                        double ci = min.imag()+row*(max.imag()-min.imag())/(height-1);
                        std::complex<double> c(cr, ci);
                        int conv = get_convergence(c);
                        rgb color = cmap.get_color(conv);
                        pixelmap[index].position = sf::Vector2f(col, row);
                        sf::Color pixelColor(color.r, color.g, color.b);
                        pixelmap[index].color = pixelColor;
                    }
                }
            #else
                for(int row=0; row<height; row++){
                    double ci = min.imag()+row*(max.imag()-min.imag())/(height-1);
                    for(int col=0; col<width; col++){
                        int index = col + row*width;
                        double cr = min.real() + col*(max.real()-min.real())/(width-1);
                        std::complex<double> c(cr, ci);
                        int conv = get_convergence(c);
                        rgb color = cmap.get_color(conv);
                        pixelmap[index].position = sf::Vector2f(col, row);
                        sf::Color pixelColor(color.r, color.g, color.b);
                        pixelmap[index].color = pixelColor;
                    }
                }
            #endif
        }

    private:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const{
            states.transform *= getTransform();
            states.texture = NULL;
            target.draw(pixelmap, states);
        }
};

int main(){
    sf::RenderWindow window(sf::VideoMode(width, height), "SFML Mandelbrot", sf::Style::Titlebar | sf::Style::Close);
    
    bool mouse_pressed = false, zoom_changed = true, init = true;
    float xy_pr[2], xy_rl[2], xy_pos[2];

    DragBox drag_box(sf::Color::White);
    Mandelbrot mandelbrot(256);

    std::complex<float> max(0.7, 1.);
    std::complex<float> min(-1.5, -1.);

    while(window.isOpen()){

        sf::Event event;
        while(window.pollEvent(event)){

            if(event.type==sf::Event::Closed){
                window.close();
            }
            else if(event.type==sf::Event::MouseButtonPressed){
                mouse_pressed = true;
                xy_pr[0] = event.mouseButton.x;
                xy_pr[1] = event.mouseButton.y;
            }
            else if(event.type==sf::Event::MouseButtonReleased){
                mouse_pressed = false;
                xy_rl[0] = event.mouseButton.x;
                xy_rl[1] = event.mouseButton.y;
                if(xy_rl[0]!=xy_pr[0] && xy_rl[1]!=xy_pr[1]){
                    zoom_changed = true;
                    std::complex<float> range = max-min;
                    float a_real = min.real() + xy_pr[0]*range.real()/float(width);
                    float a_imag = min.imag() + xy_pr[1]*range.imag()/float(height);
                    float b_real = min.real() + xy_rl[0]*range.real()/float(width);
                    float b_imag = min.imag() + xy_rl[1]*range.imag()/float(height);
                    max = (a_real > b_real) ? a_real : b_real;
                    min = (a_real > b_real) ? b_real : a_real;
                    max += (a_imag > b_imag) ? If*a_imag : If*b_imag;
                    min += (a_imag > b_imag) ? If*b_imag : If*a_imag;
                    std::cout << "New range: " << max << ", " << min << std::endl;
                }
            }
            else if(event.type==sf::Event::MouseMoved){
                xy_pos[0] = event.mouseMove.x;
                xy_pos[1] = event.mouseMove.y;
            }
        }

        if(init == true){
            window.clear(sf::Color::Black);
            window.display();
        } 

        if(mouse_pressed == true) drag_box.set(xy_pr, xy_pos);

        if(zoom_changed == true || init == true){
            mandelbrot.set(max, min);
            zoom_changed = false;
        }

        window.clear(sf::Color::Black);
        window.draw(mandelbrot);
        if(mouse_pressed==true) window.draw(drag_box);
        window.display();

        init = false;
    }
    return 0;
}