#include "objparser.h"

const char *objkeys[] = {
    "f", "mtllib", "o", "usemtl", "v", "vn", "vt"
};

enum objkeysenum { F, MTLLIB, O, USEMTL, V, VN, VT };

const char *mtlkeys[] = {
    "Ka", "Kd", "Ke", "Ks", 
    "Ni", "Ns", "d", "illum", 
    "map_Ka", "map_Kd",
    "map_Ks", "map_Ns", 
    "map_bump", "map_d", 
    "newmtl"
};

enum mtlkeysenum {
    KA, KD, KE, KS,
    NI, NS, D, ILLUM,
    MAP_KA, MAP_KD,
    MAP_KS, MAP_NS,
    MAP_BUMP, MAP_D,
    NEWMTL
};

int strcomp(const void *key, const void *elem) {
    const char *skey = *(const char **) key;
    const char *selem = *(const char **) elem;
    return strcmp(skey, selem);
}

int find_objkey(char *s) {
    char *key = strtok(s, " ");
    size_t nkeys = sizeof(objkeys) / sizeof(char *);
    char **kptr = (char **) bsearch(&key, objkeys, nkeys, sizeof(char *), strcomp);
    if (!kptr) {
        return -1;
    }
    return kptr - objkeys;
}

void mtl_filename(objctx *ctx, char *filename) {
    char *directory = strdup(ctx->filename);
    for (char *c = directory; *c; c++) {
        if (*c == '\\') {
            *c = '/';
        }
    }
    if (!strchr(directory, '/')) {
        char **path = &(ctx->materials.filename);
        *path = strdup(filename);
        free(directory);
        return;
    }
    *(strrchr(directory, '/') + 1) = 0;
    char **path = &(ctx->materials.filename);
    *path = calloc(strlen(directory) + strlen(filename) + 1, sizeof(char));
    strcat(*path, directory);
    strcat(*path, filename);
    free(directory);
}

void count_materials(objctx *ctx, char *filename) {
    mtl_filename(ctx, filename);
    FILE *f = fopen(ctx->materials.filename, "r");
    if (!f) {
        fprintf(stderr, "failed to open %s\n", ctx->materials.filename);
        exit(1);
    }
    char s[512];
    while (fgets(s, 512, f)) {
        char *key = strtok(s, " ");
        if (!strcmp(key, "newmtl")) {
            ctx->materials.nmaterials++;
        }
    }
    fclose(f);
}

void count_key(objctx *ctx, char *s) {
    int key = find_objkey(s);
    switch (key) {
    case F:
        int nverts = 0;
        ctx->nfaces++;
        while (strtok(NULL, " ")) {
            nverts++;
        }
        if (nverts > 3) {
            nverts = (nverts - 2) * 3;
        }
        ctx->nfaceverts += nverts;
        break;
    case MTLLIB:
        s = strchr(s, '\0') + 1;
        *strchr(s, '\n') = 0;
        count_materials(ctx, s);
        break;
    case O:
        ctx->nmeshes++;
        break;
    case V:
        ctx->nvertices++;
        break;
    case VN:
        ctx->nnormals++;
        break;
    case VT:
        ctx->ntexcoords++;
        break;
    default:
        break;
    }
}

void objctx_malloc(objctx *ctx) {
    ctx->vertices = malloc(3 * ctx->nvertices * sizeof(float));
    ctx->normals = malloc(3 * ctx->nnormals * sizeof(float));
    ctx->texcoords = malloc(2 * ctx->ntexcoords * sizeof(float));
    ctx->faces = malloc(3 * ctx->nfaceverts * sizeof(int));
    ctx->buffer = calloc(8 * ctx->nfaceverts, sizeof(float));
    ctx->meshoffsets = malloc((ctx->nmeshes + 1) * sizeof(int));
    ctx->matindices = malloc(ctx->nmeshes * sizeof(int));
    ctx->materials.materials = calloc(ctx->materials.nmaterials, sizeof(mtl));
}

void objctx_reset(objctx *ctx) {
    ctx->nvertices = 0;
    ctx->nnormals = 0;
    ctx->ntexcoords = 0;
    ctx->nfaces = 0;
    ctx->nfaceverts = 0;
    ctx->nmeshes = 0;
    ctx->materials.nmaterials = 0;
}

