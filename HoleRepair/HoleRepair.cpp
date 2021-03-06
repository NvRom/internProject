#include "stdafx.h"
#include"HoleRepair.h"

#pragma region 孔洞缝合-预处理

std::vector<int>mappingByHash(const FbxVector4* controlPointsVertex, int controlPointsCount)
{
	std::vector<int>mapping(controlPointsCount, -1);
	std::map<double, int>hashMap;
	const double threshold = 0.0009; //写死了
	for (int i = 0; i < controlPointsCount; ++i)
	{
		FBXSDK_printf("结点编号%d：{%f,%f,%f}\n", i, controlPointsVertex[i][0], controlPointsVertex[i][1], controlPointsVertex[i][2]);
		double hashValue = controlPointsVertex[i][0] + controlPointsVertex[i][1] + controlPointsVertex[i][2];
		while (hashMap.count(hashValue))
		{
			hashValue += HASHVALUESTEPLENGTH;
		}
		hashMap.insert(std::make_pair(controlPointsVertex[i][0] + controlPointsVertex[i][1] + controlPointsVertex[i][2], i));
	}
	auto iter_i = hashMap.begin();
	for (iter_i = hashMap.begin(); iter_i != hashMap.end(); ++iter_i)
	{
		if (mapping[iter_i->second] == -1)//说明该点没有被映射，需要处理
		{
			auto iter_j = iter_i;
			++iter_j;
			if (iter_j == hashMap.end())
			{
				mapping[iter_i->second] = iter_i->second;
				break;
			}
			double subNum = iter_i->first - iter_j->first;
			bool istheNearPoint = false;
			while (subNum * subNum < threshold)
			{
				if (theNearPoint(controlPointsVertex[iter_i->second], controlPointsVertex[iter_j->second]))
				{
					istheNearPoint = true;
					if (mapping[iter_i->second] == -1)
					{
						mapping[iter_i->second] = mapping[iter_j->second] = iter_i->second < iter_j->second ? iter_i->second : iter_j->second;
					}
					else
					{
						mapping[iter_j->second] = mapping[iter_i->second];
					}
				}
				++iter_j;
				if (iter_j == hashMap.end())
				{
					break;
				}
				subNum = iter_i->first - iter_j->first;
			}
			if (!istheNearPoint)
			{
				mapping[iter_i->second] = iter_i->second;
			}
		}
	}
	return mapping;
}

std::vector<int> mappingByBucket(const FbxVector4 * controlPointsVertex, int controlPointsCount)
{
	std::map<int, std::set<int>>bucket;
	double min_x, min_y, min_z, max_x, max_y, max_z, step_x, step_y, step_z;
	min_x = min_y = min_z = 10e5;
	max_x = max_y = max_z = -10e5;
	for (int i = 0; i < controlPointsCount; ++i)
	{
		min_x = min_x < controlPointsVertex[i][0] ? min_x : controlPointsVertex[i][0];
		min_y = min_y < controlPointsVertex[i][1] ? min_y : controlPointsVertex[i][1];
		min_z = min_z < controlPointsVertex[i][2] ? min_z : controlPointsVertex[i][2];
		max_x = max_x > controlPointsVertex[i][0] ? max_x : controlPointsVertex[i][0];
		max_y = max_y > controlPointsVertex[i][1] ? max_y : controlPointsVertex[i][1];
		max_z = max_z > controlPointsVertex[i][2] ? max_z : controlPointsVertex[i][2];
	}
	step_z = (max_z + 0.01 - min_z) / 4;
	step_y = (max_y + 0.01 - min_y) / 4;
	step_x = (max_x + 0.01 - min_x) / 4;
	std::vector<int>mapping(controlPointsCount, -1);
	for (int i = 0; i < controlPointsCount; ++i)
	{
		int label = 16 * floor((controlPointsVertex[i][0] - min_x) / step_x) + 4 * floor((controlPointsVertex[i][1] - min_y) / step_y) + floor((controlPointsVertex[i][2] - min_z) / step_z);
		bool istheNearPoint = false;
		if (bucket.find(label) == bucket.end())
		{
			std::set<int>_set;
			_set.insert(i);
			bucket.insert(std::make_pair(label, _set));
			mapping[i] = i;
		}
		else
		{
			for (auto iter = bucket[label].begin(); iter != bucket[label].end(); ++iter)
			{
				if (theNearPoint(controlPointsVertex[i], controlPointsVertex[mapping[*iter]]))
				{
					istheNearPoint = true;
					mapping[i] = mapping[*iter];
					break;
				}
			}
			if (!istheNearPoint)
			{
				mapping[i] = i;
			}
			bucket[label].insert(i);
		}
	}
	return mapping;
}

