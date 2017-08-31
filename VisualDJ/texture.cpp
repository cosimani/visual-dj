#include "texture.hpp"

Texture::Texture( QString name, QObject *parent ) : QObject( parent ), name( name ), id( 0 )
{
    glGenTextures( 1, &id );
}

void Texture::generateFromMat()
{
    glBindTexture( GL_TEXTURE_2D, id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, mat.cols, mat.rows, 1, GL_BGR, GL_UNSIGNED_BYTE, mat.data );
}

QString Texture::getName() const
{
    return name;
}

void Texture::setName( const QString &value )
{
    name = value;
}

GLuint Texture::getId() const
{
    return id;
}

void Texture::setId( const GLuint &value )
{
    id = value;
}
