#include "scene.hpp"

Scene::Scene( QWidget *parent ) : QGLWidget( parent ),

                                  resolutionRelation( 2.14 ),
                                  horizontalDisplacement( -15 ),
                                  verticalDisplacement( -94 ),
                                  rotationAngle( 0 ),

                                  videoCapture ( new cv::VideoCapture( 1 ) ),
                                  sceneTimer ( new QTimer ),

                                  textures( new QVector< Texture * > ),
                                  sounds( new QVector< Sound * > ),
                                  videos( new QVector< Video * > ),
                                  currentTrackIndex( 0 ),
                                  tracksCount( 0 ),

                                  cameraParameters( new CameraParameters ),
                                  markerDetector( new MarkerDetector )
{
    this->setMinimumSize( GRAPHICS_WIDTH, GRAPHICS_HEIGHT );
    cameraParameters->readFromXMLFile( "../files/camera_parameters.yml" );

    connect( sceneTimer, SIGNAL( timeout() ), SLOT( slotProcess() ) );
    sceneTimer->start( 10 );
}


void Scene::loadTextures()
{
    QDir directory( "../textures" );
    QStringList fileFilter;
    fileFilter << "*.jpg" << "*.png" << "*.bmp" << "*.gif";
    QStringList imageFiles = directory.entryList( fileFilter );
    qDebug() << "loadTextures() = " << imageFiles;

    for ( int i = 0; i < imageFiles.size(); i++ )
    {
        textures->append( new Texture( imageFiles.at( i ) ) );
        QString textureUri = "../textures/" + imageFiles.at( i );

        Mat textureMat = imread( textureUri.toStdString() );
        flip( textureMat, textureMat, 0 );
        textures->last()->mat = textureMat;
        textures->last()->generateFromMat();
    }
}

void Scene::loadSounds()
{
    for( int i = 0; i < sounds->size(); i++ )
    {
        sounds->at( i )->player->stop();
        delete ( sounds->at( i )->player );
    }

    sounds->clear();

    QDir tracksDirectory( "../sounds/" );
    tracksCount = tracksDirectory.entryList( QDir::Dirs ).size() - 2;

    QDir directory( "../sounds/track_" + QString::number( currentTrackIndex ) );
    QStringList fileFilter;
    fileFilter << "*.mp3";
    QStringList soundFiles = directory.entryList( fileFilter );
    qDebug() << "loadSounds() = " << soundFiles << "currentTrackIndex=" << currentTrackIndex;

    for( int i = 0; i < soundFiles.size(); i++ )
        sounds->append( new Sound( soundFiles.at( i ), currentTrackIndex ) );

    for( int i = 0; i < sounds->size(); i++ )
        sounds->at( i )->player->play();
}

void Scene::loadVideos()
{
    QDir directory( "../videos" );
    QStringList fileFilter;
    fileFilter << "*.avi" << "*.aac" << "*.wmv" << "*.mpg" << "*.mpeg" << "*.mpeg1" << "*.mpeg2" << "*.mpeg4" << "*.mp4";
    QStringList videoFiles = directory.entryList( fileFilter );
    qDebug() << "loadVideos() = " << videoFiles;

    for ( int i = 0 ; i < videoFiles.size() ; i++ )
        videos->append( new Video( videoFiles.at( i ) ) );
}