std::vector<int> mappingByClassical(const FbxVector4 * controlPointsVertex, int controlPointsCount)
{
	std::vector<int>mapping(controlPointsCount);
	for (int i = 0; i < controlPointsCount; ++i)
	{
		bool istheNearPoint = false;
		for (auto j = 0; j < i; ++j)
		{
			if (theNearPoint(controlPointsVertex[i], controlPointsVertex[j]))
			{
				istheNearPoint = true;
				mapping[i] = mapping[j];
				break;
			}
		}
		if (!istheNearPoint)
		{
			mapping[i] = i;
		}
	}
	return mapping;
}

bool weldingControlPoint(FbxMesh* lMesh, std::vector<int>&mapping)
{
	int polygonCount = lMesh->GetPolygonCount();
	int totalIndex = lMesh->GetPolygonVertexCount();
	int* vertexIndex = lMesh->GetPolygonVertices();
	for (int i = 0; i < totalIndex; ++i)
	{
		vertexIndex[i] = mapping[vertexIndex[i]];
	}
	return true;
}

bool removeBadPolygon(FbxMesh* &lMesh, std::vector<int>&mapping)
{
	int polygonCount = lMesh->GetPolygonCount();
	bool badPolygon = false;
	int sameIndex = 0;
	for (int i = 0, totalIndex = 0; totalIndex < polygonCount; ++i, ++totalIndex)
	{
		int polygonSize = lMesh->GetPolygonSize(i);
		badPolygon = false;
		std::set<int>indexOfPolygonVertex;
		//遍历多边形的所有顶点，写入vector中
		for (int j = 0; j < polygonSize; ++j)
		{
			int _index = lMesh->GetPolygonVertex(i, j);
			if (indexOfPolygonVertex.find(_index) != indexOfPolygonVertex.end())
			{
				badPolygon = true;
				sameIndex = j;
				break;
			}
			else
			{
				indexOfPolygonVertex.insert(_index);
			}
		}
		if (badPolygon)
		{
			lMesh->BeginPolygon();
			for (int j = 0; j < polygonSize; ++j)
			{
				if (j != sameIndex)
				{
					lMesh->AddPolygon(lMesh->GetPolygonVertex(i, j), lMesh->GetTextureUVIndex(i, j));
				}
			}
			lMesh->EndPolygon();
			lMesh->RemovePolygon(i--);
		}
	}
	lMesh->BuildMeshEdgeArray();
	return true;
}

#pragma endregion

#pragma region 孔洞缝合-寻找边界环

bool findAndDeleteReversalEdge(std::set<Edge, edgeSortCriterion>&edges, Edge _edge)
{
	Edge edge(_edge.controlPoint[1], _edge.controlPoint[0]);
	auto it = edges.find(edge);
	if (it != edges.end())
	{
		edges.erase(it);
		return true;
	}
	return false;
}

bool findAndDeleteSameEdge(std::set<Edge, edgeSortCriterion>&edges, Edge& _edge, std::map<Edge, int, edgeSortCriterion>&part)
{
	auto _it = edges.find(_edge);
	if (_it != edges.end())//说明找到了
	{
		if (!part.count(*_it))
		{
			part.insert(std::make_pair(*_it, 1));
		}
		else
		{
			++part[*_it];
		}
		return true;
	}
	return findAndDeleteReversalEdge(edges, _edge);
}

