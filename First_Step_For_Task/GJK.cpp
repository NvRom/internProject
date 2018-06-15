#include <math.h>
#include <iostream>
#include <cstdlib>	

#include "gjk.h"

Point Point::negate()
{
	x = -x;
	y = -y;
	z = -z;
	return *this;
}

double Point::dot(Point d)
{
	double dotproduct = x*d.x + y*d.y + z*d.z;
	return dotproduct;
}

Point Point::cross(Point d)
{
	Point crossproduct;

	crossproduct.x = y*d.z - z*d.y;
	crossproduct.y = z*d.x-x*d.z;
	crossproduct.z = x*d.y - y*d.x;

	return crossproduct;
}

void normalize(FbxVector4& normal)
{
	double sum = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= sum;
	normal[1] /= sum;
	normal[2] /= sum;
}

void Point::set(Point d)
{
	x = d.x;
	y = d.y;
	z = d.z;
}

Point Point::operator-(Point rhs)
{
	Point returnPoint;
	returnPoint.x = this->x - rhs.x;
	returnPoint.y = this->y - rhs.y;
	returnPoint.z = this->z - rhs.z;
	return returnPoint;
}

Shape Shape::expansion()
{
	Shape returnShape(this->shape, this->normal);
	FbxVector4 _normal = this->normal;
	for (int i = 0; i < this->_size; ++i)
	{
		returnShape[i].x += EXPANSION * _normal[0];
		returnShape[i].y += EXPANSION * _normal[1];
		returnShape[i].z += EXPANSION * _normal[2];
	}
	return returnShape;
}

Point & Shape::operator[](unsigned index)
{
	if (index < _size)
	{
		return shape[index];
	}
}

Point simplex::getLast(void)
{
	Point return_last;

	int i = 3;
	while (i >= 0)
	{
		if (x[i] != -DBL_MAX)
		{
			break;
		}
		i--;
	}
	return_last.x = x[i];
	return_last.y = y[i];
	return_last.z = z[i];

	return return_last;
}

Point simplex::get(int id)
{
	Point return_point;

	if (id >= 0 && id <= 3)
	{
		return_point.x = x[id];
		return_point.y = y[id];
		return_point.z = z[id];
	}

	return return_point;
}

int simplex::size(void)
{
	int found = 0;

	for (int i = 0; i < 4; i++)
		if (x[i] != -DBL_MAX)
			found++;

	return found;
}

void simplex::del(int id) // id = 1 ... 4
{
	int simplex_elements = 0;

	for (int i = 0; i < 4; i++)
		if (x[i] != -DBL_MAX)
			simplex_elements++;

	if (simplex_elements < 4 || id == 4)
	{
		std::cout << "error -> simplex elements: " << simplex_elements << " | id to remove: " << id << std::endl;
	}
	else
	{
		double cx[2], cy[2], cz[2];
		int c = 0;
		id--; 
		for (int i = 0; i < 4; i++)
		{
			cx[c] = x[i];
			cy[c] = y[i];
			cz[c] = z[i];

			x[i] = -DBL_MAX;
			y[i] = -DBL_MAX;
			z[i] = -DBL_MAX;

			if (i != id)
				c++;
		}

		for (int i = 0; i < 3; i++)
		{
			x[i] = cx[i];
			y[i] = cy[i];
			z[i] = cz[i];
		}
	}
}

void simplex::add(Point simplex_add)
{
	int i = 3;
	while (i >= 0)
	{
		if (x[i] != -DBL_MAX)
		{
			break;
		}

		i--;
	}
	i++;

	x[i] = simplex_add.x;
	y[i] = simplex_add.y;
	z[i] = simplex_add.z;
}


void simplex::set_zero(void)
{
	for (int i = 0; i < 4; i++)
	{
		x[i] = -DBL_MAX;
		y[i] = -DBL_MAX;
		z[i] = -DBL_MAX;
	}
}

double dotProd(double x1, double y1, double z1, double x2, double y2, double z2)
{
	return (x1*x2)+(y1*y2)+(z1*z2);
}

Point tripleCrossProd(Point a, Point b, Point c)
{
	Point tripleReturn;

	tripleReturn.x = b.x*a.dot(c) - c.x*a.dot(b);
	tripleReturn.y = b.y*a.dot(c) - c.y*a.dot(b);
	tripleReturn.z = b.z*a.dot(c) - c.z*a.dot(b);

	return tripleReturn;
}