void Scene::process( Mat &frame )
{
    Mat grayscaleMat;
    cvtColor( frame, grayscaleMat, CV_BGR2GRAY );

    Mat binaryMat;
    threshold( grayscaleMat, binaryMat, 128, 255, cv::THRESH_BINARY );

    std::vector< Marker > detectedMarkersVector;
    cameraParameters->resize( binaryMat.size() );
    markerDetector->detect( binaryMat, detectedMarkersVector, *cameraParameters, 0.08f );
    detectedMarkers = QVector< Marker >::fromStdVector( detectedMarkersVector );

//    qDebug() << "Marcadores" << detectedMarkers.size();


    // Si se detecta el marcador 20, le cambia su id a 4. No se por que.
    for( int i = 0; i < detectedMarkers.size(); i++ )
    {
        if( detectedMarkers.at( i ).id == 20 )
        {
            detectedMarkers.operator []( i ).id = 4;
        }
    }

    // Pone todos los sonidos en no detectado
    for( int i = 0; i < sounds->size(); i++ )
    {
        sounds->at( i )->isDetected = false;
    }


    for( int i = 0; i < detectedMarkers.size(); i++ )
    {
//        int currentMarkerId = detectedMarkers.at( i ).id - 6;  // No se por que le restaba 6
        int currentMarkerId = detectedMarkers.at( i ).id;

        detectedMarkers.at( i ).draw( textures->operator []( 1 )->mat, Scalar( 255, 0, 255 ), 1 );

        // Esto dibuja en pequeno sector, el id y el rectangulo de cada marcador detectado
        Point projectedCenter = detectedMarkers.at( i ).getCenter();
        projectedCenter.x *= resolutionRelation;
        projectedCenter.x += horizontalDisplacement;
        projectedCenter.y *= resolutionRelation;
        projectedCenter.y += verticalDisplacement;

//         drawAura( projectedCenter );

        qDebug() << "currentMarkerId" << currentMarkerId << "sounds->size()" << sounds->size();

        if( currentMarkerId < sounds->size() )
        {
            sounds->at( currentMarkerId )->isDetected = true;

            int volume = ( GRAPHICS_HEIGHT - projectedCenter.y ) * 100 / ( float )GRAPHICS_HEIGHT;
            sounds->at( currentMarkerId )->player->setVolume( volume );

            drawPeak( currentMarkerId, projectedCenter );
        }

        drawTitle( currentMarkerId, projectedCenter );
    }

    for( int i = 0; i < sounds->size(); i++ )
    {
        if( !sounds->at( i )->isDetected )
        {
            sounds->at( i )->player->setVolume( sounds->at( i )->player->volume() - 5 );
        }
    }
}


void Scene::drawAura( Point center )
{
    int lines = 50;
    int maxRadius = 200;

    for( int i = 0; i <= lines; i++ )
    {
        circle( textures->operator []( 1 )->mat, center, maxRadius / ( float )lines * i, Scalar( 0, 0, 255 ), 1 );
    }

    float angle = rotationAngle;

    lines = 50;
    for( int i = 0; i < lines; i++ )
    {
        float localAngle = angle * 3.14159264 / ( float )180;

        line( textures->operator []( 1 )->mat,
              center,
              Point( center.x + maxRadius * cos( localAngle ), center.y + maxRadius * sin( localAngle ) ),
              Scalar( 255, 0, 0 ) );

        angle += 360 / ( float )lines;
    }
}

void Scene::drawTitle( int id, Point center )
{
    if( id < sounds->size() )
    {
        QString title = sounds->at( id )->name;
        title.remove( ".mp3" );
        putText( textures->operator []( 1 )->mat,
                 title.toStdString().c_str(),
                 Point( center.x - title.length() * 10,
                        GRAPHICS_HEIGHT - 100 ),
                 FONT_HERSHEY_PLAIN, 2, Scalar( 0, 0, 255 ), 2 );

    }
}

void Scene::drawPeak( int soundIndex, Point markerCenter )
{
    float peakValue = sounds->at( soundIndex )->leftSpectrum;
    float maxLenght = GRAPHICS_HEIGHT - markerCenter.y;
    float lenght = 2 * peakValue * maxLenght;

    Rect bar( markerCenter.x - 100,
              GRAPHICS_HEIGHT - lenght,
              200,
              lenght );

    Rect bar1 = bar;
    bar1.x += 5;
    bar1.width -= 10;
    bar1.y += 5;
    bar1.height -= 5;

    Rect bar2 = bar1;
    bar2.x += 5;
    bar2.width -= 10;
    bar2.y += 5;
    bar2.height -= 5;

    rectangle( textures->at( 1 )->mat, bar, Scalar( 0, 0, 255 ), 5 );
    rectangle( textures->at( 1 )->mat, bar1, Scalar( 255, 0, 0 ), 5 );
    rectangle( textures->at( 1 )->mat, bar2, Scalar( 255, 0, 0 ), 5 );

    int radioCirculoInterior = 105;

    // Circulo interior
    circle( textures->at( 1 )->mat, markerCenter,
            radioCirculoInterior + lenght / 400, Scalar( 0, 0, 255 ), lenght / 10 );

    // Circulo medio
    circle( textures->at( 1 )->mat, markerCenter,
            radioCirculoInterior+25 + lenght / 400, Scalar( 255, 0, 255 ), lenght / 8 );

    // Circulo exterior
    circle( textures->at( 1 )->mat, markerCenter,
            radioCirculoInterior+50 + lenght / 500, Scalar( 255, 0, 0 ), lenght / 6 );
}

