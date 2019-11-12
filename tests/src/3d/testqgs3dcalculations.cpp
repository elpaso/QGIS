/***************************************************************************
     TestQgs3DCalculations.cpp
     ----------------------
    Date                 : November 2019
    Copyright            : (C) 2019 by Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include <memory>

#include <QOffscreenSurface>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QSurfaceFormat>
#include <QOpenGLFramebufferObject>

#include <Qt3DCore/QAspectEngine>
#include <Qt3DLogic/QLogicAspect>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DRender/QRenderCapture>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QRenderTargetOutput>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QTextureWrapMode>
#include <Qt3DRender/QViewport>
#include <QtGui/QOpenGLContext>

#include <QSize>

/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgs3DCalculations : public QObject
{
    Q_OBJECT
  public:
    TestQgs3DCalculations() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testShaders();

  private:
    void testCalculations();
};

//runs before all tests
void TestQgs3DCalculations::initTestCase()
{
}

//runs after all tests
void TestQgs3DCalculations::cleanupTestCase()
{
}


QImage processImage( const QImage &image,
                     const QString &vertexShader,
                     const QString &fragmentShader,
                     const QString &textureVar,
                     const QString &vertexPosVar,
                     const QString &textureCoordVar )
{

  // Step 1: Create an OpenGL context to work with.

  QOpenGLContext context;
  if ( !context.create() )
  {
    qDebug() << "Can't create GL context.";
    return {};
  }

  // Step 2: Create the offscreen surface to use for rendering on.

  QOffscreenSurface surface;
  surface.setFormat( context.format() );
  surface.create();
  if ( !surface.isValid() )
  {
    qDebug() << "Surface not valid.";
    return {};
  }

  // Step 3: Make the context “current” for the surface. Consider this as binding the context and surface together.

  if ( !context.makeCurrent( &surface ) )
  {
    qDebug() << "Can't make context current.";
    return {};
  }

  // Step 4: Create a Frame Buffer Object the same size of the input image. Set the viewport accordingly.

  QOpenGLFramebufferObject fbo( image.size() );
  context.functions()->glViewport( 0, 0, image.width(), image.height() );

  // Step 5: Time to compile, link and bind Vertex and Fragment shader source codes.

  QOpenGLShaderProgram program( &context );
  if ( !program.addShaderFromSourceCode( QOpenGLShader::Vertex, vertexShader ) )
  {
    qDebug() << "Can't add vertex shader.";
    return {};
  }
  if ( !program.addShaderFromSourceCode( QOpenGLShader::Fragment, fragmentShader ) )
  {
    qDebug() << "Can't add fragment shader.";
    return {};
  }
  if ( !program.link() )
  {
    qDebug() << "Can't link program.";
    return {};
  }
  if ( !program.bind() )
  {
    qDebug() << "Can't bind program.";
    return {};
  }

  // Step 6: Now we create an OpenGL texture and bind it accordingly.

  QOpenGLTexture texture( QOpenGLTexture::Target2D );
  texture.setAutoMipMapGenerationEnabled( false );
  texture.setData( image );

  texture.bind();
  if ( !texture.isBound() )
  {
    qDebug() << "Texture not bound.";
    return {};
  }

  // Step 7: Next, we create the vertex and index data before we bind them as necessary.

  struct VertexData
  {
    QVector2D position;
    QVector2D texCoord;
  };

  VertexData vertices[] =
  {
    {{ -1.0f, +1.0f }, { 0.0f, 1.0f }}, // top-left
    {{ +1.0f, +1.0f }, { 1.0f, 1.0f }}, // top-right
    {{ -1.0f, -1.0f }, { 0.0f, 0.0f }}, // bottom-left
    {{ +1.0f, -1.0f }, { 1.0f, 0.0f }}  // bottom-right
  };

  GLuint indices[] =
  {
    0, 1, 2, 3
  };

  QOpenGLBuffer vertexBuf( QOpenGLBuffer::VertexBuffer );
  QOpenGLBuffer indexBuf( QOpenGLBuffer::IndexBuffer );

  if ( !vertexBuf.create() )
  {
    qDebug() << "Can't create vertex buffer.";
    return {};
  }

  if ( !indexBuf.create() )
  {
    qDebug() << "Can't create index buffer.";
    return {};
  }

  if ( !vertexBuf.bind() )
  {
    qDebug() << "Can't bind vertex buffer.";
    return {};
  }
  vertexBuf.allocate( vertices, 4 * sizeof( VertexData ) );

  if ( !indexBuf.bind() )
  {
    qDebug() << "Can't bind index buffer.";
    return {};
  }
  indexBuf.allocate( indices, 4 * sizeof( GLuint ) );

  // Step 8: We need to set the attribute and uniform values before actually proceeding with the drawing.
  // This is the part where each buffer or texture is actually set to a variable in the vertex or fragment shader code.

  int offset = 0;
  program.enableAttributeArray( vertexPosVar.toLatin1().data() );
  program.setAttributeBuffer( vertexPosVar.toLatin1().data(), GL_FLOAT, offset, 2, sizeof( VertexData ) );
  offset += sizeof( QVector2D );
  program.enableAttributeArray( textureCoordVar.toLatin1().data() );
  program.setAttributeBuffer( textureCoordVar.toLatin1().data(), GL_FLOAT, offset, 2, sizeof( VertexData ) );
  program.setUniformValue( textureVar.toLatin1().data(), 0 );


  // Step 9: Now we draw. Using GL_TRIANGLE_STRIP offers a bit of cross-platform confidence since it exists almost
  // everywhere. But you can definitely use GL_TRIANGLES, GL_QUADS and so on. Just be sure to adjust the vertices
  // and indices accordingly if you opt not use GL_TRIANGLE_STRIP for whatever reason.
  context.functions()->glDrawElements( GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, Q_NULLPTR );

  // Step 10: The last step is to simply create a QImage from the FBO.
  return fbo.toImage( false );
}


void TestQgs3DCalculations::testShaders()
{

  QString vertexShader =
    "attribute vec4 aPosition;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec2 vTexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = aPosition;\n"
    "   vTexCoord = aTexCoord;\n"
    "}";

  QString fragmentShader =
    "uniform sampler2D texture;\n"
    "varying vec2 vTexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_FragColor = texture2D(texture, vTexCoord);\n"
    "}";

  QImage image = processImage( QImage( "/home/ale/Scrivania/circle_mask.png" ),
                               vertexShader,
                               fragmentShader,
                               "texture",
                               "aPosition",
                               "aTexCoord" );

  image.save( "/home/ale/Scrivania/circle_mask_2.png" );

}


void TestQgs3DCalculations::testCalculations()
{
  // Set up the default OpenGL surface format.
  QSurfaceFormat format;

  // by default we get just some older version of OpenGL from the system,
  // but for 3D lines we use "primitive restart" functionality supported in OpenGL >= 3.1
  // Qt3DWindow uses this - requesting OpenGL 4.3 - so let's request the same version.
#ifdef QT_OPENGL_ES_2
  format.setRenderableType( QSurfaceFormat::OpenGLES );
#else
  if ( QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL )
  {
    format.setVersion( 4, 3 );
    format.setProfile( QSurfaceFormat::CoreProfile );
  }
#endif

  format.setMajorVersion( 3 );
  format.setDepthBufferSize( 32 );
  format.setSamples( 8 );
  QSurfaceFormat::setDefaultFormat( format );


  // Create render target
  Qt3DRender::QRenderTarget mTextureTarget;

  // The lifetime of the objects created here is managed
  // automatically, as they become children of this object.

  // Create a render target output for rendering color.
  Qt3DRender::QRenderTargetOutput mTextureOutput( &mTextureTarget );
  mTextureOutput.setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  // Create a texture to render into.
  Qt3DRender::QTexture2D mTexture( &mTextureOutput );
  QSize mSize( 16, 16 );
  mTexture.setSize( mSize.width(), mSize.height() );
  mTexture.setFormat( Qt3DRender::QAbstractTexture::LuminanceFormat );
  mTexture.setMinificationFilter( Qt3DRender::QAbstractTexture::Nearest );
  mTexture.setMagnificationFilter( Qt3DRender::QAbstractTexture::Nearest );
  Qt3DRender::QTextureWrapMode mWrapMode;
  mWrapMode.setX( Qt3DRender::QTextureWrapMode::WrapMode::ClampToEdge );
  mWrapMode.setY( Qt3DRender::QTextureWrapMode::WrapMode::ClampToEdge );
  mTexture.setWrapMode( mWrapMode );

  // Hook the texture up to our output, and the output up to this object.
  mTextureOutput.setTexture( &mTexture );
  mTextureTarget.addOutput( &mTextureOutput );




}

QGSTEST_MAIN( TestQgs3DCalculations )
#include "testqgs3dcalculations.moc"
