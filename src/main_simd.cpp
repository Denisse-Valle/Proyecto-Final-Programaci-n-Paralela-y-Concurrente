#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <chrono>
#include <omp.h>
#include <array>

const int WIDTH = 3840;
const int HEIGHT = 2160;
const int MAX_ITER = 1000;

struct Pixel {
    unsigned char r, g, b;
};

std::vector<Pixel> image(WIDTH * HEIGHT);

std::array<int, 256> histogramAtomic = {0};
std::array<int, 256> histogramLocal = {0};

void generateMandelbrot() {

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

                // CAMBIO: Se agrega omp simd para forzar vectorización.
                // El compilador puede usar instrucciones SIMD/NEON del Apple M1.
                // La cláusula reduction evita errores al acumular r, g, b y count.
                #pragma omp simd reduction(+:r,g,b,count)
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

void computeHistogramAtomic() {

    #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(image.size()); i++) {

        int value = image[i].r;

        #pragma omp atomic
        histogramAtomic[value]++;
    }
}

void computeHistogramLocal() {

    #pragma omp parallel
    {
        std::array<int, 256> localHist = {0};

        #pragma omp for
        for (int i = 0; i < static_cast<int>(image.size()); i++) {

            int value = image[i].r;

            localHist[value]++;
        }

        #pragma omp critical
        {
            for (int i = 0; i < 256; i++) {
                histogramLocal[i] += localHist[i];
            }
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

    auto histAtomicStart =
        std::chrono::high_resolution_clock::now();

    computeHistogramAtomic();

    auto histAtomicEnd =
        std::chrono::high_resolution_clock::now();

    double atomicTime =
        std::chrono::duration<double>(
            histAtomicEnd - histAtomicStart
        ).count();

    auto histLocalStart =
        std::chrono::high_resolution_clock::now();

    computeHistogramLocal();

    auto histLocalEnd =
        std::chrono::high_resolution_clock::now();

    double localTime =
        std::chrono::duration<double>(
            histLocalEnd - histLocalStart
        ).count();

    // CAMBIO: Se guarda con nombre diferente para identificar versión SIMD.
    savePPM("images/mandelbrot_simd.ppm");

    auto end =
        std::chrono::high_resolution_clock::now();

    double totalTime =
        std::chrono::duration<double>(end - start)
            .count();

    std::cout
        << "Tiempo total con SIMD: "
        << totalTime
        << " segundos\n";

    std::cout
        << "Hilos utilizados: "
        << omp_get_max_threads()
        << "\n";

    std::cout
        << "Tiempo histogram atomic: "
        << atomicTime
        << " segundos\n";

    std::cout
        << "Tiempo histogram local: "
        << localTime
        << " segundos\n";

    return 0;
}