int find_mtlkey(char *s) {
    size_t nkeys = sizeof(mtlkeys) / sizeof(char *);
    char *key = strtok(s, " ");
    char **kptr = (char **) bsearch(&key, mtlkeys, nkeys, sizeof(char *), strcomp);
    if (!kptr) {
        return -1;
    }
    return kptr - mtlkeys;
}

void parse_mtlline(objctx *ctx, char *s) {
    char *mapstr = NULL;
    int key = find_mtlkey(s);
    mtl *mtl = ctx->materials.materials + ctx->materials.nmaterials - 1;
    switch (key) {
    case KA:
        for (int i = 0; i < 3; i++) {
            char *astr = strtok(NULL, " ");
            mtl->ambient[i] = atof(astr);
        }
        break;
    case KD:
        for (int i = 0; i < 3; i++) {
            char *dstr = strtok(NULL, " ");
            mtl->diffuse[i] = atof(dstr);
        }
        break;
    case KE:
        for (int i = 0; i < 3; i++) {
            char *estr = strtok(NULL, " ");
            mtl->emissive[i] = atof(estr);
        }
        break;
    case KS:
        for (int i = 0; i < 3; i++) {
            char *sstr = strtok(NULL, " ");
            mtl->specular[i] = atof(sstr);
        }
        break;
    case NI:
        char *rstr = strtok(NULL, " ");
        mtl->refraction = atof(rstr);
        break;
    case NS:
        char *sstr = strtok(NULL, " ");
        mtl->shininess = atof(sstr);
        break;
    case D:
        char *tstr = strtok(NULL, " ");
        mtl->transparency = atof(tstr);
        break;
    case ILLUM:
        char *istr = strtok(NULL, " ");
        mtl->illum = atoi(istr);
        break;
    case MAP_KA:
        char *mapstr = strtok(NULL, " ");
        mtl->map_ambient = strdup(mapstr);
        break;
    case MAP_KD:
        mapstr = strtok(NULL, " ");
        mtl->map_diffuse = strdup(mapstr);
        break;
    case MAP_KS:
        mapstr = strtok(NULL, " ");
        mtl->map_specular = strdup(mapstr);
        break;
    case MAP_NS:
        mapstr = strtok(NULL, " ");
        mtl->map_highlight = strdup(mapstr);
        break;
    case MAP_BUMP:
        mapstr = strtok(NULL, " ");
        mtl->map_bump = strdup(mapstr);
    case MAP_D:
        mapstr = strtok(NULL, " ");
        mtl->map_alpha = strdup(mapstr);
        break;
    case NEWMTL:
        mtl++;
        ctx->materials.nmaterials++;
        char *name = strtok(NULL, " ");
        *strchr(name, '\n') = 0;
        mtl->name = strdup(name);
        break;
    default:
        break;
    }
}

void parse_materials(objctx *ctx) {
    FILE *f = fopen(ctx->materials.filename, "r");
    char s[512];
    while (fgets(s, 512, f)) {
        parse_mtlline(ctx, s);
    }
    fclose(f);
}

void parse_face(char *s, int *fptr) {
    int i = 0;
    char *t = NULL;
    while (t = strchr(s, '/')) {
        fptr[i++] = atoi(s);
        s = t + 1;
    }
    fptr[i++] = atoi(s);
}

