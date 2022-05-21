/* *************************************************************** *\
\* *************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include<float.h>

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

    int counter;
    int loop;

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
            //printf("\n BLANK LINE: IGNORED \n");
            fputs(str,fp1);
        }
        else
        {
            SectionFlag sectionF = HeaderFlag(str);

            switch(sectionF)
            {
                case COMMENTS_FLAG:
                    //printf(" \n Comment line \n");
                    fputs(str,fp1);
                    numberofLines++;
                    break;
                case HEADER_FLAG:
                    //printf("\n Comment line \n");
                    fputs(str,fp1);
                    numberofLines++;
                    break;
                case DIMENSIONS_FLAG:
                    //printf(" \n Dimensions line \n");
                    sscanf(str,"(%d %d)",&dimension1,&dimension2);
                    fputs(str,fp1);
                    if(dimension2 != 3)
                    {
                        printf("\n Try using code mesh2D \n");
                        exit(1);
                    }
                    else{
                        numberofLines++;
                    }
                    break;
                case NODES_FLAG:
                    sscanf(str,"(10 (%lx %lx %lx %d %d)",&zoneID,&idBegin, &idEnd,&type,&ND);
                    if(ND == 2)
                    {
                        printf(" \n Error 102: Error with the file");
                        fputs(str,fp1);
                        exit(0);
                    }
                    if(zoneID == 0)
                    {
                        nNodes= idEnd - idBegin +1;// this represent total_number_of_nodes
                        totalNumberOfNodes = nNodes;
                        nodes= (coordinate3D *)calloc(nNodes,sizeof(coordinate3D));
                        flag2++;// for making sure we alot the size to nodes before.
                        fputs(str,fp1);
                    }
                    else if(zoneID > 0)
                    {
                        fputs(str,fp1);
                      //  printf("\n REading NODE DATA for zone %d ", zoneID);
                        if(flag2 !=1)
                        {
                            printf("\n Error 102: Logical Error");
                            exit(0);
                        }
                        nNodes = idEnd - idBegin +1; // doing show that multiple regin don't possess an issue
                        int start = idBegin - 1;
                        double x, y , z;
                        j=0;
                        while(j<nNodes)
                        {
                            getline(&str,&line_buf_size,fp);
                            int numChar = sscanf(str,"%lf %lf %lf",&x,&y,&z);
                            if (numChar == 3)
                            {
                                nodes[start+j].x =x;
                                nodes[start+j].y =y;
                                nodes[start+j].z =z;
                                j++;
                            }
                            fputs(str,fp1);
                        }
                        zoneCount += nNodes;
                    }
                    //printf("\n REading NODE DATA for zone %d ends here",zoneID);
                    break;
                case FACES_FLAG:
                    sscanf(str,"(13 (%lx %lx %lx %d %d)",&zoneID, &idBegin, &idEnd, &type, &elementType);
                    
                    
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
                       // printf("\n REading FACE DATA for zone %d ", zoneID);
                        nFaces = idEnd - idBegin + 1;
                        long int *nodeId, *cellId;
                        int typeFace;
                        //temporarily allocating memory
                        nodeId = (long int *)calloc(2,sizeof(long int));
                        cellId = (long int *)calloc(2,sizeof(long int));

                        faceCount = idBegin -1 ;

                        if(elementType == 0)
                        {
                            // have to be worked on, using strtok and sscanf --> idea** == to check the first element and based on that select the number of nodes for face
                            
                            j=0;
                            while(j<nFaces)
                            {
                            getline(&str,&line_buf_size,fp);
                            sscanf(str,"%d ",&typeFace);
                            if (typeFace == 3)
                            {
                                nodeId = (long int *)realloc(nodeId, sizeof(long int)*typeFace);
                                cellId = (long int *)realloc(cellId, sizeof(long int)*2);
                                int numChar = sscanf(str,"%d %lx %lx %lx %lx %lx",&typeFace,&nodeId[0],&nodeId[1],&nodeId[2],&cellId[0],&cellId[1]);
                                if(numChar == 6)
                                {
                                    faces[faceCount+j].index = faceCount+j;
                                    faces[faceCount+j].type = type;
                                    faces[faceCount+j].faceType = typeFace;
                                    //THIS REPRESENTS A PROPER CASE IN WHICH N0 N1 CR C1 GIVEN
                                    faces[faceCount+j].node = (int *)calloc(typeFace,sizeof(int));
                                    faces[faceCount+j].cellid = (int *)calloc(2,sizeof(int));
                                    faces[faceCount+j].node[0] = nodeId[0];
                                    faces[faceCount+j].node[1] = nodeId[1];
                                    faces[faceCount+j].node[2] = nodeId[2];
                                    faces[faceCount+j].cellid[0] = cellId[0];
                                    faces[faceCount+j].cellid[1] = cellId[1];
                                    
                                    j++;
                                }

                            }
                            if (typeFace == 4)
                            {
                                nodeId = (long int *)realloc(nodeId, sizeof(long int)*typeFace);
                                cellId = (long int *)realloc(cellId, sizeof(long int)*2);
                                int numChar = sscanf(str,"%d %lx %lx %lx %lx %lx %lx",&typeFace,&nodeId[0],&nodeId[1],&nodeId[2],&nodeId[3],&cellId[0],&cellId[1]);
                                if(numChar == 7)
                                {
                                    faces[faceCount+j].index = faceCount+j;
                                    faces[faceCount+j].type = type;
                                    faces[faceCount+j].faceType = typeFace;
                                    //THIS REPRESENTS A PROPER CASE IN WHICH N0 N1 CR C1 GIVEN
                                    faces[faceCount+j].node = (int *)calloc(typeFace,sizeof(int));
                                    faces[faceCount+j].cellid = (int *)calloc(2,sizeof(int));
                                    faces[faceCount+j].node[0] = nodeId[0];
                                    faces[faceCount+j].node[1] = nodeId[1];
                                    faces[faceCount+j].node[2] = nodeId[2];
                                    faces[faceCount+j].node[3] = nodeId[3];
                                    faces[faceCount+j].cellid[0] = cellId[0];
                                    faces[faceCount+j].cellid[1] = cellId[1];
                                    
                                    j++;
                                }

                            }
                           
                            fputs(str,fp1);
                            }
                         
                        }
                        else if(elementType == 3)
                        {
                            nodeId = (long int *)realloc(nodeId, sizeof(long int)*elementType);
                            cellId = (long int *)realloc(cellId, sizeof(long int)*2);

                            j=0;
                            while(j<nFaces)
                            {
                            getline(&str,&line_buf_size,fp);
                            int numChar = sscanf(str,"%lx %lx %lx %lx %lx",&nodeId[0],&nodeId[1],&nodeId[2],&cellId[0],&cellId[1]);
                                if(numChar == 5)
                                {
                                    faces[faceCount+j].index = faceCount+j;
                                    faces[faceCount+j].type = type;
                                    faces[faceCount+j].faceType = elementType;
                                    //THIS REPRESENTS A PROPER CASE IN WHICH N0 N1 CR C1 GIVEN
                                    faces[faceCount+j].node = (int *)calloc(elementType,sizeof(int));
                                    faces[faceCount+j].cellid = (int *)calloc(2,sizeof(int));
                                    faces[faceCount+j].node[0] = nodeId[0];
                                    faces[faceCount+j].node[1] = nodeId[1];
                                    faces[faceCount+j].node[2] = nodeId[2];
                                    faces[faceCount+j].cellid[0] = cellId[0];
                                    faces[faceCount+j].cellid[1] = cellId[1];
                                    
                                    j++;
                                }
                            fputs(str,fp1);
                            }
                        }
                        else if(elementType == 4)
                        {
                            nodeId = (long int *)realloc(nodeId, sizeof(long int)*elementType);
                            cellId = (long int *)realloc(cellId, sizeof(long int)*2);

                            j=0;
                            while(j<nFaces)
                                {
                                getline(&str,&line_buf_size,fp);
                                int numChar = sscanf(str,"%lx %lx %lx %lx %lx %lx",&nodeId[0],&nodeId[1],&nodeId[2],&nodeId[3],&cellId[0],&cellId[1]);
                                if(numChar == 6)
                                {
                                    faces[faceCount+j].index = faceCount+j;
                                    faces[faceCount+j].type = type;
                                    faces[faceCount+j].faceType = elementType;
                                    //THIS REPRESENTS A PROPER CASE IN WHICH N0 N1 CR C1 GIVEN
                                    faces[faceCount+j].node = (int *)calloc(elementType,sizeof(int));
                                    faces[faceCount+j].cellid = (int *)calloc(2,sizeof(int));
                                    faces[faceCount+j].node[0] = nodeId[0];
                                    faces[faceCount+j].node[1] = nodeId[1];
                                    faces[faceCount+j].node[2] = nodeId[2];
                                    faces[faceCount+j].node[3] = nodeId[3];
                                    faces[faceCount+j].cellid[0] = cellId[0];
                                    faces[faceCount+j].cellid[1] = cellId[1];
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
                        

                        if(elementType == 2)
                        {
                            nCells= idEnd -idBegin +1;
                            noOfTetrahedralCell += nCells;
                            for(i=0; i< nCells; i++)
                            {
                                cellCV[cellCount + i].index = cellCount +i ;
                                cellCV[cellCount + i].type = elementType;
                                cellCV[cellCount + i].nOfNodes = 4;
                                cellCV[cellCount + i].nOfFaces = 4;
                                cellCV[cellCount + i].nodeCount = 0;
                                cellCV[cellCount + i].faceCount = 0;
                                cellCV[cellCount + i].node = (int *)calloc(cellCV[cellCount + i].nOfNodes,sizeof(int));
                                cellCV[cellCount + i].face = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                                cellCV[cellCount + i].neighbourCellId = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                            }
                            //cellCount += nCells; 
                            //printf("\n Number of tetrahedral cells are %d in zone %d", noOfTetrahedralCell, zoneID);
                        }
                        else if(elementType == 4)
                        {
                            nCells = idEnd -idBegin +1;
                            noOfHexahedralCell += nCells;
                            for(i=0; i< nCells; i++)
                            {
                                cellCV[cellCount + i].index = cellCount +i ;
                                cellCV[cellCount + i].type = elementType;
                                cellCV[cellCount + i].nOfNodes = 8;
                                cellCV[cellCount + i].nOfFaces = 6;
                                cellCV[cellCount + i].nodeCount = 0;
                                cellCV[cellCount + i].faceCount = 0;
                                cellCV[cellCount + i].node = (int *)calloc(cellCV[cellCount + i].nOfNodes,sizeof(int));
                                cellCV[cellCount + i].face = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                                cellCV[cellCount + i].neighbourCellId = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                            }
                            //cellCount += nCells; 
                            //printf("\n Number of hexahedral cells are %d in zone %d ", noOfHexahedralCell, zoneID);
                        }
                        else if(elementType == 5)
                        {
                            nCells = idEnd -idBegin +1;
                            noOfPyramidalCell += nCells;
                            for(i=0; i< nCells; i++)
                            {
                                cellCV[cellCount + i].index = cellCount +i ;
                                cellCV[cellCount + i].type = elementType;
                                cellCV[cellCount + i].nOfNodes = 5;
                                cellCV[cellCount + i].nOfFaces = 5;
                                cellCV[cellCount + i].nodeCount = 0;
                                cellCV[cellCount + i].faceCount = 0;
                                cellCV[cellCount + i].node = (int *)calloc(cellCV[cellCount + i].nOfNodes,sizeof(int));
                                cellCV[cellCount + i].face = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                                cellCV[cellCount + i].neighbourCellId = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                            }
                            //cellCount += nCells; 
                            //printf("\n Number of pyramid cells are %d in zone %d ", noOfPyramidalCell, zoneID);
                        }
                        else if(elementType == 6)
                        {
                            nCells = idEnd -idBegin +1;
                            noOfWedgeCell += nCells;
                            for(i=0; i< nCells; i++)
                            {
                                cellCV[cellCount + i].index = cellCount +i ;
                                cellCV[cellCount + i].type = elementType;
                                cellCV[cellCount + i].nOfNodes = 6;
                                cellCV[cellCount + i].nOfFaces = 5;
                                cellCV[cellCount + i].nodeCount = 0;
                                cellCV[cellCount + i].faceCount = 0;
                                cellCV[cellCount + i].node = (int *)calloc(cellCV[cellCount + i].nOfNodes,sizeof(int));
                                cellCV[cellCount + i].face = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                                cellCV[cellCount + i].neighbourCellId = (int *)calloc(cellCV[cellCount + i].nOfFaces,sizeof(int));
                            }
                             
                            //printf("\n Number of wedge cells are %d in zone %d ", noOfWedgeCell, zoneID);
                        }
                        // done ***** // elementType ==0 need to be properly done, as sometimes line changes.... also need to put the othjer cell type condition
                        else if(elementType == 0)
                        {
                            
                            nCells= idEnd -idBegin +1;
                            counter =0; 
                            loop = 0; 

                            while (loop < nCells)
                            {
                                getline(&str,&line_buf_size,fp);
                                check = sscanf(str,"%d",&eType);
                                if(check == 0)
                                {
                                    continue;
                                }
                                else
                                {
                                p = strtok(str," ");
                                while(p != NULL)
                                {
                                    check = sscanf(p,"%d",&eType);
                                    //printf("%d \n",eType); 
                                    if (check == 1)
                                    {
                                        if(eType == 2)
                                        {
                                            
                                            cellCV[idBegin -1 + counter].index = idBegin -1 + counter ;
                                            cellCV[idBegin -1 + counter].type = 2;
                                            cellCV[idBegin -1 + counter].nOfNodes = 4;
                                            cellCV[idBegin -1 + counter].nOfFaces = 4;
                                            cellCV[idBegin -1 + counter].nodeCount = 0;
                                            cellCV[idBegin -1 + counter].faceCount = 0;
                                            cellCV[idBegin -1 + counter].node = (int *)calloc(cellCV[idBegin -1 + counter].nOfNodes,sizeof(int));
                                            cellCV[idBegin -1 + counter].face = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                            cellCV[idBegin -1 + counter].neighbourCellId = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                            
                                            
                                        }
                                        if(eType == 4)
                                        {
                                            cellCV[idBegin -1 + counter].index = idBegin -1 + counter;
                                            cellCV[idBegin -1 + counter].type = 4;
                                            cellCV[idBegin -1 + counter].nOfNodes = 8;
                                            cellCV[idBegin -1 + counter].nOfFaces = 6;
                                            cellCV[idBegin -1 + counter].nodeCount = 0;
                                            cellCV[idBegin -1 + counter].faceCount = 0;
                                            cellCV[idBegin -1 + counter].node = (int *)calloc(cellCV[idBegin -1 + counter].nOfNodes,sizeof(int));
                                            cellCV[idBegin -1 + counter].face = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                            cellCV[idBegin -1 + counter].neighbourCellId = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                        }
                                        if(eType == 5)
                                        {
                                            cellCV[idBegin -1 + counter].index = idBegin -1 + counter;
                                            cellCV[idBegin -1 + counter].type = 5;
                                            cellCV[idBegin -1 + counter].nOfNodes = 5;
                                            cellCV[idBegin -1 + counter].nOfFaces = 5;
                                            cellCV[idBegin -1 + counter].nodeCount = 0;
                                            cellCV[idBegin -1 + counter].faceCount = 0;
                                            cellCV[idBegin -1 + counter].node = (int *)calloc(cellCV[idBegin -1 + counter].nOfNodes,sizeof(int));
                                            cellCV[idBegin -1 + counter].face = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                            cellCV[idBegin -1 + counter].neighbourCellId = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                        }
                                        if(eType == 6)
                                        {
                                            cellCV[idBegin -1 + counter].index = idBegin -1 + counter;
                                            cellCV[idBegin -1 + counter].type = 6;
                                            cellCV[idBegin -1 + counter].nOfNodes = 6;
                                            cellCV[idBegin -1 + counter].nOfFaces = 5;
                                            cellCV[idBegin -1 + counter].nodeCount = 0;
                                            cellCV[idBegin -1 + counter].faceCount = 0;
                                            cellCV[idBegin -1 + counter].node = (int *)calloc(cellCV[idBegin -1 + counter].nOfNodes,sizeof(int));
                                            cellCV[idBegin -1 + counter].face = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                            cellCV[idBegin -1 + counter].neighbourCellId = (int *)calloc(cellCV[idBegin -1 + counter].nOfFaces,sizeof(int));
                                        }
                                        loop ++ ;
                                        counter ++ ;
                                    }
                                    p = strtok(NULL," ");
                                }
                                }


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
                              //  printf("\n %d",count);
                            }
                            /* if(count == 3)
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
                            } */

                        }
                    }
                    break;
                default:
                    //printf("\n Section not of importance");
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
   

    FILE *fp;
    fp=fopen("Output/CHeckCEll.txt","w");

    int cell0, cell1;

    int i;
    
    int counter;

    for(i=0; i < totalNumberOfFaces; i++)
    {
        cell0 = faces[i].cellid[0];
        cell1= faces[i].cellid[1];

        if(cell0 > 0)
        {
            
            fprintf(fp,"\n %d %d",faces[i].index,faces[i].cellid[0] - 1);
            cellCV[cell0 -1].face[cellCV[cell0-1].faceCount] = faces[i].index;
            counter = 0;
            while (counter < faces[i].faceType)
            {
                if(cellCV[faces[i].cellid[0] -1].nodeCount == 0)
                {
                    cellCV[cell0-1].node[cellCV[cell0 -1].nodeCount] = faces[i].node[counter] - 1;
                    fprintf(fp," %d",cellCV[cell0 -1].nodeCount);
                    cellCV[cell0 -1].nodeCount += 1;
                    
                }
                else
                {
                    if(FindIfValuePresent(cellCV[cell0 -1].node, faces[i].node[counter] - 1, cellCV[cell0 -1].nodeCount ) ==0 )
                    {
                        cellCV[cell0-1].node[cellCV[cell0 -1].nodeCount] = faces[i].node[counter] - 1;
                        fprintf(fp," %d",cellCV[cell0 -1].nodeCount);
                        cellCV[cell0 -1].nodeCount += 1;
                        
                    }
                }
                counter++;

            }
             fprintf(fp," \n End %d",cell0 -1);
            cellCV[cell0 -1].faceCount += 1;
        }
        if(cell1 > 0)
        {
            fprintf(fp,"\n%d %d",faces[i].index,faces[i].cellid[1]-1);
            counter =0;
            cellCV[cell1 -1].face[cellCV[cell1-1].faceCount] = faces[i].index;
            while (counter < faces[i].faceType)
            {
                if(cellCV[cell1 -1].nodeCount == 0)
                {
                    cellCV[cell1-1].node[cellCV[cell1 -1].nodeCount] = faces[i].node[counter] - 1;
                    fprintf(fp," %d",cellCV[cell1 -1].nodeCount);
                    cellCV[cell1 -1].nodeCount += 1;
                    
                }
                else
                {
                    if(FindIfValuePresent(cellCV[cell1 -1].node, faces[i].node[counter] - 1, cellCV[cell1 -1].nodeCount ) ==0 )
                    {
                        cellCV[cell1-1].node[cellCV[cell1 -1].nodeCount] = faces[i].node[counter] - 1;
                        fprintf(fp," %d",cellCV[cell1 -1].nodeCount);
                        cellCV[cell1 -1].nodeCount += 1;
                        
                    }
                }
                counter ++ ;

            }
            fprintf(fp," \n End %d",faces[i].cellid[1] -1);
            cellCV[cell1 -1].faceCount += 1;
        }
        
        
    }
    fclose(fp);
    printf("\n DONE");
}
/**********************************************************************************************\
 * 
\**********************************************************************************************/
double DotProduct(double vect_A[], double vect_B[])
{
 
    double product = 0;
    for (int i = 0; i < 3; i++)
    {
        product = product + vect_A[i] * vect_B[i];
    }
    return product;
}
 
