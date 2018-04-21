#include "Image.h"
#include <math.h>
#include <iostream>
#include <fstream>

Image::Image( QImage & _image )
{
  initCL();
  m_image = _image;

  width = m_image.width();
  height = m_image.height();
//  std::cout << "image w: " << width << " image h: " << height << '\n';

  regionWidth = ceil(float(width)/float(m_res));
  regionHeight = ceil(float(height)/float(m_res));
//  std::cout << "region w: " << regionWidth << " region h: " << regionHeight << '\n';
  m_intensity.resize( width );
  m_chroma.resize( width );
  m_shadingMap.resize( width );

  for( int i = 0; i < width; ++i )
  {
    m_intensity[i].resize( height );
    m_shadingMap[i].resize( height );
    m_chroma[i].resize( height );
  }

  B.resize( int(regionWidth) );
  A.resize( int(regionWidth) );
  regions.resize(int(regionWidth));
  whichPixelWhichRegion.resize( int( regionWidth ) );
  numberOfPixelsInChromaRegions.resize( int(regionWidth) );

  for( int i = 0; i < regionWidth; ++i )
  {
    A[i].resize( int(regionHeight) );
    B[i].resize( int(regionHeight) );
    regions[i].resize( int(regionHeight) );
    whichPixelWhichRegion[i].resize( int(regionHeight) );
    numberOfPixelsInChromaRegions[i].resize( int(regionHeight) );
  }
}

//---------------------------------------------------------------------------------------------------------------------

void Image::initCL()
{
  std::vector<cl::Platform> all_platforms;
  cl::Platform::get(&all_platforms);
  if( all_platforms.size() == 0 )
  {
    std::cout<<" No platforms found. Check OpenCL installation!\n";
    exit(1);
  }
  cl::Platform m_CLPlatform = all_platforms[0];

  std::vector<cl::Device> all_devices;
  m_CLPlatform.getDevices( CL_DEVICE_TYPE_ALL, &all_devices );
  if( all_devices.size() == 0 )
  {
    std::cout<<" No devices found. Check OpenCL installation!\n";
    exit(1);
  }
  m_device = all_devices[0];

  m_CLContext = cl::Context( m_device );
}

//---------------------------------------------------------------------------------------------------------------------


void Image::vectorAdd()
{
  //-----------------------------------------------> set up kernel, to be removed later

  std::ifstream clFile("cl/src/toNormal.cl");
  std::string programSrc((std::istreambuf_iterator<char>(clFile)), std::istreambuf_iterator<char>());

  m_program = cl::Program( m_CLContext, programSrc.c_str() );
  if(m_program.build( {m_device} ) != CL_SUCCESS)
  {
    std::cout << " Error building: " << m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device) << "\n";
    exit(1);
  }

  //-----------------------------------------------> launch kernel, to be removed later

  // create buffers on the device
  cl::Buffer buffer_A(m_CLContext,CL_MEM_READ_WRITE,sizeof(int)*10);
  cl::Buffer buffer_B(m_CLContext,CL_MEM_READ_WRITE,sizeof(int)*10);
  cl::Buffer buffer_C(m_CLContext,CL_MEM_READ_WRITE,sizeof(int)*10);

  int A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  int B[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 0};

  //create queue to which we will push commands for the device.
  cl::CommandQueue queue( m_CLContext, m_device );

  //write arrays A and B to the device
  queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, sizeof(int)*10, A);
  queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, sizeof(int)*10, B);

  cl::Kernel kernel_add = cl::Kernel( m_program, "simple_add" );
  kernel_add.setArg(0,buffer_A);
  kernel_add.setArg(1,buffer_B);
  kernel_add.setArg(2,buffer_C);
  queue.enqueueNDRangeKernel(kernel_add, cl::NullRange, cl::NDRange(10), cl::NullRange);
  queue.finish();

  int C[10];
  //read result C from the device to array C
  queue.enqueueReadBuffer(buffer_C, CL_TRUE, 0, sizeof(int)*10, C);
}

//---------------------------------------------------------------------------------------------------------------------

