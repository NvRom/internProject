#include "stdafx.h"


Point Point::negate()
{
	x = -x;
	y = -y;
	z = -z;
	return *this;
}

double Point::dot(Point d)
{
	double dotproduct = x * d.x + y * d.y + z * d.z;
	return dotproduct;
}

Point Point::cross(Point d)
{
	Point crossproduct;

	crossproduct.x = y * d.z - z * d.y;
	crossproduct.y = z * d.x - x * d.z;
	crossproduct.z = x * d.y - y * d.x;

	return crossproduct;
}

void Point::set(Point d)
{
	x = d.x;
	y = d.y;
	z = d.z;
}

bool Point::equal(Point & p)
{
	Point v = *this - p;
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z) < POINTDISTANCE;
}

Point Point::operator-(Point rhs)
{
	Point returnPoint;
	returnPoint.x = this->x - rhs.x;
	returnPoint.y = this->y - rhs.y;
	returnPoint.z = this->z - rhs.z;
	return returnPoint;
}

Point Point::operator+(Point rhs)
{
	Point returnPoint;
	returnPoint.x = this->x + rhs.x;
	returnPoint.y = this->y + rhs.y;
	returnPoint.z = this->z + rhs.z;
	return returnPoint;
}

Shape Shape::expansion()
{
	Shape returnShape(this->_shape, this->normal);
	FbxVector4 _normal = this->normal;
	for (unsigned i = 0; i < this->_size; ++i)
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
		return _shape[index];
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
		double cx[3], cy[3], cz[3];
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

double absoluteValue(Point v)
{
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

void normalize(FbxVector4& normal)
{
	double sum = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= sum;
	normal[1] /= sum;
	normal[2] /= sum;
}

double dotProd(double x1, double y1, double z1, double x2, double y2, double z2)
{
	return (x1*x2) + (y1*y2) + (z1*z2);
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
	for (unsigned i = 0; i < _size; i++)
	{
		returnPoint.x += A[i].x;
		returnPoint.y += A[i].y;
		returnPoint.z += A[i].z;
	}
	returnPoint.x = returnPoint.x / _size;
	returnPoint.y = returnPoint.y / _size;
	returnPoint.z = returnPoint.z / _size;
	return returnPoint;
}

Point getFarthestPointInDirection(Shape A, Point direction)
{
	double maxVal = -DBL_MAX;
	int maxIndex = -1;
	double dotp = 0.0;
	unsigned dim_a = A.size();
	for (unsigned i = 0; i < dim_a; i++)
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

	return p1 - p2;

}

bool containsOrigin(simplex& simplex, Point& d)
{
	Point a = simplex.getLast();
	Point a0 = a;
	a0.negate();
	//size为4时，核心部分
	if (simplex.size() == 4)
	{
		Point p_d = simplex.get(0);
		Point p_c = simplex.get(1);
		Point p_b = simplex.get(2);
		Point ac = p_c - a;
		Point ab = p_b - a;
		Point ad = p_d - a;
		// ABC
		Point ac_c_ab = ac.cross(ab);
		double v_abc = ac_c_ab.dot(a0);
		// ABD
		Point ab_c_ad = ab.cross(ad);
		double v_abd = ab_c_ad.dot(a0);
		//ACD
		Point ad_c_ac = ad.cross(ac);
		double v_acd = ad_c_ac.dot(a0);

		int amount_neg = 0;
		int amount_pos = 0;
		//考虑健壮性，用EPSILON表示容错值
		if (v_acd > PLANETHICNESS)
			amount_pos++;
		else
			amount_neg++;
		if (v_abd > PLANETHICNESS)
			amount_pos++;
		else
			amount_neg++;
		if (v_abc > PLANETHICNESS)
			amount_pos++;
		else
			amount_neg++;
		//当有一个为3时，说明包含原点
		if (amount_pos == 3)
		{
			return true;
		}
		else if (amount_neg == 3)
		{
			return true;
		}
		else
		{
			//根据不同的类型，确定需要在simplex里删除的点以及方向
			//删除策略可见论文以及书本
			if (amount_neg == 2 && amount_pos == 1)
			{

				if (v_acd > PLANETHICNESS)
				{
					simplex.del(3); 
					d.set(ad_c_ac);
				}
				else if (v_abd > PLANETHICNESS)
				{
					simplex.del(2);
					d.set(ab_c_ad);
				}
				else
				{
					simplex.del(1);
					d.set(ac_c_ab);
				}
			}
			else if (amount_neg == 1 && amount_pos == 2)
			{
				if (v_acd < PLANETHICNESS)
				{
					ad_c_ac.negate();
					simplex.del(3);
					d.set(ad_c_ac);
				}
				else if (v_abd < PLANETHICNESS)
				{
					ab_c_ad.negate();
					simplex.del(2);
					d.set(ab_c_ad);
				}
				else
				{
					ac_c_ab.negate();
					simplex.del(1);
					d.set(ac_c_ab);
				}
			}
		}
	}
	//simplex里有三个点时的处理策略
	else if (simplex.size() == 3)
	{
		Point return_sd;
		Point b = simplex.get(1);
		Point c = simplex.get(0);

		Point ab = b - a;
		Point ac = c - a;
		// ABC = AB x AC
		Point abc = ab.cross(ac);
		Point x(1,0,0);
		Point y(0,1,0);
		Point z = x.cross(y);
		// ABC x AC
		Point abc_c_ac = abc.cross(ac);
		// AB x ABC
		Point ab_c_abc = ab.cross(abc);

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
	//simplex里有两个点时的处理策略
	else if (simplex.size() == 2)
	{
		Point b = simplex.get(0);
		Point ab = b - a;
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

bool gjk(Shape A, Shape B)
{
	bool check_failsafe = true;
	bool check = true;
	int i = 0;
	simplex simplex;

	while (check_failsafe)
	{
		Point mma = middlePoint(A);
		Point mmb = middlePoint(B);
		Point d = mma - mmb;
		//迭代10次如还没判断是否包含，则随机生成新的方向。如情况：添加A，删除B；添加B，删除A；不断循环往复
		if (i > 10)
		{
			i = 0;
			check_failsafe = true;
			check = true;
			int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0;
			while (r1 == 0 || r2 == 0 || r3 == 0 || r4 == 0 || r5 == 0 || r6 == 0)
			{
				r1 = rand() % 19 + (-9);
				r2 = rand() % 19 + (-9);
				r3 = rand() % 19 + (-9);
				r4 = rand() % 19 + (-9);
				r5 = rand() % 19 + (-9);
				r6 = rand() % 19 + (-9);
			}
			//随机生成新的方向
			d.x = (double)r1 / (double)r2;
			d.y = (double)r3 / (double)r4;
			d.z = (double)r5 / (double)r6;
		}
		simplex.set_zero();
		simplex.add(support(A, B, d));
		d.negate();
		while (check)
		{
			simplex.add(support(A, B, d));
			if (simplex.size() == 4)
			{
				++i;
			}
			//没有发生碰撞
			if (simplex.getLast().dot(d) <= 0)
			{
				check_failsafe = false;
				check = false;
			}
			else
			{
				//发生了碰撞
				if (containsOrigin(simplex, d))
				{
					check = true;
					check_failsafe = false;
					break;
				}
				else if (i > 10)
				{
					check = false;
				}
			}
		}
	}
	return check;
};