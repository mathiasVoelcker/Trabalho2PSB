// **********************************************************************
//	BVHViewer.c
//  Desenha e anima um esqueleto a partir de um arquivo BVH (BioVision)
//  Marcelo Cohen
//  marcelo.cohen@pucrs.br
// **********************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>    // somente no Windows
#include "gl/glut.h"
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

typedef struct Node Node;

struct Node {
    char name[20];       // nome
    float offset[3];     // offset (deslocamento)
    int channels;        // qtd de canais (3 ou 6)
    float* channelData;  // vetor com os dados dos canais
    int numChildren;     // qtd de filhos
    Node** children;     // vetor de ponteiros para os filhos
    Node* parent;        // ponteiro para o pai
};

// Raiz da hierarquia
Node* root;

// Total de frames
int totalFrames = 282;

// Frame atual
int curFrame = 0;

// Funcoes para liberacao de memoria da hierarquia
void freeTree();
void freeNode(Node* node);

// Variaveis globais para manipulacao da visualizacao 3D
int width,height;
float deltax=0, deltay=0;
GLfloat angle=60, fAspect=1.0;
GLfloat rotX=0, rotY=0, rotX_ini=0, rotY_ini=0;
GLfloat ratio;
GLfloat angY, angX;
int x_ini=0,y_ini=0,bot=0;
float Obs[3] = {0,0,-500};
float Alvo[3];
float ObsIni[3];

// **********************************************************************
//  Cria um nodo novo para a hierarquia, fazendo também a ligacao com
//  o seu pai (se houver)
//  Parametros:
//  - name: string com o nome do nodo
//  - parent: ponteiro para o nodo pai (NULL se for a raiz)
//  - numChannels: quantidade de canais de transformacao (3 ou 6)
//  - ofx, ofy, ofz: offset (deslocamento) lido do arquivo
//  - numChildren: quantidade de filhos que serao inseridos posteriormente
// **********************************************************************
Node* createNode(char name[20], Node* parent, int numChannels, float ofx, float ofy, float ofz, int numChildren)
{
    Node* aux = malloc(sizeof(Node));
    aux->channels = numChannels;
    aux->channelData = calloc(sizeof(float), numChannels);
    strcpy(aux->name, name);
    aux->offset[0] = ofx;
    aux->offset[1] = ofy;
    aux->offset[2] = ofz;
    aux->numChildren = numChildren;
    if(numChildren > 0)
        aux->children = calloc(sizeof(Node*), numChildren);
    else
        aux->children = NULL;
    aux->parent = parent;
    if(parent)
        for(int i=0; i<parent->numChildren; i++)
            if(!parent->children[i]) {
                //                printf("Insert at parent: %d\n", i);
                parent->children[i] = aux;
                break;
            }
    printf("Created %s\n", name);
    return aux;
}

//
// DADOS DE EXEMPLO DO PRIMEIRO FRAME
//
float animData[282][69];

float data[] = { -326.552, 98.7701, 317.634, 71.4085, 60.8487, 17.2406, -70.1915, 0, 88.8779, 84.6529, 68.0632,
                 -5.27801, 0.719492, 15.2067, 13.3733, -135.039, 24.774, 172.053, -171.896, 64.9682, -165.105,
                 3.6548, 1.03593, -36.4128, -55.7886, 37.8019, -120.338, 9.39682, 14.0503, -27.1815, 4.41274,
                 -0.125185, -1.52942, 1.33299, -4.20935, 46.1022, -92.5385, -35.676, 63.2656, -5.23096, -15.2195,
                 9.30354, 11.1114, -0.982512, -11.0421, -86.4319, -3.01435, 76.3394, 1.71268, 24.9011, -2.42099,
                 9.483, 17.5267, -1.42749, -37.0021, -44.3019, -39.1702, -46.2538, -2.58689, 78.4703, 1.9216, 29.8211,
                 -1.99744, -3.70506, 1.06523, 0.577189, 0.146783, 3.70013, 2.9702 };

// Pos. da aplicacao dos dados

int dataPos;


void applyData(float data[], Node* n)
{
    //printf("%s:\n", n->name);
    if(n->numChildren == 0)
        return;
    for(int c=0; c<n->channels; c++) {
        //printf("   %d -> %f\n", c, data[dataPos]);
        n->channelData[c] = data[dataPos++];
    }
    for(int i=0; i<n->numChildren; i++)
        if(n->children[i])
            applyData(data, n->children[i]);
}

void apply()
{
    dataPos = 0;
    applyData(animData[curFrame], root);
}

