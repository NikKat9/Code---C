/* ************************************************************** *\
\* ************************************************************** */
#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED


/* ************************************************************** *\
\* ************************************************************** */
enum
{
    mixed,
    triangular,
    tetrahedral,
    quadrilateral,
    hexahedral,
    pyramid,
    wedge

}elementType;

typedef struct
{
    // for 3-D
    double x,y,z;

} coordinate3D;

typedef struct
{
    int index;

    int type; // BC
    int faceType;

    int nbnodes;//no. of nodes
    int *node; // to store each node
    int *cellid;// storing cell on left and right of face

    double Ar; //area


} cellFace;

typedef struct
{
    int index;

    int type;

    int *neighbourCellId; //neighbour cell id corresponding to every face of the cells

    int nOfNodes;//no. of nodes
    int *node;// to store each node
    int nodeCount;// used for maintaining the check for no. of nodes entered

    int nOfFaces;
    int *face;
    int faceCount;// used for maintaining the check for no. of faces entered

    coordinate3D centroid;
    
    double Vol; //volume


} cell;

//Mesh data
int totalNumberOfNodes;
int totalNumberOfFaces;
int totalNumberOfCells;

int noOfTetrahedralCell;
int noOfWedgeCell;
int noOfPyramidalCell;
int noOfHexahedralCell;

coordinate3D *nodes;
cellFace *faces;
cell *cellCV;

// Mesh reading
int ImportMesh(char *file);

//To free memory
int FreeMemory();

// Flagging the header values for ICEM-mesh
typedef enum
{
COMMENTS_FLAG = 0,    // "(0"
HEADER_FLAG = 1,      // "(1"
DIMENSIONS_FLAG = 2,  // "(2"
NODES_FLAG = 10,      // "(10"
CELLS_FLAG = 12,      // "(12"
FACES_FLAG = 13,      // "(13"
SECTIONBEGIN_FLAG = 21, // "("
SECTIONEND_FLAG = 22, // "))"
ZONES_FLAG = 45,      // "(45"
UNKNOWN_FLAG = 99     // unknown
} SectionFlag;

SectionFlag HeaderFlag(char *line);

void printNodes (coordinate3D *node , int totNodes);

void printFaces(cellFace *face, int totFaces);

int CellData();//Creating cell data
void PrintCellData();
void NodeInCellOrientation();

//Mesh output in vTU format
void WriteMeshToVTU();
#endif // MESH_H_INCLUDED