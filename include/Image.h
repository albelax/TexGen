#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>
#include <vector>
#include <glm.hpp>

#ifdef __APPLE__
#include <cl/cl.hpp>
#else
#include <CL/cl.hpp>
#endif


class Image
{
public:
  enum map { ORIGINAL, INTENSITY, ALBEDO, SHADING, NORMAL, SPECULAR, ROUGHNESS, CHROMA, AO };
  Image();
  Image( QImage & _image );
  void threshold();
  void intensity();
  void chroma();
  void separation();
  void shading();
  void strokeRefinement( QImage _stroke );
  void specular(float _brightness, float _contrast, bool _invert, int _sharpness, bool _equalize, Image::map _map );
  void diffuse(float _brightness, float _contrast, int _sharpness, bool _equalize);

  void metallic(int _x, int _y, float _range );
  float contrast( float _amount, float _value );
  float desaturate( float _r, float _g, float _b );
  void equalizeHistogram1f( map _map );
  void equalizeDiffuseHistogram();

  void toggleMetallic(bool _b) {m_noMetallic = _b;}
  void toggleAO(bool _b) {m_noAO = _b;}

  float clampF(float value, float high, float low);
  float clampI(int value, int high, int low);

  glm::vec3 colorToVec3(QColor _col);
  QColor vec3ToColor(glm::vec3 _vec);

  void save( std::vector<std::vector<std::vector<float>>> & _image, std::string _destination );
  void save( std::vector<std::vector<float>> & _image, std::string _destination );
  void save( map _image, std::string _destination );
  void vectorAdd();
  void loadImage( QImage _image );
  QImage calculateNormalMap( QImage & image, int _depth, bool _invert, map _map);
  QImage getDiffuse() { return m_diffuse; }
  QImage getSpecular();
  QImage getRoughness();
  QImage getIntensity();
  QImage getMetallic();
  QImage getNormal() { return m_normal; }
  QImage getAO();

  bool isNull() {return m_image.isNull();}
private:
  int width;   // aliasing avoids loading m_image in the cache every time we need the width
  int height;
  int regionWidth;   /// \brief regionWidth, the amount of regions in the x
  int regionHeight; /// \brief regionHeight, the amount of regions in the y
  int m_iterations = 20;  /// \brief m_iterations, number of iteration for the separation
  int m_res = 16;  /// \brief m_res, resolution of each region
  bool inRange( QColor & _sample, QColor & _color, std::array<int, 3> _lowerBound, std::array<int, 3> _upperBound );
  void initCL();
  void rgbToHsv();

  bool m_noMetallic = false;
  bool m_noAO = false;

  // CL
  cl::Platform m_CLPlatform;
  cl::Device m_device;
  cl::Context m_CLContext;
  cl::Program::Sources m_sources;
  cl::Program m_program;
  // CL end

  QImage m_image;

  std::vector<std::vector<float>> m_intensity;
  std::vector<std::vector<float>> albedoIntensityMap;
  std::vector<std::vector<float>> m_shadingMap;
  std::vector<std::vector<float>> m_specular;
  std::vector<std::vector<float>> m_roughness;
  std::vector<std::vector<float>> m_metallic;
  std::vector<std::vector<float>> m_ao;
  QImage m_diffuse;
  QImage m_aoNormal;
  std::vector<std::vector<float>> B;
  std::vector<std::vector<std::vector<float>>> A;
  std::vector<std::vector<std::vector<float>>> m_chroma;
  QImage m_normal;
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