std::set<Edge, edgeSortCriterion> getBoundaryEdges(FbxMesh* &lMesh, std::vector<int>&mapping) 
{
	std::set<Edge, edgeSortCriterion>edges;
	std::map<Edge, int, edgeSortCriterion>part;
	int polygonCount = lMesh->GetPolygonCount();
	FbxVector4* polygons = lMesh->GetControlPoints();
	for (int i = 0; i < polygonCount; ++i)
	{
		bool hasSamePoint = false;
		int sameIndex = 0;
		int polygonSize = lMesh->GetPolygonSize(i);
		std::vector<int>indexOfPolygonVertex(polygonSize);
		//遍历多边形的所有顶点，写入vector中
		for (int j = 0; j < polygonSize; ++j)
		{
			indexOfPolygonVertex[j] = lMesh->GetPolygonVertex(i, j);
			if (i == 581)
			{
				int s = 1;
			}
		}
		//构造set集合，表示未成功匹配的线段，因为两个顶点可以确定一个一条线段
		//线段结构按vertex索引值大小排序，小的在前面
		for (int j = 0; j < polygonSize; ++j)
		{
			int index_j = j % polygonSize;
			int index_j_1 = (j + 1) % polygonSize;
			int edgeStart = mapping[indexOfPolygonVertex[index_j]];
			int edgeEnd = mapping[indexOfPolygonVertex[index_j_1]];
			int UVStart = lMesh->GetTextureUVIndex(i, index_j);
			int UVEnd = lMesh->GetTextureUVIndex(i, index_j_1);
			FbxVector4 normalStart, normalEnd;
			Plane plane;
			lMesh->GetPolygonVertexNormal(i, index_j, normalStart);
			lMesh->GetPolygonVertexNormal(i, index_j_1, normalEnd);
			plane = computerPlane(normalStart, polygons[edgeStart]);
			Edge _edge(edgeStart, edgeEnd, UVStart, UVEnd, normalStart, normalEnd, plane);
			if (!findAndDeleteSameEdge(edges, _edge, part))//说明该线段不在set即合里,添加线段；否则删除掉此线段
			{
				edges.insert(_edge);
			}
		}
	}
	for (auto it = part.begin(); it != part.end(); ++it)
	{
		for (int i = 0; i < it->second; --it->second)
		{
			if (!findAndDeleteReversalEdge(edges, it->first))
			{
				edges.insert(it->first);
			}
		}
	}
	return edges;
}

bool findNextConnectedEdge(std::set<Edge, edgeSortCriterion>&edges, Edge& _edge)
{
	int nextConnectedVertex = _edge.controlPoint[1];
	for (std::set<Edge, edgeSortCriterion>::iterator it = edges.begin(); it != edges.end(); ++it)
	{
		if (it->controlPoint[0] == nextConnectedVertex)
		{
			_edge = *it;
			edges.erase(it);
			return true;
		}
	}
	//说明原始edges中存在多个圈子共享一个点的情况
	return false;
}

void getSingleRing(std::vector<Ring>& rings, std::vector<Edge>& ringCode, std::vector<int>& indexVertex)
{
	int startJ = 0;
	//因为circleCode数组不大,可以每次都判断是否在之前已经访问了当前值
	for (unsigned i = 0; i < ringCode.size(); ++i)
	{
		if (ringCode[i].controlPoint[0] == -1)
		{
			//将之前的全置-1,相当于重置startJ的值
			startJ = i + 1;
		}
		else
		{
			int j;
			for (j = startJ; j < i + 1; ++j)
			{
				if (indexVertex[j] == indexVertex[i + 1])
				{
					Ring _ring;
					for (; j < i + 1; ++j)
					{
						if (indexVertex[j + 1] != -1)
						{
							_ring.insert(ringCode[j]);
							indexVertex[j + 1] = -1;
						}
					}
					if (_ring._size <= 10)
					{
						rings.push_back(_ring);
					}
					break;
				}
			}
		}
	}
}

