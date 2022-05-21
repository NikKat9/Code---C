#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include<float.h>

#include "mesh.h"
#include "global.h"

int main()
{
    char file[FILE_MAX_LIMIT];
    printf(" \n Enter the FileName \n");
    scanf("%s",file);
    //fgets(file,FILE_MAX_LIMIT,stdin);
    printf("\n %s",file);
    ImportMesh(file);
    //printf("\n %d",totalNumberOfCells);
    //printf("\n %d",totalNumberOfFaces);
    printNodes(nodes, totalNumberOfNodes);    
    printFaces(faces, totalNumberOfFaces);
    printf("\n %d",totalNumberOfFaces);
    printf("\n %d",totalNumberOfCells);
    printf("\n Faces written");
    
    //int cell0 = faces[0].cellid[0];
    //printf("\n %d",cell0);
    //printf("\n Pass");
    //printf("\n %d",cellCV[cell0-1].faceCount);
    //printf(" \n %d ",cellCV[cell0-1].face[0]);
    CellData();
    
    printf("\n DONE");
    printf("\n CellData");
    PrintCellData();
    NodeInCellOrientation();
    
    //WriteMeshToVTK();
    WriteMeshToVTU();
    
    FreeMemory();
}
