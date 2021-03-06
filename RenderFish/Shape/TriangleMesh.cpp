#include "TriangleMesh.hpp"
#include "ModelIO.hpp"

Triangle::Triangle(const Transform *o2w, const Transform *w2o, bool reverse_orientation, TriangleMesh *m, int n) : Shape(o2w, w2o, reverse_orientation)
{
	mesh = m;
	v = &mesh->vertex_index[3 * n];
	Assert(v[0] < mesh->n_vertices);
	Assert(v[1] < mesh->n_vertices);
	Assert(v[2] < mesh->n_vertices);

	const auto &p1 = mesh->position[v[0]];
	const auto &p2 = mesh->position[v[1]];
	const auto &p3 = mesh->position[v[2]];
	auto e1 = p2 - p1;
	auto e2 = p3 - p1;
	//Vec3 dpdu, dpdv;
	Vec2 uvs[3];
	get_uvs(uvs);

	float du1 = uvs[0].x - uvs[2].x;
	float du2 = uvs[1].x - uvs[2].x;
	float dv1 = uvs[0].y - uvs[2].y;
	float dv2 = uvs[1].y - uvs[2].y;
	auto dp1 = p1 - p3, dp2 = p2 - p3;

	float determinant = du1 * dv2 - dv1 * du2;
	if (determinant == 0.f) {
		coordinate_system(normalize(cross(e2, e1)), &dpdu, &dpdv);
	}
	else {
		float inv_def = 1.f / determinant;
		dpdu = (dv2 * dp1 - dv1 * dp2) * inv_def;
		dpdv = (-du2 * dp1 + du1 * dp2) * inv_def;
	}
}

BBox Triangle::object_bound() const
{
	const auto &p1 = mesh->position[v[0]];
	const auto &p2 = mesh->position[v[1]];
	const auto &p3 = mesh->position[v[2]];
	return combine(BBox((*world_to_object)(p1), (*world_to_object)(p2)), (*world_to_object)(p3));
}

BBox Triangle::world_bound() const
{
	const auto &p1 = mesh->position[v[0]];
	const auto &p2 = mesh->position[v[1]];
	const auto &p3 = mesh->position[v[2]];
	return combine(BBox(p1, p2), p3);
}

bool Triangle::intersect(const Ray &ray, float *t_hit, float *ray_epsilon, DifferentialGeometry *diff_geo) const
{
	const auto &p1 = mesh->position[v[0]];
	const auto &p2 = mesh->position[v[1]];
	const auto &p3 = mesh->position[v[2]];

	auto s = ray.o - p1;
	auto e1 = p2 - p1;
	auto e2 = p3 - p1;
	auto s1 = cross(ray.d, e2);
	float divisor = dot(s1, e1);
	if (divisor == 0)
		return false;
	float inv_divisor = 1.0f / divisor;
	
	float b1 = dot(s1, s) * inv_divisor;
	if (b1 < -0.00001f || b1 > 1.00001f)
	//if (b1 < 0. || b1 > 1.)
		return false;

	auto s2 = cross(s, e1);
	float b2 = dot(ray.d, s2) * inv_divisor;
	if (b2 < -0.00001f || b1 + b2 > 1.00001f)
	//if (b2 < 0. || b1 + b2 > 1.)
		return false;

	float t = dot(e2, s2) * inv_divisor;
	if (t < ray.mint || t > ray.maxt)
		return false;

	//Vec3 dpdu, dpdv;
	Vec2 uvs[3];
	get_uvs(uvs);

	//float du1 = uvs[0].x - uvs[2].x;
	//float du2 = uvs[1].x - uvs[2].x;
	//float dv1 = uvs[0].y - uvs[2].y;
	//float dv2 = uvs[1].y - uvs[2].y;
	//auto dp1 = p1 - p3, dp2 = p2 - p3;

	//float determinant = du1 * dv2 - dv1 * du2;
	//if (determinant == 0.f) {
	//	coordinate_system(normalize(cross(e2, e1)), &dpdu, &dpdv);
	//}
	//else {
	//	float inv_def = 1.f / determinant;
	//	dpdu = ( dv2 * dp1 - dv1 * dp2) * inv_def;
	//	dpdv = (-du2 * dp1 + du1 * dp2) * inv_def;
	//}

	float b0 = 1 - b1 - b2;
	float tu = b0*uvs[0].x + b1*uvs[1].x + b2*uvs[2].x;
	float tv = b0*uvs[0].y + b1*uvs[1].y + b2*uvs[2].y;

	if (mesh->m_alpha_texture) {
		DifferentialGeometry dg_local(ray(t), dpdu, dpdv, Normal(0, 0, 0), Normal(0, 0, 0), tu, tv, this);
		if (mesh->m_alpha_texture->evaluate(dg_local) == 0.f)
			return false; 
	}

	*diff_geo = DifferentialGeometry(ray(t), dpdu, dpdv, Normal(0, 0, 0), Normal(0, 0, 0), tu, tv, this);
	*t_hit = t;
	*ray_epsilon = 1e-3f * t;
	diff_geo->b1 = b1;
	diff_geo->b2 = b2;
	return true;
}