std::vector<Ring> getRingsFromDirectedGraph(const std::map<int, std::set<Edge, edgeSortCriterion>>&edgeRings)
{
	std::vector<Ring>rings;
	for (std::map<int, std::set<Edge, edgeSortCriterion>>::const_iterator iter = edgeRings.begin(); iter != edgeRings.end(); ++iter)
	{
		std::set<Edge, edgeSortCriterion>edges = iter->second;
		std::vector<int>indexVertex;
		std::vector<Edge>ringCode;
		Edge _edge = *edges.begin();
		ringCode.push_back(_edge);
		indexVertex.push_back(_edge.controlPoint[0]);
		indexVertex.push_back(_edge.controlPoint[1]);
		edges.erase(edges.begin());
		while (!edges.empty())
		{
			Edge flag(-1, -1);
			if (!findNextConnectedEdge(edges, _edge))//说明剩下的edges是原来edges的子集，且拥有独立的圈子（环）
			{
				ringCode.push_back(flag);//解析的时候遇到-1，需要将前面的全部归零
				_edge = *edges.begin();
				ringCode.push_back(_edge);
				indexVertex.push_back(_edge.controlPoint[0]);
				indexVertex.push_back(_edge.controlPoint[1]);
				edges.erase(edges.begin());
			}
			else
			{
				ringCode.push_back(_edge);
				indexVertex.push_back(_edge.controlPoint[1]);
			}
		}
		//解析circle，得到多个圈子（环）
		getSingleRing(rings, ringCode, indexVertex);
	}
	return rings;
}

std::vector<Ring> processEdges(const std::set<Edge, edgeSortCriterion>&edgeSet, int vertexCount)
{
	//找到同属于一个域的线段（or点）
	int label = 0;
	//-1表示该点没有分配lable
	//将edge也按label分好。采用这么做的原因：
	//如果只记录点，当一个圈子里存在多个点的时候，需要判断这些点是由原图中的哪些edge组成的；先不说是否能正确推导出edge，推导的过程也十分繁琐
	std::vector<int>id(vertexCount, -1);
	std::map<int, std::set<Edge, edgeSortCriterion>>edgeCircles;
	for (std::set<Edge, edgeSortCriterion>::const_iterator iter = edgeSet.begin(); iter != edgeSet.end(); ++iter)
	{
		//p->q的连接
		int p = iter->controlPoint[0], q = iter->controlPoint[1];
		if (id[p] == -1 && id[q] == -1)
		{
			id[p] = id[q] = ++label;
			std::set<Edge, edgeSortCriterion>_set;
			_set.insert(*iter);
			edgeCircles.insert(std::make_pair(label, _set));
		}
		else if (id[p] * id[q] < 0)//说明一个属于圈子，另一个不属于圈子，这时要加入圈子中
		{
			if (id[p] > 0)
			{
				id[q] = id[p];
				edgeCircles[id[q]].insert(*iter);
			}
			else//id[q]>0
			{
				id[p] = id[q];
				edgeCircles[id[q]].insert(*iter);
			}
		}
		else if (id[p] != id[q])//说明两个都曾属于圈子，且圈子还不一样，这时要更新圈子
		{
			int label_p = id[p];
			int label_q = id[q];
			edgeCircles[label_p].insert(*iter);
			for (int i = 0; i < vertexCount; ++i)
			{
				if (id[i] == label_q)
				{
					id[i] = label_p;
				}
			}
			//将edgeCircles[label_q]里面的数据全都转移到edgeCircles[label_p]中;然后删除掉label_q的线段
			for (std::set<Edge, edgeSortCriterion> ::iterator it = edgeCircles[label_q].begin(); it != edgeCircles[label_q].end(); ++it)
			{
				edgeCircles[label_p].insert(*it);
			}
			edgeCircles.erase(label_q);
		}
		else//说明两个都属于一个圈子，得到一个闭合的圈子,
		{
			//TO DO
			edgeCircles[id[p]].insert(*iter);
		}

	}
	return getRingsFromDirectedGraph(edgeCircles);
}
#pragma endregion

