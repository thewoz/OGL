/*
 * GNU GENERAL PUBLIC LICENSE
 *
 * Copyright (C) 2017-2026
 * Created by Leonardo Parisi (leonardo.parisi[at]gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _H_OGL_PLOT_H_
#define _H_OGL_PLOT_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glPlot
  //****************************************************************************/
  class glPlot : public glObject {

  public:

    struct AxisRange {
      
      AxisRange(float _min = -1.0f, float _max = 1.0f) : min(_min), max(_max) {}

      float min;
      float max;
      
    };

  private:

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;

    glm::vec3 axisColor = glm::vec3(1.0f);
    glm::vec3 majorTickColor = glm::vec3(0.9f);
    glm::vec3 minorTickColor = glm::vec3(0.6f);

    AxisRange xRange;
    AxisRange yRange;
    AxisRange zRange;
    glm::vec3 axisOrigin = glm::vec3(0.0f);

    bool useManualRange = true;

    bool xLog = false;
    bool yLog = false;
    bool zLog = false;

    bool drawMajorTicks = true;
    bool drawMinorTicks = true;
    bool drawAxisLabels = true;
    bool drawTickLabels = true;

    // se true i tick vengono disegnati solo verso l'interno (direzione positiva
    // rispetto all'origine degli assi) invece che simmetrici sui due lati.
    bool tickInward = false;

    float majorTickStep = 1.0f;
    float minorTickStep = 0.5f;

    float majorTickSize = 0.03f;
    float minorTickSize = 0.015f;
    float axisLabelScale = 0.6f;
    float tickLabelScale = 0.45f;

    // distanza (in unita' mondo) delle scritte dall'asse. 0 => default automatico
    // proporzionale all'ampiezza degli assi. Disaccoppiato da majorTickSize, cosi'
    // i tick possono essere piccoli senza spostare/affollare le scritte.
    float axisLabelOffset = 0.0f;
    float tickLabelOffset = 0.0f;

    std::string xAxisLabel = "X";
    std::string yAxisLabel = "Y";
    std::string zAxisLabel = "Z";

    glPrint3D axisLabelPrinter;
    glPrint3D tickLabelPrinter;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec4> colors;
    std::vector<GLuint> indices;

  public:

    //****************************************************************************/
    // glPlot()
    //****************************************************************************/
    glPlot(const std::string & _name = "") {
      name = _name;
      axisLabelPrinter.setName(name + "_axis_labels");
      tickLabelPrinter.setName(name + "_tick_labels");
    }

    //****************************************************************************/
    // ~glPlot()
    //****************************************************************************/
    ~glPlot() { cleanInGpu(); }

    glPlot(glPlot &&) noexcept = default;
    glPlot & operator = (glPlot &&) noexcept = default;

    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const AxisRange & _xRange = AxisRange(), const AxisRange & _yRange = AxisRange(), const AxisRange & _zRange = AxisRange()) {

      DEBUG_LOG("glPlot::init(" + name + ")");

      shader.setName(name);
      shader.initLine();

      axisLabelPrinter.init("", glm::vec3(0.0f), axisColor, axisLabelScale);
      tickLabelPrinter.init("", glm::vec3(0.0f), majorTickColor, tickLabelScale);

      setRanges(_xRange, _yRange, _zRange);

      isInited = true;
      
    }

    //****************************************************************************/
    // setRanges()
    //****************************************************************************/
    void setRanges(const AxisRange & _xRange, const AxisRange & _yRange,  const AxisRange & _zRange) {

      xRange = sanitizeRange(_xRange);
      yRange = sanitizeRange(_yRange);
      zRange = sanitizeRange(_zRange);
      
      useManualRange = true;
      isToUpdateInGpu = true;
      
    }

    //****************************************************************************/
    // setAutoRangeFromData()
    //****************************************************************************/
    void setAutoRangeFromData(const std::vector<glm::vec3> & points, float padding = 0.05f) {

      if(points.empty()) return;

      float minX =  std::numeric_limits<float>::max();
      float minY =  std::numeric_limits<float>::max();
      float minZ =  std::numeric_limits<float>::max();
      float maxX = -std::numeric_limits<float>::max();
      float maxY = -std::numeric_limits<float>::max();
      float maxZ = -std::numeric_limits<float>::max();

      for(const auto & p : points) {
        if(p.x < minX) minX = p.x;
        if(p.y < minY) minY = p.y;
        if(p.z < minZ) minZ = p.z;
        if(p.x > maxX) maxX = p.x;
        if(p.y > maxY) maxY = p.y;
        if(p.z > maxZ) maxZ = p.z;
      }

      xRange = buildPaddedRange(minX, maxX, padding);
      yRange = buildPaddedRange(minY, maxY, padding);
      zRange = buildPaddedRange(minZ, maxZ, padding);

      useManualRange = false;
      
      isToUpdateInGpu = true;
      
    }

    //****************************************************************************/
    // setTickSteps()
    //****************************************************************************/
    void setTickSteps(float _majorTickStep, float _minorTickStep = -1.0f) {
      majorTickStep = (_majorTickStep > 0.0f) ? _majorTickStep : majorTickStep;
      if(_minorTickStep > 0.0f) minorTickStep = _minorTickStep;
      isToUpdateInGpu = true;
    }

    //****************************************************************************/
    // setTickVisibility()
    //****************************************************************************/
    void setTickVisibility(bool _drawMajorTicks, bool _drawMinorTicks) {
      drawMajorTicks = _drawMajorTicks;
      drawMinorTicks = _drawMinorTicks;
      isToUpdateInGpu = true;
    }

    //****************************************************************************/
    // setTickInward() - true => tick solo verso l'interno, false => simmetrici
    //****************************************************************************/
    void setTickInward(bool _tickInward) {
      tickInward = _tickInward;
      isToUpdateInGpu = true;
    }

    //****************************************************************************/
    // setLabelVisibility()
    //****************************************************************************/
    void setLabelVisibility(bool _drawAxisLabels, bool _drawTickLabels) {
      drawAxisLabels = _drawAxisLabels;
      drawTickLabels = _drawTickLabels;
    }

    //****************************************************************************/
    // setTickSize()
    //****************************************************************************/
    void setTickSize(float _majorTickSize, float _minorTickSize) {
      majorTickSize = (_majorTickSize > 0.0f) ? _majorTickSize : majorTickSize;
      minorTickSize = (_minorTickSize > 0.0f) ? _minorTickSize : minorTickSize;
      isToUpdateInGpu = true;
    }

    //****************************************************************************/
    // setLineThickness()
    //****************************************************************************/
    void setLineThickness(float _lineThickness) { setLineWidth(_lineThickness); }

    //****************************************************************************/
    // setLabelScale()
    //****************************************************************************/
    void setLabelScale(float _axisLabelScale, float _tickLabelScale = -1.0f) {
      axisLabelScale = (_axisLabelScale > 0.0f) ? _axisLabelScale : axisLabelScale;
      if(_tickLabelScale > 0.0f) tickLabelScale = _tickLabelScale;
    }

    //****************************************************************************/
    // setLabelOffsets() - distanza delle scritte dall'asse (0 => default auto)
    //****************************************************************************/
    void setLabelOffsets(float _axisLabelOffset, float _tickLabelOffset = -1.0f) {
      if(_axisLabelOffset >= 0.0f) axisLabelOffset = _axisLabelOffset;
      if(_tickLabelOffset >= 0.0f) tickLabelOffset = _tickLabelOffset;
    }

    //****************************************************************************/
    // setAxisLabels()
    //****************************************************************************/
    void setAxisLabels(const std::string & _xAxisLabel, const std::string & _yAxisLabel, const std::string & _zAxisLabel) {
      xAxisLabel = _xAxisLabel;
      yAxisLabel = _yAxisLabel;
      zAxisLabel = _zAxisLabel;
    }

    //****************************************************************************/
    // setLogScale()
    //****************************************************************************/
    void setLogScale(bool _xLog, bool _yLog, bool _zLog) {
      xLog = _xLog;
      yLog = _yLog;
      zLog = _zLog;
      isToUpdateInGpu = true;
    }

    //****************************************************************************/
    // setColors()
    //****************************************************************************/
    void setColors(const glm::vec3 & _axisColor, const glm::vec3 & _majorTickColor, const glm::vec3 & _minorTickColor) {
      axisColor = _axisColor;
      majorTickColor = _majorTickColor;
      minorTickColor = _minorTickColor;
      isToUpdateInGpu = true;
    }

    //****************************************************************************/
    // setAxisOrigin()
    //****************************************************************************/
    void setAxisOrigin(const glm::vec3 & _axisOrigin) {
      axisOrigin = _axisOrigin;
      isToUpdateInGpu = true;
    }

    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera) {

      DEBUG_LOG("glPlot::render(" + name + ")");

      if(!isInited) {
        fprintf(stderr, "ERROR [glPlot]: must be initialized before rendering\n");
        abort();
      }

      if(isToInitInGpu()) initInGpu();

      shader.use();

      shader.setUniform("projection", camera.getProjection());
      shader.setUniform("view",       camera.getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("uniformColor", glm::vec4(1.0f));
      shader.setUniform("lineWidth",  lineWidth);
      shader.setUniform("viewport",   camera.getViewport());

      glBindVertexArray(vao);

      glDisable(GL_CULL_FACE);
      glEnableVertexAttribArray(1);

      glDrawElements(GL_LINES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);

      glDisableVertexAttribArray(1);
      glBindVertexArray(0);

      renderLabels(camera);

      glCheckError();
      
    }

  private:

    //****************************************************************************/
    // addSegment()
    //****************************************************************************/
    void addSegment(const glm::vec3 & from, const glm::vec3 & to, const glm::vec3 & color) {

      GLuint base = (GLuint)vertices.size();
      vertices.push_back(from);
      vertices.push_back(to);
      colors.push_back(glm::vec4(color, 1.0f));
      colors.push_back(glm::vec4(color, 1.0f));
      indices.push_back(base);
      indices.push_back(base + 1);
      
    }

    //****************************************************************************/
    // buildAxisLinearTicks()
    //****************************************************************************/
    void buildAxisLinearTicks(int axis, const AxisRange & range, float step, float tickSize, const glm::vec3 & color) {

      if(step <= 0.0f) return;

      const float eps = 1e-6f;
      float start = std::ceil(range.min / step) * step;

      for(float v = start; v <= range.max + eps; v += step) {

        if(step < majorTickStep && isOnMajorTick(v, range, majorTickStep)) continue;

        float axisValue = mapAxisValue(range, v, axisOriginByAxis(axis), false);

        // tick solo verso l'interno (t0=0) oppure simmetrico (t0=-tickSize)
        float t0 = tickInward ? 0.0f : -tickSize;
        float t1 = tickSize;

        if(axis == 0) addSegment(glm::vec3(axisValue, axisOrigin.y + t0, axisOrigin.z), glm::vec3(axisValue, axisOrigin.y + t1, axisOrigin.z), color);
        if(axis == 1) addSegment(glm::vec3(axisOrigin.x + t0, axisValue, axisOrigin.z), glm::vec3(axisOrigin.x + t1, axisValue, axisOrigin.z), color);
        if(axis == 2) addSegment(glm::vec3(axisOrigin.x, axisOrigin.y + t0, axisValue), glm::vec3(axisOrigin.x, axisOrigin.y + t1, axisValue), color);
      }
      
    }

    //****************************************************************************/
    // buildAxisLogTicks()
    //****************************************************************************/
    void buildAxisLogTicks(int axis, const AxisRange & range, float tickSize, const glm::vec3 & color, bool addMinor) {

      if(range.min <= 0.0f || range.max <= 0.0f) return;

      int expMin = (int)std::floor(std::log10(range.min));
      int expMax = (int)std::ceil(std::log10(range.max));

      for(int e = expMin; e <= expMax; ++e) {

        float major = std::pow(10.0f, (float)e);

        float t0 = tickInward ? 0.0f : -tickSize;
        float t1 = tickSize;

        if(major >= range.min && major <= range.max) {
          if(axis == 0) addSegment(glm::vec3(major, axisOrigin.y + t0, axisOrigin.z), glm::vec3(major, axisOrigin.y + t1, axisOrigin.z), color);
          if(axis == 1) addSegment(glm::vec3(axisOrigin.x + t0, major, axisOrigin.z), glm::vec3(axisOrigin.x + t1, major, axisOrigin.z), color);
          if(axis == 2) addSegment(glm::vec3(axisOrigin.x, axisOrigin.y + t0, major), glm::vec3(axisOrigin.x, axisOrigin.y + t1, major), color);
        }

        if(!addMinor) continue;

        float m0 = tickInward ? 0.0f : -minorTickSize;
        float m1 = minorTickSize;

        for(int i = 2; i < 10; ++i) {
          float minor = major * (float)i;
          if(minor >= range.max) break;
          if(minor < range.min) continue;
          if(axis == 0) addSegment(glm::vec3(minor, axisOrigin.y + m0, axisOrigin.z), glm::vec3(minor, axisOrigin.y + m1, axisOrigin.z), minorTickColor);
          if(axis == 1) addSegment(glm::vec3(axisOrigin.x + m0, minor, axisOrigin.z), glm::vec3(axisOrigin.x + m1, minor, axisOrigin.z), minorTickColor);
          if(axis == 2) addSegment(glm::vec3(axisOrigin.x, axisOrigin.y + m0, minor), glm::vec3(axisOrigin.x, axisOrigin.y + m1, minor), minorTickColor);
        }
      }
      
    }

    //****************************************************************************/
    // buildGeometry()
    //****************************************************************************/
    void buildGeometry() {

      vertices.clear();
      colors.clear();
      indices.clear();

      AxisRange xDrawRange = getDrawRange(xRange, axisOrigin.x, xLog);
      AxisRange yDrawRange = getDrawRange(yRange, axisOrigin.y, yLog);
      AxisRange zDrawRange = getDrawRange(zRange, axisOrigin.z, zLog);

      addSegment(glm::vec3(mapAxisValue(xRange, xDrawRange.min, axisOrigin.x, xLog), axisOrigin.y, axisOrigin.z), glm::vec3(mapAxisValue(xRange, xDrawRange.max, axisOrigin.x, xLog), axisOrigin.y, axisOrigin.z), axisColor);
      addSegment(glm::vec3(axisOrigin.x, mapAxisValue(yRange, yDrawRange.min, axisOrigin.y, yLog), axisOrigin.z), glm::vec3(axisOrigin.x, mapAxisValue(yRange, yDrawRange.max, axisOrigin.y, yLog), axisOrigin.z), axisColor);
      addSegment(glm::vec3(axisOrigin.x, axisOrigin.y, mapAxisValue(zRange, zDrawRange.min, axisOrigin.z, zLog)), glm::vec3(axisOrigin.x, axisOrigin.y, mapAxisValue(zRange, zDrawRange.max, axisOrigin.z, zLog)), axisColor);

      if(drawMajorTicks) {
        if(xLog) buildAxisLogTicks(0, xRange, majorTickSize, majorTickColor, false);
        else     buildAxisLinearTicks(0, xRange, majorTickStep, majorTickSize, majorTickColor);

        if(yLog) buildAxisLogTicks(1, yRange, majorTickSize, majorTickColor, false);
        else     buildAxisLinearTicks(1, yRange, majorTickStep, majorTickSize, majorTickColor);

        if(zLog) buildAxisLogTicks(2, zRange, majorTickSize, majorTickColor, false);
        else     buildAxisLinearTicks(2, zRange, majorTickStep, majorTickSize, majorTickColor);
      }

      if(drawMinorTicks) {
        if(xLog) buildAxisLogTicks(0, xRange, majorTickSize, minorTickColor, true);
        else     buildAxisLinearTicks(0, xRange, minorTickStep, minorTickSize, minorTickColor);

        if(yLog) buildAxisLogTicks(1, yRange, majorTickSize, minorTickColor, true);
        else     buildAxisLinearTicks(1, yRange, minorTickStep, minorTickSize, minorTickColor);

        if(zLog) buildAxisLogTicks(2, zRange, majorTickSize, minorTickColor, true);
        else     buildAxisLinearTicks(2, zRange, minorTickStep, minorTickSize, minorTickColor);
      }
      
    }

    //****************************************************************************/
    // maxAxisExtent() - massima ampiezza tra gli assi (per offset/scale di default)
    //****************************************************************************/
    float maxAxisExtent() const {
      float ex = std::fabs(xRange.max - xRange.min);
      float ey = std::fabs(yRange.max - yRange.min);
      float ez = std::fabs(zRange.max - zRange.min);
      return std::max(ex, std::max(ey, ez));
    }

    //****************************************************************************/
    // resolved offset delle scritte: usa quello impostato o un default automatico
    //****************************************************************************/
    float resolvedAxisLabelOffset() const {
      return (axisLabelOffset > 0.0f) ? axisLabelOffset : maxAxisExtent() * 0.03f;
    }
    float resolvedTickLabelOffset() const {
      return (tickLabelOffset > 0.0f) ? tickLabelOffset : maxAxisExtent() * 0.025f;
    }

    //****************************************************************************/
    // renderLabels()
    //****************************************************************************/
    void renderLabels(const glCamera & camera) {

      AxisRange xDrawRange = getDrawRange(xRange, axisOrigin.x, xLog);
      AxisRange yDrawRange = getDrawRange(yRange, axisOrigin.y, yLog);
      AxisRange zDrawRange = getDrawRange(zRange, axisOrigin.z, zLog);

      if(drawAxisLabels) {

        float xOff = resolvedAxisLabelOffset();
        float yOff = resolvedAxisLabelOffset();
        float zOff = resolvedAxisLabelOffset();

        // il nome dell'asse va al CENTRO dell'asse (punto medio del range), non in cima
        float xMid = 0.5f * (xDrawRange.min + xDrawRange.max);
        float yMid = 0.5f * (yDrawRange.min + yDrawRange.max);
        float zMid = 0.5f * (zDrawRange.min + zDrawRange.max);

        // X: offset in -Z → rimane sul piano XY (y=axisOrigin.y)
        axisLabelPrinter.render(camera, xAxisLabel, glm::vec3(mapAxisValue(xRange, xMid, axisOrigin.x, xLog), axisOrigin.y, axisOrigin.z - xOff), axisColor, axisLabelScale);
        // Y: offset in -X → rimane sul piano XY (z=axisOrigin.z)
        axisLabelPrinter.render(camera, yAxisLabel, glm::vec3(axisOrigin.x - yOff, mapAxisValue(yRange, yMid, axisOrigin.y, yLog), axisOrigin.z), axisColor, axisLabelScale);
        // Z: offset in -X → rimane sul piano XZ (y=axisOrigin.y)
        axisLabelPrinter.render(camera, zAxisLabel, glm::vec3(axisOrigin.x - zOff, axisOrigin.y, mapAxisValue(zRange, zMid, axisOrigin.z, zLog)), axisColor, axisLabelScale);
        
      }

      if(drawTickLabels && drawMajorTicks) {
        
        if(xLog) renderAxisLogTickLabels(camera, 0, xRange);
        else renderAxisLinearTickLabels(camera, 0, xRange, majorTickStep);

        if(yLog) renderAxisLogTickLabels(camera, 1, yRange);
        else renderAxisLinearTickLabels(camera, 1, yRange, majorTickStep);

        if(zLog) renderAxisLogTickLabels(camera, 2, zRange);
        else renderAxisLinearTickLabels(camera, 2, zRange, majorTickStep);
        
      }
      
    }

    //****************************************************************************/
    // renderAxisLinearTickLabels()
    //****************************************************************************/
    void renderAxisLinearTickLabels(const glCamera & camera, int axis, const AxisRange & range, float step) {

      if(step <= 0.0f) return;

      const float eps = 1e-6f;
      float start = std::ceil(range.min / step) * step;
      float off = resolvedTickLabelOffset();

      for(float v = start; v <= range.max + eps; v += step) {
        std::string label = formatTickValue(v);
        float axisValue = mapAxisValue(range, v, axisOriginByAxis(axis), false);
        if(axis == 0) tickLabelPrinter.render(camera, label, glm::vec3(axisValue, axisOrigin.y - off, axisOrigin.z), majorTickColor, tickLabelScale);
        if(axis == 1) tickLabelPrinter.render(camera, label, glm::vec3(axisOrigin.x - off, axisValue, axisOrigin.z), majorTickColor, tickLabelScale);
        if(axis == 2) tickLabelPrinter.render(camera, label, glm::vec3(axisOrigin.x, axisOrigin.y - off, axisValue), majorTickColor, tickLabelScale);
      }
      
    }

    //****************************************************************************/
    // renderAxisLogTickLabels()
    //****************************************************************************/
    void renderAxisLogTickLabels(const glCamera & camera, int axis, const AxisRange & range) {

      if(range.min <= 0.0f || range.max <= 0.0f) return;

      int expMin = (int)std::floor(std::log10(range.min));
      int expMax = (int)std::ceil(std::log10(range.max));
      float off = resolvedTickLabelOffset();

      for(int e = expMin; e <= expMax; ++e) {
        float major = std::pow(10.0f, (float)e);
        if(major < range.min || major > range.max) continue;

        std::string label = formatTickValue(major);
        if(axis == 0) tickLabelPrinter.render(camera, label, glm::vec3(major, axisOrigin.y - off, axisOrigin.z), majorTickColor, tickLabelScale);
        if(axis == 1) tickLabelPrinter.render(camera, label, glm::vec3(axisOrigin.x - off, major, axisOrigin.z), majorTickColor, tickLabelScale);
        if(axis == 2) tickLabelPrinter.render(camera, label, glm::vec3(axisOrigin.x, axisOrigin.y - off, major), majorTickColor, tickLabelScale);
      }
      
    }

    //****************************************************************************/
    // formatTickValue()
    //****************************************************************************/
    std::string formatTickValue(float value) const {

      float absV = std::fabs(value);
      std::ostringstream ss;
      if((absV > 0.0f && absV < 1e-3f) || absV >= 1e4f) {
        ss << std::scientific << std::setprecision(2) << value;
      } else {
        ss << std::fixed << std::setprecision(3) << value;
      }

      std::string out = ss.str();
      std::size_t epos = out.find('e');
      if(epos == std::string::npos) {
        while(out.size() > 1 && out.back() == '0') out.pop_back();
        if(!out.empty() && out.back() == '.') out.pop_back();
        if(out == "-0") out = "0";
      }
      return out;
    }

    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() override {

      DEBUG_LOG("glPlot::setInGpu(" + name + ")");

      cleanInGpu();

      buildGeometry();

      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3) + colors.size() * sizeof(glm::vec4), nullptr, GL_STATIC_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), vertices.data());
      glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), colors.size() * sizeof(glm::vec4), colors.data());

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
      glEnableVertexAttribArray(0);

      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)(vertices.size() * sizeof(glm::vec3)));
      glEnableVertexAttribArray(1);

      glGenBuffers(1, &ibo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      glCheckError();
      
    }

    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() override {

      if(isInitedInGpu) {

        if(vbo != 0) glDeleteBuffers(1, &vbo);
        if(ibo != 0) glDeleteBuffers(1, &ibo);
        if(vao != 0) glDeleteVertexArrays(1, &vao);

        vao = 0;
        vbo = 0;
        ibo = 0;

        isInitedInGpu = false;

      }

    }

    //****************************************************************************/
    // sanitizeRange()
    //****************************************************************************/
    AxisRange sanitizeRange(const AxisRange & range) const {

      AxisRange out = range;
      
      if(out.min > out.max) {
        float tmp = out.min;
        out.min = out.max;
        out.max = tmp;
      }
      
      if(std::fabs(out.max - out.min) < 1e-6f) {
        out.min -= 1.0f;
        out.max += 1.0f;
      }
      
      return out;
      
    }

    //****************************************************************************/
    // buildPaddedRange()
    //****************************************************************************/
    AxisRange buildPaddedRange(float minV, float maxV, float padding) const {

      AxisRange out;
      if(minV > maxV) {
        float tmp = minV;
        minV = maxV;
        maxV = tmp;
      }

      float span = maxV - minV;
      if(span < 1e-6f) span = 1.0f;

      float pad = span * ((padding >= 0.0f) ? padding : 0.0f);
      out.min = minV - pad;
      out.max = maxV + pad;

      return sanitizeRange(out);
    }

    //****************************************************************************/
    // mapAxisValue()
    //****************************************************************************/
    float mapAxisValue(const AxisRange & range, float value, float axisOriginValue, bool isLogScale) const {
      if(isLogScale) return value;
      if(range.max <= range.min) return axisOriginValue;
      return value - range.min + axisOriginValue;
    }

    //****************************************************************************/
    // axisOriginByAxis()
    //****************************************************************************/
    float axisOriginByAxis(int axis) const {
      if(axis == 1) return axisOrigin.y;
      if(axis == 2) return axisOrigin.z;
      return axisOrigin.x;
    }

    //****************************************************************************/
    // isOnMajorTick()
    //****************************************************************************/
    bool isOnMajorTick(float value, const AxisRange & range, float majorStep) const {

      if(majorStep <= 0.0f) return false;

      const float eps = 1e-4f;
      float majorStart = std::ceil(range.min / majorStep) * majorStep;
      float k = std::round((value - majorStart) / majorStep);
      float major = majorStart + k * majorStep;

      return std::fabs(value - major) <= eps;
    }

    //****************************************************************************/
    // getDrawRange()
    //****************************************************************************/
    AxisRange getDrawRange(const AxisRange & range, float /*originValue*/, bool /*isLogScale*/) const {
      return sanitizeRange(range);
    }

  };

} /* namespace ogl */

#endif /* _H_OGL_PLOT_H_ */
