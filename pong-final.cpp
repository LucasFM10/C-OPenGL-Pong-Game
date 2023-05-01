// LINHA PARA COMPILAR O CÓDIGO NO TERMINAL: g++ pong-final.cpp -o pong-final -lglut -lGLU -lGL -lm -lasound

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include <string>
#include <math.h>

#include <alsa/asoundlib.h>

// window size and update rate (60 fps)
int width = 1000;
int height = 450;
int interval = 1000 / 60;

// verify if game is isPaused
int isPaused = 0;

// verify current state of game: 0 -> start screen, 1 -> running game, 2 -> game finished screen
int gameState = 0;

// score
int scoreLeft = 0;
int scoreRight = 0;

// indicate someone made a point, 1 for right and 2 for left
int goalScored = 0;

// rackets in general
int racketWidth = 20;
int racketHeight = 120;
int racketSpeed = 5;

// left racket position
float racketLeftX = 20.0f;
float racketLeftY = height/2 - racketHeight/2;

// right racket position
float racketRightX = width - racketWidth - racketLeftX;
float racketRightY = height/2 - racketHeight/2;

// ball
float ballPosX = width / 2;
float ballPosY = height / 2 - 1;
float ballDirX = -1.0f;
float ballDirY = 0.0f;
int ballSize = 16;
float ballSpeedInit = 8;
float ballSpeed = ballSpeedInit;
float ballSpeedTemp = ballSpeed; // used to resume ball speed from isPaused

// variable to control keys
bool keyStates[256] = {false};
bool specialStates[256] = {false};

// speed increment factor
float factor = 1.1;

// mouse coordinates;
int mouseX = 0;
int mouseY = 0;

void beep(unsigned int freq, unsigned int duration);
void draw();
void drawCircle(float cx, float cy, float r, int num_segments);
void drawField();
void drawFinishedGameWindow();
void drawRect(float x, float y, float width, float height);
void drawScore(float x, float y);
void drawStartWindow();
void enable2D(int width, int height);
void motionCallback(int x, int y);
void mouseCallback(int button, int state, int x, int y);
void onKeyDown(unsigned char key, int x, int y);
void onKeyUp(unsigned char key, int x, int y);
void onSpecialDown(int key, int x, int y);
void onSpecialUp(int key, int x, int y);
void setBallPositionLeftRacket();
void setBallPositionRightRacket();
void update(int value);
void updateBall();
void vecToNorm(float& x, float &y);

// program entry point
int main(int argc, char** argv) {

    printf("INTEGRANTES DO GRUPO:\nLUCAS FARIAS DE MEDEIROS 20220054884\nGABRIEL MONTEIRO DE ANDRADE 20190162570\nVINICIUS NEGREIROS DE MELO 20210138953\nNELLY STANFORD FERNANDES MARTINS 20220160368\n");

    // initialize opengl (via glut)
    glutInit(&argc, argv);
    
    // screen
    int screen_width = glutGet(GLUT_SCREEN_WIDTH);
    int screen_height = glutGet(GLUT_SCREEN_HEIGHT);

    // screen center
    int center_x = screen_width / 2;
    int center_y = screen_height / 2;

    // center window
    int window_pos_x = center_x - (width / 2);
    int window_pos_y = center_y - (height / 2);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(window_pos_x, window_pos_y);
    glutCreateWindow("Pong");

    // Register callback functions
    glutDisplayFunc(draw);
    glutTimerFunc(interval, update, 0);
    
    // scene to 2d mode
    enable2D(width, height);

    // start the whole thing
    glutMainLoop();
    return 0;
}



void beep(unsigned int freq, unsigned int duration)
{
    // Initialize the PCM device
    snd_pcm_t* pcm;
    snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 44100, 1, 500000);

    // Generate the waveform
    const int sample_rate = 44100;
    const double amplitude = 32767 / 2.0;
    const int num_samples = sample_rate * duration / 1000;
    short samples[num_samples];
    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / (double)sample_rate;
        double freq_hz = freq;
        double sine_wave = amplitude * sin(2.0 * M_PI * freq_hz * t);
        samples[i] = (short)sine_wave;
    }

    // Play the waveform
    snd_pcm_sframes_t frames = snd_pcm_writei(pcm, samples, num_samples);
    if (frames < 0) {
        frames = snd_pcm_recover(pcm, frames, 0);
    }

    // Wait for the playback to finish
    snd_pcm_drain(pcm);
    snd_pcm_close(pcm);
}

void draw() {

    //printf("%d\n",gameState);

    switch(gameState) {
        case 0:
            drawStartWindow();
            return;
        case 1:
            break;
        case 2:
            drawFinishedGameWindow();
            return;
    }

    //printf("running game\n");

    // clear (has to be done at the beginning)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    drawField();

    // draw score
    drawScore(width /4, height - 50);

    // draw rackets
    drawRect(racketLeftX, racketLeftY, racketWidth, racketHeight);
    drawRect(racketRightX, racketRightY, racketWidth, racketHeight);
    
    // draw ball
    drawCircle(ballPosX, ballPosY, ballSize, 100);

    // score color
    glColor3f(1.0f, 1.0f, 1.0f);
    
    // swap buffers (has to be done at the end)
    glutSwapBuffers();
}