#pragma region 孔洞缝合-缝合

bool inTheSamePlane(FbxVector4 normal_1, FbxVector4 normal_2)
{
	double dotProduct = dot(normal_1, normal_2);
	double scalarProduct = celVectorLength(normal_1)*celVectorLength(normal_2);
	return dotProduct / scalarProduct > ACOS;
}

std::vector<std::vector<Edge>> splitFeatureRegion(std::vector<Edge>&edges)
{
	std::vector<int>featureLabel(edges.size(), -1);
	std::vector<std::vector<Edge>>featureRegion;
	std::vector<Edge>_regional;
	int label = 0;
	featureLabel[0] = label++;
	_regional.push_back(edges[0]);
	featureRegion.push_back(_regional);
	unsigned _size = edges.size();
	for (unsigned j = 0; j < _size; ++j)
	{
		int j_1 = (j + 1) % _size;
		FbxVector4 edge_1 = edges[j].normal[0], edge_2 = edges[j_1].normal[0];
		if (inTheSamePlane(edge_1, edge_2))
		{
			if (featureLabel[j_1] == -1)
			{
				featureLabel[j_1] = featureLabel[j];
				featureRegion[featureLabel[j]].push_back(edges[j_1]);
			}
			else
			{
				if (featureLabel[j] != featureLabel[j_1])
				{
					for (unsigned i = 0; i < featureRegion[featureLabel[j]].size(); ++i)
					{
						featureRegion[featureLabel[j_1]].insert(featureRegion[featureLabel[j_1]].begin(), featureRegion[featureLabel[j]][i]);
					}
					featureRegion.erase(featureRegion.begin() + featureLabel[j]);
				}
			}
		}
		else
		{
			if (featureLabel[j_1] == -1)
			{
				featureLabel[j_1] = label++;
				std::vector<Edge>_f;
				_f.push_back(edges[j + 1]);
				featureRegion.push_back(_f);
			}
		}
	}
	return featureRegion;
}

std::vector<Plane> getRegionPlanes(std::vector<std::vector<Edge>>&featureRegion)
{
	unsigned featureRegionSize = featureRegion.size();
	std::vector<Plane> planes(featureRegionSize);
	for (unsigned i = 0; i < featureRegionSize; ++i)
	{
		planes[i] = featureRegion[i][0].plane;
	}
	return planes;
}

bool directAddPolygonToMesh(FbxMesh* lMesh, std::vector<Edge>edges)
{
	if (edges.size() < 2)
	{
		return false;
	}
	unsigned edgeCount = edges.size();
	int first = edges[edgeCount - 1].controlPoint[1];
	int first_UV = edges[edgeCount - 1].UVIndex[1];
	int second = edges[edgeCount - 1].controlPoint[0];
	int second_UV = edges[edgeCount - 1].UVIndex[0];
	for (int i = edgeCount - 2; i >= 0; --i)
	{
		lMesh->BeginPolygon();
		lMesh->AddPolygon(first, first_UV);
		lMesh->AddPolygon(second, second_UV);
		second = edges[i].controlPoint[0];//应该需要判断下对否与first相等，但是后续可以用removeBadPolygon来解决
		second_UV = edges[i].UVIndex[0];
		lMesh->AddPolygon(second, second_UV);
		lMesh->EndPolygon();
	}
	return true;
}

