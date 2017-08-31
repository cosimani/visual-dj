#ifndef SCENE_HPP
#define SCENE_HPP

//#define GRAPHICS_WIDTH  1920
//#define GRAPHICS_HEIGHT 1080

#define GRAPHICS_WIDTH  1280
#define GRAPHICS_HEIGHT 720

#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QGLWidget>
#include <QKeyEvent>
#include <QStringList>
#include <QGLFunctions>

#include <cmath>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "aruco/aruco.h"
#include "texture.hpp"
#include "sound.hpp"
#include "video.hpp"

using namespace cv;
using namespace aruco;

class Scene : public QGLWidget, protected QGLFunctions
{
    Q_OBJECT

private:

    // Graphics
    float resolutionRelation;
    float horizontalDisplacement;
    float verticalDisplacement;
    float rotationAngle;

    // Scene
    VideoCapture *videoCapture;
    QTimer *sceneTimer;

    // Mixer
    QVector< Texture * > *textures;
    QVector< Sound * > *sounds;
    QVector< Video *> *videos;
    int currentTrackIndex;
    int tracksCount;

    // Marker detection
    CameraParameters *cameraParameters;
    MarkerDetector *markerDetector;
    QVector< Marker > detectedMarkers;

    void loadTextures();
    void loadSounds();
    void loadVideos();

    void process( Mat &frame );
    void drawAura( Point center );
    void drawTitle( int id, Point center );
    void drawPeak( int soundIndex, Point markerCenter );
    void drawBox( QString textureName, int percentage = 100 );
    void drawVideo( QString videoName );

public:

    Scene( QWidget *parent = 0 );

protected:

    void initializeGL();
    void resizeGL( int width, int height );
    void paintGL();
    void keyPressEvent( QKeyEvent *event );

private slots:

    void slotProcess();
};

#endif // SCENE_HPP
