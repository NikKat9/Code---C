/* *************************************************************** *\
\* *************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include"global.h"
#include"mesh.h"

int FreeMemory()
{
    int i;
    for(i=0; i < totalNumberOfFaces; i++)
    {
        {
            free(faces[i].node);
            free(faces[i].cellid);
        }
    }
    free(faces);

    for(i=0; i < totalNumberOfCells; i++)
    {
        free(cellCV[i].node);
        free(cellCV[i].face);
        free(cellCV[i].neighbourCellId);
    }
    free(cellCV);

    free(nodes);

    

    return LOGICAL_TRUE;
}


SectionFlag HeaderFlag(char *line)
{
    if(strncmp(line,"(0",2)==0)
    {
        return COMMENTS_FLAG;
    }
    else if(strncmp(line,"(2",2)==0)
    {
        return DIMENSIONS_FLAG;
    }
    else if(strncmp(line,"(10",3)==0)
    {
        return NODES_FLAG;
    }
    else if(strncmp(line,"(12",3)==0)
    {
        return CELLS_FLAG;
    }
    else if(strncmp(line,"(13",3)==0)
    {
        return FACES_FLAG;
    }
    else if(strncmp(line,"(1",2)==0)
    {
        return HEADER_FLAG;
    }
    else if(strncmp(line,"(45",3)==0)
    {
        return ZONES_FLAG;
    }
    else{
        return UNKNOWN_FLAG;
    }
}


int ImportMesh(char *file)
{
    int i,j;

    //required for getline
    char *str =NULL;
    size_t line_buf_size=0;

    char *p =NULL;
    int eType;

    // number of nodes, cells, faces
    int nNodes=0;
    int nCells=0;
    int nFaces=0;

    //required for dimension line in File
    int dimension1=0;
    int dimension2=0;

    int numberofLines=0; // to get the number of lines in File
    int idBegin=0;
    int idEnd=0;

    int type=0; // used to determine whether the line is giving total number of nodes,elements,faces or list of nodes,element,faces

    int zoneID=0;
    int ND=0;

    int check=0; // used to check whether value is assigned to variable in CELLS_FLAG

    int flag2=0; // using this flag so that the line denoting the number of nodes comes first and nodes description(i.e, x and y position) later. This value set as 1 when zone-id =0
    int zoneCount=0; // required for multiple zone index value calculation
    int faceCount=0;
    int cellCount=0;
    int elementType;
    int tagForFaceSetToComeBeforeCellSet ;

    FILE *fp;
    FILE *fp1;// writing purpose
    FILE *fp3;

    fp = fopen(file,"r");
    fp1= fopen("Output/mesh.txt","w");
    fp3= fopen("Output/facedataTrial.txt","w");

    if (fp == NULL)
    {
        printf(" \n Error 101: FILE NOT FOUND \n");
        printf ("%s\n\n", file);
        exit(LOGICAL_ERROR);
    }

    printf("\n Reading ICEM-mesh %s.msh file",file);

    do
    {
        getline(&str,&line_buf_size,fp);

        if(str == "")
        {
            printf("\n BLANK LINE: IGNORED \n");
            fputs(str,fp1);
        }
        else
        {
            SectionFlag sectionF = HeaderFlag(str);

            switch(sectionF)
            {
                case COMMENTS_FLAG:
                    printf(" \n Comment line \n");
                    fputs(str,fp1);
                    numberofLines++;
                    break;
                case HEADER_FLAG:
                    printf("\n Comment line \n");
                    fputs(str,fp1);
                    numberofLines++;
                    break;
                case DIMENSIONS_FLAG:
                    printf(" \n Dimensions line \n");
                    sscanf(str,"(%d %d)",&dimension1,&dimension2);
                    fputs(str,fp1);
                    if(dimension2 != 2)
                    {
                        printf("Error 102: Deal with 2-D mesh");
                        exit(1);
                    }
                    else{
                        numberofLines++;
                    }
                    break;
                case NODES_FLAG:
                    sscanf(str,"(10 (%lx %lx %lx %d %d)",&zoneID,&idBegin, &idEnd,&type,&ND);
                    if(ND == 3)
                    {
                        printf(" \n Error 102: Error with the file");
                        fputs(str,fp1);
                        exit(0);
                    }
                    if(zoneID == 0)
                    {
                        nNodes= idEnd - idBegin +1;// this represent total_number_of_nodes
                        totalNumberOfNodes = nNodes;
                        nodes= (coordinates *)calloc(nNodes,sizeof(coordinates));
                        flag2++;// for making sure we alot the size to nodes before.
                        fputs(str,fp1);
                    }
                    else if(zoneID > 0)
                    {
                        fputs(str,fp1);
                        printf("\n REading NODE DATA for zone %d ", zoneID);
                        if(flag2 !=1)
                        {
                            printf("\n Error 102: Logical Error");
                            exit(0);
                        }
                        nNodes = idEnd - idBegin +1; // doing show that multiple regin don't possess an issue
                        int start = idBegin - 1;
                        double x, y;
                        j=0;
                        while(j<nNodes)
                        {
                            getline(&str,&line_buf_size,fp);
                            int numChar = sscanf(str,"%lf %lf",&x,&y);
                            if (numChar == 2)
                            {
                                nodes[start+j].x =x;
                                nodes[start+j].y =y;
                                j++;
                            }
                            fputs(str,fp1);
                        }
                        zoneCount += nNodes;
                    }
                    printf("\n REading NODE DATA ends here");
                    break;
                case FACES_FLAG:
                    sscanf(str,"(13 (%lx %lx %lx %d %d)",&zoneID, &idBegin, &idEnd, &type, &elementType);
                    fprintf(fp3,"\n \n Printing face data");
                    fprintf(fp3,"\n ");
                    fprintf(fp3,"\n Index \t Type \t Node1 \t Node2 \t CellId1 \t CellId2");
                    if(zoneID == 0)
                    {
                        nFaces= idEnd - idBegin +1;// this represent total number of faces
                        totalNumberOfFaces = nFaces;
                        faces= (cellFace *)calloc(nFaces,sizeof(cellFace));
                        fputs(str,fp1);
                    }
                    else if(zoneID > 0)
                    {
                        tagForFaceSetToComeBeforeCellSet = 1;
                        fputs(str,fp1);
                        printf("\n REading FACE DATA for zone %d ", zoneID);
                        nFaces = idEnd - idBegin + 1;
                        long int nodeId1,nodeId2,cellId1,cellId2;
                        int typeFace;
                        faceCount = idBegin -1 ;

                        if(elementType == 0)
                        {
                            j=0;
                            while(j<nFaces)
                            {
                            getline(&str,&line_buf_size,fp);
                            int numChar = sscanf(str,"%d %lx %lx %lx %lx",&typeFace,&nodeId1,&nodeId2,&cellId1,&cellId2);
                            if(numChar == 5)
                            {
                                faces[faceCount+j].index = faceCount+j;
                                faces[faceCount+j].type = typeFace;
                                //THIS REPRESENTS A PROPER CASE IN WHICH N0 N1 CR C1 GIVEN
                                faces[faceCount+j].node = (int *)calloc(2,sizeof(int));
                                faces[faceCount+j].cellid = (int *)calloc(2,sizeof(int));
                                faces[faceCount+j].node[0] = nodeId1;
                                faces[faceCount+j].node[1] = nodeId2;
                                faces[faceCount+j].cellid[0] = cellId1;
                                faces[faceCount+j].cellid[1] = cellId2;
                                fprintf(fp3," \n %d \t %d \t %ld \t %ld \t %ld \t %ld", faces[faceCount+j].index , faces[faceCount+j].type , faces[faceCount+j].node[0] , faces[faceCount+j].node[1] , faces[faceCount+j].cellid[0], faces[faceCount+j].cellid[1]);
                                j++;
                            }
                            fputs(str,fp1);
                        }
                        //faceCount += nFaces;                            
                        }
                        else
                        {
                            j=0;
                            while(j<nFaces)
                            {
                                getline(&str,&line_buf_size,fp);
                                int numChar = sscanf(str,"%lx %lx %lx %lx",&nodeId1,&nodeId2,&cellId1,&cellId2);
                                if(numChar == 4)
                                {
                                    faces[faceCount+j].index = faceCount+j;
                                    faces[faceCount+j].type = type;
                                    //THIS REPRESENTS A PROPER CASE IN WHICH N0 N1 CR C1 GIVEN
                                    faces[faceCount+j].node = (int *)calloc(2,sizeof(int));
                                    faces[faceCount+j].cellid = (int *)calloc(2,sizeof(int));
                                    faces[faceCount+j].node[0] = nodeId1;
                                    faces[faceCount+j].node[1] = nodeId2;
                                    faces[faceCount+j].cellid[0] = cellId1;
                                    faces[faceCount+j].cellid[1] = cellId2;
                                    fprintf(fp3," \n %d \t %d \t %ld \t %ld \t %ld \t %ld", faces[faceCount+j].index , faces[faceCount+j].type , faces[faceCount+j].node[0] , faces[faceCount+j].node[1] , faces[faceCount+j].cellid[0], faces[faceCount+j].cellid[1]);
                                    j++;
                                }
                                fputs(str,fp1);
                            }
                        //faceCount += nFaces;
                        }

                    }
                    break;
                case CELLS_FLAG:
                    sscanf(str,"(12 (%lx %lx %lx %d %d)",&zoneID,&idBegin,&idEnd,&type,&elementType);
                    cellCount = idBegin -1;
                    
                    if(zoneID==0)
                    {
                        totalNumberOfCells= idEnd- idBegin +1;
                        cellCV = (cell *)calloc(totalNumberOfCells,sizeof(cell));
                        fputs(str,fp1);
                        printf("\n Number of cells are %d ", totalNumberOfCells);
                    }
                    else
                    {
                        

                        if(elementType == 1)
                        {
                            nCells= idEnd -idBegin +1;
                            noOfTriangularCell += nCells;
                            for(i=0; i< nCells; i++)
                            {
                                cellCV[cellCount + i].index = cellCount +i ;
                                cellCV[cellCount + i].type = 1;
                                cellCV[cellCount + i].nOfNodes = 3;
                                cellCV[cellCount + i].nOfFaces = 3;
                                cellCV[cellCount + i].nodeCount = 0;
                                cellCV[cellCount + i].faceCount = 0;
                                cellCV[cellCount + i].node = (int *)calloc(cellCV[cellCount + i].nOfNodes,sizeof(int));
                                cellCV[cellCount + i].face = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                                cellCV[cellCount + i].neighbourCellId = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                            }
                            //cellCount += nCells; 
                            printf("\n Number of triangular cells are %d in zone %d", noOfTriangularCell, zoneID);
                        }
                        else if(elementType == 3)
                        {
                            nCells = idEnd -idBegin +1;
                            noOfQuadrilateralCell += nCells;
                            for(i=0; i< nCells; i++)
                            {
                                cellCV[cellCount + i].index = cellCount +i ;
                                cellCV[cellCount + i].type = 3;
                                cellCV[cellCount + i].nOfNodes = 4;
                                cellCV[cellCount + i].nOfFaces = 4;
                                cellCV[cellCount + i].nodeCount = 0;
                                cellCV[cellCount + i].faceCount = 0;
                                cellCV[cellCount + i].node = (int *)calloc(cellCV[cellCount + i].nOfNodes,sizeof(int));
                                cellCV[cellCount + i].face = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                                cellCV[cellCount + i].neighbourCellId = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                            }
                            //cellCount += nCells; 
                            printf("\n Number of quadrilateral cells are %d in zone %d ", noOfQuadrilateralCell, zoneID);
                        }
                        else if(elementType == 0)
                        {
                            getline(&str,&line_buf_size,fp);
                            p = strtok(str," ");
                            nCells= idEnd -idBegin +1;
                            int counter =0;
                            

                            while(p != NULL)
                            {
                                check = sscanf(p,"%d",&eType);
                                //printf("%d \n",eType); 
                                if (check == 1)
                                {
                                    if(eType == 1)
                                    {
                                        
                                        cellCV[idBegin -1 + counter].index = idBegin -1 + counter ;
                                        cellCV[idBegin -1 + counter].type = 1;
                                        cellCV[idBegin -1 + counter].nOfNodes = 3;
                                        cellCV[idBegin -1 + counter].nOfFaces = 3;
                                        cellCV[idBegin -1 + counter].nodeCount = 0;
                                        cellCV[idBegin -1 + counter].faceCount = 0;
                                        cellCV[idBegin -1 + counter].node = (int *)calloc(cellCV[idBegin -1 + counter].nOfNodes,sizeof(int));
                                        cellCV[idBegin -1 + counter].face = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                        cellCV[idBegin -1 + counter].neighbourCellId = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                        
                                        
                                    }
                                    if(eType == 3)
                                    {
                                        cellCV[idBegin -1 + counter].index = idBegin -1 + counter;
                                        cellCV[idBegin -1 + counter].type = 3;
                                        cellCV[idBegin -1 + counter].nOfNodes = 4;
                                        cellCV[idBegin -1 + counter].nOfFaces = 4;
                                        cellCV[idBegin -1 + counter].nodeCount = 0;
                                        cellCV[idBegin -1 + counter].faceCount = 0;
                                        cellCV[idBegin -1 + counter].node = (int *)calloc(cellCV[idBegin -1 + counter].nOfNodes,sizeof(int));
                                        cellCV[idBegin -1 + counter].face = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                        cellCV[idBegin -1 + counter].neighbourCellId = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                    }

                                    
                                    counter ++ ;
                                }
                                p = strtok(NULL," ");
                            }
                        }
                        else
                        {
                            int count = 0;

                            if (tagForFaceSetToComeBeforeCellSet == 1)
                            {
                                for( i=0 ; i< totalNumberOfFaces ; i++)
                                {
                                    if(faces[i].cellid[0] == 1 || faces[i].cellid[1] == 1)
                                    {
                                        count++; 
                                    }

                                }
                                printf("\n %d",count);
                            }
                            if(count == 3)
                            {
                                nCells= idEnd -idBegin +1;
                                noOfTriangularCell += nCells;
                                for(i=0; i< nCells; i++)
                                {
                                    cellCV[cellCount + i].index = cellCount +i ;
                                    cellCV[cellCount + i].type = 1;
                                    cellCV[cellCount + i].nOfNodes = 3;
                                    cellCV[cellCount + i].nOfFaces = 3;
                                    cellCV[cellCount + i].nodeCount = 0;
                                    cellCV[cellCount + i].faceCount = 0;
                                    cellCV[cellCount + i].node = (int *)calloc(cellCV[cellCount + i].nOfNodes,sizeof(int));
                                    cellCV[cellCount + i].face = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                                    cellCV[cellCount + i].neighbourCellId = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                                }
                                //cellCount += nCells; 
                                printf("\n Number of triangular cells are %d in zone %d", noOfTriangularCell, zoneID);
                            }
                            if(count !=3)
                            {
                                printf(" \n Dealing with 2D triangular and quadrilateral mesh only"); 
                                exit(0);
                            }

                        }
                    }
                    break;
                default:
                    printf("\n Section not of importance");
                    fputs(str,fp1);
                    break;


            }
        }
        


    }while(!feof (fp));

    fclose(fp);
    fclose(fp1);
    fclose(fp3);
    //bool EndOfFile = false;

}

/* ******************************************************************************************* *\
Processing Cell Data 
1. FindIfValuePresent
2. CellData
3. AngleCalculator
4. swap
5. partition
6. quicksort
7. NodeInCellOrientation
''' 4 to 7 used to make cell nodes in clockwise or anti-clockwise direction'''
'''This is required for visualisation in vtk --> Paraview'''
\* ****************************************************************************************** */