bool intersectPlanes(Plane pre, Plane current, Plane post, FbxVector4& p)
{
	FbxVector4 vec_x(pre._normal[0], current._normal[0], post._normal[0]);
	FbxVector4 vec_y(pre._normal[1], current._normal[1], post._normal[1]);
	FbxVector4 vec_z(pre._normal[2], current._normal[2], post._normal[2]);

	FbxVector4 vec_u = cross(vec_y, vec_z);
	double cramer = dot(vec_x, vec_u);
	if (abs(cramer) < INTERSECT_PLANE_EPSILON)//说明没有交于一点
	{
		return false;
	}
	FbxVector4 dis(pre._distance, current._distance, post._distance);
	FbxVector4 vec_v = cross(vec_x, dis);
	p[0] = dot(dis, vec_u) / cramer;
	p[1] = dot(vec_z, vec_v) / cramer;
	p[2] = -dot(vec_y, vec_v) / cramer;
	return true;
}

bool _update(FbxMesh* lMesh, std::vector<Edge>&region, int pointIndex)
{
	if (region.empty())
	{
		return false;
	}
	unsigned regionSize = region.size();
	int pointUVIndex;
	for (unsigned i = 0; i < regionSize; ++i)
	{
		pointUVIndex = region[i].UVIndex[0];
		lMesh->BeginPolygon();
		lMesh->AddPolygon(pointIndex, pointUVIndex);
		lMesh->AddPolygon(region[i].controlPoint[1], region[i].UVIndex[1]);
		lMesh->AddPolygon(region[i].controlPoint[0], region[i].UVIndex[0]);
		lMesh->EndPolygon();
	}
	FbxVector4 normal = region[0].normal[0];
	Plane plane = region[0].plane;
	Edge first(region[0].controlPoint[0], pointIndex, region[0].UVIndex[0], pointUVIndex, normal, normal, plane);
	Edge second(pointIndex, region[regionSize - 1].controlPoint[1], pointUVIndex, region[regionSize - 1].UVIndex[1], normal, normal, plane);
	region.clear();
	region.push_back(first);
	region.push_back(second);
	return true;
}

void updateFeatureRegion(FbxMesh* lMesh, std::vector<std::vector<Edge>>&featureRegion, std::vector<Plane>&planes, int pre, int current, int post, int pointIndex)
{
	_update(lMesh, featureRegion[pre], pointIndex);
	_update(lMesh, featureRegion[current], pointIndex);
	_update(lMesh, featureRegion[post], pointIndex);
	featureRegion[pre].erase(featureRegion[pre].begin() + 1);//删掉第二个线段
	featureRegion[post].erase(featureRegion[post].begin());//删掉第一个线段
	featureRegion.erase(featureRegion.begin() + current);//删掉current部分的特征区域
	planes.erase(planes.begin() + current);
}

void beforeDirectAddFeatureRegionPolygon(FbxMesh* lMesh, std::vector<Edge>&edges)
{
	if ((edges.size() <= 1))
	{
		return;
	}
	directAddPolygonToMesh(lMesh, edges);
	unsigned _size = edges.size();
	FbxVector4 normal = edges[0].normal[0];
	Plane plane = edges[0].plane;
	Edge _edge(edges[0].controlPoint[0], edges[_size - 1].controlPoint[1], edges[0].UVIndex[0], edges[_size - 1].UVIndex[1], normal, normal, plane);
	edges.clear();
	edges.push_back(_edge);
}

bool directAddFeatureRegionPolygon(FbxMesh* lMesh, std::vector<std::vector<Edge>>&featureRegion)
{
	unsigned featureRegionSize = featureRegion.size();
	for (unsigned i = 0; i < featureRegionSize; ++i)
	{
		if (featureRegion[i].size() > 1)
		{
			beforeDirectAddFeatureRegionPolygon(lMesh, featureRegion[i]);
		}
	}
	std::vector<Edge>_data;
	featureRegionSize = featureRegion.size();
	for (int i = 0; i < featureRegionSize; ++i)
	{
		for (int j = 0; j < featureRegion[i].size(); ++j)
		{
			_data.push_back(featureRegion[i][j]);
		}
	}
	return directAddPolygonToMesh(lMesh, _data);
}

