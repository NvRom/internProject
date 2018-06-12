#pragma once

#pragma region �׶����-Ԥ����
//�ٽ����ж�
//����hashӳ��ķ�ʽ�����������ٶ�
std::vector<int>mappingByHash(const FbxVector4* controlPointsVertex, int controlPointsCount);

//�ٽ����ж�
//����Bucket��ʽ�������򻮷�Ϊ4*4*4��Bucket�������ٶȲ��Ǻܿ�
std::vector<int>mappingByBucket(const FbxVector4* controlPointsVertex, int controlPointsCount);

//�ٽ����ж�
//��������ѭ���������е�<�㣬��>�ԣ��������
std::vector<int>mappingByClassical(const FbxVector4* controlPointsVertex, int controlPointsCount);

//���㺸�ӣ������ڵĶ��㺸�ӵ�һ��
//mapping:��¼�ٽ������Ϣ��mapping[i]=j����ʾ��i�����ӵ���j�ϣ�������iʱҪ��Ϊ������j
bool weldingControlPoint(FbxMesh* lMesh, std::vector<int>&mapping);

//��Ҫ��ȥһЩ����polygon��polygon�����ظ��Ķ���
bool removeBadPolygon(FbxMesh* &lMesh, std::vector<int>&mapping);
#pragma endregion

#pragma region �׶����-Ѱ�ұ߽绷
//�ú����ǲ���edges���Ƿ���ڸ�_edge�����෴��������ͬ�ı�
//edges������ߵļ���
//_edge�������
bool findAndDeleteReversalEdge(std::set<Edge, edgeSortCriterion>&edges, Edge _edge);

//�ú����ǲ���edges���Ƿ���ڸ�_edge������ͬ��������ͬ�ıߡ����߼������溯�����
bool findAndDeleteSameEdge(std::set<Edge, edgeSortCriterion>&edges, Edge& _edge, std::map<Edge, int, edgeSortCriterion>&part);

//�õ�δƥ���edge����Щedgeֻ��һ���湲��,���߽��
//mapping:��¼�ٽ������Ϣ��mapping[i]=j����ʾ��i�����ӵ���j�ϣ�������iʱҪ��Ϊ������j
std::set<Edge, edgeSortCriterion> getBoundaryEdges(FbxMesh* &lMesh, std::vector<int>&mapping);

//�ҵ���_edge�����ı�
bool findNextConnectedEdge(std::set<Edge, edgeSortCriterion>&edges, Edge& _edge);

//���һ����ͨ���򣬼���õ��߽绷�洢��Rings
//RingCode:����ߵ�����
//indexVertex:�����ʹ�������-1��ʾ�ö��㱻ɾ����
void getSingleRing(std::vector<Ring>&rings, std::vector<Edge>&ringCode, std::vector<int>&indexVertex);

//��ÿһ������ͼ������DFS�õ�����ߵ����������Ȼ�����getSingleRing�����õ��߽绷
//edgeRings:�洢��ͬ��ͨ����ı߽��
std::vector<Ring> getRingsFromDirectedGraph(const std::map<int, std::set<Edge, edgeSortCriterion>>&edgeRings);

//�����߶μ��ϣ��Ƚ��߽�߻���Ϊ�����ͨ����Ȼ�����getRingsFromDirectedGraph�����õ��߽绷
std::vector<Ring> processEdges(const std::set<Edge, edgeSortCriterion>&edgeSet, int vertexCount);
#pragma endregion

#pragma region �׶����-���
//��������ƽ��ķ����������������ǣ����ݼ���ֵ�ж��Ƿ�����ͬһ��������
bool inTheSamePlane(FbxVector4 normal_1, FbxVector4 normal_2);

//ͨ�����ú���inTheSamePlane�ж��Ƿ�����ͬһ�������򣬽��߽��滮��Ϊ��ͬ����������
std::vector<std::vector<Edge>> splitFeatureRegion(std::vector<Edge>&edges);

//�õ�������������ı��ʽ
std::vector<Plane> getRegionPlanes(std::vector<std::vector<Edge>>&featureRegion);

//����������ıߣ�ֱ�����������
bool directAddPolygonToMesh(FbxMesh* lMesh, std::vector<Edge>edges);

//�ж��������Ƿ���һ��,������һ�㣬���㱣����p�в�����true�����򷵻�false
bool intersectPlanes(Plane pre, Plane current, Plane post, FbxVector4& p);

//��������Σ��������������б���Ϣ
bool _update(FbxMesh* lMesh, std::vector<Edge>&region, int pointIndex);

//�ҵ�������󣬵���_update����������������
void updateFeatureRegion(FbxMesh* lMesh, std::vector<std::vector<Edge>>&featureRegion, std::vector<Plane>&planes, int pre, int current, int post, int pointIndex);

//��ִ��directAddFeatureRegionPolygon֮ǰ����Ҫ��һЩԤ�������
void beforeDirectAddFeatureRegionPolygon(FbxMesh* lMesh, std::vector<Edge>&edges);

//��3������������棬���Ǹ��������������ֲ��ཻ��һ�㣬�����������ʱ�����ô˺���
bool directAddFeatureRegionPolygon(FbxMesh* lMesh, std::vector<std::vector<Edge>>&featureRegion);

//�ҵ��������ǵ㣬�ж���������ǵ��Ƿ���������Ҫ��
//point�������ǵ�
bool isCandidatePoint(FbxVector4 point, std::vector<Edge>&edges, FbxVector4* vertexs);

//���������������֮��Ӧ������ʽ����mesh�����������Ƭ
bool addPolygon(FbxMesh* lMesh, std::vector<std::vector<Edge>>&featureRegion, std::vector<Plane>&planes);

//�׶��޸�
//rings���߽绷
void repairHole(FbxMesh* lMesh, std::vector<Ring> rings);
#pragma endregion

#pragma region ������
//��ں���
void holeRepair(FbxNode* meshNode);
#pragma endregion