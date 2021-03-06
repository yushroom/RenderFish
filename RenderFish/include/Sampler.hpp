#pragma once
#include "RenderFish.hpp"
#include "Math.hpp"
#include "Spectrum.hpp"

//class Spectrum;
class RayDifferential;
class Ray;
struct Intersection;
class SurfaceIntegrator;
class VolumeIntegrator;
class Scene;

struct Sample;

class Sampler
{
public:
	const int x_pixel_start, x_pixel_end;
	const int y_pixel_start, y_pixel_end;
	const int samples_per_pixel;
	//const float shutter_open, shutter_close;

public:
	Sampler(int x_start, int x_end, int y_start, int y_end, int samples_per_pixel)
		: x_pixel_start(x_start), x_pixel_end(x_end), y_pixel_start(y_start), y_pixel_end(y_end),
		samples_per_pixel(samples_per_pixel) {
	}

	bool report_results(Sample *samples, const RayDifferential *rays,
		const Spectrum *Ls, const Intersection *isects, int count) {
		return true;
	}
	void compute_sub_window(int num, int count, int *new_x_start,
		int *new_x_end, int * nex_y_start, int *new_y_end) const;

	virtual int maximum_sample_count() = 0;
	virtual int get_more_samples(Sample *sample, RNG &rng) = 0;
	virtual Sampler * get_sub_sampler(int num, int count) = 0;
	virtual int round_size(int size) const = 0;
};


// represents just the sample values that are needed for generating camera rays
struct CameraSample {
	float image_x, image_y;
	//float lens_u, lens_v;
	//float time;
};

// Samplers are used by Samplers to strore a single sample
struct Sample : public CameraSample 
{
public:
	vector<uint32_t> n1D, n2D;
	float **oneD = nullptr;
	float **twoD = nullptr;

public:
	Sample(Sampler *sampler, SurfaceIntegrator *surf, VolumeIntegrator *vol, const Scene *scene);

	~Sample();

	Sample * duplicate(int count) const;

	//virtual int round_size(int size) const = 0;
	
	uint32_t add_1D(uint32_t num) {
		n1D.push_back(num);
		return uint32_t(n1D.size() - 1);
	}
	uint32_t add_2D(uint32_t num) {
		n2D.push_back(num);
		return uint32_t(n2D.size() - 1);
	}

private:
	Sample() {}
	void allocate_sample_memory();
};