void CrossProduct(double vect_A[], double vect_B[], double cross_P[])
 
{
    cross_P[0] = vect_A[1] * vect_B[2] - vect_A[2] * vect_B[1];
    cross_P[1] = vect_A[2] * vect_B[0] - vect_A[0] * vect_B[2];
    cross_P[2] = vect_A[0] * vect_B[1] - vect_A[1] * vect_B[0];
    int i;
    // for unit normal
    for(i=0;i<3;i++)
    {
        cross_P[i] = cross_P[i]/(sqrt(pow(cross_P[0],2)+ pow(cross_P[1],2) + pow(cross_P[2],2)));
    }
}

double *CreateVector(int arr[])
{
    double *vector;
    vector = (double *)calloc(3,sizeof(double));
    vector[0] = nodes[arr[1]].x - nodes[arr[0]].x ;
    vector[1] = nodes[arr[1]].y - nodes[arr[0]].y ;
    vector[2] = nodes[arr[1]].z - nodes[arr[0]].z ;

    return vector;

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
 

int partition (int *arr, coordinate3D centre, int low, int high, int assign)
{
    float pivot;
    int i = (low - 1); // Index of smaller element and indicates the right position of pivot found so far
    ////face normal vector in Z-direction
    if (assign == 0)
    {
        pivot = AngleCalculator(nodes[arr[high]].y - centre.y,nodes[arr[high]].x - centre.x); // pivot
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
    //face normal vector in X-direction
    if(assign == 1)
    {
        pivot = AngleCalculator(nodes[arr[high]].y - centre.y,nodes[arr[high]].z - centre.z); // pivot
        for (int j = low; j <= high - 1; j++)
        {
            // If current element is smaller than the pivot
            if (AngleCalculator(nodes[arr[j]].y - centre.y , nodes[arr[j]].z - centre.z ) < pivot)
            {
                i++; // increment index of smaller element
                swap(&arr[i], &arr[j]);
            }
        }
        swap(&arr[i + 1], &arr[high]);
        return (i + 1);
    }
    //face normal vector in X-direction
    if(assign == 2)
    {
        pivot = AngleCalculator(nodes[arr[high]].x - centre.x,nodes[arr[high]].z - centre.z); // pivot
        for (int j = low; j <= high - 1; j++)
        {
            // If current element is smaller than the pivot
            if (AngleCalculator(nodes[arr[j]].x - centre.x , nodes[arr[j]].z - centre.z ) < pivot)
            {
                i++; // increment index of smaller element
                swap(&arr[i], &arr[j]);
            }
        }
        swap(&arr[i + 1], &arr[high]);
        return (i + 1);
    }
    
}
 

void quickSort(int* arr, coordinate3D centre, int low, int high,int assign)
{
    if (low < high)
    {
     
        int pi = partition(arr,centre, low, high, assign);
        quickSort(arr,centre, low, pi - 1, assign);
        quickSort(arr,centre, pi + 1, high , assign);
    }
}

int findMissingNodePyramidIndex(int *a, int *b,int n, int m)
{
    for (int i = 0; i < n; i++)
    {
        int j;
        while(j < m)
        {
            if (a[i] == b[j])
                break;
            j++;
        }
        if (j == m)
        {
            return i;
        }
    }
}

int* print2Smallest(double *arr, int arr_size)
{
    int i;
    double first, second;
    int *index;
    index = (int *)calloc(2,sizeof(int));

    first = second = DBL_MAX;
    for (i = 0; i < arr_size ; i ++)
    {
        /* If current element is smaller than first
        then update both first and second */
        if (arr[i] < first)
        {
            second = first;
            first = arr[i];
            index[1]= index[0];
            index[0] = i ;

        }
        /* If arr[i] is in between first and second
        then update second */
        else if (arr[i] < second)
            second = arr[i];
            index[1]=i;
    }
    
    return index;
}

bool isSubset(int arr1[], int arr2[],
              int m, int n)
{
    int i = 0;
    int j = 0;
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            if (arr2[i] == arr1[j])
                break;
        }
 
        /* If the above inner loop was
        not broken at all then arr2[i]
        is not present in arr1[] */
        if (j == m)
            return 0;
    }
 
    /* If we reach here then all
    elements of arr2[] are present
    in arr1[] */
    return 1;
}