void myInitMaleSKel()
{
    char *line;
    FILE* arq = fopen("bvh/Male1_A6_LiftBox.bvh", "r");
    if(arq == NULL) {
        printf("Erro! Arquivo não encontrado!\n");
        exit(EXIT_FAILURE);
    }
    float offsets [27][3]; //guarda as posicoes para cada membro do corpo sendo [membro do corpo][posicao x, y ou z]
    int channels [27];
    int countOffsets = 0;
    int referenceNumChild = 0;
    int numChild [27];
    char aux [2000];
    for(int i = 0; i < 125; i++){
        line =  strstr(fgets(aux, 200, arq), "{");
        //printf("The substring is: %s\n", line);
        if(line != NULL){
            char lixo[9];
            fscanf(arq, "%s %f %f %f", lixo, &offsets[countOffsets][0], &offsets[countOffsets][1], &offsets[countOffsets][2]); // tenta ler o no.
            fscanf(arq, "%s %d", lixo, &channels[countOffsets]);
            if(channels[countOffsets] > 100 || channels[countOffsets] < 0){
                channels[countOffsets] = 0;
            }
                        //printf("offset string: %s\n", offsetStr);
            if(countOffsets < 26){
                printf("Número1: %f\n", offsets[countOffsets][0]);
                printf("Número2: %f\n", offsets[countOffsets][1]);
                printf("Número3: %f\n", offsets[countOffsets][2]);
                printf("Channel: %d\n", channels[countOffsets]);
            }
            countOffsets++;
        }
    }

    //printf("c: %c\n", fgetc(arq));
    rewind(arq);
    root = createNode("Hips", NULL, channels[0], offsets[0][0], offsets[0][1], offsets[0][2], 3);

    Node* toSpine = createNode("ToSpine", root, channels[1], offsets[1][0], offsets[1][1], offsets[1][2], 1);
    Node* spine = createNode("Spine", toSpine, channels[2], offsets[2][0], offsets[2][1], offsets[2][2], 1);
    Node* spine1 = createNode("Spine1", spine, channels[3], offsets[3][0], offsets[3][1], offsets[3][2], 3);

    Node* neck = createNode("Neck", spine1, channels[4], offsets[4][0], offsets[4][1], offsets[4][2], 1);
    Node* head = createNode("Head", neck, channels[5], offsets[5][0], offsets[5][1], offsets[5][2], 1);
    Node* top = createNode("Top", head, channels[6], offsets[6][0], offsets[6][1], offsets[6][2], 0);

    /**/
    Node* leftShoulder = createNode("LeftShoulder", spine1, channels[7], offsets[7][0], offsets[7][1], offsets[7][2], 1);
    Node* leftArm = createNode("LeftArm", leftShoulder, channels[8], offsets[8][0], offsets[8][1], offsets[8][2], 1);
    Node* leftForeArm = createNode("LeftForeArm", leftArm, channels[9], offsets[9][0], offsets[9][1], offsets[9][2], 1);
    Node* leftHand = createNode("LeftHand", leftForeArm, channels[10], offsets[10][0], offsets[10][1], offsets[10][2], 1);
    Node* endLeftHand = createNode("EndLHand", leftHand, channels[11], offsets[11][0], offsets[11][1], offsets[11][2], 0);

    /**/
    Node* rShoulder = createNode("RShoulder", spine1, channels[12], offsets[12][0], offsets[12][1], offsets[12][2], 1);
    Node* rArm = createNode("RArm", rShoulder, channels[13], offsets[13][0], offsets[13][1], offsets[13][2], 1);
    Node* rForeArm = createNode("RForeArm", rArm, channels[14], offsets[14][0], offsets[14][1], offsets[14][2], 1);
    Node* rHand = createNode("RHand", rForeArm, channels[15], offsets[15][0], offsets[15][1], offsets[15][2], 1);
    Node* endRHand = createNode("RLHand", rHand, channels[16], offsets[16][0], offsets[16][1], offsets[16][2], 0);

    Node* lUpLeg = createNode("LUpLeg", root, channels[17], offsets[17][0], offsets[17][1], offsets[17][2], 1);
    Node* lLeg = createNode("LLeg", lUpLeg, channels[18], offsets[18][0], offsets[18][1], offsets[18][2], 1);
    Node* lFoot = createNode("LFoot", lLeg, channels[19], offsets[19][0], offsets[19][1], offsets[19][2], 1);
    Node* lToe = createNode("LToe", lFoot, channels[20], offsets[20][0], offsets[20][1], offsets[20][2], 1);
    Node* lToe2 = createNode("LToe2", lToe, channels[21], offsets[21][0], offsets[21][1], offsets[21][2], 0);

    Node* rUpLeg = createNode("RUpLeg", root, channels[22], offsets[22][0], offsets[22][1], offsets[22][2], 1);
    Node* rLeg = createNode("RLeg", rUpLeg, channels[23], offsets[23][0], offsets[23][1], offsets[23][2], 1);
    Node* rFoot = createNode("RFoot", rLeg, channels[24], offsets[24][0], offsets[24][1], offsets[24][2], 1);
    Node* rToe = createNode("RToe", rFoot, channels[25], offsets[25][0], offsets[25][1], offsets[25][2], 1);
    Node* rToe2 = createNode("RToe2", rToe, channels[26], offsets[26][0], offsets[26][1], offsets[26][2], 0);


    int dataCount = 1;
    for(int i = 0; i < 134; i++){
        fgets(aux, 200, arq);
    }
    printf("aux: %s\n", aux);
    int channelSum = 0;
    for(int i = 0; i < 27; i++){
        printf("channel: %d\n", channels[i]);
        for(int j = 0; j < channels[i]; j++){
            channelSum = channelSum + 1;
        }
    }
    for(int i = 0; i < 282; i++){
        for(int j = 0; j < channelSum; j++){
            fscanf(arq, "%f", &animData[i][j]);
            printf("data read: %f \n", animData[i][j]);
        }
    }

    //createData(&channels);
    apply();
}

