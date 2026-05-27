#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <chrono>
#include <omp.h>   // CAMBIO: Librería necesaria para usar OpenMP

const int WIDTH = 3840;
const int HEIGHT = 2160;
const int MAX_ITER = 1000;

struct Pixel {
    unsigned char r, g, b;
};

std::vector<Pixel> image(WIDTH * HEIGHT);

void generateMandelbrot() {

    // CAMBIO: Se paraleliza el ciclo externo.
    // Cada hilo procesa diferentes filas de la imagen.
    #pragma omp parallel for schedule(runtime)
    for (int y = 0; y < HEIGHT; y++) {

        for (int x = 0; x < WIDTH; x++) {

            double zx = 0.0;
            double zy = 0.0;

            double cx = (x - WIDTH / 2.0) * 4.0 / WIDTH;
            double cy = (y - HEIGHT / 2.0) * 4.0 / WIDTH;

            int iter = 0;

            while (zx * zx + zy * zy < 4.0 &&
                   iter < MAX_ITER) {

                double temp = zx * zx - zy * zy + cx;

                zy = 2.0 * zx * zy + cy;
                zx = temp;

                iter++;
            }

            int index = y * WIDTH + x;

            unsigned char color =
                (unsigned char)(255.0 * iter / MAX_ITER);

            image[index] = {color, color, color};
        }
    }
}

void applyBlur() {

    std::vector<Pixel> temp = image;

    int kernelSize = 5;
    int radius = kernelSize / 2;

    // CAMBIO: Se paraleliza el filtro de convolución.
    // Cada hilo procesa diferentes filas de la imagen.
    #pragma omp parallel for schedule(runtime)
    for (int y = radius;
         y < HEIGHT - radius;
         y++) {

        for (int x = radius;
             x < WIDTH - radius;
             x++) {

            int r = 0;
            int g = 0;
            int b = 0;
            int count = 0;

            for (int ky = -radius;
                 ky <= radius;
                 ky++) {

                for (int kx = -radius;
                     kx <= radius;
                     kx++) {

                    Pixel p =
                        temp[(y + ky) * WIDTH + (x + kx)];

                    r += p.r;
                    g += p.g;
                    b += p.b;

                    count++;
                }
            }

            image[y * WIDTH + x] = {
                (unsigned char)(r / count),
                (unsigned char)(g / count),
                (unsigned char)(b / count)
            };
        }
    }
}

void savePPM(const std::string& filename) {

    std::ofstream file(filename, std::ios::binary);

    file << "P6\n"
         << WIDTH
         << " "
         << HEIGHT
         << "\n255\n";

    for (const auto& p : image) {
        file.write((char*)&p, 3);
    }

    file.close();
}

int main() {

    auto start =
        std::chrono::high_resolution_clock::now();

    generateMandelbrot();

    applyBlur();

    savePPM("images/mandelbrot_openmp.ppm");

    auto end =
        std::chrono::high_resolution_clock::now();

    double time =
        std::chrono::duration<double>(end - start)
            .count();

    std::cout
        << "Tiempo OpenMP: "
        << time
        << " segundos\n";

    std::cout
        << "Hilos utilizados: "
        << omp_get_max_threads()
        << "\n";

    return 0;
}