int FindIfValuePresent(int *arr, int valueToCheck , int iterCount)
{
    
    /*
    itercount --> till which index the array needs to be seen --> faceCount basically
    returns a = 1 , if value found
    else a= 0, if value not found
    */
    //int size = sizeof(arr);
    int a=1;
    int i;

    for( i=0; i< iterCount; i++)
    {
        if(arr[i] == valueToCheck)
        {
            return a;
        }
     }
    return (a-1);
}


int CellData()
{


    int i;
    int node0, node1, cell0, cell1, faceIndex;
    for(i=0; i < totalNumberOfFaces; i++)
    {
        node0 = faces[i].node[0];
        node1 =faces[i].node[1];
        cell0 = faces[i].cellid[0];
        cell1 = faces[i].cellid[1];
        faceIndex = faces[i].index;

        if (cell0 > 0 )
        {
            cellCV[cell0 -1].face[cellCV[cell0-1].faceCount] = faceIndex;
            cellCV[cell0 -1].neighbourCellId[cellCV[cell0-1].faceCount] = cell1 -1;
            //printf("\n Pass");
            if ( cellCV[cell0 -1].nodeCount == 0 )
            {
                cellCV[cell0 -1].node[0] = node0 -1;
                cellCV[cell0 -1].node[1] = node1 -1;
                cellCV[cell0 -1].nodeCount = 2;
            }
            else
            {  
                if(FindIfValuePresent(cellCV[cell0 -1].node, node0 -1 , cellCV[cell0 -1].nodeCount ) ==0 && FindIfValuePresent(cellCV[cell0 -1].node, node1 -1 , cellCV[cell0 -1].nodeCount )==0 )
                {
                    // for 2D triangular element above statement always result in FALSE as already 2 nodes data present out of 3
                    cellCV[cell0 -1].node[ cellCV[cell0 -1].nodeCount ] = node0 -1;
                    cellCV[cell0 -1].nodeCount += 1;
                    cellCV[cell0 -1].node[ cellCV[cell0 -1].nodeCount ] = node1 -1;
                    cellCV[cell0 -1].nodeCount += 1;
                    
                }
                else if(FindIfValuePresent(cellCV[cell0 -1].node, node0 -1 , cellCV[cell0 -1].nodeCount ) ==0 && FindIfValuePresent(cellCV[cell0 -1].node, node1 -1  , cellCV[cell0 -1].nodeCount )==1 )
                {
                    cellCV[cell0 -1].node[ cellCV[cell0 -1].nodeCount ] = node0 -1;
                    cellCV[cell0 -1].nodeCount += 1;
                }
                else if(FindIfValuePresent(cellCV[cell0 -1].node, node0 - 1 , cellCV[cell0 -1].nodeCount ) ==1 && FindIfValuePresent(cellCV[cell0 -1].node, node1 - 1 , cellCV[cell0 -1].nodeCount )==0 )
                {
                    cellCV[cell0 -1].node[ cellCV[cell0 -1].nodeCount ] = node1 -1;
                    cellCV[cell0 -1].nodeCount += 1;
                }
            }
            cellCV[cell0 -1].faceCount += 1;

            
        }
        if(cell1 >0)
        {
            cellCV[cell1 -1].face[cellCV[cell1-1].faceCount] = faceIndex;
            cellCV[cell1 -1].neighbourCellId[cellCV[cell1-1].faceCount] = cell0 -1;
            if ( cellCV[cell1 -1].nodeCount == 0 )
            {
                cellCV[cell1 -1].node[0] = node0 -1;
                cellCV[cell1 -1].node[1] = node1 -1;
                cellCV[cell1 -1].nodeCount = 2;
            }
            else
            {  
                if(FindIfValuePresent(cellCV[cell1 -1].node, node0 -1 , cellCV[cell1 -1].nodeCount ) ==0 && FindIfValuePresent(cellCV[cell1 -1].node, node1 - 1 , cellCV[cell1 -1].nodeCount )==0 )
                {
                    // for 2D triangular element above statement always result in FALSE as already 2 nodes data present out of 3
                    cellCV[cell1 -1].node[ cellCV[cell1 -1].nodeCount ] = node0 -1;
                    cellCV[cell1 -1].nodeCount += 1;
                    cellCV[cell1 -1].node[ cellCV[cell1 -1].nodeCount ] = node1 -1;
                    cellCV[cell1 -1].nodeCount += 1;
                    
                }
                else if(FindIfValuePresent(cellCV[cell1 -1].node, node0 -1  , cellCV[cell1 -1].nodeCount ) ==0 && FindIfValuePresent(cellCV[cell1 -1].node, node1 - 1 , cellCV[cell1 -1].nodeCount )==1 )
                {
                    cellCV[cell1 -1].node[ cellCV[cell1 -1].nodeCount ] = node0 -1;
                    cellCV[cell1 -1].nodeCount += 1;
                }
                else if(FindIfValuePresent(cellCV[cell1 -1].node, node0 -1 , cellCV[cell1 -1].nodeCount ) ==1 && FindIfValuePresent(cellCV[cell1 -1].node, node1 - 1 , cellCV[cell1 -1].nodeCount )==0 )
                {
                    cellCV[cell1 -1].node[ cellCV[cell1 -1].nodeCount ] = node1 -1;
                    cellCV[cell1 -1].nodeCount += 1;
                }
            }
            cellCV[cell1 -1].faceCount += 1;
        }
        if (cell1 == 0)
        {
            // when boundary face is encountered
            
        }
        
    }
}
float AngleCalculator(double y, double x)
{
    float a;
    a= atan2(y,x);
    if(a < 0)
    {
        return (2*PI + a);
    }
    return a;
}
// A utility function to swap two elements
void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}
 