/*void createData(int *channels[]){
    float auxf = 0;
    while(dataCount < 5){
        fscanf(arq, "%f", &auxf);
        if(auxf == 0){
            break;
        }
        animData[0][0] = auxf;
        printf("animData: %f\n", animData[0][0]);
        for(int i = 0; i < 27; i++){
            for(int j = 0; j < channels[i]; j++){
                fscanf(arq, "%f", &animData[0][dataCount]);
                //printf("animData: %f -- ", animData[0][dataCount]);
                dataCount++;
            }
        }

        dataCount = 1;
    }
}*/

float orange[] = { 1, 0.5, 0 };
float yellow[] = { 1, 1, 0 };
float red[] = { 1, 0, 0 };
float white[] = { 1, 1, 1 };

// Desenha um segmento do esqueleto (bone)
void renderBone( float x0, float y0, float z0, float x1, float y1, float z1 )
{
    GLdouble  dir_x = x1 - x0;
    GLdouble  dir_y = y1 - y0;
    GLdouble  dir_z = z1 - z0;
    GLdouble  bone_length = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );

    static GLUquadricObj* quad_obj = NULL;
    if ( quad_obj == NULL )
        quad_obj = gluNewQuadric();
    gluQuadricDrawStyle( quad_obj, GLU_FILL );
    gluQuadricNormals( quad_obj, GLU_SMOOTH );

    glPushMatrix();

    glTranslated(x0, y0, z0);

    double  length;
    length = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );
    if ( length < 0.0001 ) {
        dir_x = 0.0; dir_y = 0.0; dir_z = 1.0;  length = 1.0;
    }
    dir_x /= length;  dir_y /= length;  dir_z /= length;

    GLdouble  up_x, up_y, up_z;
    up_x = 0.0;
    up_y = 1.0;
    up_z = 0.0;

    double  side_x, side_y, side_z;
    side_x = up_y * dir_z - up_z * dir_y;
    side_y = up_z * dir_x - up_x * dir_z;
    side_z = up_x * dir_y - up_y * dir_x;

    length = sqrt( side_x*side_x + side_y*side_y + side_z*side_z );
    if ( length < 0.0001 ) {
        side_x = 1.0; side_y = 0.0; side_z = 0.0;  length = 1.0;
    }
    side_x /= length;  side_y /= length;  side_z /= length;

    up_x = dir_y * side_z - dir_z * side_y;
    up_y = dir_z * side_x - dir_x * side_z;
    up_z = dir_x * side_y - dir_y * side_x;

    GLdouble  m[16] = { side_x, side_y, side_z, 0.0,
                        up_x,   up_y,   up_z,   0.0,
                        dir_x,  dir_y,  dir_z,  0.0,
                        0.0,    0.0,    0.0,    1.0 };
    glMultMatrixd(m);

    GLdouble radius= 1;    // raio
    GLdouble slices = 8.0; // fatias horizontais
    GLdouble stack = 3.0;  // fatias verticais

    // Desenha como cilindros
    gluCylinder(quad_obj, radius, radius, bone_length, slices, stack);

    glPopMatrix();
}

