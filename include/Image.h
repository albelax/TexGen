#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>
#include <vector>

class Image
{
public:
  enum map { INTENSITY, CHROMA, ALBEDO };
  Image() = default;
  Image( QImage & _image );
  void threshold();
  void intensity();
  void chroma();
  void separation();
  void save( std::vector<std::vector<std::vector<float>>> & _image, std::string _destination );
  void save( std::vector<std::vector<float>> & _image, std::string _destination );
  void save( map _image, std::string _destination );

private:
  int width;   // aliasing avoids loading m_image in the cache every time we need the width
  int height;
  int regionWidth;   /// \brief regionWidth, the amount of regions in the x
  int regionHeight; /// \brief regionHeight, the amount of regions in the y
	int m_iterations = 20;  /// \brief m_iterations, number of iteration for the separation
  int m_res = 20;  /// \brief m_res, resolution of each region

  QImage m_image;
  std::vector<std::vector<float>> albedoIntensityMap;
  std::vector<std::vector<float>> m_intensity;
  std::vector<std::vector<float>> B;
  std::vector<std::vector<std::vector<float>>> A;
  std::vector<std::vector<std::vector<float>>> m_chroma;

  /// \brief numberOfPixelsInChromaRegions, First two vectors are regions x and y
  /// Third vector is the chroma region index
  std::vector<std::vector<std::vector<int>>> numberOfPixelsInChromaRegions;

  /// \brief whichPixelWhichRegion Tells us which region each pixel belongs to
  /// First two vectors are the region x and y
  /// the second two vectors are the pixels x and y
  std::vector<std::vector<std::vector<std::vector<int>>>> whichPixelWhichRegion;

  /// \brief regions, First two vectors are regions x and y
  /// Third vector is the chroma region index
  /// Fourth vector is the r g b
  std::vector<std::vector<std::vector<std::vector<float>>>> regions;

  void findChromaRegions();
  void updateSums();
};

#endif // IMAGE_H
