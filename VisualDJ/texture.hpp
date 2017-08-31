#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <QGLWidget>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

class Texture : public QObject
{
    Q_OBJECT

private:

    QString name;
    GLuint id;

public:

    Mat mat;

    explicit Texture( QString name = "", QObject *parent = 0 );

    void generateFromMat();

    QString getName() const;
    void setName( const QString &value );

    GLuint getId() const;
    void setId( const GLuint &value );
};

#endif // TEXTURE_HPP
