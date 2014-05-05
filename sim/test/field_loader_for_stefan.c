#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>


unsigned int iNumNodes = 0;
double *pos = 0;
double *field = 0;

double dXMin = 9999999.;
double dXMax = -9999999.;
double dYMin = 9999999.;
double dYMax = -9999999.;
double dZMin = 9999999.;
double dZMax = -9999999.;

double dXStep = 0.5;
double dYStep = 0.05;
double dZStep = 0.05;


/*********************************************************************************/
/* Interpolations-Zeugs */

double vec_angle(const double *pVec1, const double *pVec2)
{
	double dot = 0.;
	double len1 = 0.;
	double len2 = 0.;

	unsigned int i;
	for(i=0; i<3; ++i)
	{
			dot += pVec1[i]*pVec2[i];

			len1 += pVec1[i]*pVec1[i];
			len2 += pVec2[i]*pVec2[i];
	}

	len1 = sqrt(len1);
	len2 = sqrt(len2);

	dot /= len1;
	dot /= len2;

	return acos(dot);
}


void lerp(const double *pVec1, const double *pVec2, double t, double *pVecRet)
{
	unsigned int i=0;
	for(i=0; i<3; ++i)
	{
		pVecRet[i] = pVec1[i] + (pVec2[i] - pVec1[i])*t;
	}
}

void slerp(const double *pVec1, const double *pVec2, double t, double *pVecRet)
{
	double angle = vec_angle(pVec1, pVec2);

	double dSkalar1 = sin((1.-t)*angle)/sin(angle);
	double dSkalar2 = sin(t*angle)/sin(angle);

	unsigned int i=0;
	for(i=0; i<3; ++i)
	{
		pVecRet[i] = dSkalar1*pVec1[i] + dSkalar2*pVec2[i];
	}
}

/*********************************************************************************/



void get_node_count(char *pLine, ssize_t iSize, unsigned int* iNumNodes)
{
	if(iSize < 8)
		return;

	if(pLine[2]=='N' && pLine[3]=='o' && pLine[4]=='d' && pLine[5]=='e' && pLine[6]=='s')
	{
		char *pNumber = pLine + 8;
		sscanf(pNumber, "%d", iNumNodes);
	}
}

void interpolate(double x, double y, double z, 
				 double xmid, double ymid, double zmid, 
				const double *dB_1, const double *dB_2,
				const double *dB_3, const double *dB_4,
				const double *dB_5, const double *dB_6,
				const double *dB_7, const double *dB_8,
				double *pB)
{
	double dX_1 = xmid-dXStep;
	double dX_2 = xmid+dXStep;

	double dY_1 = ymid-dYStep;
	double dY_2 = ymid+dYStep;

	double dZ_1 = zmid-dZStep;
	double dZ_2 = zmid+dZStep;


	/* welche Interpolation benutzen? */
	void (*interp)(const double*, const double*, double, double*) 
			= slerp;
			/*= lerp;*/

	double tx = (x - dX_1) / (dX_2 - dX_1);
	double ty = (y - dY_1) / (dY_2 - dY_1);
	double tz = (z - dZ_1) / (dZ_2 - dZ_1);

	/*printf("t: %lg %lg %lg\n", tx, ty, tz);*/


	/* Würfel-Vorderseite, Punkte 1,2,3,4*/
	double dB_12[3];
	interp(dB_1, dB_2, tx, dB_12);
	double dB_34[3];
	interp(dB_3, dB_4, tx, dB_34);

	double dB_12_34[3];
	interp(dB_12, dB_34, ty, dB_12_34);


	/* Würfel-Rüchseite, Punkte 5,6,7,8 */
	double dB_56[3];
	interp(dB_5, dB_6, tx, dB_56);
	double dB_78[3];
	interp(dB_7, dB_8, tx, dB_78);

	double dB_56_78[3];
	interp(dB_56, dB_78, ty, dB_56_78);


	/* Finale Interpolation */
	interp(dB_12_34, dB_56_78, tz, pB);
}