void Scene::drawBox( QString textureName, int percentage )
{
    for( int i = 0; i < textures->size(); i++ )
    {
        if( textures->at( i )->getName() == textureName )
        {
            float sideLength = percentage / ( float )2300;

            glEnable( GL_TEXTURE_2D );
            glBindTexture( GL_TEXTURE_2D, textures->at( i )->getId() );
            glColor3f( 1, 1, 1 );
            glRotated( 90, 1, 0, 0 );
            glTranslatef( 0, 0, -sideLength );
            glEnable( GL_LIGHTING );
            glBegin( GL_QUADS );

                glNormal3f( 0.0f, 0.0f, 1.0f ); // Frontal
                glTexCoord2f( 0.0f, 0.0f ); glVertex3f(-sideLength, -sideLength,  sideLength );
                glTexCoord2f( 1.0f, 0.0f ); glVertex3f( sideLength, -sideLength,  sideLength );
                glTexCoord2f( 1.0f, 1.0f ); glVertex3f( sideLength,  sideLength,  sideLength );
                glTexCoord2f( 0.0f, 1.0f ); glVertex3f(-sideLength,  sideLength,  sideLength );

                glNormal3f( 0.0f, 0.0f,-1.0f ); // Anterior
                glTexCoord2f( 1.0f, 0.0f ); glVertex3f(-sideLength, -sideLength, -sideLength );
                glTexCoord2f( 1.0f, 1.0f ); glVertex3f(-sideLength,  sideLength, -sideLength );
                glTexCoord2f( 0.0f, 1.0f ); glVertex3f( sideLength,  sideLength, -sideLength );
                glTexCoord2f( 0.0f, 0.0f ); glVertex3f( sideLength, -sideLength, -sideLength );

                glNormal3f( 0.0f, 1.0f, 0.0f ); // Superior
                glTexCoord2f( 0.0f, 1.0f ); glVertex3f(-sideLength,  sideLength, -sideLength );
                glTexCoord2f( 0.0f, 0.0f ); glVertex3f(-sideLength,  sideLength,  sideLength );
                glTexCoord2f( 1.0f, 0.0f ); glVertex3f( sideLength,  sideLength,  sideLength );
                glTexCoord2f( 1.0f, 1.0f ); glVertex3f( sideLength,  sideLength, -sideLength );

                glNormal3f( 0.0f,-1.0f, 0.0f ); // Inferior
                glTexCoord2f( 1.0f, 1.0f ); glVertex3f(-sideLength, -sideLength, -sideLength );
                glTexCoord2f( 0.0f, 1.0f ); glVertex3f( sideLength, -sideLength, -sideLength );
                glTexCoord2f( 0.0f, 0.0f ); glVertex3f( sideLength, -sideLength,  sideLength );
                glTexCoord2f( 1.0f, 0.0f ); glVertex3f(-sideLength, -sideLength,  sideLength );

                glNormal3f( 1.0f, 0.0f, 0.0f ); // Derecha
                glTexCoord2f( 1.0f, 0.0f ); glVertex3f( sideLength, -sideLength, -sideLength );
                glTexCoord2f( 1.0f, 1.0f ); glVertex3f( sideLength,  sideLength, -sideLength );
                glTexCoord2f( 0.0f, 1.0f ); glVertex3f( sideLength,  sideLength,  sideLength );
                glTexCoord2f( 0.0f, 0.0f ); glVertex3f( sideLength, -sideLength,  sideLength );

                glNormal3f( -1.0f, 0.0f, 0.0f ); // Izquierda
                glTexCoord2f( 0.0f, 0.0f ); glVertex3f(-sideLength, -sideLength, -sideLength );
                glTexCoord2f( 1.0f, 0.0f ); glVertex3f(-sideLength, -sideLength,  sideLength );
                glTexCoord2f( 1.0f, 1.0f ); glVertex3f(-sideLength,  sideLength,  sideLength );
                glTexCoord2f( 0.0f, 1.0f ); glVertex3f(-sideLength,  sideLength, -sideLength );

            glEnd();
            glDisable( GL_LIGHTING );
            glDisable( GL_TEXTURE_2D );
        }
    }
}

