/* OutlineShader.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "OutlineShader.h"

#include "Color.h"
#include "Point.h"
#include "Screen.h"
#include "Shader.h"
#include "Sprite.h"

using namespace std;

namespace {
	Shader shader;
	GLint scaleI;
	GLint transformI;
	GLint positionI;
	GLint colorI;

	GLuint vao;
	GLuint vbo;
}



void OutlineShader::Init()
{
	static const char *vertexCode =
		"uniform mat2 transform;\n"
		"uniform vec2 position;\n"
		"uniform vec2 scale;\n"

		//by lusky
		//"in vec2 vert;\n"
		//"in vec2 vertTexCoord;\n"
		//"out vec2 tc;\n"
		//"out vec2 off;\n"
		"attribute vec2 vert;\n"
		"attribute vec2 vertTexCoord;\n"
		"varying vec2 tc;\n"
		"varying vec2 off;\n"

		"void main() {\n"
		"  tc = vertTexCoord;\n"
		"  mat2 sq = matrixCompMult(transform, transform);\n"
		"  off = vec2(.5 / sqrt(sq[0][0] + sq[0][1]), .5 / sqrt(sq[1][0] + sq[1][1]));\n"
		"  gl_Position = vec4((transform * vert + position) * scale, 0, 1);\n"
		"}\n";

	static const char *fragmentCode =
		"uniform sampler2D tex;\n"
		"uniform vec4 color = vec4(1, 1, 1, 1);\n"

		//by lusky
//		"in vec2 tc;\n"
//		"in vec2 off;\n"
//		"out vec4 finalColor;\n"
		"varying vec2 tc;\n"
		"varying vec2 off;\n"

		"void main() {\n"
		"  float sum = 0;\n"
		"  for(int dy = -1; dy <= 1; ++dy)\n"
		"  {\n"
		"    for(int dx = -1; dx <= 1; ++dx)\n"
		"    {\n"
		"      vec2 d = vec2(.618 * dx * off.x, .618 * dy * off.x);\n"
		//"      vec2 d = vec2(.0, .0);\n"

		//by lusky
//		"      float ae = texture(tex, d + vec2(tc.x - off.x, tc.y)).a;\n"
//		"      float aw = texture(tex, d + vec2(tc.x + off.x, tc.y)).a;\n"
//		"      float an = texture(tex, d + vec2(tc.x, tc.y - off.y)).a;\n"
//		"      float as = texture(tex, d + vec2(tc.x, tc.y + off.y)).a;\n"
//		"      float ane = texture(tex, d + vec2(tc.x - off.x, tc.y - off.y)).a;\n"
//		"      float anw = texture(tex, d + vec2(tc.x + off.x, tc.y - off.y)).a;\n"
//		"      float ase = texture(tex, d + vec2(tc.x - off.x, tc.y + off.y)).a;\n"
//		"      float asw = texture(tex, d + vec2(tc.x + off.x, tc.y + off.y)).a;\n"
		"      float ae = texture2D(tex, d + vec2(tc.x - off.x, tc.y)).a;\n"
		"      float aw = texture2D(tex, d + vec2(tc.x + off.x, tc.y)).a;\n"
		"      float an = texture2D(tex, d + vec2(tc.x, tc.y - off.x)).a;\n"
		"      float as = texture2D(tex, d + vec2(tc.x, tc.y + off.x)).a;\n"
		"      float ane = texture2D(tex, d + vec2(tc.x - off.x, tc.y - off.x)).a;\n"
		"      float anw = texture2D(tex, d + vec2(tc.x + off.x, tc.y - off.x)).a;\n"
		"      float ase = texture2D(tex, d + vec2(tc.x - off.x, tc.y + off.x)).a;\n"
		"      float asw = texture2D(tex, d + vec2(tc.x + off.x, tc.y + off.x)).a;\n"

		"      float h = (ae * 2 + ane + ase) - (aw * 2 + anw + asw);\n"
		"      float v = (an * 2 + ane + anw) - (as * 2 + ase + asw);\n"

		"      sum += h * h + v * v;\n"
		"    }\n"
		"  }\n"

		//by lusky
//		"  finalColor = color * sqrt(sum / 144);\n"
		"  gl_FragColor = color * sqrt(sum / 144);\n"

		"}\n";

	shader = Shader(vertexCode, fragmentCode);
	scaleI = shader.Uniform("scale");
	transformI = shader.Uniform("transform");
	positionI = shader.Uniform("position");
	colorI = shader.Uniform("color");

	//glUniform1ui(shader.Uniform("tex"), 0);
	//by lusky
    glUniform1i(shader.Uniform("tex"), 0);

	// Generate the vertex data for drawing sprites.
	//by lusky
	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLfloat vertexData[] = {
		-.5f, -.5f, 0.f, 0.f,
		 .5f, -.5f, 1.f, 0.f,
		-.5f,  .5f, 0.f, 1.f,
		 .5f,  .5f, 1.f, 1.f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader.Attrib("vert"));
	glVertexAttribPointer(shader.Attrib("vert"), 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);

	glEnableVertexAttribArray(shader.Attrib("vertTexCoord"));
	glVertexAttribPointer(shader.Attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE,
		4 * sizeof(GLfloat), (const GLvoid*)(2 * sizeof(GLfloat)));

	//by lusky
	//glBindVertexArray(0);
}



void OutlineShader::Draw(const Sprite *sprite, const Point &pos, const Point &size, const Color &color, const Point &unit)
{
	glUseProgram(shader.Object());
	//by lusky
	//glBindVertexArray(vao);
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(shader.Attrib("vert"));
	glVertexAttribPointer(shader.Attrib("vert"), 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);

	glEnableVertexAttribArray(shader.Attrib("vertTexCoord"));
	glVertexAttribPointer(shader.Attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE,
		4 * sizeof(GLfloat), (const GLvoid*)(2 * sizeof(GLfloat)));
	//end by lusky

	glActiveTexture(GL_TEXTURE0);

	GLfloat scale[2] = {2.f / Screen::Width(), -2.f / Screen::Height()};
	glUniform2fv(scaleI, 1, scale);

	Point uw = unit * size.X();
	Point uh = unit * size.Y();
	GLfloat transform[4] = {
		static_cast<float>(-uw.Y()),
		static_cast<float>(uw.X()),
		static_cast<float>(-uh.X()),
		static_cast<float>(-uh.Y())
	};
	glUniformMatrix2fv(transformI, 1, false, transform);

	GLfloat position[2] = {
		static_cast<float>(pos.X()), static_cast<float>(pos.Y())};
	glUniform2fv(positionI, 1, position);

	glUniform4fv(colorI, 1, color.Get());

	glBindTexture(GL_TEXTURE_2D, sprite->Texture());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//by lusky
	//glBindVertexArray(0);
    // unbind the VBO and VAO
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glPopClientAttrib();

	glUseProgram(0);
}