int *findMissing(int *a, int b[],int n, int m, int cellType)
{
    int *result;
    if(cellType == 4)
    {
        result = (int *)calloc(2,sizeof(int));
    }
    int index =0;
    for (int i = 0; i < n; i++)
    {
        int j;
        for (j = 0; j < m; j++)
            if (a[i] == b[j])
                break;
 
        if (j == m)
        {
            result[index] = a[i];
            index ++; 
        }
    }
    return result;
}


/* //
double magnitude(double *arr, int size)
{
    double result;
    for(int i=0;i<size;i++)
    {
        result += arr[i] * arr[i];
    }
    return result;
}

double tCalculator(double *nV,coordinate3D r, coordinate3D c, double *pV)
{
    double rcVec[3];
    rcVec[0] = r.x -c.x ;
    rcVec[1] = r.y -c.y ;
    rcVec[2] = r.z -c.z ;

    double product[3];

    CrossProduct(rcVec,pV,product);

    double result = DotProduct(nV,product);
    return result;
}
double uCalculator(double *nV,coordinate3D r, coordinate3D c, double *qV)
{
    double rcVec[3];
    rcVec[0] = r.x -c.x ;
    rcVec[1] = r.y -c.y ;
    rcVec[2] = r.z -c.z ;

    double product[3];

    CrossProduct(rcVec,qV,product);

    double result = DotProduct(nV,product);
    return result;

}
int partition1 (int *arr, coordinate3D centre, int low, int high,double *nV, double *pV, double *qV)
{
    float pivot;
    int i = (low - 1); // Index of smaller element and indicates the right position of pivot found so far
    ////face normal vector in Z-direction
    
    pivot = AngleCalculator(uCalculator(nV,nodes[arr[high]],centre,qV),tCalculator(nV,nodes[arr[high]],centre,pV));
    
    for (int j = low; j <= high - 1; j++)
    {
        // If current element is smaller than the pivot
        if (AngleCalculator(uCalculator(nV,nodes[arr[j]],centre,qV),tCalculator(nV,nodes[arr[j]],centre,pV)) < pivot)
        {
            i++; // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);

    
    
}
 

void quickSort1(int* arr, coordinate3D centre, int low, int high,double *nV, double *pV, double *qV)
{
    if (low < high)
    {
     
        int pi = partition1(arr,centre, low, high,nV, pV, qV);
        quickSort1(arr,centre, low, pi - 1,nV, pV, qV);
        quickSort1(arr,centre, pi + 1, high,nV, pV, qV );
    }
} */