void Scene::drawVideo( QString videoName )
{
    int halfWidth  = GRAPHICS_WIDTH / ( float )2;
    int halfHeight = GRAPHICS_WIDTH / ( float )2;
    for ( int i = 0 ; i < videos->size(); i++ )
    {
        if ( videos->at( i )->name == videoName )
        {
            videos->at( i )->player->play();

            glEnable( GL_TEXTURE_2D );
            glColor3f( 1, 1, 1 );
            glBindTexture( GL_TEXTURE_2D, videos->at( i )->grabber->textureId );
            glColor3f( 1, 1, 1 );
            glBegin( GL_QUADS );

                glNormal3f( 0.0f, 0.0f,-1.0f);
                glTexCoord2f( 0, 0 ); glVertex3f(-halfWidth, halfHeight,-900 );
                glTexCoord2f( 1, 0 ); glVertex3f( halfWidth, halfHeight,-900 );
                glTexCoord2f( 1, 1 ); glVertex3f( halfWidth,-halfHeight,-900 );
                glTexCoord2f( 0, 1 ); glVertex3f(-halfWidth,-halfHeight,-900 );

            glEnd();
            glDisable( GL_TEXTURE_2D );
        }
        else
        {
            videos->at( i )->player->pause();
        }
    }
}

void Scene::initializeGL()
{
    initializeGLFunctions();

    glClearColor( 0, 0, 0, 0 );
    glShadeModel( GL_SMOOTH );
    glEnable( GL_DEPTH_TEST );

    GLfloat lightAmb[4]; lightAmb[0] = 0.5f; lightAmb[1] = 0.5f; lightAmb[2] = 0.5f; lightAmb[3] = 1.0f;
    GLfloat lightDif[4]; lightDif[0] = 1.0f; lightDif[1] = 1.0f; lightDif[2] = 1.0f; lightDif[3] = 1.0f;
    GLfloat lightPos[4]; lightPos[0] = 0.0f; lightPos[1] = 0.0f; lightPos[2] = 2.0f; lightPos[3] = 1.0f;

    glLightfv( GL_LIGHT1, GL_AMBIENT, lightAmb );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, lightDif );
    glLightfv( GL_LIGHT1, GL_POSITION,lightPos );

    glEnable( GL_LIGHT1 );

    textures->append( new Texture( "camera_texture" ) );
    textures->append( new Texture( "camera_graphics" ) );

    textures->operator []( 1 )->mat = Mat( GRAPHICS_HEIGHT, GRAPHICS_WIDTH, CV_8UC3 );
    textures->operator []( 1 )->mat.setTo( Scalar( 0, 0, 0 ) );

    loadTextures();
    loadSounds();
    loadVideos();
}

void Scene::resizeGL( int width, int height )
{
    glViewport( 0, 0, width, height );
}