void get_field(double x, double y, double z, double *pB)
{
	int iXLen = (int)((dXMax-dXMin)/dXStep)+1;
	int iYLen = (int)((dYMax-dYMin)/dYStep)+1;
	int iZLen = (int)((dZMax-dZMin)/dZStep)+1;

	int iXMid = (int)((x - dXMin)/dXStep);
	int iYMid = (int)((y - dYMin)/dYStep);
	int iZMid = (int)((z - dZMin)/dZStep);

	int iIdxMid = (iZMid*iYLen*iXLen + iYMid*iXLen + iXMid)*3;

	/**pBx = field[iIdxMid+0];
	*pBy = field[iIdxMid+1];
	*pBz = field[iIdxMid+2];*/

	/*printf("Middle pos: %lg %lg %lg\n", pos[iIdxMid], pos[iIdxMid+1], pos[iIdxMid+2]);
	printf("Middle field: %lg %lg %lg\n", field[iIdxMid], field[iIdxMid+1], field[iIdxMid+2]);*/


	int iIdx_1 = ((iZMid-1)*iYLen*iXLen + (iYMid-1)*iXLen + (iXMid-1))*3;
	int iIdx_2 = ((iZMid-1)*iYLen*iXLen + (iYMid-1)*iXLen + (iXMid+1))*3;
	int iIdx_3 = ((iZMid-1)*iYLen*iXLen + (iYMid+1)*iXLen + (iXMid-1))*3;
	int iIdx_4 = ((iZMid-1)*iYLen*iXLen + (iYMid+1)*iXLen + (iXMid+1))*3;

	int iIdx_5 = ((iZMid+1)*iYLen*iXLen + (iYMid-1)*iXLen + (iXMid-1))*3;
	int iIdx_6 = ((iZMid+1)*iYLen*iXLen + (iYMid-1)*iXLen + (iXMid+1))*3;
	int iIdx_7 = ((iZMid+1)*iYLen*iXLen + (iYMid+1)*iXLen + (iXMid-1))*3;
	int iIdx_8 = ((iZMid+1)*iYLen*iXLen + (iYMid+1)*iXLen + (iXMid+1))*3;


	/*printf("Indices: %d, %d, %d, %d, %d, %d, %d, %d\n",
		iIdx_1, iIdx_2, iIdx_3, iIdx_4, iIdx_5, iIdx_6, iIdx_7, iIdx_8);*/

	if(iIdx_1 >= iNumNodes*3 || iIdx_1 < 0 || 
		iIdx_2 >= iNumNodes*3 || iIdx_2 < 0 || 
		iIdx_3 >= iNumNodes*3 || iIdx_3 < 0 || 
		iIdx_4 >= iNumNodes*3 || iIdx_4 < 0 || 
		iIdx_5 >= iNumNodes*3 || iIdx_5 < 0 || 
		iIdx_6 >= iNumNodes*3 || iIdx_6 < 0 ||
		iIdx_7 >= iNumNodes*3 || iIdx_7 < 0 ||
		iIdx_8 >= iNumNodes*3 || iIdx_8 < 0)
	{
		printf("Index out of bounds.\n");
		return;
	}

	double dB_1[3], dB_2[3], dB_3[3], dB_4[3], dB_5[3], dB_6[3], dB_7[3], dB_8[3];

	dB_1[0] = field[iIdx_1 + 0]; dB_1[1] = field[iIdx_1 + 1]; dB_1[2] = field[iIdx_1 + 2];
	dB_2[0] = field[iIdx_2 + 0]; dB_2[1] = field[iIdx_2 + 1]; dB_2[2] = field[iIdx_2 + 2];
	dB_3[0] = field[iIdx_3 + 0]; dB_3[1] = field[iIdx_3 + 1]; dB_3[2] = field[iIdx_3 + 2];
	dB_4[0] = field[iIdx_4 + 0]; dB_4[1] = field[iIdx_4 + 1]; dB_4[2] = field[iIdx_4 + 2];
	dB_5[0] = field[iIdx_5 + 0]; dB_5[1] = field[iIdx_5 + 1]; dB_5[2] = field[iIdx_5 + 2];
	dB_6[0] = field[iIdx_6 + 0]; dB_6[1] = field[iIdx_6 + 1]; dB_6[2] = field[iIdx_6 + 2];
	dB_7[0] = field[iIdx_7 + 0]; dB_7[1] = field[iIdx_7 + 1]; dB_7[2] = field[iIdx_7 + 2];
	dB_8[0] = field[iIdx_8 + 0]; dB_8[1] = field[iIdx_8 + 1]; dB_8[2] = field[iIdx_8 + 2];

	/*printf("pos: %lg %lg %lg,\t mid: %lg %lg %lg\n", x,y,z, pos[iIdxMid+0],pos[iIdxMid+1],pos[iIdxMid+2]);*/
	interpolate(x,y,z, pos[iIdxMid+0],pos[iIdxMid+1],pos[iIdxMid+2], dB_1,dB_2,dB_3,dB_4,dB_5,dB_6,dB_7,dB_8, pB);
}