Point middlePoint(Shape A)
{
	Point returnPoint;
	unsigned _size = A.size();
	for (int i = 0 ; i < _size; i++)
	{
		returnPoint.x += A[i].x;
		returnPoint.y += A[i].y;
		returnPoint.z += A[i].z;
	}
	returnPoint.x = returnPoint.x/ _size;
	returnPoint.y = returnPoint.y/ _size;
	returnPoint.z = returnPoint.z/ _size;

	return returnPoint;

}

Point getFarthestPointInDirection(Shape A, Point direction)
{
	double maxVal = -DBL_MAX;
	int maxIndex = -1;

	double dotp = 0.0;
	unsigned dim_a = A.size();
	for (int i = 0; i < dim_a; i++)
	{
		dotp = dotProd(A[i].x, A[i].y, A[i].z, direction.x, direction.y, direction.z);

		if (dotp > maxVal)
		{
			maxVal = dotp;
			maxIndex = i;
		}
	}
	return A[maxIndex];
}

Point support(Shape A, Shape B, Point direction)
{
	Point p1 = getFarthestPointInDirection(A, direction);
	Point p2 = getFarthestPointInDirection(B, direction.negate());

	return p1-p2;

}

bool containsOrigin(simplex& simplex, Point& d)
{
	Point a = simplex.getLast();
	Point a0 = a;
	a0.negate();

	if (simplex.size() == 4)
	{
		Point p_d = simplex.get(0);
		Point p_c = simplex.get(1);
		Point p_b = simplex.get(2);
		// AC
		Point ac = p_c - a;
		// AB
		Point ab = p_b - a;
		// AD
		Point ad = p_d - a;
		
		// ABC
		Point mmp_abc; // mass middle point of the triangle ABC
		mmp_abc.x = (a.x + p_b.x + p_c.x)/3;
		mmp_abc.y = (a.y + p_b.y + p_c.y)/3;
		mmp_abc.z = (a.z + p_b.z + p_c.z)/3;
		Point ac_c_ab = ac.cross(ab); // ac_c_ab = AC x AB  -> normal of the (triangle) surface
		double v_abc = ac_c_ab.dot(a0);

		// ABD
		Point mmp_abd; // mass middle point of the triangle ABC
		mmp_abd.x = (a.x + p_b.x + p_d.x)/3;
		mmp_abd.y = (a.y + p_b.y + p_d.y)/3;
		mmp_abd.z = (a.z + p_b.z + p_d.z)/3;
		Point ab_c_ad = ab.cross(ad); // ab_c_ad = AB x AD  -> normal of the (triangle) surface
		double v_abd = ab_c_ad.dot(a0);

		//ACD
		Point mmp_acd; // mass middle point of the triangle ABC
		mmp_acd.x = (a.x + p_c.x + p_d.x)/3;
		mmp_acd.y = (a.y + p_c.y + p_d.y)/3;
		mmp_acd.z = (a.z + p_c.z + p_d.z)/3;
		Point ad_c_ac = ad.cross(ac); // ad_c_ac = AD x AC  -> normal of the (triangle) surface
		double v_acd = ad_c_ac.dot(a0);


		int amount_neg = 0;
		int amount_pos = 0;

		if (v_acd > 0)
			amount_pos++;
		else
			amount_neg++;

		if (v_abd > 0)
			amount_pos++;
		else
			amount_neg++;

		if (v_abc > 0)
			amount_pos++;
		else
			amount_neg++;

		if (amount_pos == 3) // origin enclosed in the tetrahedron -> we got a collision
		{
			return true;
		}
		else if (amount_neg == 3) // origin enclosed in the tetrahedron -> we got a collision
		{
			return true;
		}
		else // ditch one point, determine new search direction
		{
			if (amount_neg == 2 && amount_pos == 1)
			{

				if(v_acd > 0) // v_acd < 0 -> new search direction: ad_c_ac, ditch point b
				{
					simplex.del(3); // remove point b of the simplex list
					d.set(ad_c_ac); // set new search direction
				}
				else if (v_abd > 0) // v_abd < 0 ->  new search direction: ab_c_ad, ditch point c
				{
					simplex.del(2); // remove point b of the simplex list
					d.set(ab_c_ad); // set new search direction
				}
				else	// v_abc < 0 -> new search direction: ac_c_ab, ditch point d
				{
					simplex.del(1); // remove point b of the simplex list
					d.set(ac_c_ab); // set new search direction
				}
			}
			else if (amount_neg == 1 && amount_pos == 2)
			{
				if (v_acd < 0) // v_acd < 0 -> new search direction: -ad_c_ac, ditch point b
				{

					ad_c_ac.negate();

					simplex.del(3); // remove point b of the simplex list
					d.set(ad_c_ac); // set new search direction
				}
				else if (v_abd < 0) // v_abd < 0 ->  new search direction: -ab_c_ad, ditch point c
				{
					ab_c_ad.negate();

					simplex.del(2); // remove point b of the simplex list
					d.set(ab_c_ad); // set new search direction
				}
				else	// v_abc < 0 -> new search direction: -ac_c_ab, ditch point d
				{

					ac_c_ab.negate();

					simplex.del(1); // remove point b of the simplex list
					d.set(ac_c_ab); // set new search direction
				}
			}
			else
			{
				std::cout << "error(number pos/neg)" << std::endl;
			}
		}
	}
	else if (simplex.size() == 3) // 3 elements in the simplex
	{
		Point return_sd; // new search direction

		Point b = simplex.get(1);
		Point c = simplex.get(0);

		Point ab = b - a; // b - a

		Point ac = c - a; // c - a

		Point abc = ab.cross(ac); // ABC = AB x AC  -> normal of the (triangle) surface

		Point x;  x.x = 1;  x.y = 0;  x.z = 0;
		Point y;  y.x = 0;  y.y = 1;  y.z = 0;
		Point z = x.cross(y);

		Point abc_c_ac = abc.cross(ac); // ABC x AC
		Point ab_c_abc = ab.cross(abc); // AB x ABC

		if (abc_c_ac.dot(a0) > 0)
		{
			if (ac.dot(a0) > 0)
			{
				return_sd = tripleCrossProd(ac, a0, ac);
			}
			else
			{
				if (ab.dot(a0) > 0)
				{
					return_sd = tripleCrossProd(ab, a0, ab);
				}
				else
				{
					return_sd = a0;
				}
			}
		}
		else
		{
			if (ab_c_abc.dot(a0) > 0)
			{
				if (ab.dot(a0) > 0)
				{
					return_sd = tripleCrossProd(ab, a0, ab);
				}
				else
				{
					return_sd = a0;
				}
			}
			else
			{
				if (abc.dot(a0) > 0)
				{
					return_sd = abc;
				}
				else
				{
					return_sd.x = -abc.x;
					return_sd.y = -abc.y;
					return_sd.z = -abc.z;
				}
			}
		}

		d.set(return_sd);
		return false;
	}
	else if (simplex.size() == 2) // 2 elements in the simplex
	{
		Point b = simplex.get(0); // get the first element
 
		Point ab; // b - a
		ab.x = b.x - a.x;
		ab.y = b.y - a.y;
		ab.z = b.z - a.z;

		Point triple_cp;

		if (ab.dot(a0) > 0)
		{
			triple_cp = tripleCrossProd(ab, a0, ab);
		}
		else
		{
			triple_cp.x = a0.x;
			triple_cp.y = a0.y;
			triple_cp.z = a0.z;
		}

		d.set(triple_cp);
	}
	return false;
};
// check inside simplex evolution (2, 3, 4 points inside the simplex list)

// GJK functions

//main GJK function
bool gjk(Shape A, Shape B) // main gjk function -> point A, point B, dimension of A(=amount of elements in the point A)
{
	simplex simplex; // list of points of the simplex stored here

	Point mma = middlePoint(A);

	Point mmb = middlePoint(B);

	Point d = mma - mmb;

	simplex.set_zero(); // set everything to zero

	simplex.add(support(A, B, d));

	d.negate();

	while (true)
	{
		simplex.add(support(A, B, d));
		if (simplex.getLast().dot(d) <= 0) // no collision -> break
		{
			return false;
		}
		else if (containsOrigin(simplex, d))
		{
			return true;
		}
	}
};
