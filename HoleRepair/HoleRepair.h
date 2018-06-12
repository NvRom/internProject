#pragma once

#pragma region 孔洞缝合-预处理
//临近点判断
//采用hash映射的方式，具有最快的速度
std::vector<int>mappingByHash(const FbxVector4* controlPointsVertex, int controlPointsCount);

//临近点判断
//采用Bucket方式，将区域划分为4*4*4块Bucket，计算速度不是很快
std::vector<int>mappingByBucket(const FbxVector4* controlPointsVertex, int controlPointsCount);

//临近点判断
//采用两层循环遍历所有的<点，点>对，性能最差
std::vector<int>mappingByClassical(const FbxVector4* controlPointsVertex, int controlPointsCount);

//顶点焊接：将相邻的顶点焊接到一起
//mapping:记录临近点的信息，mapping[i]=j，表示点i被焊接到点j上，操作点i时要视为操作点j
bool weldingControlPoint(FbxMesh* lMesh, std::vector<int>&mapping);

//需要除去一些坏的polygon：polygon出现重复的顶点
bool removeBadPolygon(FbxMesh* &lMesh, std::vector<int>&mapping);
#pragma endregion

#pragma region 孔洞缝合-寻找边界环
//该函数是查找edges中是否存在跟_edge方向相反、顶点相同的边
//edges：有向边的集合
//_edge：有向边
bool findAndDeleteReversalEdge(std::set<Edge, edgeSortCriterion>&edges, Edge _edge);

//该函数是查找edges中是否存在跟_edge方向相同、顶点相同的边。与逻辑与上面函数差不多
bool findAndDeleteSameEdge(std::set<Edge, edgeSortCriterion>&edges, Edge& _edge, std::map<Edge, int, edgeSortCriterion>&part);

//得到未匹配的edge，这些edge只被一个面共享,即边界边
//mapping:记录临近点的信息，mapping[i]=j，表示点i被焊接到点j上，操作点i时要视为操作点j
std::set<Edge, edgeSortCriterion> getBoundaryEdges(FbxMesh* &lMesh, std::vector<int>&mapping);

//找到与_edge相连的边
bool findNextConnectedEdge(std::set<Edge, edgeSortCriterion>&edges, Edge& _edge);

//针对一个连通区域，计算得到边界环存储在Rings
//RingCode:有向边的连接
//indexVertex:顶点的使用情况，-1表示该顶点被删除了
void getSingleRing(std::vector<Ring>&rings, std::vector<Edge>&ringCode, std::vector<int>&indexVertex);

//对每一个有向图，采用DFS得到有向边的连接情况，然后调用getSingleRing函数得到边界环
//edgeRings:存储不同连通区域的边界边
std::vector<Ring> getRingsFromDirectedGraph(const std::map<int, std::set<Edge, edgeSortCriterion>>&edgeRings);

//处理线段集合，先将边界边划分为多个连通区域，然后调用getRingsFromDirectedGraph函数得到边界环
std::vector<Ring> processEdges(const std::set<Edge, edgeSortCriterion>&edgeSet, int vertexCount);
#pragma endregion

#pragma region 孔洞缝合-缝合
//根据两个平面的法向量，计算其二面角，根据计算值判断是否属于同一特征区域
bool inTheSamePlane(FbxVector4 normal_1, FbxVector4 normal_2);

//通过调用函数inTheSamePlane判断是否属于同一特征区域，将边界面划分为不同的特征区域
std::vector<std::vector<Edge>> splitFeatureRegion(std::vector<Edge>&edges);

//得到特征区域中面的表达式
std::vector<Plane> getRegionPlanes(std::vector<std::vector<Edge>>&featureRegion);

//特征区域里的边，直接添加三角形
bool directAddPolygonToMesh(FbxMesh* lMesh, std::vector<Edge>edges);

//判断三个面是否交于一点,若交于一点，交点保存在p中并返回true，否则返回false
bool intersectPlanes(Plane pre, Plane current, Plane post, FbxVector4& p);

//添加三角形，更新特征区域中边信息
bool _update(FbxMesh* lMesh, std::vector<Edge>&region, int pointIndex);

//找到特征点后，调用_update函数更新特征区域
void updateFeatureRegion(FbxMesh* lMesh, std::vector<std::vector<Edge>>&featureRegion, std::vector<Plane>&planes, int pre, int current, int post, int pointIndex);

//在执行directAddFeatureRegionPolygon之前，需要做一些预处理操作
void beforeDirectAddFeatureRegionPolygon(FbxMesh* lMesh, std::vector<Edge>&edges);

//有3个或更多特征面，但是各个相邻特征面又不相交于一点，出现这种情况时，调用此函数
bool directAddFeatureRegionPolygon(FbxMesh* lMesh, std::vector<std::vector<Edge>>&featureRegion);

//找到了特征角点，判断这个特征角点是否是我们需要的
//point：特征角点
bool isCandidatePoint(FbxVector4 point, std::vector<Edge>&edges, FbxVector4* vertexs);

//根据特征区域和与之对应的面表达式，往mesh里添加三角面片
bool addPolygon(FbxMesh* lMesh, std::vector<std::vector<Edge>>&featureRegion, std::vector<Plane>&planes);

//孔洞修复
//rings：边界环
void repairHole(FbxMesh* lMesh, std::vector<Ring> rings);
#pragma endregion

#pragma region 主函数
//入口函数
void holeRepair(FbxNode* meshNode);
#pragma endregion