void parse_objline(objctx *ctx, char *s) {
    int key = find_objkey(s);
    switch (key) {
    case F:
        int nvert = 0;
        char *fverts = NULL;
        int *fptr = ctx->faces + 3 * ctx->nfaceverts;
        int *fvptr = fptr;
        while(fverts = strtok(NULL, " ")) {
            parse_face(fverts, fvptr);
            fvptr += 3;
            nvert++;
        }
        if (nvert > 3) {
            int *fcptr = malloc(3 * nvert * sizeof(int));
            memcpy(fcptr, fptr, 3 * nvert * sizeof(int));
            for (int i = 0; i < nvert - 2; i++) {
                memcpy(fptr + 9 * i + 0, fcptr + 3 * 0, 3 * sizeof(int));
                memcpy(fptr + 9 * i + 3, fcptr + 3 * (i + 1), 3 * sizeof(int));
                memcpy(fptr + 9 * i + 6, fcptr + 3 * (i + 2), 3 * sizeof(int));
            }
            free(fcptr);
            nvert = (nvert - 2) * 3;
        }
        ctx->nfaceverts += nvert;
        ctx->nfaces++;
        break;
    case O:
        ctx->meshoffsets[ctx->nmeshes++] = ctx->nfaceverts;
        break;
    case USEMTL:
        char *mtlname = strtok(NULL, " ");
        *strchr(mtlname, '\n') = 0;
        for (int i = 0; i < ctx->materials.nmaterials; i++) {
            if (!strcmp(mtlname, ctx->materials.materials[i].name)) {
                ctx->matindices[ctx->nmeshes - 1] = i;
            }
        }
        break;
    case V:
        float *vptr = ctx->vertices + 3 * ctx->nvertices++;
        for (int i = 0; i < 3; i++) {
            char *verts = strtok(NULL, " ");
            float vert = atof(verts);
            vptr[i] = vert;
        }
        break;
    case VN:
        float *nptr = ctx->normals + 3 * ctx->nnormals++;
        for (int i = 0; i < 3; i++) {
            char *norms = strtok(NULL, " ");
            float norm = atof(norms);
            nptr[i] = norm;
        }
        break;
    case VT:
        float *tptr = ctx->texcoords + 2 * ctx->ntexcoords++;
        for (int i = 0; i < 2; i++) {
            char *texs = strtok(NULL, " ");
            float tex = atof(texs);
            tptr[i] = tex;
        }
        break;
    default:
        break;
    }
}

void build_buffer(objctx *ctx) {
    for (int i = 0; i < ctx->nfaceverts; i++) {
        float *bptr = ctx->buffer + 8 * i;
        int *fptr = ctx->faces + 3 * i;
        int vert = fptr[0];
        int texc = fptr[1];
        int norm = fptr[2];
        if (vert) {
            memcpy(bptr, ctx->vertices + 3 * (vert - 1), 3 * sizeof(float));
        }
        if (texc) {
            memcpy(bptr + 3, ctx->texcoords + 2 * (texc - 1), 2 * sizeof(float));
        }
        if (norm) {
            memcpy(bptr + 5, ctx->normals + 3 * (norm - 1), 3 * sizeof(float));
        }
    }
}

void mtl_print(objctx *ctx) {
    printf("materials:\n");
    for (int i = 0; i < ctx->materials.nmaterials; i++) {
        mtl *mtl = ctx->materials.materials + i;
        printf("name:\t\t%s\n", mtl->name);
        printf("ambient:\t[ ");
        for (int j = 0; j < 3; j++) {
            printf("%8.4f ", mtl->ambient[j]);
        }
        printf("]\n");
        printf("diffuse:\t[ ");
        for (int j = 0; j < 3; j++) {
            printf("%8.4f ", mtl->diffuse[j]);
        }
        printf("]\n");
        printf("specular:\t[ ");
        for (int j = 0; j < 3; j++) {
            printf("%8.4f ", mtl->specular[j]);
        }
        printf("]\n");
        printf("emissive:\t[ ");
        for (int j = 0; j < 3; j++) {
            printf("%8.4f ", mtl->emissive[j]);
        }
        printf("]\n");
        printf("shininess:\t%10.4f\n", mtl->shininess);
        printf("refraction:\t%10.4f\n", mtl->refraction);
        printf("transparency:\t%10.4f\n", mtl->transparency);
        printf("illum:\t\t%5d\n", mtl->illum);
        if (mtl->map_ambient) {
            printf("ambient map: %s\n", mtl->map_ambient);
        }
        if (mtl->map_diffuse) {
            printf("diffuse map: %s\n", mtl->map_diffuse);
        }
        if (mtl->map_specular) {
            printf("specular map: %s\n", mtl->map_specular);
        }
        if (mtl->map_highlight) {
            printf("highlight map: %s\n", mtl->map_highlight);
        }
        if (mtl->map_alpha) {
            printf("alpha map: %s\n", mtl->map_alpha);
        }
        if (mtl->map_bump) {
            printf("bump map: %s\n", mtl->map_bump);
        }
    }
}