void unload_field()
{
	if(pos) free(pos);
	if(field) free(field);
}

int load_field_file(const char* pcFile)
{
	FILE *pf = fopen(pcFile, "rt");
	if(pf == 0)
	{
		printf("Cannot open file \"%s\"\n", pcFile);
		return 0;
	}

	unsigned int iCurNode = 0;

	while(1)
	{
		char *pLine = 0;
		ssize_t iSize = 0;
		ssize_t iRead = getline(&pLine, &iSize, pf);
		if(iRead == -1 || feof(pf))
			break;

		/* Kommentare ignorieren */
		if(pLine[0] == '%')
		{
			if(iNumNodes == 0)
				get_node_count(pLine, iSize, &iNumNodes);

			free(pLine);
			continue;
		}

		if(iNumNodes == 0)
		{
			printf("Number of nodes not found!\n");
			break;
		}


		if(pos == 0 && field == 0)
		{
			printf("Number of nodes: %d\n", iNumNodes);

			pos = (double*)malloc(sizeof(double) * iNumNodes * 3);
			field = (double*)malloc(sizeof(double) * iNumNodes * 3);
		}

		/*if((iCurNode % 1000) == 0)
		{
			printf("Current node: %d\n", iCurNode);
			fflush(stdout);
		}*/

		/* printf("%s", pLine); */
		double *curpos = pos + iCurNode*3;
		double *curfield = field + iCurNode*3;

		sscanf(pLine, "%lg %lg %lg %lg %lg %lg",
				&curpos[0], &curpos[1], &curpos[2],
				&curfield[0], &curfield[1], &curfield[2]);

		/* x,y,z - Minima und -Maxima */
		dXMin = fmin(curpos[0], dXMin);
		dXMax = fmax(curpos[0], dXMax);
		dYMin = fmin(curpos[1], dYMin);
		dYMax = fmax(curpos[1], dYMax);
		dZMin = fmin(curpos[2], dZMin);
		dZMax = fmax(curpos[2], dZMax);

		/*printf("%lg %lg %lg\n", curpos[0], curpos[1], curpos[2]);
		printf("%lg %lg %lg\n", curfield[0], curfield[1], curfield[2])*/;

		free(pLine);
		++iCurNode;
	}

	printf("Ranges: x=%lg..%lg, y=%lg..%lg, z=%lg..%lg\n", 
			dXMin, dXMax, dYMin, dYMax, dZMin, dZMax);

	printf("Sizes: %lg, %lg, %lg\n", 
			(dXMax-dXMin)/dXStep + 1., 
			(dYMax-dYMin)/dYStep + 1., 
			(dZMax-dZMin)/dZStep + 1.);

	fclose(pf);
	return 1;
}


// McStas-kompatible Feldfunktion
void fieldfkt(double x, double y, double z, double t, 
		double *pBx, double *pBy, double *pBz)
{
	double dB[3];
	get_field(x,y,z, dB);

	*pBx = dB[0];
	*pBy = dB[1];
	*pBz = dB[2];
}


int main()
{
	load_field_file("data.txt");

	double dB[3];

	double dX = 0.;
	for(dX=69.; dX<71.; dX+=0.1)
	{
		get_field(dX, 0., 0., dB);
		printf("Interpolated field: %lg %lg %lg\n", dB[0], dB[1], dB[2]);
	}

	unload_field();
	return 0;
}