int partition (int *arr, coordinates centre, int low, int high)
{
    float pivot = AngleCalculator(nodes[arr[high]].y - centre.y,nodes[arr[high]].x - centre.x); // pivot
    int i = (low - 1); // Index of smaller element and indicates the right position of pivot found so far
 
    for (int j = low; j <= high - 1; j++)
    {
        // If current element is smaller than the pivot
        if (AngleCalculator(nodes[arr[j]].y - centre.y , nodes[arr[j]].x - centre.x ) < pivot)
        {
            i++; // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}
 

void quickSort(int* arr, coordinates centre, int low, int high)
{
    if (low < high)
    {
     
        int pi = partition(arr,centre, low, high);
        quickSort(arr,centre, low, pi - 1);
        quickSort(arr,centre, pi + 1, high);
    }
}

void NodeInCellOrientation()
{
    coordinates centroid;
    int i,j;
    for(i=0; i< totalNumberOfCells; i++)
    {
        if(cellCV[i].nOfNodes ==4)
        {
            centroid.x =0;
            centroid.y=0 ;
            for(j=0; j< cellCV[i].nOfNodes; j++)
            {
                centroid.x += nodes[cellCV[i].node[j]].x;
                centroid.y += nodes[cellCV[i].node[j]].y;
            }
            centroid.x = centroid.x / cellCV[i].nOfNodes;
            centroid.y = centroid.y / cellCV[i].nOfNodes;
            quickSort(cellCV[i].node, centroid, 0, 3);
        }
        
    }
}
/* *************************************************************** *\
Cell Data Process ends
......................
......................
......................
WrItInG in VtK 
WrItInG in vTU
\* *************************************************************** */

void WriteMeshToVTK()
{
    int i;

    FILE *fp;
    fp=fopen("postProcess/File.vtk","w");
    
    
    fprintf(fp,"# vtk DataFile Version 3.0 \n");
    fprintf(fp,"Unstructured grid \n");
    fprintf(fp,"ASCII \n");
    fprintf(fp,"DATASET UNSTRUCTURED_GRID \n");
    fprintf(fp,"\n");
    fprintf(fp,"POINTS %d double \n",totalNumberOfNodes);
    for( i=0; i < totalNumberOfNodes; i++)
    {
        fprintf(fp,"%f %f %f \n",nodes[i].x, nodes[i].y , 0);
    }
    fprintf(fp,"\n");
    fprintf(fp,"CELLS %d %d \n",totalNumberOfCells, totalNumberOfCells + noOfTriangularCell*3 + noOfQuadrilateralCell*4);
    for(i=0; i< totalNumberOfCells; i++)
    {
        if(cellCV[i].type == 1)
        {
            fprintf(fp,"3 %d %d %d \n",cellCV[i].node[0], cellCV[i].node[1], cellCV[i].node[2]);
        }
        if(cellCV[i].type == 3)
        {
            fprintf(fp,"4 %d %d %d %d \n",cellCV[i].node[0], cellCV[i].node[1], cellCV[i].node[2], cellCV[i].node[3]); 
        }
    }
    fprintf(fp," \n");
    fprintf(fp,"CELL_TYPES %d \n",totalNumberOfCells);
    for(i=0; i< totalNumberOfCells; i++)
    {
        if(cellCV[i].type == 1)
        {
            fprintf(fp,"5 \n");
        }
        if(cellCV[i].type == 3)
        {
            fprintf(fp,"9 \n"); 
        }
    }
    fprintf(fp,"CELL_DATA %d \n",totalNumberOfCells);
    fprintf(fp,"POINT_DATA %d \n",totalNumberOfNodes);
    fclose(fp);
}

void WriteMeshToVTU()
{
    int i;
    int offsetCount = 0;

    //for proper sequencing
    char TAB[] = "  ";
    char TAB2[]="       ";
    char TAB3[]="           ";
    char TAB4[]="               ";
    char TAB5[]="                   ";

    FILE *fp;
    fp=fopen("postProcess/File.vtu","w");
    
    
    fprintf(fp,"<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\"> compressor=\"vtkZLibDataCompressor\" \n");
    fprintf(fp,"%s <UnstructuredGrid> \n",TAB);
    fprintf(fp,"%s <Piece NumberOfPoints=\"%d\" NumberOfCells=\"%d\"> \n",TAB2,totalNumberOfNodes,totalNumberOfCells);
    fprintf(fp,"%s <Points> \n",TAB3);
    fprintf(fp,"%s <DataArray type=\"Float32\" NumberOfComponents=\"3\" Format=\"ascii\"> \n",TAB4);
    for( i=0; i < totalNumberOfNodes; i++)
    {
        fprintf(fp,"%s %f %f %f \n",TAB5,nodes[i].x, nodes[i].y , 0);
    }
    fprintf(fp,"%s </DataArray> \n",TAB4);
    fprintf(fp,"%s </Points> \n",TAB3);
    fprintf(fp,"%s <Cells> \n",TAB3);
    fprintf(fp,"%s <DataArray type=\"Int32\" Name=\"connectivity\" Format=\"ascii\"> \n",TAB4);
    for(i=0; i< totalNumberOfCells; i++)
    {
        if(cellCV[i].type == 1)
        {
            fprintf(fp,"%s %d %d %d \n",TAB5,cellCV[i].node[0], cellCV[i].node[1], cellCV[i].node[2]);
        }
        if(cellCV[i].type == 3)
        {
            fprintf(fp,"%s %d %d %d %d \n",TAB5,cellCV[i].node[0], cellCV[i].node[1], cellCV[i].node[2], cellCV[i].node[3]); 
        }
    }
    fprintf(fp,"%s </DataArray> \n",TAB4);
    fprintf(fp,"%s <DataArray type=\"Int32\" Name=\"offsets\" Format=\"ascii\"> \n",TAB4);
    fprintf(fp,"%s",TAB5);
    for(i=0; i< totalNumberOfCells; i++)
    {
        offsetCount += cellCV[i].nodeCount;
        fprintf(fp,"%d ",offsetCount);
    }
    fprintf(fp,"\n");
    fprintf(fp,"%s </DataArray> \n",TAB4);
    fprintf(fp,"%s <DataArray type=\"Int32\" Name=\"types\" Format=\"ascii\"> \n",TAB4);
    fprintf(fp,"%s",TAB5);
    for(i=0; i< totalNumberOfCells; i++)
    {
        if(cellCV[i].type ==1)
        {
            fprintf(fp,"%d ",5);
        }
        if(cellCV[i].type ==3)
        {
            fprintf(fp,"%d ",9);
        }
        
    }
    fprintf(fp,"\n");
    fprintf(fp,"%s </DataArray> \n",TAB4);
    fprintf(fp,"%s </Cells> \n",TAB3);
    fprintf(fp,"%s </Piece> \n",TAB2,totalNumberOfNodes,totalNumberOfCells);
    fprintf(fp,"%s </UnstructuredGrid> \n",TAB);
    fprintf(fp,"</VTKFile> \n");

    fclose(fp);
}

void PrintCellData()
{
    FILE *fp;
    fp= fopen("Output/Cell_data.txt","w");
    fprintf(fp,"\n Printing Cell data");
    fprintf(fp,"\n ");
    fprintf(fp,"Total number of Triangular cells: %d \t Quadrilateral cells: %d", noOfTriangularCell, noOfQuadrilateralCell);
    fprintf(fp,"\n %-14s %-14s %-14s %-14s %-14s %-14s %-14s %-14s %-14s %-14s %-14s ","Index","Node0","Node1","Node2","Node3","NodeCount","Face0","Face1","Face2","Face3","FaceCount");
    fprintf(fp,"\n ");
    int i,j;
    for(i=0; i < totalNumberOfCells; i++)
    {
        if(cellCV[i].type == 1)
        {
            fprintf(fp,"%-10d ", cellCV[i].index);
            fprintf(fp," \t %-10d \t %-10d \t %-10d \t \t %-10d ", cellCV[i].node[0] , cellCV[i].node[1], cellCV[i].node[2] ,cellCV[i].nodeCount );
            fprintf(fp," \t %-10d \t %-10d \t %-10d \t \t %-10d", cellCV[i].face[0] , cellCV[i].face[1], cellCV[i].face[2] ,cellCV[i].faceCount  );
            fprintf(fp,"\n");    
        }
        if(cellCV[i].type == 3)
        {
            fprintf(fp,"%-10d ", cellCV[i].index);
            fprintf(fp," \t %-10d \t %-10d \t %-10d \t %-10d \t %-10d", cellCV[i].node[0] , cellCV[i].node[1], cellCV[i].node[2], cellCV[i].node[3], cellCV[i].nodeCount   );
            fprintf(fp," \t %-10d \t %-10d \t %-10d \t %-10d \t %-10d", cellCV[i].face[0] , cellCV[i].face[1], cellCV[i].face[2] ,cellCV[i].face[3] ,cellCV[i].faceCount  );          
            fprintf(fp,"\n"); 
        }
    }
}

void printNodes (coordinates *node , int totNodes)
{
    FILE *fp;
    fp= fopen("Output/Nodes_data.txt","w");

    fprintf(fp,"\n \n Printing node data");
    fprintf(fp,"\n ");
    fprintf(fp, "\n Index \t Node.x \t Node.y");
    int i;
    for(i=0; i< totNodes; i++)
    {
        fprintf(fp," \n %d \t %f \t %f",i, node[i].x , node[i].y );
    }
    fclose(fp);
    printf("\n Done");
}

void printFaces(cellFace *face, int totFaces)
{
    FILE *fp2;
    fp2= fopen("Output/face_data.txt","w");
    int i;
    fprintf(fp2,"\n \n Printing face data");
    fprintf(fp2,"\n ");
    fprintf(fp2,"\n Index \t Type \t Node1 \t Node2 \t CellId1 \t CellId2");
    for(i=0; i<totFaces ;i++)
	{
		fprintf(fp2," \n %d \t %d \t %ld \t %ld \t %ld \t %ld", face[i].index , face[i].type , face[i].node[0] , face[i].node[1] , face[i].cellid[0], face[i].cellid[1]);
	}
    fclose(fp2);
    printf("\n Done");
}