void NodeInCellOrientation()
{
    coordinate3D centroid;
    FILE *fp;
    fp = fopen("Output/NodeInCell.txt","w");

    int nodeBackCounter = 0;
    int nodeFrontCounter = 0;
    int *tempArray,*tempBackArray;
    tempArray = (int *)calloc(2,sizeof(int));
    double *tempFrontVector,*tempBackVector;

    /* double vecI[]={1,0,0};
    double vecJ[]={0,1,0};
    double vecK[]={0,0,1};
    double dotProductValue;
    double *vectorDotProductArray;
    int * minimumVectorDotProductIndexArray; 
    int subXY[2]={0,1};
    int subXZ[2]={0,2};
    int subYZ[2]={1,2};

    double vecA[3],vecB[3],vecCrossProduct[3];
    // new line
    double pVector[3],qVector[3],nIVector[3],nJVector[3],nKVector[3];
    //

    int assign; // show that partition know that face is normal to i,j,k vector

    vectorDotProductArray = (double *)calloc(3,sizeof(double)); */

    int i,j;
    for(i=0; i< totalNumberOfCells; i++)
    {
        // Hex
        if(cellCV[i].type == 4)
        { 
            while (nodeBackCounter < 4)
            {
                tempArray[0]=cellCV[i].node[nodeFrontCounter] ;
                tempArray[1]=cellCV[i].node[nodeFrontCounter + 1] ;
                //printf("\n %d %d",tempArray[0],tempArray[1]);
                tempFrontVector = CreateVector(tempArray);
                tempArray[0] += 1;
                tempArray[1] += 1;
                for(j=0;j<6;j++)
                {
                    if(isSubset(faces[cellCV[i].face[j]].node,tempArray,4,2)==1 && j!=0)
                    {

                        tempBackArray = findMissing(faces[cellCV[i].face[j]].node,tempArray,4,2,4);
                        tempBackArray[0] = tempBackArray[0] - 1;
                        tempBackArray[1] = tempBackArray[1] - 1;
                        tempBackVector = CreateVector(tempBackArray);
                        //printf("\n In %d",i);
                        if(DotProduct(tempFrontVector,tempBackVector)>0)
                        {
                           
                            if(nodeBackCounter == 0)
                            {
                                cellCV[i].node[4+ nodeBackCounter] = tempBackArray[0] ;
                                nodeBackCounter ++ ;
                                cellCV[i].node[4 + nodeBackCounter] = tempBackArray[1] ;
                                nodeBackCounter ++ ;
                            }
                            else
                            {
                                if(FindIfValuePresent(cellCV[i].node, tempBackArray[0] , 4 + nodeBackCounter ) ==0 )
                                {
                                    cellCV[i].node[4+ nodeBackCounter] = tempBackArray[0] ;
                                    nodeBackCounter ++ ;
                                }
                                if(FindIfValuePresent(cellCV[i].node, tempBackArray[1] , 4 + nodeBackCounter ) ==0 )
                                {
                                    cellCV[i].node[4+ nodeBackCounter] = tempBackArray[1] ;
                                    nodeBackCounter ++ ;
                                }
                            }
                            
                            nodeFrontCounter ++;
                        }
                        else
                        {
                            
                            if(nodeBackCounter == 0)
                            {
                                cellCV[i].node[4+ nodeBackCounter] = tempBackArray[1] ;
                                nodeBackCounter ++ ;
                                cellCV[i].node[4 + nodeBackCounter] = tempBackArray[0] ;
                                nodeBackCounter ++ ;
                            }
                            else
                            {
                                if(FindIfValuePresent(cellCV[i].node, tempBackArray[1] , 4 + nodeBackCounter ) ==0 )
                                {
                                    cellCV[i].node[4+ nodeBackCounter] = tempBackArray[1] ;
                                    nodeBackCounter ++ ;
                                }
                                if(FindIfValuePresent(cellCV[i].node, tempBackArray[0] , 4 + nodeBackCounter ) ==0 )
                                {
                                    cellCV[i].node[4+ nodeBackCounter] = tempBackArray[0] ;
                                    nodeBackCounter ++ ;
                                }
                            }
                            
                            nodeFrontCounter ++;
                        }
                        break;
                    }
                }
            }
            nodeBackCounter = 0;
            nodeFrontCounter = 0;


            /* vecA[0] = nodes[cellCV[i].node[1]].x - nodes[cellCV[i].node[0]].x;
            vecA[1] = nodes[cellCV[i].node[1]].y - nodes[cellCV[i].node[0]].y;
            vecA[2] = nodes[cellCV[i].node[1]].z - nodes[cellCV[i].node[0]].z;

            vecB[0] = nodes[cellCV[i].node[2]].x - nodes[cellCV[i].node[0]].x;
            vecB[1] = nodes[cellCV[i].node[2]].y - nodes[cellCV[i].node[0]].y;
            vecB[2] = nodes[cellCV[i].node[2]].z - nodes[cellCV[i].node[0]].z;

            CrossProduct(vecA,vecB,vecCrossProduct);

            
            vectorDotProductArray[0] = fabs(DotProduct(vecCrossProduct, vecI));
            vectorDotProductArray[1] = fabs(DotProduct(vecCrossProduct, vecJ));
            vectorDotProductArray[2] = fabs(DotProduct(vecCrossProduct, vecK));

            minimumVectorDotProductIndexArray = print2Smallest(vectorDotProductArray,3); */


            /* if(isSubset(minimumVectorDotProductIndexArray,subXY,2,2))
            {
                fprintf(fp,"XY %f %f %f\n",fabs(DotProduct(vecCrossProduct, vecI)),fabs(DotProduct(vecCrossProduct, vecJ)),fabs(DotProduct(vecCrossProduct, vecK)));
                assign = 0 ; //imp
                //printf("\n K");
                // ones for one face
                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.x += nodes[cellCV[i].node[j]].x;
                    centroid.y += nodes[cellCV[i].node[j]].y;
                }
                centroid.x = centroid.x / 4;
                centroid.y = centroid.y / 4;
                quickSort(cellCV[i].node, centroid, 0, 3, assign);
                fprintf(fp,"XY %d %d %d %d ",cellCV[i].node[0],cellCV[i].node[1],cellCV[i].node[2],cellCV[i].node[3]);
                // for second face
                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.x += nodes[cellCV[i].node[4+j]].x;
                    centroid.y += nodes[cellCV[i].node[4+j]].y;
                }
                centroid.x = centroid.x / 4;
                centroid.y = centroid.y / 4;
                quickSort(cellCV[i].node, centroid, 4, 7, assign);
                fprintf(fp," %d %d %d %d \n ",cellCV[i].node[4],cellCV[i].node[5],cellCV[i].node[6],cellCV[i].node[7]);

            }
            if(isSubset(minimumVectorDotProductIndexArray,subXZ,2,2))
            {
                fprintf(fp,"XZ %f %f %f\n",fabs(DotProduct(vecCrossProduct, vecI)),fabs(DotProduct(vecCrossProduct, vecJ)),fabs(DotProduct(vecCrossProduct, vecK)));
                assign = 2 ; 
                //printf("\n J");
                // ones for one face
                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.x += nodes[cellCV[i].node[j]].x;
                    centroid.z += nodes[cellCV[i].node[j]].z;
                }
                centroid.z = centroid.z / 4;
                centroid.x = centroid.x / 4;
                quickSort(cellCV[i].node, centroid, 0, 3, assign);
                fprintf(fp,"XZ  %d %d %d %d ",cellCV[i].node[0],cellCV[i].node[1],cellCV[i].node[2],cellCV[i].node[3]);
                // for second face
                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.z += nodes[cellCV[i].node[4+j]].z;
                    centroid.x += nodes[cellCV[i].node[4+j]].x;
                }
                centroid.z = centroid.z / 4;
                centroid.x = centroid.x / 4;
                quickSort(cellCV[i].node, centroid, 4, 7, assign);
                fprintf(fp,"%d %d %d %d \n ",cellCV[i].node[4],cellCV[i].node[5],cellCV[i].node[6],cellCV[i].node[7]);
            }
            if(isSubset(minimumVectorDotProductIndexArray,subYZ,2,2))
            {
                fprintf(fp,"YZ %f %f %f\n",fabs(DotProduct(vecCrossProduct, vecI)),fabs(DotProduct(vecCrossProduct, vecJ)),fabs(DotProduct(vecCrossProduct, vecK)));
                assign = 1;
                //printf("\n I");
                // ones for one face
                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.z += nodes[cellCV[i].node[j]].z;
                    centroid.y += nodes[cellCV[i].node[j]].y;
                }
                centroid.z = centroid.z / 4;
                centroid.y = centroid.y / 4;
                quickSort(cellCV[i].node, centroid, 0, 3, assign);
                fprintf(fp,"YZ %d %d %d %d ",cellCV[i].node[0],cellCV[i].node[1],cellCV[i].node[2],cellCV[i].node[3]);
                // for second face
                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.z += nodes[cellCV[i].node[4+j]].z;
                    centroid.y += nodes[cellCV[i].node[4+j]].y;
                }
                centroid.z = centroid.z / 4;
                centroid.y = centroid.y / 4;
                quickSort(cellCV[i].node, centroid, 4, 7, assign);
                fprintf(fp," %d %d %d %d \n ",cellCV[i].node[4],cellCV[i].node[5],cellCV[i].node[6],cellCV[i].node[7]);
            }  */
            /* vecA[0] = nodes[cellCV[i].node[1]].x - nodes[cellCV[i].node[0]].x;
            vecA[1] = nodes[cellCV[i].node[1]].y - nodes[cellCV[i].node[0]].y;
            vecA[2] = nodes[cellCV[i].node[1]].z - nodes[cellCV[i].node[0]].z;

            vecB[0] = nodes[cellCV[i].node[2]].x - nodes[cellCV[i].node[0]].x;
            vecB[1] = nodes[cellCV[i].node[2]].y - nodes[cellCV[i].node[0]].y;
            vecB[2] = nodes[cellCV[i].node[2]].z - nodes[cellCV[i].node[0]].z;

            CrossProduct(vecA,vecB,vecCrossProduct);
            CrossProduct(vecCrossProduct,vecI,nIVector);
            CrossProduct(vecCrossProduct,vecJ,nJVector);
            CrossProduct(vecCrossProduct,vecK,nKVector);

            if (magnitude(nIVector,3) >= magnitude(nJVector,3) && magnitude(nIVector,3) >= magnitude(nKVector,3))
            {
                for(j=0;j<3;j++)
                {
                    pVector[j]= (nIVector[j]);
                }
            }

            // if n2 is greater than both n1 and n3, n2 is the largest
            if (magnitude(nJVector,3) >= magnitude(nIVector,3) && magnitude(nJVector,3) >= magnitude(nKVector,3))
            {
                for(j=0;j<3;j++)
                {
                    pVector[j]= (nJVector[j]);
                }
            }

            // if n3 is greater than both n1 and n2, n3 is the largest
            if (magnitude(nKVector,3) >= magnitude(nIVector,3) && magnitude(nKVector,3) >= magnitude(nJVector,3))
            {
                for(j=0;j<3;j++)
                {
                    pVector[j]= (nKVector[j]);
                }
            }

            CrossProduct(vecCrossProduct,pVector,qVector);

            centroid.x =0;
            centroid.y=0 ;
            centroid.z=0 ;
            for(j=0; j< 4 ; j++)
            {
                centroid.x += nodes[cellCV[i].node[j]].x;
                centroid.y += nodes[cellCV[i].node[j]].y;
                centroid.z += nodes[cellCV[i].node[j]].z;
            }
            centroid.x = centroid.x / 4;
            centroid.y = centroid.y / 4;
            centroid.z = centroid.z / 4;
            quickSort1(cellCV[i].node, centroid, 0, 3, vecCrossProduct,pVector,qVector);

            centroid.x =0;
            centroid.y=0 ;
            centroid.z=0 ;
            for(j=0; j< 4 ; j++)
            {
                centroid.x += nodes[cellCV[i].node[4+j]].x;
                centroid.y += nodes[cellCV[i].node[4+j]].y;
                centroid.z += nodes[cellCV[i].node[4+j]].z;
            }
            centroid.x = centroid.x / 4;
            centroid.y = centroid.y / 4;
            centroid.z = centroid.z / 4;

            quickSort1(cellCV[i].node, centroid, 4, 7, vecCrossProduct,pVector,qVector); */

            /* //1. calculate normal for face[0] --- > node points 0,1,2,3
            
            vecA[0] = nodes[cellCV[i].node[1]].x - nodes[cellCV[i].node[0]].x;
            vecA[1] = nodes[cellCV[i].node[1]].y - nodes[cellCV[i].node[0]].y;
            vecA[2] = nodes[cellCV[i].node[1]].z - nodes[cellCV[i].node[0]].z;

            vecB[0] = nodes[cellCV[i].node[2]].x - nodes[cellCV[i].node[0]].x;
            vecB[1] = nodes[cellCV[i].node[2]].y - nodes[cellCV[i].node[0]].y;
            vecB[2] = nodes[cellCV[i].node[2]].z - nodes[cellCV[i].node[0]].z;

            CrossProduct(vecA,vecB,vecCrossProduct);
            
            if(fabs(DotProduct(vecCrossProduct, vecK)) == 1.00)
            {
                assign = 0 ; //imp
                fprintf(fp,"K %f \n",fabs(DotProduct(vecCrossProduct, vecK)));
                //printf("\n K");
                // ones for one face
                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.x += nodes[cellCV[i].node[j]].x;
                    centroid.y += nodes[cellCV[i].node[j]].y;
                }
                centroid.x = centroid.x / 4;
                centroid.y = centroid.y / 4;
                quickSort(cellCV[i].node, centroid, 0, 3, assign);
                // for second face
                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.x += nodes[cellCV[i].node[4+j]].x;
                    centroid.y += nodes[cellCV[i].node[4+j]].y;
                }
                centroid.x = centroid.x / 4;
                centroid.y = centroid.y / 4;
                quickSort(cellCV[i].node, centroid, 4, 7, assign);

            }
            else if(fabs(DotProduct(vecCrossProduct, vecI)) == 1.00)
            {
                assign = 1;
                //printf("\n I");
                // ones for one face
                fprintf(fp,"I %f \n",fabs(DotProduct(vecCrossProduct, vecI)));

                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.z += nodes[cellCV[i].node[j]].z;
                    centroid.y += nodes[cellCV[i].node[j]].y;
                }
                centroid.z = centroid.z / 4;
                centroid.y = centroid.y / 4;
                quickSort(cellCV[i].node, centroid, 0, 3, assign);
                // for second face
                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.z += nodes[cellCV[i].node[4+j]].z;
                    centroid.y += nodes[cellCV[i].node[4+j]].y;
                }
                centroid.z = centroid.z / 4;
                centroid.y = centroid.y / 4;
                quickSort(cellCV[i].node, centroid, 4, 7, assign);

            }
            else if(fabs(DotProduct(vecCrossProduct, vecJ)) == 1.00)
            {
                assign = 2 ; 
                //printf("\n J");
                // ones for one face
                fprintf(fp,"J %f \n",fabs(DotProduct(vecCrossProduct, vecJ)));
                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.x += nodes[cellCV[i].node[j]].x;
                    centroid.z += nodes[cellCV[i].node[j]].z;
                }
                centroid.z = centroid.z / 4;
                centroid.x = centroid.x / 4;
                quickSort(cellCV[i].node, centroid, 0, 3, assign);
                // for second face
                centroid.x =0;
                centroid.y=0 ;
                centroid.z=0 ;
                for(j=0; j< 4 ; j++)
                {
                    centroid.z += nodes[cellCV[i].node[4+j]].z;
                    centroid.x += nodes[cellCV[i].node[4+j]].x;
                }
                centroid.z = centroid.z / 4;
                centroid.x = centroid.x / 4;
                quickSort(cellCV[i].node, centroid, 4, 7, assign);

            }
            else
            {
                fprintf(fp,"else %f %f %f\n",fabs(DotProduct(vecCrossProduct, vecI)),fabs(DotProduct(vecCrossProduct, vecJ)),fabs(DotProduct(vecCrossProduct, vecK)));
                vectorDotProductArray[0] = fabs(DotProduct(vecCrossProduct, vecI));
                vectorDotProductArray[1] = fabs(DotProduct(vecCrossProduct, vecJ));
                vectorDotProductArray[2] = fabs(DotProduct(vecCrossProduct, vecK));

                minimumVectorDotProductIndexArray = print2Smallest(vectorDotProductArray,3);

                if(isSubset(minimumVectorDotProductIndexArray,subXY,2,2))
                {
                    assign = 0 ; //imp
                    //printf("\n K");
                    // ones for one face
                    centroid.x =0;
                    centroid.y=0 ;
                    centroid.z=0 ;
                    for(j=0; j< 4 ; j++)
                    {
                        centroid.x += nodes[cellCV[i].node[j]].x;
                        centroid.y += nodes[cellCV[i].node[j]].y;
                    }
                    centroid.x = centroid.x / 4;
                    centroid.y = centroid.y / 4;
                    quickSort(cellCV[i].node, centroid, 0, 3, assign);
                    // for second face
                    centroid.x =0;
                    centroid.y=0 ;
                    centroid.z=0 ;
                    for(j=0; j< 4 ; j++)
                    {
                        centroid.x += nodes[cellCV[i].node[4+j]].x;
                        centroid.y += nodes[cellCV[i].node[4+j]].y;
                    }
                    centroid.x = centroid.x / 4;
                    centroid.y = centroid.y / 4;
                    quickSort(cellCV[i].node, centroid, 4, 7, assign);

                }
                if(isSubset(minimumVectorDotProductIndexArray,subXZ,2,2))
                {
                    assign = 2 ; 
                    //printf("\n J");
                    // ones for one face
                    centroid.x =0;
                    centroid.y=0 ;
                    centroid.z=0 ;
                    for(j=0; j< 4 ; j++)
                    {
                        centroid.x += nodes[cellCV[i].node[j]].x;
                        centroid.z += nodes[cellCV[i].node[j]].z;
                    }
                    centroid.z = centroid.z / 4;
                    centroid.x = centroid.x / 4;
                    quickSort(cellCV[i].node, centroid, 0, 3, assign);
                    // for second face
                    centroid.x =0;
                    centroid.y=0 ;
                    centroid.z=0 ;
                    for(j=0; j< 4 ; j++)
                    {
                        centroid.z += nodes[cellCV[i].node[4+j]].z;
                        centroid.x += nodes[cellCV[i].node[4+j]].x;
                    }
                    centroid.z = centroid.z / 4;
                    centroid.x = centroid.x / 4;
                    quickSort(cellCV[i].node, centroid, 4, 7, assign);
                }
                if(isSubset(minimumVectorDotProductIndexArray,subYZ,2,2))
                {
                    assign = 1;
                    //printf("\n I");
                    // ones for one face
                    centroid.x =0;
                    centroid.y=0 ;
                    centroid.z=0 ;
                    for(j=0; j< 4 ; j++)
                    {
                        centroid.z += nodes[cellCV[i].node[j]].z;
                        centroid.y += nodes[cellCV[i].node[j]].y;
                    }
                    centroid.z = centroid.z / 4;
                    centroid.y = centroid.y / 4;
                    quickSort(cellCV[i].node, centroid, 0, 3, assign);
                    // for second face
                    centroid.x =0;
                    centroid.y=0 ;
                    centroid.z=0 ;
                    for(j=0; j< 4 ; j++)
                    {
                        centroid.z += nodes[cellCV[i].node[4+j]].z;
                        centroid.y += nodes[cellCV[i].node[4+j]].y;
                    }
                    centroid.z = centroid.z / 4;
                    centroid.y = centroid.y / 4;
                    quickSort(cellCV[i].node, centroid, 4, 7, assign);
                } */
            }
            
        //}
        if(cellCV[i].type == 5)
        {
            int *nodeArrayPyramid;
            int indexMissingNode;
            for(j=0; j < cellCV[i].nOfFaces; j++)
            {
                if(faces[cellCV[i].face[j]].faceType == 4)
                {
                    
                    nodeArrayPyramid = (int *)calloc(5,sizeof(int));
                    for(int k=0; k < 4; k++)
                    {
                        nodeArrayPyramid[k] = faces[cellCV[i].face[j]].node[k];
                    }
                    indexMissingNode = findMissingNodePyramidIndex(faces[cellCV[i].face[j]].node, nodeArrayPyramid , 5, 4);
                    nodeArrayPyramid[4] = faces[cellCV[i].face[j]].node[indexMissingNode];
                    memcpy(faces[cellCV[i].face[j]].node,nodeArrayPyramid,5*sizeof(int));


                }
                
            }
        }
        // WeDgE CelL
        int counter;
        int *nodeArrayWedge;
        int *tempNodeArray;
        nodeArrayWedge = (int *)calloc(6,sizeof(int));
        int faceNotToCheck ;

        if(cellCV[i].type == 6)
        {
            fprintf(fp,"Cell: %d \n",cellCV[i].index);
            counter =0 ;
            for(j=0;j< cellCV[i].nOfFaces; j++)
            {
                if(faces[cellCV[i].face[j]].faceType == 3)
                {
                    if(counter == 0)
                    {
                        faceNotToCheck = j ;
                    }
                    tempNodeArray = (int *)calloc(3,sizeof(int));
                    memcpy(tempNodeArray,faces[cellCV[i].face[j]].node,faces[cellCV[i].face[j]].faceType * sizeof(int));
                    memcpy(nodeArrayWedge + 3*counter,tempNodeArray,3*sizeof(int));
                    free(tempNodeArray);
                    tempNodeArray = NULL;
                    counter ++ ;
                    
                }
            }
            //
            fprintf(fp,"Original %d %d %d %d %d %d \n", cellCV[i].node[0],cellCV[i].node[1],cellCV[i].node[2],cellCV[i].node[3],cellCV[i].node[4],cellCV[i].node[5]);
            fprintf(fp,"nodeArray %d %d %d %d %d %d \n", nodeArrayWedge[0] -1 ,nodeArrayWedge[1] -1,nodeArrayWedge[2]-1,nodeArrayWedge[3]-1,nodeArrayWedge[4]-1,nodeArrayWedge[5]-1);
            if(counter != 2)
            {
                printf("\n THe no. of triangular faces for wedge cell not correct");
            }
            
            nodeBackCounter = 0;
            nodeFrontCounter = 0;

            while (nodeBackCounter < 3)
            {
                tempArray[0]=nodeArrayWedge[nodeFrontCounter] - 1;
                tempArray[1]=nodeArrayWedge[nodeFrontCounter + 1] -1  ;
                //printf("\n %d %d",tempArray[0],tempArray[1]);
                tempFrontVector = CreateVector(tempArray);
                tempArray[0] += 1;
                tempArray[1] += 1;

                for(j=0;j<cellCV[i].nOfFaces;j++)
                {
                    if(isSubset(faces[cellCV[i].face[j]].node,tempArray,4,2)==1 && j!=faceNotToCheck)
                    {

                        tempBackArray = findMissing(faces[cellCV[i].face[j]].node,tempArray,4,2,4);
                        tempBackArray[0] = tempBackArray[0] - 1;
                        tempBackArray[1] = tempBackArray[1] - 1;
                        tempBackVector = CreateVector(tempBackArray);
                        //printf("\n In %d",i);
                        if(DotProduct(tempFrontVector,tempBackVector)>0)
                        {
                           
                            if(nodeBackCounter == 0)
                            {
                                nodeArrayWedge[3+ nodeBackCounter] = tempBackArray[0] + 1 ;
                                nodeBackCounter ++ ;
                                nodeArrayWedge[3 + nodeBackCounter] = tempBackArray[1] + 1 ;
                                nodeBackCounter ++ ;
                            }
                            else
                            {
                                if(FindIfValuePresent(nodeArrayWedge, tempBackArray[0] + 1 , 3 + nodeBackCounter ) ==0 )
                                {
                                    nodeArrayWedge[3+ nodeBackCounter] = tempBackArray[0] + 1 ;
                                    nodeBackCounter ++ ;
                                }
                                if(FindIfValuePresent(nodeArrayWedge, tempBackArray[1] +1 , 3 + nodeBackCounter ) ==0 )
                                {
                                    nodeArrayWedge[3+ nodeBackCounter] = tempBackArray[1] + 1;
                                    nodeBackCounter ++ ;
                                }
                            }
                            
                            nodeFrontCounter ++;
                        }
                        else
                        {
                            
                            if(nodeBackCounter == 0)
                            {
                                nodeArrayWedge[3 + nodeBackCounter] = tempBackArray[1] + 1;
                                nodeBackCounter ++ ;
                                nodeArrayWedge[3 + nodeBackCounter] = tempBackArray[0] + 1;
                                nodeBackCounter ++ ;
                            }
                            else
                            {
                                if(FindIfValuePresent(nodeArrayWedge, tempBackArray[1] + 1 , 3 + nodeBackCounter ) ==0 )
                                {
                                    nodeArrayWedge[3 + nodeBackCounter] = tempBackArray[1] + 1 ;
                                    nodeBackCounter ++ ;
                                }
                                if(FindIfValuePresent(nodeArrayWedge, tempBackArray[0] + 1, 3 + nodeBackCounter ) ==0 )
                                {
                                    nodeArrayWedge[3 + nodeBackCounter] = tempBackArray[0] + 1;
                                    nodeBackCounter ++ ;
                                }
                            }
                            
                            nodeFrontCounter ++;
                        }
                        break;
                    }
                }
            }
            

            for(j=0; j < cellCV[i].nOfNodes; j++)
            {
                nodeArrayWedge[j] -= 1;
            }
            /* if(isSubset(cellCV[i].node , nodeArrayWedge ,6 ,6) != 1)
            {
                printf("\n Issues");
            } */
            memcpy(cellCV[i].node, nodeArrayWedge, cellCV[i].nOfNodes * sizeof(int));
            fprintf(fp,"Sorting %d %d %d %d %d %d \n", cellCV[i].node[0],cellCV[i].node[1],cellCV[i].node[2],cellCV[i].node[3],cellCV[i].node[4],cellCV[i].node[5]);

            /* int *nodeArrayWedge;
            counter = 0; 
            for(j=0; j < cellCV[i].nOfFaces; j++)
            {
                
                if(faces[cellCV[i].face[j]].faceType == 3 && counter ==0)
                {
                    counter ++ ; 
                    double vecA[3],vecB[3],vecCrossProduct[3];;
                    vecA[0] = nodes[cellCV[i].node[1]].x - nodes[cellCV[i].node[0]].x;
                    vecA[1] = nodes[cellCV[i].node[1]].y - nodes[cellCV[i].node[0]].y;
                    vecA[2] = nodes[cellCV[i].node[1]].z - nodes[cellCV[i].node[0]].z;

                    vecB[0] = nodes[cellCV[i].node[2]].x - nodes[cellCV[i].node[0]].x;
                    vecB[1] = nodes[cellCV[i].node[2]].y - nodes[cellCV[i].node[0]].y;
                    vecB[2] = nodes[cellCV[i].node[2]].z - nodes[cellCV[i].node[0]].z;
                    
                    CrossProduct(vecA,vecB,vecCrossProduct);

                    vectorDotProductArray[0] = abs(DotProduct(vecCrossProduct, vecI));
                    vectorDotProductArray[1] = abs(DotProduct(vecCrossProduct, vecJ));
                    vectorDotProductArray[2] = abs(DotProduct(vecCrossProduct, vecK));

                    minimumVectorDotProductIndexArray = print2Smallest(vectorDotProductArray,3);

                    if(isSubset(minimumVectorDotProductIndexArray,subXY,2,2))
                    {
                        assign = 0 ; //imp
                        //printf("\n K");
                        // ones for one face
                        centroid.x =0;
                        centroid.y=0 ;
                        centroid.z=0 ;
                        for(j=0; j< 3 ; j++)
                        {
                            centroid.x += nodes[cellCV[i].node[j]].x;
                            centroid.y += nodes[cellCV[i].node[j]].y;
                        }
                        centroid.x = centroid.x / 3;
                        centroid.y = centroid.y / 3;
                        quickSort(cellCV[i].node, centroid, 0, 2, assign);
                        // for second face
                        centroid.x =0;
                        centroid.y=0 ;
                        centroid.z=0 ;
                        for(j=0; j< 3 ; j++)
                        {
                            centroid.x += nodes[cellCV[i].node[3+j]].x;
                            centroid.y += nodes[cellCV[i].node[3+j]].y;
                        }
                        centroid.x = centroid.x / 3;
                        centroid.y = centroid.y / 3;
                        quickSort(cellCV[i].node, centroid, 3, 5, assign);

                    }
                    if(isSubset(minimumVectorDotProductIndexArray,subXZ,2,2))
                    {
                        assign = 2 ; 
                        //printf("\n J");
                        // ones for one face
                        centroid.x =0;
                        centroid.y=0 ;
                        centroid.z=0 ;
                        for(j=0; j< 3 ; j++)
                        {
                            centroid.x += nodes[cellCV[i].node[j]].x;
                            centroid.z += nodes[cellCV[i].node[j]].z;
                        }
                        centroid.z = centroid.z / 3;
                        centroid.x = centroid.x / 3;
                        quickSort(cellCV[i].node, centroid, 0, 2, assign);
                        // for second face
                        centroid.x =0;
                        centroid.y=0 ;
                        centroid.z=0 ;
                        for(j=0; j< 3 ; j++)
                        {
                            centroid.z += nodes[cellCV[i].node[3+j]].z;
                            centroid.x += nodes[cellCV[i].node[3+j]].x;
                        }
                        centroid.z = centroid.z / 3;
                        centroid.x = centroid.x / 3;
                        quickSort(cellCV[i].node, centroid, 3, 5, assign);
                    }
                    if(isSubset(minimumVectorDotProductIndexArray,subYZ,2,2))
                    {
                        assign = 1;
                        //printf("\n I");
                        // ones for one face
                        centroid.x =0;
                        centroid.y=0 ;
                        centroid.z=0 ;
                        for(j=0; j< 3 ; j++)
                        {
                            centroid.z += nodes[cellCV[i].node[j]].z;
                            centroid.y += nodes[cellCV[i].node[j]].y;
                        }
                        centroid.z = centroid.z / 3;
                        centroid.y = centroid.y / 3;
                        quickSort(cellCV[i].node, centroid, 0, 2, assign);
                        // for second face
                        centroid.x =0;
                        centroid.y=0 ;
                        centroid.z=0 ;
                        for(j=0; j< 3 ; j++)
                        {
                            centroid.z += nodes[cellCV[i].node[3+j]].z;
                            centroid.y += nodes[cellCV[i].node[3+j]].y;
                        }
                        centroid.z = centroid.z / 3;
                        centroid.y = centroid.y / 3;
                        quickSort(cellCV[i].node, centroid, 3, 5, assign);
                    } */
                    /* nodeArrayWedge = (int *)calloc(6,sizeof(int));
                    for(int k=0; k < 3; k++)
                    {
                        nodeArrayWedge[counter + k] = faces[cellCV[i].face[j]].node[k];
                        

            
                    }
                    counter = 3;
 */
           /*      }
            }
        } */
       
        
    }
    
    }
    fclose(fp);
}
/* *************************************************************** *\
Cell Data Process ends
......................
......................
......................
WrItInG in vTU
\* *************************************************************** */