void Scene::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    int halfWidth  = GRAPHICS_WIDTH / ( float )2;
    int halfHeight = GRAPHICS_WIDTH / ( float )2;
    glOrtho( -halfWidth, halfWidth, -halfHeight, halfHeight, 1, 1000 );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // Video - OpenGL
    glEnable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE);

    bool localTrackVideoBackgroundDetected = false;
    for( int i = 0; i < detectedMarkers.size(); i++ )
    {
        if( detectedMarkers.at( i ).id == 10 )
        {
            localTrackVideoBackgroundDetected = true;
            drawVideo( "video_" + QString::number( currentTrackIndex ) + ".mp4" );
        }
    }
    if( !localTrackVideoBackgroundDetected )
    {
        // drawVideo( "background.mp4" );
    }

    // Graficos - OpenCV
    glEnable( GL_TEXTURE_2D );
    glColor3f( 1, 1, 1 );
    glBindTexture( GL_TEXTURE_2D, textures->at( 1 )->getId() );
    glBegin( GL_QUADS );

        glTexCoord2f( 0, 0 ); glVertex3f(-halfWidth, halfHeight,-500 );
        glTexCoord2f( 1, 0 ); glVertex3f( halfWidth, halfHeight,-500 );
        glTexCoord2f( 1, 1 ); glVertex3f( halfWidth,-halfHeight,-500 );
        glTexCoord2f( 0, 1 ); glVertex3f(-halfWidth,-halfHeight,-500 );

    glEnd();
    glDisable( GL_TEXTURE_2D );

    // Logo - OpenGL
    for( int i = 0; i < textures->size(); i++ )
    {
        if( textures->at( i )->getName() == "visual_dj_logo.png" )
        {
            glEnable( GL_TEXTURE_2D );
            glColor3f( 1, 1, 1 );
            glBindTexture( GL_TEXTURE_2D, textures->at( i )->getId() );
            glBegin( GL_QUADS );

            int width = 204;
            int height = 204;
            int x = 800;
            int y = 700;

                glTexCoord2f( 0, 1 ); glVertex3f(x-(width/(float)2), y+(height/(float)2*(16/(float)9)),-100 );
                glTexCoord2f( 1, 1 ); glVertex3f(x+(width/(float)2), y+(height/(float)2*(16/(float)9)),-100 );
                glTexCoord2f( 1, 0 ); glVertex3f(x+(width/(float)2), y-(height/(float)2*(16/(float)9)),-100 );
                glTexCoord2f( 0, 0 ); glVertex3f(x-(width/(float)2), y-(height/(float)2*(16/(float)9)),-100 );

            glEnd();
            glDisable( GL_TEXTURE_2D );
        }
    }

    glFlush();
}

void Scene::keyPressEvent( QKeyEvent *event )
{
    switch( event->key() )
    {
    case Qt::Key_Plus:  resolutionRelation     += 0.01; break;
    case Qt::Key_Minus: resolutionRelation     -= 0.01; break;
    case Qt::Key_Left:  horizontalDisplacement -= 1.00; break;
    case Qt::Key_Right: horizontalDisplacement += 1.00; break;
    case Qt::Key_Up:    verticalDisplacement   -= 1.00; break;
    case Qt::Key_Down:  verticalDisplacement   += 1.00; break;

    case Qt::Key_B:
        if( currentTrackIndex == 0 ) return;
        currentTrackIndex--;
        loadSounds();
        break;

    case Qt::Key_N:
        if( currentTrackIndex == tracksCount - 1 ) return;
        currentTrackIndex++;
        loadSounds();
        break;

    case Qt::Key_Escape:
        this->close();
        break;

    default: break;
    }

    qDebug() << "Relación de resolucion: " << resolutionRelation;
    qDebug() << "Desplazamiento horizontal: " << horizontalDisplacement;
    qDebug() << "Desplazamiento vertical: " << verticalDisplacement;
}

void Scene::slotProcess()
{

    videoCapture->operator >>( textures->operator []( 0 )->mat );
    textures->operator []( 1 )->mat.setTo( Scalar( 0, 0, 0 ) );

    rotationAngle += 1;
    process( textures->operator []( 0 )->mat );

    textures->operator []( 0 )->generateFromMat();
    textures->operator []( 1 )->generateFromMat();

    this->updateGL();
}