void Image::calculateNormalMap( QImage & image )
{
  float * r = static_cast<float *>( malloc( width * height * sizeof( float ) ) );
  float * g = static_cast<float *>( malloc( width * height * sizeof( float ) ) );
  float * b = static_cast<float *>( malloc( width * height * sizeof( float ) ) );

  for ( int i = 0; i < width; ++i )
  {
    for ( int j = 0; j < height; ++j )
    {
      QColor myPixel = QColor( image.pixel(i,j) );
      r[j * width + i] = myPixel.redF();
      g[j * width + i] = myPixel.greenF();
      b[j * width + i] = myPixel.blueF();
    }
  }

  std::ifstream clFile( "cl/src/toNormal.cl" );
  std::string programSrc((std::istreambuf_iterator<char>(clFile)), std::istreambuf_iterator<char>());

  m_program = cl::Program( m_CLContext, programSrc.c_str() );
  if(m_program.build( {m_device} ) != CL_SUCCESS)
  {
    std::cout << " Error building: " << m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device) << "\n";
    exit(1);
  }

  cl::Buffer buffer_A(m_CLContext, CL_MEM_READ_WRITE, width * height * sizeof( float ));
  cl::Buffer buffer_B(m_CLContext, CL_MEM_READ_WRITE, width * height * sizeof( float ));
  cl::Buffer buffer_C(m_CLContext, CL_MEM_READ_WRITE, width * height * sizeof( float ));

  //create queue to which we will push commands for the device.
  cl::CommandQueue queue( m_CLContext, m_device );

  //write arrays A and B to the device
  queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, width * height * sizeof( float ), r);
  queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, width * height * sizeof( float ), g);
  queue.enqueueWriteBuffer(buffer_C, CL_TRUE, 0, width * height * sizeof( float ), b);


  cl::Kernel kernel_add = cl::Kernel( m_program, "calculateMap" );
  kernel_add.setArg(0,buffer_A);
  kernel_add.setArg(1,buffer_B);
  kernel_add.setArg(2,buffer_C);
  kernel_add.setArg(3,width);

  queue.enqueueNDRangeKernel(kernel_add, cl::NullRange, cl::NDRange(width, height), cl::NullRange);
  queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, width * height * sizeof( float ), r);
  queue.enqueueReadBuffer(buffer_B, CL_TRUE, 0, width * height * sizeof( float ), g);
  queue.enqueueReadBuffer(buffer_C, CL_TRUE, 0, width * height * sizeof( float ), b);

  queue.finish();

  QImage out;
  out = image.copy();

  for ( int i = 0; i < width; ++i )
  {
    for ( int j = 0; j < height; ++j )
    {
      out.setPixel(i,j,qRgb( r[j * width + i]*255, g[j * width + i]*255, b[j * width + i]*255 ));
    }
  }

  out.save( "images/normal.jpg", 0, -1 );
  free( r );
  free( g );
  free( b );
}

//---------------------------------------------------------------------------------------------------------------------

void Image::threshold()
{

  for( int i = 0; i < width; ++i )
  {
    for( int j = 0; j < height; ++j )
    {
      QColor myPixel = QColor( m_image.pixel(i,j) );

      int gray = ( myPixel.red() * 11 + myPixel.green() * 16 + myPixel.blue()*5 ) / 32;
      if( gray > 235 || gray < 25 )
      {
        int r,g,b;
        float fraction = 235.0f/gray;
        r = myPixel.red() * fraction;
        g = myPixel.green() * fraction;
        b = myPixel.blue() * fraction;

        m_image.setPixel( i, j, qRgb(r, g, b) );
      }
    }
  }
}


//---------------------------------------------------------------------------------------------------------------------