void drawCircle(float cx, float cy, float r, int num_segments) {
    
    glBegin(GL_TRIANGLE_FAN);
    for (int ii = 0; ii < num_segments; ii++)   {
        float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle 
        float x = r * cos(theta);//calculate the x component 
        float y = r * sin(theta);//calculate the y component
        glColor3f(1,1,1);
        glVertex2f(x + cx, y + cy);//output vertex
    }
    glEnd();
}

void drawField() {

    // circle
    glBegin(GL_LINE_LOOP);

    int num_segments = 10000;
    for (int ii = 0; ii < num_segments; ii++)   {
        float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle 
        float x = 20 * cos(theta);//calculate the x component 
        float y = 20 * sin(theta);//calculate the y component
        glColor3f(1,1,1);
        glVertex2f(x + width/2, y + height/2);//output vertex
    }
    glEnd();

    glBegin(GL_LINES);
        glVertex2f(width / 2, height);
        glVertex2f(width / 2, 0);
    glEnd();

    // line
}

void drawFinishedGameWindow() {

    std::string winner;

    if (scoreLeft == 15) {

        winner.assign("                                         LEFT WINNER!!!                   ");
    } else {

        winner.assign("                                          RIGHT WINNER!!!                  ");
    }


    scoreRight = 0;
    scoreLeft = 0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(width/2 - 370, height/2);
    std::string str("HIT SPACE TO RESTART THE GAME OR HIT ESC TO LEAVE");
    int len1 = str.size();
    for (int i = 0; i <= len1; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
    }

    glRasterPos2f(width/2 - 370, height/2 - 30);
    int len2 = winner.size();
    for (int i = 0; i <= len2; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, winner[i]);
    }

    glutSwapBuffers();
}

// Function used to draw rackets
void drawRect(float x, float y, float width, float height) {
    glBegin(GL_QUADS);
        glColor3f(1,1,1);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
    glEnd();
}

void drawScore(float x, float y) {
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    int len1 = (std::to_string(scoreLeft)).size();
    for (int i = 0; i <= len1; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, std::to_string(scoreLeft)[i]);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(width - x, y);
    int len2 = std::to_string(scoreRight).size();
    for (int i = 0; i <= len2; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, std::to_string(scoreRight)[i]);
    }
}

void drawStartWindow() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(width/2 - 200, height/2);
        std::string str("HIT SPACE TO START THE GAME");
        int len = str.size();
        for (int i = 0; i <= len; i++)
        {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
        }

    glutSwapBuffers();
}

void enable2D(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, width, 0.0f, height, 0.0f, 1.0f);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
}

void motionCallback(int x, int y)
{

    //printf("mouse hover: %d %d\n",x,y);
}

void mouseCallback(int button, int state, int x, int y)
{

    //printf("-- mouse activated: %d %d\n",x,y);
}

void onKeyDown(unsigned char key, int x, int y) {
    
    keyStates[key] = true;
    // printf("%d\n",key);

    if(gameState == 2 && key == 27) {
        glutDestroyWindow(glutGetWindow());
    }
}

void onKeyUp(unsigned char key, int x, int y) {
    
    keyStates[key] = false;
    
    if (key == 32) {
    	
    	if  (isPaused == 0 && gameState == 1) {
    	
    	    // printf("Pausando %d\n", isPaused);
    	    isPaused = 1;
    	} else if(isPaused == 1 && gameState == 1) {
    	
    	    // printf("Despausando %d\n", isPaused);
    	    isPaused = 0;
    	} else {
            ballSpeed = ballSpeedInit;
            ballPosX = width/2;
            ballPosY = height/2 - 1;
            ballDirX = -1.0f;
            racketLeftX = 20.0f;
            racketLeftY = 165.0f;
            racketRightX = width - racketWidth - racketLeftX;
            racketRightY = racketLeftY;
            gameState = 1;
        }
    }
}


void onSpecialDown(int key, int x, int y) {

    specialStates[key] = true;
    // printf("%d\n--%d\n",key,gameState);
}

void onSpecialUp(int key, int x, int y) {

    specialStates[key] = false;
}

void setBallPositionLeftRacket() {

   ballPosX = racketLeftX + ballSize + racketWidth + 10;
   ballPosY = racketLeftY + racketHeight / 2 - 1;
}

void setBallPositionRightRacket() {

   ballPosX = racketRightX - ballSize - 10;
   ballPosY = racketRightY + racketHeight / 2 - 1;
}