bool Triangle::intersect_p(const Ray & ray) const
{
	const auto &p1 = mesh->position[v[0]];
	const auto &p2 = mesh->position[v[1]];
	const auto &p3 = mesh->position[v[2]];

	auto s = ray.o - p1;
	auto e1 = p2 - p1;
	auto e2 = p3 - p1;
	auto s1 = cross(ray.d, e2);
	float divisor = dot(s1, e1);
	if (divisor == 0)
		return false;
	float inv_divisor = 1.0f / divisor;

	float b1 = dot(s1, s) * inv_divisor;
	if (b1 < 0. || b1 > 1.)
		return false;

	auto s2 = cross(s, e1);
	float b2 = dot(ray.d, s2) * inv_divisor;
	if (b2 < 0.f || b1 + b2 > 1.)
		return false;

	float t = dot(e2, s2) * inv_divisor;
	if (t < ray.mint || t > ray.maxt)
		return false;
	return true;
}

void Triangle::get_uvs(Vec2 uv[3]) const
{
	if (mesh->uv) {
		uv[0] = mesh->uv[v[0]];
		uv[1] = mesh->uv[v[1]];
		uv[2] = mesh->uv[v[2]];
	}
	else {
		uv[0].x = 0.; uv[0].y = 0.;
		uv[1].x = 1.; uv[1].y = 0.;
		uv[2].x = 1.; uv[2].y = 1.;
	}
}

float Triangle::area() const
{
	const auto &p1 = mesh->position[v[0]];
	const auto &p2 = mesh->position[v[1]];
	const auto &p3 = mesh->position[v[2]];
	return 0.5f * cross(p2 - p1, p3 - p1).length();
}

void Triangle::get_shading_geometry(const Transform & obj2world, const DifferentialGeometry & dg, DifferentialGeometry * dg_shading) const
{
	if (!mesh->normal && !mesh->tangent) {
		*dg_shading = dg;
		return;
	}

	float b1 = dg.b1;
	float b2 = dg.b2;
	float b0 = 1 - b1 - b2;

	Normal ns;
	Vec3 ss, ts;
	if (mesh->normal)
		ns = normalize(obj2world(b0 * mesh->normal[v[0]] +
			b1 * mesh->normal[v[1]] +
			b2 * mesh->normal[v[2]]));
	else
		ns = dg.normal;

	if (mesh->tangent)
		ss = normalize(obj2world(b0 * mesh->tangent[v[0]] +
			b1 * mesh->tangent[v[1]] +
			b2 * mesh->tangent[v[2]]));
	else
		ss = normalize(dg.dpdu);

	ts = cross(ss, ns);
	if (ts.length_squared() > 0.f) {
		ts = normalize(ts);
		ss = cross(ts, ns);
	}
	else
		coordinate_system(Vec3(ns), &ss, &ts);

	Normal dndu, dndv;
	Vec2 uvs[3];
	get_uvs(uvs);

	float du1 = uvs[0].x - uvs[2].x;
	float du2 = uvs[1].x - uvs[2].x;
	float dv1 = uvs[0].y - uvs[2].y;
	float dv2 = uvs[1].y - uvs[2].y;
	const Normal& n1 = mesh->normal[v[0]];
	const Normal& n2 = mesh->normal[v[1]];
	const Normal& n3 = mesh->normal[v[2]];
	auto dn1 = n1 - n3, dn2 = n2 - n3;

	float determinant = du1 * dv2 - dv1 * du2;
	if (determinant == 0.f) {
		coordinate_system(normalize(cross(n3-n1, n2-n1)), &dndu, &dndv);
	}
	else {
		float inv_def = 1.f / determinant;
		dndu = (dv2 * dn1 - dv1 * dn2) * inv_def;
		dndv = (-du2 * dn1 + du1 * dn2) * inv_def;
	}

	*dg_shading = DifferentialGeometry(dg.p, ss, ts,
		(*object_to_world)(dndu), (*object_to_world)(dndv),
		dg.u, dg.v, dg.shape);
}

TriangleMesh::TriangleMesh(const Transform * o2w, const Transform * w2o, bool reverse_orientation, 
	int nt, int nv, const int * vi, const Point * P, const Normal * normal, const Vec3 * tangent, 
	const Vec2 * uv, const Reference<Texture<float>>& alpha_texture)
	: Shape(o2w, w2o, reverse_orientation), m_alpha_texture(alpha_texture)
{
	_set_data(nt, nv, vi, P, normal, tangent, uv);
}

TriangleMesh::TriangleMesh(const Transform *o2w, const Transform * w2o, bool reverse_orientation, 
	const string& mesh_file_path, const Reference<Texture<float> > &alpha_texture) 
	: Shape(o2w, w2o, reverse_orientation), m_alpha_texture(alpha_texture)
{
	ModelIO::load(mesh_file_path, this);
}

void TriangleMesh::_set_data(int nt, int nv, const int* vi, const Point *P, const Normal *normal, 
	const Vec3 *tangent, const Vec2 *uv)
{
	//warning("TriangleMesh::_set_data\n");
	n_triangles = nt;
	n_vertices = nv;
	vertex_index = new int[3 * nt];
	memcpy(vertex_index, (void *)vi, 3 * nt * sizeof(int));
	if (normal != nullptr) {
		this->normal = new Normal[nv];
		memcpy(this->normal, normal, nv * sizeof(Normal));
	}
	if (tangent != nullptr) {
		this->tangent = new Vec3[nv];
		memcpy(this->tangent, tangent, nv * sizeof(Vec3));
	}
	if (uv != nullptr) {
		this->uv = new Vec2[nv];
		memcpy(this->uv, uv, nv * sizeof(Vec2));
	}
	position = new Point[n_vertices];
	for (int i = 0; i < n_vertices; ++i) {
		position[i] = (*object_to_world)(P[i]);
	}
}
