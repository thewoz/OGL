/*
 * GNU GENERAL PUBLIC LICENSE
 *
 * Copyright (C) 2026
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

    float majorTickStep = 1.0f;
    float minorTickStep = 0.5f;

    float majorTickSize = 0.03f;
    float minorTickSize = 0.015f;
    float axisLabelScale = 0.6f;
    float tickLabelScale = 0.45f;

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
    void render(const glCamera * camera) {

      DEBUG_LOG("glPlot::render(" + name + ")");

      if(!isInited) {
        fprintf(stderr, "glPlot must be inited before render\n");
        abort();
      }

      if(isToInitInGpu()) initInGpu();

      shader.use();

      shader.setUniform("projection", camera->getProjection());
      shader.setUniform("view",       camera->getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("uniformColor", glm::vec4(1.0f));
      shader.setUniform("lineWidth",  lineWidth);
      shader.setUniform("viewport",   camera->getViewport());

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

        if(axis == 0) addSegment(glm::vec3(v, axisOrigin.y - tickSize, axisOrigin.z), glm::vec3(v, axisOrigin.y + tickSize, axisOrigin.z), color);
        if(axis == 1) addSegment(glm::vec3(axisOrigin.x - tickSize, v, axisOrigin.z), glm::vec3(axisOrigin.x + tickSize, v, axisOrigin.z), color);
        if(axis == 2) addSegment(glm::vec3(axisOrigin.x, axisOrigin.y - tickSize, v), glm::vec3(axisOrigin.x, axisOrigin.y + tickSize, v), color);
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

        if(major >= range.min && major <= range.max) {
          if(axis == 0) addSegment(glm::vec3(major, axisOrigin.y - tickSize, axisOrigin.z), glm::vec3(major, axisOrigin.y + tickSize, axisOrigin.z), color);
          if(axis == 1) addSegment(glm::vec3(axisOrigin.x - tickSize, major, axisOrigin.z), glm::vec3(axisOrigin.x + tickSize, major, axisOrigin.z), color);
          if(axis == 2) addSegment(glm::vec3(axisOrigin.x, axisOrigin.y - tickSize, major), glm::vec3(axisOrigin.x, axisOrigin.y + tickSize, major), color);
        }

        if(!addMinor) continue;

        for(int i = 2; i < 10; ++i) {
          float minor = major * (float)i;
          if(minor >= range.max) break;
          if(minor < range.min) continue;
          if(axis == 0) addSegment(glm::vec3(minor, axisOrigin.y - minorTickSize, axisOrigin.z), glm::vec3(minor, axisOrigin.y + minorTickSize, axisOrigin.z), minorTickColor);
          if(axis == 1) addSegment(glm::vec3(axisOrigin.x - minorTickSize, minor, axisOrigin.z), glm::vec3(axisOrigin.x + minorTickSize, minor, axisOrigin.z), minorTickColor);
          if(axis == 2) addSegment(glm::vec3(axisOrigin.x, axisOrigin.y - minorTickSize, minor), glm::vec3(axisOrigin.x, axisOrigin.y + minorTickSize, minor), minorTickColor);
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

      addSegment(glm::vec3(xDrawRange.min, axisOrigin.y, axisOrigin.z), glm::vec3(xDrawRange.max, axisOrigin.y, axisOrigin.z), axisColor);
      addSegment(glm::vec3(axisOrigin.x, yDrawRange.min, axisOrigin.z), glm::vec3(axisOrigin.x, yDrawRange.max, axisOrigin.z), axisColor);
      addSegment(glm::vec3(axisOrigin.x, axisOrigin.y, zDrawRange.min), glm::vec3(axisOrigin.x, axisOrigin.y, zDrawRange.max), axisColor);

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
    // renderLabels()
    //****************************************************************************/
    void renderLabels(const glCamera * camera) {

      AxisRange xDrawRange = getDrawRange(xRange, axisOrigin.x, xLog);
      AxisRange yDrawRange = getDrawRange(yRange, axisOrigin.y, yLog);
      AxisRange zDrawRange = getDrawRange(zRange, axisOrigin.z, zLog);

      if(drawAxisLabels) {
        
        float xOff = majorTickSize * 2.2f;
        float yOff = majorTickSize * 2.2f;
        float zOff = majorTickSize * 2.2f;

        axisLabelPrinter.render(camera, xAxisLabel, glm::vec3(xDrawRange.max, axisOrigin.y + xOff, axisOrigin.z), axisColor, axisLabelScale);
        axisLabelPrinter.render(camera, yAxisLabel, glm::vec3(axisOrigin.x + yOff, yDrawRange.max, axisOrigin.z), axisColor, axisLabelScale);
        axisLabelPrinter.render(camera, zAxisLabel, glm::vec3(axisOrigin.x, axisOrigin.y + zOff, zDrawRange.max), axisColor, axisLabelScale);
        
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
    void renderAxisLinearTickLabels(const glCamera * camera, int axis, const AxisRange & range, float step) {

      if(step <= 0.0f) return;

      const float eps = 1e-6f;
      float start = std::ceil(range.min / step) * step;
      float off = majorTickSize * 2.0f;

      for(float v = start; v <= range.max + eps; v += step) {
        std::string label = formatTickValue(v);
        if(axis == 0) tickLabelPrinter.render(camera, label, glm::vec3(v, axisOrigin.y - off, axisOrigin.z), majorTickColor, tickLabelScale);
        if(axis == 1) tickLabelPrinter.render(camera, label, glm::vec3(axisOrigin.x - off, v, axisOrigin.z), majorTickColor, tickLabelScale);
        if(axis == 2) tickLabelPrinter.render(camera, label, glm::vec3(axisOrigin.x, axisOrigin.y - off, v), majorTickColor, tickLabelScale);
      }
      
    }

    //****************************************************************************/
    // renderAxisLogTickLabels()
    //****************************************************************************/
    void renderAxisLogTickLabels(const glCamera * camera, int axis, const AxisRange & range) {

      if(range.min <= 0.0f || range.max <= 0.0f) return;

      int expMin = (int)std::floor(std::log10(range.min));
      int expMax = (int)std::ceil(std::log10(range.max));
      float off = majorTickSize * 2.0f;

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
    void setInGpu() {

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
    void cleanInGpu() {

      if(vbo != 0) glDeleteBuffers(1, &vbo);
      if(ibo != 0) glDeleteBuffers(1, &ibo);
      if(vao != 0) glDeleteVertexArrays(1, &vao);

      vao = 0;
      vbo = 0;
      ibo = 0;
      
      isInitedInGpu = false;

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
    // getDrawRange()
    //****************************************************************************/
    AxisRange getDrawRange(const AxisRange & range, float originValue, bool isLogScale) const {

      AxisRange out = range;

      if(isLogScale) {
        if(originValue > 0.0f) {
          if(originValue < out.min) out.min = originValue;
          if(originValue > out.max) out.max = originValue;
        }
      } else {
        if(originValue < out.min) out.min = originValue;
        if(originValue > out.max) out.max = originValue;
      }

      return sanitizeRange(out);
    }

  };

} /* namespace ogl */

#endif /* _H_OGL_PLOT_H_ */