void update(int value) {

    // input handling
    glutKeyboardFunc(onKeyDown);
    glutKeyboardUpFunc(onKeyUp);

    glutSpecialFunc(onSpecialDown);
    glutSpecialUpFunc(onSpecialUp);

    glutMouseFunc(mouseCallback);
    glutPassiveMotionFunc(motionCallback);

    // Call update() again in 'interval' milliseconds
    glutTimerFunc(interval, update, 0);

    if (gameState != 1) {
        return;
    }

    // update ball
    updateBall();

    // isPaused game
    // isPaused == 1 --> pause game
    if( isPaused == 1) {

        ballSpeed = 0;
        return;
    }

    if(scoreRight == 15 || scoreLeft == 15) {

        gameState = 2;
    }

    ballSpeed = ballSpeedTemp;

    // left racket
    if (keyStates[119] == true && racketLeftY + racketHeight < height) racketLeftY += racketSpeed;
    if (keyStates[115] == true && racketLeftY > 0) racketLeftY -= racketSpeed;

    // right racket
    if (specialStates[101] == true && racketRightY + racketHeight < height) racketRightY += racketSpeed;
    if (specialStates[103] == true && racketRightY > 0) racketRightY -= racketSpeed;

    // start round (enter)

    // goalScored == 1 -> right serve
    if (keyStates[13] == true && goalScored == 1) {
        
        ballSpeed = ballSpeedInit;
        ballSpeedTemp = ballSpeedInit;
        ballDirX = -fabs(ballDirX); // force it to be negative
        goalScored = 0;
    }

    // goalScored == 2 -> left serve
    if (keyStates[13] == true && goalScored == 2) {
        
        ballSpeed = ballSpeedInit;
        ballSpeedTemp = ballSpeedInit;
        ballDirX = fabs(ballDirX); // force it to be positive
        goalScored = 0;
    }

    // Redisplay frame
    glutPostRedisplay();
}

void updateBall() {

    if( ballSpeed > 15 ) {

        factor = 1.05;
    }
    
    if (goalScored == 1) {
        
        setBallPositionRightRacket();
        return;
    }
    
    if (goalScored == 2) {
        
        setBallPositionLeftRacket();
        return;
    }
    
    // starting game
    ballPosX += ballDirX * ballSpeed;
    ballPosY += ballDirY * ballSpeed;
    
    if ( isPaused == 0) ballSpeed = ballSpeedTemp;
   
    // hit by left racket?
    if (ballPosX - ballSize < racketLeftX + racketWidth &&
        ballPosX - ballSize > racketLeftX &&
        ballPosY < racketLeftY + racketHeight + ballSize &&
        ballPosY > racketLeftY - ballSize) {
        // set fly direction depending on where it hit the racket
        // (t is 0.5 if hit at top, 0 at center, -0.5 at bottom)
        float t = ((ballPosY - racketLeftY) / racketHeight) - 0.5f;
        ballDirX = fabs(ballDirX); // force it to be positive
        ballDirY = t;
        ballSpeed = ballSpeed * factor;
	    ballSpeedTemp = ballSpeed;
        beep(1600, 100);
    }
   
    // hit by right racket?
    if (ballPosX + ballSize > racketRightX &&
        ballPosX + ballSize < racketRightX + racketWidth &&
        ballPosY < racketRightY + racketHeight + ballSize &&
        ballPosY > racketRightY - ballSize) {
        // set fly direction depending on where it hit the racket
        // (t is 0.5 if hit at top, 0 at center, -0.5 at bottom)
        float t = ((ballPosY - racketRightY) / racketHeight) - 0.5f;
        ballDirX = -fabs(ballDirX); // force it to be negative
        ballDirY = t;
        ballSpeed = ballSpeed * factor;
	    ballSpeedTemp = ballSpeed;
        beep(1600, 100);
    }
    
    // hit left wall?
    if (ballPosX < 0) {
        ++scoreRight;
        ballSpeed = 0;
        ballDirY = 0;
        goalScored = 1;
        
        racketLeftY = height/2 - racketHeight/2;
        racketRightY = height/2 - racketHeight/2;
        beep(440, 100);
    }

    // hit right wall?
    if (ballPosX > width) {
        ++scoreLeft;
        ballSpeed = 0;
        ballDirY = 0;
        goalScored = 2;
        
        racketLeftY = height/2 - racketHeight/2;
        racketRightY = height/2 - racketHeight/2;
        beep(440, 100);
    }

    // hit top wall?
    if (ballPosY + ballSize  > height) {
        ballDirY = -fabs(ballDirY); // force it to be negative
    }

    // hit bottom wall?
    if (ballPosY - ballSize  < 0) {
        ballDirY = fabs(ballDirY); // force it to be positive
    }

    // make sure that length of dir stays at 1
    vecToNorm(ballDirX, ballDirY);
}

void vecToNorm(float& x, float &y) {
        // sets a vectors length to 1 (which means that x + y == 1)
        float length = sqrt((x * x) + (y * y));
        if (length != 0.0f) {
            length = 1.0f / length;
            x *= length;
            y *= length;
        }
    }
