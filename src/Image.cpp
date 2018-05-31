#include "Image.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <algorithm>

Image::Image()
{
  initCL();
}

//---------------------------------------------------------------------------------------------------------------------

Image::Image( QImage & _image )
{
  initCL();
  m_image = _image;

  width = m_image.width();
  height = m_image.height();

  regionWidth = ceil(float(width)/float(m_res));
  regionHeight = ceil(float(height)/float(m_res));

  m_intensity.resize( width );
  m_chroma.resize( width );
  m_shadingMap.resize( width );
  m_specular.resize( width );
  m_roughness.resize( width );
  m_metallic.resize( width );


  for( int i = 0; i < width; ++i )
  {
    m_intensity[i].resize( height );
    m_shadingMap[i].resize( height );
    m_chroma[i].resize( height );
    m_specular[i].resize( height );
    m_roughness[i].resize( height );
    m_metallic[i].resize( height );
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
    std::cout<< " No platforms found. Check OpenCL installation!\n";
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

void Image::loadImage( QImage _image )
{
  m_image = _image;

  width = m_image.width();
  height = m_image.height();
  regionWidth = ceil(float(width)/float(m_res));
  regionHeight = ceil(float(height)/float(m_res));

  m_intensity.resize( width );
  m_chroma.resize( width );
  m_shadingMap.resize( width );
  m_specular.resize( width );
  m_roughness.resize( width );
  m_metallic.resize( width );

  for( int i = 0; i < width; ++i )
  {
    m_intensity[i].resize( height );
    m_shadingMap[i].resize( height );
    m_chroma[i].resize( height );
    m_specular[i].resize( height );
    m_roughness[i].resize( height );
    m_metallic[i].resize( height );

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

void Image::vectorAdd()
{
  //-----------------------------------------------> set up kernel, to be removed later

  std::ifstream clFile("cl/src/toNormal.cl");
  std::string programSrc((std::istreambuf_iterator<char>(clFile)), std::istreambuf_iterator<char>());

  m_program = cl::Program( m_CLContext, programSrc.c_str() );
  if(m_program.build( {m_device} ) != CL_SUCCESS)
  {
    std::cout << " Error building: " << m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>( m_device ) << "\n";
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

QImage Image::calculateNormalMap( QImage & image, int _depth, bool _invert )
{
  float * r = static_cast<float *>( malloc( width * height * sizeof( float ) ) );
  float * g = static_cast<float *>( malloc( width * height * sizeof( float ) ) );
  float * b = static_cast<float *>( malloc( width * height * sizeof( float ) ) );

  float * o_r = static_cast<float *>( malloc( width * height * sizeof( float ) ) );
  float * o_g = static_cast<float *>( malloc( width * height * sizeof( float ) ) );
  float * o_b = static_cast<float *>( malloc( width * height * sizeof( float ) ) );

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

  cl::Buffer bufferOutR(m_CLContext, CL_MEM_READ_WRITE, width * height * sizeof( float ));
  cl::Buffer bufferOutG(m_CLContext, CL_MEM_READ_WRITE, width * height * sizeof( float ));
  cl::Buffer bufferOutB(m_CLContext, CL_MEM_READ_WRITE, width * height * sizeof( float ));

  //create queue to which we will push commands for the device.
  cl::CommandQueue queue( m_CLContext, m_device );

  //write arrays A and B to the device
  queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, width * height * sizeof( float ), r);
  queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, width * height * sizeof( float ), g);
  queue.enqueueWriteBuffer(buffer_C, CL_TRUE, 0, width * height * sizeof( float ), b);

  queue.enqueueWriteBuffer(bufferOutR, CL_TRUE, 0, width * height * sizeof( float ), o_r);
  queue.enqueueWriteBuffer(bufferOutG, CL_TRUE, 0, width * height * sizeof( float ), o_g);
  queue.enqueueWriteBuffer(bufferOutB, CL_TRUE, 0, width * height * sizeof( float ), o_b);


  cl::Kernel kernelNormal = cl::Kernel( m_program, "calculateMap" );
  kernelNormal.setArg(0,buffer_A);
  kernelNormal.setArg(1,buffer_B);
  kernelNormal.setArg(2,buffer_C);

  kernelNormal.setArg(3,bufferOutR);
  kernelNormal.setArg(4,bufferOutG);
  kernelNormal.setArg(5,bufferOutB);

  kernelNormal.setArg(6, width);
  kernelNormal.setArg(7, _depth);
  kernelNormal.setArg(8, (int)_invert);



  queue.enqueueNDRangeKernel(kernelNormal, cl::NullRange, cl::NDRange(width, height), cl::NullRange);


  queue.enqueueReadBuffer(bufferOutR, CL_TRUE, 0, width * height * sizeof( float ), r);
  queue.enqueueReadBuffer(bufferOutG, CL_TRUE, 0, width * height * sizeof( float ), g);
  queue.enqueueReadBuffer(bufferOutB, CL_TRUE, 0, width * height * sizeof( float ), b);

  queue.finish();

//  QImage out;
  m_normal = image.copy();

  for ( int i = 0; i < width; ++i )
  {
    for ( int j = 0; j < height; ++j )
    {
      m_normal.setPixel(i,j,qRgb( r[j * width + i]*255, g[j * width + i]*255, b[j * width + i]*255 ));
    }
  }

  free( r );
  free( g );
  free( b );
  free( o_r );
  free( o_g );
  free( o_b );

  return m_normal;
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

void Image::strokeRefinement( QImage _stroke )
{
  std::vector<glm::vec2> strokePixels;
  strokePixels.reserve( _stroke.width() * _stroke.height());

  for(int i = 0; i < _stroke.width(); ++i)
  {
    for(int j = 0; j<_stroke.height(); ++j)
    {
      if(qRed(_stroke.pixel(i,j)) > 0)
      {
        strokePixels.push_back( glm::vec2( i, j ) );
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
      out.setPixel( i, j, qRgb( r, g, b ) );
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
    case map::SPECULAR: this->save( m_specular, _destination ); break;
    default: break;
  }
}

//---------------------------------------------------------------------------------------------------------------------

void Image::rgbToHsv()
{

}

//---------------------------------------------------------------------------------------------------------------------
float Image::contrast(float _amount, float _value)
{
  int specInt =  _value * 255;
  float contrast = 255*_amount;
  float factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));
  float newSpec = ( factor*(specInt-128) + 128 );
  newSpec = newSpec < 0 ? 0 : (newSpec > 255 ? 255 : newSpec);
  return newSpec / 255.0f;
}

//---------------------------------------------------------------------------------------------------------------------

float Image::desaturate(float _r, float _g, float _b)
{
  return (std::min(_r, std::min(_g, _b)) + std::max(_r,std:: max(_g, _b))) * 0.5f;
}

float Image::clampF(float value, float high, float low)
{
  float newValue = value < low ? low : (value > high ? high : value);
  return newValue;
}

float Image::clampI(int value, int high, int low)
{
  int newValue = value < low ? low : (value > high ? high : value);
  return newValue;
}

void Image::equalizeHistogram(map _map)
{

  std::vector<std::vector<float>> * activeMap;

  switch ( _map )
  {
    case Image::SPECULAR: activeMap = &m_specular; break;
    case Image::ROUGHNESS: activeMap = &m_roughness; break;
    default: break;
  }

  int max_val = 255;
  int total = width*height;
  int n_bins = max_val + 1;

  // Compute histogram
  std::vector<int> hist(n_bins, 0);
  for (int i = 0; i < width; ++i)
  {
    for(int j = 0 ; j< height ; ++j)
    {
      hist[int(( *activeMap )[i][j]*255)]++;
    }
  }

  // Build LUT from cumulative histrogram

  // Find first non-zero bin
  unsigned int p = 0;
  while (!hist[p]) ++p;

  if (hist[p] == total)
  {
    for (int i = 0; i < width; ++i)
    {
      for(int j = 0 ; j< height ; ++j)
      {
        ( *activeMap )[i][j] = float(p)/255.0f;
      }
    }
    return;
  }

  // Compute scale
  float scale = (n_bins - 1.f) / (total - hist[p]);

  // Initialize lut
  std::vector<int> lut(n_bins, 0);
  p++;

  int sum = 0;
  for (; p < hist.size(); ++p)
  {
    sum += hist[p];
    // the value is saturated in range [0, max_val]
    lut[p] = std::max(0, std::min(int(round(sum * scale)), max_val));
  }

  // Apply equalization
  for (int i = 0; i < width; ++i)
  {
    for(int j = 0 ; j< height ; ++j)
    {
      ( *activeMap )[i][j] = float(lut[int(( *activeMap )[i][j]*255)])/255.0f;
    }
  }
}


//---------------------------------------------------------------------------------------------------------------------

void Image::specular( float _brightness, float _contrast, bool _invert, int _sharpness, bool _equalize, Image::map _map )
{
  std::vector<std::vector<float>> * activeMap;

  switch ( _map )
  {
    case Image::SPECULAR: activeMap = &m_specular; break;
    case Image::ROUGHNESS: activeMap = &m_roughness; break;
    default: break;
  }

  for( int i = 0; i < width; ++i )
  {
    for( int j = 0 ; j < height; ++j )
    {
      QColor myColor = m_image.pixelColor( i, j );

      //---DESATURATE---------------
      // https://stackoverflow.com/a/28873770
      ( *activeMap )[i][j] = desaturate( myColor.redF(), myColor.greenF(), myColor.blueF() );
    }
  }
  //---SHARPEN-----------
  int k = 16;

  for(int n = 0; n<_sharpness; ++n)
  {
    std::vector<std::vector<float>> m_specular2 = ( *activeMap );
    for (int i = 1; i < width-1; i++)
    {
      for (int j = 1; j < height-1; j++)
      {
        double sum = ( *activeMap )[i][j] * k;

        double weight = k;

        int l[3] = { -1, 0, 1 };
        for (int m = 0; m < 3; m++)
        {
          for (int n = 0; n < 3; n++)
          {
            if ( l[m] + i != i && l[n] + j != j )
            {
              sum = sum + m_specular2[l[m] + i][ l[n] + j] * (-k / 8);
              weight = weight + (-k / 8);
            }
          }
        }
        sum = sum / weight;
        ( *activeMap )[i][j] = sum;
      }
    }
  }


  float newContrast = clampF(_contrast,1.0f,0.0f);
  newContrast*=5;
  float newBrightness = clampF(_brightness,1.0f,0.0f);
  newBrightness = (newBrightness*2) - 1;

  //std::cout<<"Contrast: "<<newContrast<<" Brightness: "<<newBrightness<<std::endl;

  for( int i = 0; i < width; ++i )
  {
    for( int j = 0 ; j < height; ++j )
    {
      //---BRIGHTNESS & CONTRAST--------------
      // https://math.stackexchange.com/a/906280
      ( *activeMap )[i][j] = newContrast*(( *activeMap )[i][j] - 0.5f) + 0.5f + newBrightness;
      ( *activeMap )[i][j] = clampF(( *activeMap )[i][j],1.0f,0.0f);

      //---INVERT---------------
      if ( _invert )
        ( *activeMap )[i][j] = 1.0f - ( *activeMap )[i][j];
    }
  }

  if( _equalize )
  {
    equalizeHistogram(_map);
  }
}

//---------------------------------------------------------------------------------------------------------------------

QImage Image::getSpecular()
{
  QImage out;
  out = m_image.copy();
  for ( int i = 0; i < width; ++i )
  {
    for ( int j = 0; j < height; ++j )
    {
      float pixel = m_specular[i][j] * 255;
      out.setPixel(i,j,qRgb( pixel, pixel, pixel));
    }
  }
  return out;
}

//---------------------------------------------------------------------------------------------------------------------

QImage Image::getRoughness()
{
  QImage out;
  out = m_image.copy();
  for ( int i = 0; i < width; ++i )
  {
    for ( int j = 0; j < height; ++j )
    {
      float pixel = m_roughness[i][j] * 255;
      out.setPixel(i,j,qRgb( pixel, pixel, pixel));
    }
  }
  return out;
}

//---------------------------------------------------------------------------------------------------------------------

QImage Image::getIntensity()
{
  QImage out;
  out = m_image.copy();
  for ( int i = 0; i < width; ++i )
  {
    for ( int j = 0; j < height; ++j )
    {
      float pixel = m_intensity[i][j] * 255;
      out.setPixel( i, j, qRgb( pixel, pixel, pixel ) );
    }
  }
  return out;
}

//---------------------------------------------------------------------------------------------------------------------

void Image::metallic( int _x, int _y, std::array<int, 3> _lowerBound, std::array<int, 3> _upperBound )
{
  float r = 0;
  float g = 0;
  float b = 0;

  for ( int i = -1; i < 2; ++i )
  {
    for ( int j = -0; j < 2; ++j )
    {
      r += m_image.pixelColor( _x + i, _y + j ).red();
      g += m_image.pixelColor( _x + i, _y + j ).green();
      b += m_image.pixelColor( _x + i, _y + j ).blue();
    }
  }

  QColor sample;// = m_image.pixelColor( _x, _y );
  sample.setRed( r / 9.0f);
  sample.setGreen( g / 9.0f);
  sample.setBlue( b / 9.0f);

  for ( int i = 0; i < width; ++i )
  {
    for ( int j = 0; j < height; ++j )
    {
      QColor pixel = m_image.pixelColor( i, j );
      if ( inRange( sample, pixel, _lowerBound, _upperBound )  )
        m_metallic[i][j] = 255;
      else
        m_metallic[i][j] = 0;
    }
  }
  QImage out;
  out = m_image.copy();
  for ( int i = 0; i < width; ++i )
  {
    for ( int j = 0; j < height; ++j )
    {
      float pixel = m_metallic[i][j];
      out.setPixel(i, j, qRgb( pixel, pixel, pixel));
    }
  }
  out.save( "pippo.jpg", 0, -1);
}

//---------------------------------------------------------------------------------------------------------------------

bool Image::inRange( QColor & _sample, QColor & _color,  std::array<int, 3> _lowerBound, std::array<int, 3> _upperBound )
{
  bool inRange = false;
  bool r = _color.red() < _sample.red() + _upperBound[0] && _color.red() > _sample.red() - _lowerBound[0];
  bool g = _color.green() < _sample.green() + _upperBound[1] && _color.green() > _sample.green() - _lowerBound[1];
  bool b = _color.blue() < _sample.blue() + _upperBound[2] && _color.blue() > _sample.blue() - _lowerBound[2];

  if ( r & g & b )
    inRange = true;
  return inRange;
}

//---------------------------------------------------------------------------------------------------------------------
