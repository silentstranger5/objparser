#ifndef OBJPARSER

#define OBJPARSER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// material
typedef struct {
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float emissive[3];
    float shininess;
    float refraction;
    float transparency;
    int   illum;
    char  *name;
    char  *map_ambient;
    char  *map_diffuse;
    char  *map_specular;
    char  *map_highlight;
    char  *map_alpha;
    char  *map_bump;
} mtl;

// mtl context data
typedef struct {
    int  nmaterials;
    mtl  *materials;
    char *filename;
} mtlctx;

// obj context data
typedef struct {
    int nmeshes;
    int nvertices;
    int nnormals;
    int ntexcoords;
    int nfaces;
    int nfaceverts;
    int *faces;
    int *meshoffsets;
    int *matindices;
    float *vertices;
    float *normals;
    float *texcoords;
    float *buffer;
    char  *filename;
    mtlctx materials;
} objctx;

// parse obj file
void parse_obj(objctx *ctx, const char *filename);
// print obj context
void objctx_print(objctx *ctx);
// free obj context
void objctx_free(objctx *ctx);

#endif