void WriteMeshToVTU()
{
    int i,j;
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
        fprintf(fp,"%s %f %f %f \n",TAB5,nodes[i].x, nodes[i].y , nodes[i].z);
    }
    fprintf(fp,"%s </DataArray> \n",TAB4);
    fprintf(fp,"%s </Points> \n",TAB3);
    fprintf(fp,"%s <Cells> \n",TAB3);
    fprintf(fp,"%s <DataArray type=\"Int32\" Name=\"connectivity\" Format=\"ascii\"> \n",TAB4);
    for(i=0; i< totalNumberOfCells; i++)
    {
        fprintf(fp,"%s ",TAB5);
        for(j=0; j < cellCV[i].nOfNodes; j++)
        {
            fprintf(fp,"%d ",cellCV[i].node[j]);
        }
        fprintf(fp,"\n ");
        /* if(cellCV[i].type == 2)
        {
            fprintf(fp,"%s ",TAB5);
            for(j=0; j < cellCV[i].nOfNodes; j++)
            {
                fprintf(fp,"%d ",cellCV[i].node[j]);
            }
            fprintf(fp,"\n ");
        }
        if(cellCV[i].type == 4)
        {
            fprintf(fp,"%s ",TAB5);
            for(j=0; j < cellCV[i].nOfNodes; j++)
            {
                fprintf(fp,"%d ",cellCV[i].node[j]);
            }
            fprintf(fp,"\n "); 
        }*/
    }
    fprintf(fp,"%s </DataArray> \n",TAB4);
    fprintf(fp,"%s <DataArray type=\"Int32\" Name=\"offsets\" Format=\"ascii\"> \n",TAB4);
    fprintf(fp,"%s",TAB5);
    for(i=0; i< totalNumberOfCells; i++)
    {
        offsetCount += cellCV[i].nOfNodes;
        fprintf(fp,"%d ",offsetCount);
    }
    fprintf(fp,"\n");
    fprintf(fp,"%s </DataArray> \n",TAB4);
    fprintf(fp,"%s <DataArray type=\"Int32\" Name=\"types\" Format=\"ascii\"> \n",TAB4);
    fprintf(fp,"%s",TAB5);
    for(i=0; i< totalNumberOfCells; i++)
    {
        if(cellCV[i].type == 2)
        {
            fprintf(fp,"%d ",10);
        }
        if(cellCV[i].type == 4 )
        {
            fprintf(fp,"%d ",12);
        }
        if(cellCV[i].type == 5 )
        {
            fprintf(fp,"%d ",14);
        }
        if(cellCV[i].type == 6 )
        {
            fprintf(fp,"%d ",13);
        }
        
    }
    fprintf(fp,"\n");
    fprintf(fp,"%s </DataArray> \n",TAB4);
    fprintf(fp,"%s </Cells> \n",TAB3);
    fprintf(fp,"%s </Piece> \n",TAB2);
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
    fprintf(fp,"\n %-14s %-14s %-14s %-14s %-14s %-14s %-14s %-14s %-14s %-14s %-14s ","Index","NodeCount","Node0","Node1","Node2","Node3","FaceCount","Face0","Face1","Face2","Face3");
    fprintf(fp,"\n ");
    int i,j;
    for(i=0; i < totalNumberOfCells; i++)
    {
        fprintf(fp,"%-10d \t %-10d", cellCV[i].index,cellCV[i].nodeCount);
        for(j=0; j<cellCV[i].nOfNodes; j++)
        {
            fprintf(fp," \t %-10d  ", cellCV[i].node[j]  );
        }
        fprintf(fp,"\t %-10d ", cellCV[i].faceCount);
        for(j=0; j<cellCV[i].nOfFaces; j++)
        {
            fprintf(fp," \t %-10d  ", cellCV[i].face[j]  );
        }
        fprintf(fp,"\t %-10d",cellCV[i].type);
        
        fprintf(fp,"\n");    
        
        
    }
}