void Image::intensity()
{
  for( int i = 0; i < width; ++i )
  {
    for( int j = 0 ; j < height; ++j )
    {
      QColor myColor = m_image.pixelColor( i, j );
      float average = ( myColor.redF() + myColor.greenF() + myColor.blueF() ) / 3.0f;
      m_intensity[i][j] = average;
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------

void Image::chroma()
{
  std::vector<float> pixel = { 0.0f, 0.0f, 0.0f };

  for ( int i = 0; i < width; ++i )
  {
    for ( int j = 0; j < height; ++j )
    {
      float intensity = m_intensity[i][j];
      float red = float( m_image.pixelColor(i,j).redF() ) / float(intensity);
      float green = float( m_image.pixelColor(i,j).greenF() ) / float(intensity);
      float blue = 3.0f - red - green;
      pixel[0] = red;
      pixel[1] = green;
      pixel[2] = blue;
      m_chroma[i][j] = pixel;
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------

void Image::findChromaRegions()
{
  float regionStep = 3.0f/20.0f;

  for( int x = 0; x < regionWidth; ++x )
  {
    for(int y = 0; y < regionHeight; ++y )
    {
      int indexY = m_res * y;
      if( indexY > height - 1 ) break;

      int indexX = m_res * x;
      if( indexX > width - 1 ) break;

      whichPixelWhichRegion[x][y].resize( m_res );
      // Now we go through the 20*20 region's pixels

      for( int i = 0; i < m_res; ++i )
      {
        whichPixelWhichRegion[x][y][i].resize( m_res );
        indexY = m_res * y;
        if( indexY > height - 1 ) break;

        for( int j = 0; j < m_res; ++j )
        {
          // See if the pixel is in any existing regions.
          bool found = false;
          for( unsigned int z = 0; z < regions[x][y].size(); ++z )
          {
            // If within an existing region
            if(m_chroma[indexX][indexY][0] > regions[x][y][z][0] && m_chroma[indexX][indexY][0] < regions[x][y][z][0]+regionStep &&
               m_chroma[indexX][indexY][1] > regions[x][y][z][1] && m_chroma[indexX][indexY][1] < regions[x][y][z][1]+regionStep)
            {
              numberOfPixelsInChromaRegions[x][y][z]++;
              whichPixelWhichRegion[x][y][i][j] = z;
              found = true;
              break;
            }
          }
          // If no existing chroma region
          if(!found)
          {
            numberOfPixelsInChromaRegions[x][y].push_back(1);
            whichPixelWhichRegion[x][y][i][j]=(numberOfPixelsInChromaRegions[x][y].size() - 1);
            // Quantize the chroma regions and add it to the list of regions
            float rTemp = m_chroma[indexX][indexY][0]/regionStep;
            float gTemp = m_chroma[indexX][indexY][1]/regionStep;
            float r = floor(rTemp) * regionStep;
            float g = floor(gTemp) * regionStep;
            std::vector<float> temp;
            temp.push_back(r);
            temp.push_back(g);
            regions[x][y].push_back(temp);
          }
          indexY++;
          if( indexY >= height ) break;
        }
        indexX++;
        if( indexX >= width ) break;
      }
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------

void Image::updateSums()
{
  // clear previous sums
  // First two vectors are regions x and y
  for( int i = 0; i < regionWidth; ++i )
  {
    for( int j = 0; j < regionHeight; ++j )
    {
      B[i][j] = 0.0f;

      A[i][j].resize( regions[i][j].size() );
      for( unsigned int p = 0; p < A[i][j].size(); ++p )
      {
        A[i][j][p] = 0.0f;
      }
    }
  }

  for( int x = 0; x < regionWidth; ++x )
  {
    for( int y = 0; y < regionHeight; ++y )
    {
      int indexY = m_res * y;
      if( indexY > height - 1 ) break;

      int indexX = m_res * x;
      if( indexX > width - 1 ) break;

      for( int i = 0; i < m_res; ++i )
      {
        indexY = m_res * y;
        if(indexY > height - 1) break;

        for(int j = 0; j < m_res; ++j)
        {
          // CHeck if dividing by zero
          if( albedoIntensityMap[indexX][indexY]< 0.000001f )
          {
            albedoIntensityMap[indexX][indexY] = 0.000001f;
          }

          //sum1[x][y]+=_imageIntensity[indexX][indexY]/(albedoIntensityMap[indexX][indexY]*albedoIntensityMap[indexX][indexY]);
          B[x][y]+=m_intensity[indexX][indexY] / albedoIntensityMap[indexX][indexY];
          if( isinf(B[x][y]) ) std::cout << "is inf: "<<albedoIntensityMap[indexX][indexY]<<"\n";
          A[x][y][whichPixelWhichRegion[x][y][i][j]]+=m_intensity[indexX][indexY];

          indexY++;
          if( indexY >= height ) break;
        }
        indexX++;
        if( indexX >= width ) break;
      }
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------

void Image::separation()
{
  std::vector<std::vector<float>> prevDiffF0;
  prevDiffF0.resize( width );
  for( int i = 0; i < width; ++i )
  {
    prevDiffF0[i].resize( height );
  }

  findChromaRegions();
  albedoIntensityMap = m_intensity;
  for( int r = 0; r < m_iterations; ++r )
  {
    updateSums();
    for( int x = 0; x < regionWidth; ++x )
    {
      for( int y = 0; y < regionHeight; ++y )
      {
        int indexY = m_res * y;
        if( indexY > height - 1 ) break;

        int indexX = m_res * x;
        if( indexX > width - 1 ) break;

        for( int i = 0; i< m_res; ++i )
        {
          indexY = m_res * y;
          if( indexY > height - 1 ) break;
          for( int j = 0; j < m_res; ++j )
          {
            int ourChromaRegion = whichPixelWhichRegion[x][y][i][j];
            float noPixelsChroma = float(numberOfPixelsInChromaRegions[x][y][ourChromaRegion]);

            float T = albedoIntensityMap[indexX][indexY];
            float C = m_intensity[indexX][indexY]/( m_res * m_res );
            float ourB = (1.0f/(m_res*m_res))*(B[x][y] - ( m_intensity[indexX][indexY]/albedoIntensityMap[indexX][indexY]));
            float ourA = A[x][y][ourChromaRegion] * (1.0f/noPixelsChroma);
            float DiffF0 = 2 * (T - ourA/(ourB+C/T)*(1.0f - (ourA/((ourB+C/T)*(ourB+C/T)))*(C/(T*T))));

            //  Change albedo based on DiffF0
            //                        if( DiffF0 > 0.1f ) albedoIntensityMap[indexX][indexY] -= 0.01f;
            //                        else if( DiffF0 < -0.1f ) albedoIntensityMap[indexX][indexY]+= 0.01f;

            albedoIntensityMap[indexX][indexY] = albedoIntensityMap[indexX][indexY]-0.01 * DiffF0;

            prevDiffF0[indexX][indexY] = DiffF0;

            indexY++;
            if(indexY >= height) break;
          }
          indexX++;
          if( indexX >= width ) break;
        }
      }
    }

  }
}
//---------------------------------------------------------------------------------------------------------------------

void Image::strokeRefinement(QImage _stroke)
{
  std::vector<glm::vec2> strokePixels;
  strokePixels.reserve( _stroke.width() * _stroke.height());

  for(int i = 0; i<_stroke.width(); ++i)
  {
    for(int j = 0; j<_stroke.height(); ++j)
    {
      if(qRed(_stroke.pixel(i,j)) > 0)
      {
        strokePixels.push_back(glm::vec2(i,j));
      }
    }
  }

  for( int x = 0; x < regionWidth; ++x )
  {
    for( int y = 0; y < regionHeight; ++y )
    {
      int indexY = m_res * y;
      if( indexY > height - 1 ) break;

      int indexX = m_res * x;
      if( indexX > width - 1 ) break;

      for( int i = 0; i < m_res; ++i )
      {
        indexY = m_res * y;
        if(indexY > height - 1) break;

        for(int j = 0; j < m_res; ++j)
        {
          float T = albedoIntensityMap[indexX][indexY];
          float ourA = m_intensity[indexX][indexY];
          float ourB = ((1.0f/20.0f*20.0f)*B[x][y]);
          float ourC = m_intensity[indexX][indexY]/( m_res * m_res );
          float DiffEs = (T - ourA/(ourB+ourC/T)*(1.0f - (ourA/((ourB+ourC/T)*(ourB+ourC/T)))*(ourC/(T*T))));

          int whichPixel = 0;
          float distance = 100000;
          glm::vec2 start = glm::vec2(indexX,indexY);

          for(unsigned int d = 0; d<strokePixels.size(); ++d)
          {
            glm::vec2 end = strokePixels[d];
            glm::vec2 diff = start-end;
            float distance2= glm::length(diff);
            if(distance2<distance)
            {
              distance = distance2;
              whichPixel = d;
            }
          }

          float weight = 10.0f * glm::exp(- (distance*distance)/3.0f);

          for(unsigned int d = 0; d<strokePixels.size(); ++d)
          {

          }


          indexY++;
          if( indexY >= height ) break;
        }
        indexX++;
        if( indexX >= width ) break;
      }
    }
  }

}


//---------------------------------------------------------------------------------------------------------------------

void Image::shading()
{
  for( int i = 0; i < width; ++i )
  {
    for( int j = 0; j < height; ++j )
    {
      m_shadingMap[i][j] = m_intensity[i][j] / albedoIntensityMap[i][j];
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------

void Image::save( std::vector<std::vector<std::vector<float>>> & _image, std::string _destination )
{
  QImage out;
  out = m_image.copy();

  for ( unsigned int i = 0; i < _image.size(); ++i )
  {
    for ( unsigned int j = 0; j < _image[i].size(); ++j )
    {
      float r = _image[i][j][0]*255.0f;
      float g = _image[i][j][1]*255.0f;
      float b = _image[i][j][2]*255.0f;
      out.setPixel(i,j,qRgb( r, g, b ));
    }
  }
  out.save( _destination.c_str(), 0, -1 );
}

//---------------------------------------------------------------------------------------------------------------------

void Image::save( std::vector<std::vector<float>> & _image, std::string _destination )
{
  QImage out;
  out = m_image.copy();

  for ( unsigned int i = 0; i < _image.size(); ++i )
  {
    for ( unsigned int j = 0; j < _image[i].size(); ++j )
    {
      float r = _image[i][j]*255.0f;
      float g = _image[i][j]*255.0f;
      float b = _image[i][j]*255.0f;
      out.setPixel(i,j,qRgb( r, g, b ));
    }
  }
  out.save( _destination.c_str(), 0, -1 );
}

//---------------------------------------------------------------------------------------------------------------------

void Image::save( map _image, std::string _destination )
{
  switch ( _image )
  {
    case map::INTENSITY: this->save( m_intensity, _destination ); break;
    case map::CHROMA: this->save( m_chroma, _destination ); break;
    case map::ALBEDO: this->save( albedoIntensityMap, _destination ); break;
    case map::SHADING: this->save( m_shadingMap, _destination ); break;

    default: break;
  }
}

//---------------------------------------------------------------------------------------------------------------------

void Image::rgbToHsv()
{

}

//---------------------------------------------------------------------------------------------------------------------