// Desenha um nodo da hierarquia (chamada recursiva)
void drawNode(Node* node)
{
    int c = 0;
    glPushMatrix ();

    if(node->channels == 6) {
        glTranslatef(node->channelData[0], node->channelData[1], node->channelData[2]);
        c = 3;
    }
    else
        glTranslatef(node->offset[0], node->offset[1], node->offset[2]);
    glRotatef (node->channelData[c++], 0,0,1);
    glRotatef (node->channelData[c++], 1,0,0);
    glRotatef (node->channelData[c++], 0,1,0);

    if(node->numChildren == 0)
        renderBone(0, 0, 0, node->offset[0], node->offset[1], node->offset[2]);
    else if(node->numChildren == 1) {
        Node* child = node->children[0];
        renderBone(0, 0, 0, child->offset[0], child->offset[1], child->offset[2]);
    }
    else {
        int nc = 0;
        float center[3] = { 0.0f, 0.0f, 0.0f };
        for (int i=0; i<node->numChildren; i++ )
        {
            if(!node->children[i])
                continue;
            nc++;
            Node* child = node->children[i];
            center[0] += child->offset[0];
            center[1] += child->offset[1];
            center[2] += child->offset[2];
        }
        center[0] /= nc + 1;
        center[1] /= nc + 1;
        center[2] /= nc + 1;

        renderBone(0.0f, 0.0f, 0.0f, center[0], center[1], center[2]);

        for(int i=0; i<nc; i++) {
            Node* child = node->children[i];
            renderBone(center[0], center[1], center[2],
                    child->offset[0], child->offset[1], child->offset[2]);
        }
    }

    for(int i=0; i<node->numChildren; i++)
        if(node->children[i])
            drawNode(node->children[i]);
    glPopMatrix();
}

void drawSkeleton()
{
    drawNode(root);
}

void freeTree()
{
    freeNode(root);
}

void freeNode(Node* node)
{
    //printf("Freeing %s %p\n", node->name,node->children);
    if(node == NULL) return;
    //printf("Freeing children...\n");
    for(int i=0; i<node->numChildren; i++) {
        //printf(">>> child %d\n", i);
        freeNode(node->children[i]);
    }
    //printf("Freeing channel data...\n");
    free(node->channelData);
    if(node->numChildren>0) {
        //printf("Freeing children array...\n");
        free(node->children);
    }
}

// **********************************************************************
//  Desenha um quadriculado para representar um piso
// **********************************************************************
void drawFloor()
{
    float  size = 50;
    int  num_x = 100, num_z = 100;
    double  ox, oz;
    glBegin( GL_QUADS );
    glNormal3d( 0.0, 1.0, 0.0 );
    ox = -(num_x * size) / 2;
    for ( int x=0; x<num_x; x++, ox+=size )
    {
        oz = -(num_z * size) / 2;
        for ( int z=0; z<num_z; z++, oz+=size )
        {
            if ( ((x + z) % 2) == 0 )
                glColor3f( 1.0, 1.0, 1.0 );
            else
                glColor3f( 0.8, 0.8, 0.8 );
            glVertex3d( ox, 0.0, oz );
            glVertex3d( ox, 0.0, oz+size );
            glVertex3d( ox+size, 0.0, oz+size );
            glVertex3d( ox+size, 0.0, oz );
        }
    }
    glEnd();
}

// Função callback para eventos de botões do mouse
void mouse(int button, int state, int x, int y)
{
    if(state==GLUT_DOWN)
    {
        // Salva os parâmetros atuais
        x_ini = x;
        y_ini = y;
        ObsIni[0] = Obs[0];
        ObsIni[1] = Obs[1];
        ObsIni[2] = Obs[2];
        rotX_ini = rotX;
        rotY_ini = rotY;
        bot = button;
    }
    else bot = -1;
}

// Função callback para eventos de movimento do mouse
#define SENS_ROT	5.0
#define SENS_OBS	5.0
void move(int x, int y)
{
    // Botão esquerdo ?
    if(bot==GLUT_LEFT_BUTTON)
    {
        // Calcula diferenças
        int deltax = x_ini - x;
        int deltay = y_ini - y;
        // E modifica ângulos
        rotY = rotY_ini - deltax/SENS_ROT;
        rotX = rotX_ini - deltay/SENS_ROT;
    }
    // Botão direito ?
    else if(bot==GLUT_RIGHT_BUTTON)
    {
        // Calcula diferença
        int deltaz = y_ini - y;
        // E modifica distância do observador
        //Obs.x = x;
        //Obs.y = y;
        Obs[2] = ObsIni[2] - deltaz/SENS_OBS;
    }
    //PosicionaObservador();
    glutPostRedisplay();
}