void objctx_print(objctx *ctx) {
    printf("nmeshes: %d\n", ctx->nmeshes);
    printf("nvertices: %d\n", ctx->nvertices);
    printf("nnormals: %d\n", ctx->nnormals);
    printf("ntexcoords: %d\n", ctx->ntexcoords);
        printf("vertices:\n");
    printf("%4d ", 0);
    for (int i = 0; i < ctx->nvertices; i++) {
        if (i > 0 && (i % 3) == 0) {
            printf("\n%4d ", i);
        }
        printf("[ ");
        for (int j = 0; j < 3; j++) {
            printf("%8.4f ", ctx->vertices[i * 3 + j]);
        }
        printf("] ");
    }
    putchar('\n');
    printf("normals:\n");
    printf("%4d ", 0);
    for (int i = 0; i < ctx->nnormals; i++) {
        if (i > 0 && (i % 3) == 0) {
            printf("\n%4d ", i);
        }
        printf("[ ");
        for (int j = 0; j < 3; j++) {
            printf("%8.4f ", ctx->normals[i * 3 + j]);
        }
        printf("] ");
    }
    putchar('\n');
    printf("texcoords:\n");
    printf("%4d ", 0);
    for (int i = 0; i < ctx->ntexcoords; i++) {
        if (i > 0 && (i % 3) == 0) {
            printf("\n%4d ", i);
        }
        printf("[ ");
        for (int j = 0; j < 2; j++) {
            printf("%8.4f ", ctx->texcoords[i * 2 + j]);
        }
        printf("] ");
    }
    putchar('\n');
    printf("mesh offsets:\n");
    for (int i = 0; i < ctx->nmeshes + 1; i++) {
        if (i > 0 && (i % 16) == 0) {
            putchar('\n');
        }
        printf("%4d ", ctx->meshoffsets[i]);
    }
    putchar('\n');
    printf("faces:\n");
    printf("%4d ", 0);
    for (int i = 0; i < ctx->nfaceverts; i++) {
        if (i > 0 && (i % 3) == 0) {
            printf("\n%4d ", i);
        }
        printf("[ ");
        for (int j = 0; j < 3; j++) {
            printf("%4d ", ctx->faces[i * 3 + j]);
        }
        printf("] ");
    }
    putchar('\n');
    printf("buffer:\n");
    for (int i = 0; i < ctx->nfaceverts; i++) {
        printf("%4d ", i);
        int nd[] = {3, 2, 3};
        int offs[] = {0, 3, 5};
        for (int j = 0; j < 3; j++) {
            printf("[ ");
            for (int k = 0; k < nd[j]; k++) {
                printf("%8.4f ", ctx->buffer[8 * i + offs[j] + k]);
            }
            printf("] ");
        }
        putchar('\n');
    }
    printf("material indices:\n");
    for (int i = 0; i < ctx->nmeshes; i++) {
        if (i > 0 && (i % 16 == 0)) {
            putchar('\n');
        }
        printf("%4d ", ctx->matindices[i]);
    }
    putchar('\n');
    mtl_print(ctx);
}

void mtlctx_free(mtlctx *ctx) {
    free(ctx->filename);
    for (int i = 0; i < ctx->nmaterials; i++) {
        mtl *mtl = ctx->materials + i;
        if (mtl->map_ambient) {
            free(mtl->map_ambient);
        }
        if (mtl->map_diffuse) {
            free(mtl->map_diffuse);
        }
        if (mtl->map_specular) {
            free(mtl->map_specular);
        }
        if (mtl->map_highlight) {
            free(mtl->map_highlight);
        }
        if (mtl->map_alpha) {
            free(mtl->map_alpha);
        }
        if (mtl->map_bump) {
            free(mtl->map_bump);
        }
        free(mtl->name);
    }
    free(ctx->materials);
}

void objctx_free(objctx *ctx) {
    free(ctx->vertices);
    free(ctx->normals);
    free(ctx->texcoords);
    free(ctx->faces);
    free(ctx->buffer);
    free(ctx->meshoffsets);
    free(ctx->matindices);
    mtlctx_free(&ctx->materials);
}

void parse_obj(objctx *ctx, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "failed to open file %s\n", filename);
        exit(1);
    }
    char s[512];
    ctx->filename = (char *) filename;
    while (fgets(s, 512, f)) {
        count_key(ctx, s);
    }
    objctx_malloc(ctx);
    objctx_reset(ctx);
    parse_materials(ctx);
    rewind(f);
    while (fgets(s, 512, f)) {
        parse_objline(ctx, s);
    }
    ctx->meshoffsets[ctx->nmeshes] = ctx->nfaceverts;
    build_buffer(ctx);
    fclose(f);
}