void printNodes (coordinate3D *node , int totNodes)
{
    FILE *fp;
    fp= fopen("Output/Nodes_data.txt","w");

    fprintf(fp,"\n \n Printing node data");
    fprintf(fp,"\n ");
    fprintf(fp, "\n Index \t Node.x \t Node.y \t Node.z");
    int i;
    for(i=0; i< totNodes; i++)
    {
        fprintf(fp," \n %d \t %f \t %f \t %f",i, node[i].x , node[i].y, node[i].z );
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
    fprintf(fp2,"\n Index \t Type \t Node1 \t Node2 \t Node3 \t Node4 \t CellId1 \t CellId2");
    for(i=0; i<totFaces ;i++)
	{
        if(face[i].faceType == 3)
        {
            fprintf(fp2," \n %d \t %d \t %ld \t %ld \t %ld \t %ld \t %ld", face[i].index , face[i].faceType , face[i].node[0] , face[i].node[1] , face[i].node[2] , face[i].cellid[0], face[i].cellid[1]);
        }
        if(face[i].faceType == 4)
        {
            fprintf(fp2," \n %d \t %d \t %ld \t %ld \t %ld \t %ld \t %ld \t %ld", face[i].index , face[i].faceType , face[i].node[0] , face[i].node[1] , face[i].node[2] , face[i].node[3], face[i].cellid[0], face[i].cellid[1]);
        }
		
	}
    fclose(fp2);
    printf("\n Done");
}