bool isCandidatePoint(FbxVector4 point, std::vector<Edge>&edges, FbxVector4* vertexs)
{
	double distance_point = celDistanceBetweenTwoPoints(point, vertexs[edges[0].controlPoint[0]]);
	double distance_edges = celDistanceBetweenTwoPoints(vertexs[edges[0].controlPoint[0]], vertexs[edges[edges.size() - 1].controlPoint[1]]);
	return distance_point < 4 * distance_edges;
}

bool addPolygon(FbxMesh* lMesh, std::vector<std::vector<Edge>>&featureRegion, std::vector<Plane>&planes)
{
	unsigned regionSize = featureRegion.size();
	if (regionSize == 1)
	{
		return directAddPolygonToMesh(lMesh, featureRegion[0]);
	}
	if (regionSize == 2)
	{
		directAddPolygonToMesh(lMesh, featureRegion[0]);
		directAddPolygonToMesh(lMesh, featureRegion[1]);
	}
	//处理三种以上区域，前方高能
	bool intersectPlaneAtPoint = false;
	while (featureRegion.size() > 2)
	{
		intersectPlaneAtPoint = false;
		regionSize = featureRegion.size();
		for (int i = 0; i < regionSize; ++i)
		{
			int pre = (i - 1 + regionSize) % regionSize;
			int post = (i + 1 + regionSize) % regionSize;
			FbxVector4 p;
			if (intersectPlanes(planes[pre], planes[i], planes[post], p))
			{
				FbxVector4* vertexs = lMesh->GetControlPoints();
				if (isCandidatePoint(p, featureRegion[i], vertexs))
				{
					intersectPlaneAtPoint = true;
					int controlPointIndex = lMesh->GetControlPointsCount();
					lMesh->SetControlPointAt(p, controlPointIndex);
					updateFeatureRegion(lMesh, featureRegion, planes, pre, i, post, controlPointIndex);
					break;
				}
			}
		}
		//说明featureRegion中相邻的三个面不共点
		if (!intersectPlaneAtPoint)
		{
			return directAddFeatureRegionPolygon(lMesh, featureRegion);
		}
	}
	return true;
}

void repairHole(FbxMesh* lMesh, std::vector<Ring> rings)
{
	unsigned circleSize = rings.size();
	for (unsigned i = 0; i < circleSize; ++i)
	{
		std::vector<std::vector<Edge>>featureRegion = splitFeatureRegion(rings[i]._mData);
		if (rings[i]._size == 3 && featureRegion.size() < 3)//直接添加
		{
			directAddPolygonToMesh(lMesh, rings[i]._mData);
			continue;
		}
		std::vector<Plane>planes = getRegionPlanes(featureRegion);
		addPolygon(lMesh, featureRegion, planes);
	}
	lMesh->BuildMeshEdgeArray();
	lMesh->RemoveBadPolygons();
}

#pragma endregion

#pragma region 入口函数
void holeRepair(FbxNode* meshNode)
{
	FbxMesh* lMesh = meshNode->GetMesh();
	//当node属于mesh时，才会进行下面的步骤
	if (lMesh == nullptr)
	{
		return;
	}
	lMesh->BuildMeshEdgeArray();
	FbxVector4* controlPointsVertex = lMesh->GetControlPoints();
	//因为发现有些点的距离非常近，可以认为是同一个点，所以需要建立个映射机制，将模型中的距离相近的点在处理时看成是一个点
	//mapping[i]=j表示将节点i映射为j，程序遇到节点i时，都要看成是j
	std::vector<int>mapping = mappingByHash(controlPointsVertex, lMesh->GetControlPointsCount());
	weldingControlPoint(lMesh, mapping);
	//遍历所有的图形，得到未匹配线段集合
	std::set<Edge, edgeSortCriterion>edgeSet = getBoundaryEdges(lMesh, mapping);
	////得到所有满足要求的圈子（环）
	std::vector<Ring> rings = processEdges(edgeSet, lMesh->GetControlPointsCount());
	removeBadPolygon(lMesh, mapping);
	repairHole(lMesh, rings);
}
#pragma endregion