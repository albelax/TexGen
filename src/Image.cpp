#include "Image.h"
#include <math.h>
#include <iostream>

Image::Image( QImage & _image )
{
    m_image = _image;
    width = m_image.width();
    height = m_image.height();
    std::cout << "image w: " << width << " image h: " << height << '\n';

    regionWidth = ceil(float(width)/float(m_res));
    regionHeight = ceil(float(height)/float(m_res));
    std::cout << "region w: " << regionWidth << " region h: " << regionHeight << '\n';
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
    int count = 0;
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
                        float DiffF0 = (T - ourA/(ourB+C/T)*(1.0f - (ourA/((ourB+C/T)*(ourB+C/T)))*(C/(T*T))));

                        //            m_totDiffF0[r][indexX][indexY] = DiffF0;

                        //  Change albedo based on DiffF0
                        if( DiffF0 > 0.1f ) { albedoIntensityMap[indexX][indexY] -= 0.01f; count++; }
                        else if( DiffF0 < -0.1f ) { albedoIntensityMap[indexX][indexY]+= 0.01f; count++; }
                        //						albedoIntensityMap[indexX][indexY] = (float(y))/25.0f;


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
