#include "Sphere.hpp"
#include "DifferentialGeometry.hpp"

bool Sphere::intersect(const Ray &ray, float *t_hit, float *ray_epsilon, DifferentialGeometry *diff_geo) const
{
	// Transform Ray to object space
	Ray w_ray;
	(*world_to_object)(ray, &w_ray);

	// Compute quadratic sphere coefficients
	float phi;
	Point phit;
	float A = w_ray.d.x * w_ray.d.x + w_ray.d.y * w_ray.d.y + w_ray.d.z * w_ray.d.z;
	float B = 2 * (w_ray.d.x * w_ray.o.x + w_ray.d.y * w_ray.o.y + w_ray.d.z * w_ray.o.z);
	float C = w_ray.o.x*w_ray.o.x + w_ray.o.y*w_ray.o.y + w_ray.o.z*w_ray.o.z - _radius*_radius;

	// Solve quadratic equation for t values
	float t0, t1;
	if (!quadratic(A, B, C, &t0, &t1))
		return false;

	// Compute intersection distance along ray
	if (t0 > w_ray.maxt || t1 < w_ray.mint)
		return false;
	float thit = t0;
	if (t0 < w_ray.mint) {
		thit = t1;
		if (thit > w_ray.maxt) return false;
	}

	// Compute sphere hit position and phi
	phit = w_ray(thit);
	if (phit.x == 0.f && phit.y == 0.f) phit.x = 1e-5f * _radius;
	phi = atan2f(phit.y, phit.x);
	if (phi < 0.) phi += 2.f * M_PI;

	// Test sphere intersection against clipping parameters
	if ((_z_min > -_radius && phit.z < _z_min) ||
		(_z_max <  _radius && phit.z > _z_max) || phi > _phi_max) { // clip t0(t1)
		if (thit == t1) return false;
		if (t1 > w_ray.maxt) return false;
		thit = t1;
		// Compute sphere hit position and phi
		phit = w_ray(thit);
		if (phit.x == 0.f && phit.y == 0.f) phit.x = 1e-5f * _radius;
		phi = atan2f(phit.y, phit.x);
		if (phi < 0.) phi += 2.f * M_PI;
		if ((_z_min > -_radius && phit.z < _z_min) ||
			(_z_max <  _radius && phit.z > _z_max) || phi > _phi_max)	// clip t1
			return false;
	}
	// Find parametric representatio n of sphere hit
	float u = phi / _phi_max;
	float theta = acosf(clamp(phit.z / _radius, -1.f, 1.f));
	float v = (theta - _theta_min) / (_theta_max - _theta_min);

	float z_radius = sqrtf(phit.x * phit.x + phit.y * phit.y);
	float inv_z_radius = 1.f / z_radius;
	float cos_phi = phit.x * inv_z_radius;
	float sin_phi = phit.y * inv_z_radius;
	Vec3 dpdu(-_phi_max * phit.y, _phi_max * phit.x, 0);
	Vec3 dpdv = (_theta_max - _theta_min) *
		Vec3(phit.z * cos_phi, phit.z * sin_phi, _radius * sinf(theta));

	//auto d2Pduu = -_phi_max * _phi_max * Vec3(phit.x, phit.y, 0);
	//auto d2Pduv = (_theta_max - _theta_min) * phit.z * _phi_max * Vec3(-sin_phi, cos_phi, 0.f);
	//auto d2Pdvv = (_theta_max - _theta_min) * (_theta_max - _theta_min) * Vec3(phit.x, phit.y, phit.z);
	//Normal dndu, dndv;
	//calc_dndu_dndv(dpdu, dpdv, d2Pduu, d2Pduv, d2Pdvv, &dndu, &dndv);
	Normal dndu(dpdu);
	Normal dndv(dpdv);

	const auto &o2w = *object_to_world;
	*diff_geo = DifferentialGeometry(o2w(phit), o2w(dpdu), o2w(dpdv),
		o2w(dndu), o2w(dndv), u, v, this);
	*t_hit = thit;
	*ray_epsilon = 5e-4f * *t_hit;
	return true;
}

bool Sphere::intersect_p(const Ray& r) const
{
	// Transform Ray to object space
	Ray ray;
	(*world_to_object)(r, &ray);

	// Compute quadratic sphere coefficients
	float phi;
	Point phit;
	float A = ray.d.x * ray.d.x + ray.d.y * ray.d.y + ray.d.z * ray.d.z;
	float B = 2 * (ray.d.x * ray.o.x + ray.d.y * ray.o.y + ray.d.z * ray.o.z);
	float C = ray.o.x*ray.o.x + ray.o.y*ray.o.y + ray.o.z*ray.o.z - _radius*_radius;

	// Solve quadratic equation for t values
	float t0, t1;
	if (!quadratic(A, B, C, &t0, &t1))
		return false;

	// Compute intersection distance along ray
	if (t0 > ray.maxt || t1 < ray.mint)
		return false;
	float thit = t0;
	if (t0 < ray.mint) {
		thit = t1;
		if (thit > ray.maxt) return false;
	}

	// Compute sphere hit position and phi
	phit = ray(thit);
	if (phit.x == 0.f && phit.y == 0.f) phit.x = 1e-5f * _radius;
	phi = atan2f(phit.y, phit.x);
	if (phi < 0.) phi += 2.f * M_PI;

	// Test sphere intersection against clipping parameters
	if ((_z_min > -_radius && phit.z < _z_min) ||
		(_z_max <  _radius && phit.z > _z_max) || phi > _phi_max) { // clip t0(t1)
		if (thit == t1) return false;
		if (t1 > ray.maxt) return false;
		thit = t1;
		// Compute sphere hit position and phi
		phit = ray(thit);
		if (phit.x == 0.f && phit.y == 0.f) phit.x = 1e-5f * _radius;
		phi = atan2f(phit.y, phit.x);
		if (phi < 0.) phi += 2.f * M_PI;
		if ((_z_min > -_radius && phit.z < _z_min) ||
			(_z_max <  _radius && phit.z > _z_max) || phi > _phi_max)	// clip t1
			return false;
	}

	return true;
}