// **********************************************************************
//  Posiciona observador
// **********************************************************************
void posUser()
{
    // Set the clipping volume
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, ratio, 0.01, 2000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Especifica posição do observador e do alvo
    glTranslatef(Obs[0], Obs[1], Obs[2]);
    glRotatef(rotX,1,0,0);
    glRotatef(rotY,0,1,0);

}

// **********************************************************************
//  Callback para redimensionamento da janela OpenGL
// **********************************************************************
void reshape( int w, int h )
{
    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if(h == 0)
        h = 1;

    ratio = 1.0f * w / h;
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    posUser();
}

// **********************************************************************
//  Callback para desenho da tela
// **********************************************************************
void display()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    posUser();

    glMatrixMode(GL_MODELVIEW);

    drawFloor();

    glPushMatrix();
    glColor3f(0.7, 0.0, 0.0); // vermelho
    drawSkeleton();
    glPopMatrix();

    glutSwapBuffers();
}

// **********************************************************************
//  Callback para eventos de teclado
// **********************************************************************
void keyboard ( unsigned char key, int x, int y )
{
    switch ( key )
    {
    case 27:        // Termina o programa qdo
        freeTree();
        exit ( 0 );   // a tecla ESC for pressionada
        break;

    default:
        break;
    }
}

// **********************************************************************
//  Callback para eventos de teclas especiais
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
    float passo = 3.0;
    switch ( a_keys )
    {
    case GLUT_KEY_RIGHT:
        if(++curFrame >= totalFrames)
            curFrame = 0;
        apply();
        glutPostRedisplay();
        break;
    case GLUT_KEY_LEFT:
        if(--curFrame < 0)
            curFrame = totalFrames-1;
        apply();
        glutPostRedisplay();
        break;
    case GLUT_KEY_UP:
        //
        glutPostRedisplay();
        break;
    case GLUT_KEY_DOWN:
        //
        glutPostRedisplay();
        break;
    default:
        break;
    }
}

// **********************************************************************
//	Inicializa os parâmetros globais de OpenGL
// **********************************************************************
void init()
{
    // Parametros da fonte de luz
    float  light0_position[] = { 10.0, 100.0, 100.0, 1.0 };
    float  light0_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
    float  light0_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    float  light0_ambient[] = { 0.1, 0.1, 0.1, 1.0 };

    // Ajusta
    glLightfv( GL_LIGHT0, GL_POSITION, light0_position );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, light0_diffuse );
    glLightfv( GL_LIGHT0, GL_SPECULAR, light0_specular );
    glLightfv( GL_LIGHT0, GL_AMBIENT, light0_ambient );

    // Habilita estados necessarios
    glEnable( GL_LIGHT0 );
    glEnable( GL_LIGHTING );
    glEnable( GL_COLOR_MATERIAL );
    glEnable( GL_DEPTH_TEST );
    glCullFace( GL_BACK );
    glEnable( GL_CULL_FACE );

    glClearColor( 0.5, 0.5, 0.8, 0.0 );

    angX = 0.0;
    angY = 0.0;
    rotY = 170;
    rotX = 35;
}

// **********************************************************************
//  Programa principal
// **********************************************************************
int main (int argc, char** argv)
{
    glutInit            ( &argc, argv );
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowPosition (0,0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize  (650, 500);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de título da janela.
    glutCreateWindow ("BVH Viewer" );

    // executa algumas inicializações
    init();

    // Exemplo: monta manualmente um esqueleto
    // (no trabalho, deve-se ler do arquivo)
    myInitMaleSKel();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // será chamada automaticamente quando
    // for necessário redesenhar a janela
    glutDisplayFunc ( display );

    // Define que a função que irá rodar a
    // continuamente. Esta função é usada para fazer animações
    // A funcao "display" será chamada automaticamente
    // sempre que for possível
    //glutIdleFunc ( display );

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // será chamada automaticamente quando
    // o usuário alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // será chamada automaticamente sempre
    // o usuário pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" será chamada
    // automaticamente sempre o usuário
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );

    // Registra a função callback para eventos de botões do mouse
    glutMouseFunc(mouse);

    // Registra a função callback para eventos de movimento do mouse
    glutMotionFunc(move);

    // inicia o tratamento dos eventos
    glutMainLoop ( );
}
