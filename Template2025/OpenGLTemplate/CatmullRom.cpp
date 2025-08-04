#include "CatmullRom.h"
#define _USE_MATH_DEFINES
#include <math.h>



CCatmullRom::CCatmullRom()
{
	m_vertexCount = 0;
}

CCatmullRom::~CCatmullRom()
{
	m_texture.Release();
	m_trackVBO.Release();
	if (m_vaoTrack != 0) {
		glDeleteVertexArrays(1, &m_vaoTrack);
	}
}

// Perform Catmull Rom spline interpolation between four points, interpolating the space between p1 and p2
glm::vec3 CCatmullRom::Interpolate(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, float t)
{
	float t2 = t * t;
	float t3 = t2 * t;

	glm::vec3 a = p1;
	glm::vec3 b = 0.5f * (-p0 + p2);
	glm::vec3 c = 0.5f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3);
	glm::vec3 d = 0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3);

	return a + b * t + c * t2 + d * t3;

}


void CCatmullRom::SetControlPoints()
{
	// Set control points (m_controlPoints) here, or load from disk
	m_controlPoints.clear();

	const float trackHeight = 0.5f;

	m_controlPoints.push_back(glm::vec3(0.0f, trackHeight, 0.0f));
	m_controlPoints.push_back(glm::vec3(100.0f, trackHeight, 0.0f));
	m_controlPoints.push_back(glm::vec3(150.0f, trackHeight, 50.0f));
	m_controlPoints.push_back(glm::vec3(150.0f, trackHeight, 100.0f));
	m_controlPoints.push_back(glm::vec3(100.0f, trackHeight, 150.0f));
	m_controlPoints.push_back(glm::vec3(0.0f, trackHeight, 150.0f));
	m_controlPoints.push_back(glm::vec3(-50.0f, trackHeight, 100.0f));
	m_controlPoints.push_back(glm::vec3(-50.0f, trackHeight, 0.0f));
	m_controlPoints.push_back(glm::vec3(0.0f, trackHeight, 0.0f));

	// Optionally, set upvectors (m_controlUpVectors, one for each control point as well)
	m_controlUpVectors.clear();
	for (int i = 0; i < m_controlPoints.size(); i++) {
		m_controlUpVectors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
	}

}


// Determine lengths along the control points, which is the set of control points forming the closed curve
void CCatmullRom::ComputeLengthsAlongControlPoints()
{
	int M = (int)m_controlPoints.size();

	float fAccumulatedLength = 0.0f;
	m_distances.push_back(fAccumulatedLength);
	for (int i = 1; i < M; i++) {
		fAccumulatedLength += glm::distance(m_controlPoints[i - 1], m_controlPoints[i]);
		m_distances.push_back(fAccumulatedLength);
	}

	// Get the distance from the last point to the first
	fAccumulatedLength += glm::distance(m_controlPoints[M - 1], m_controlPoints[0]);
	m_distances.push_back(fAccumulatedLength);
}


// Return the point (and upvector, if control upvectors provided) based on a distance d along the control polygon
bool CCatmullRom::Sample(float d, glm::vec3& p, glm::vec3& up)
{
	if (d < 0)
		return false;

	int M = (int)m_controlPoints.size();
	if (M == 0)
		return false;


	float fTotalLength = m_distances[m_distances.size() - 1];

	// The the current length along the control polygon; handle the case where we've looped around the track
	float fLength = d - (int)(d / fTotalLength) * fTotalLength;

	// Find the current segment
	int j = -1;
	for (int i = 0; i < (int)m_distances.size() - 1; i++) {
		if (fLength >= m_distances[i] && fLength < m_distances[i + 1]) {
			j = i; // found it!
			break;
		}
	}

	if (j == -1)
		return false;

	// Interpolate on current segment -- get t
	float fSegmentLength = m_distances[j + 1] - m_distances[j];
	float t = (fLength - m_distances[j]) / fSegmentLength;

	// Get the indices of the four points along the control polygon for the current segment
	int iPrev = ((j - 1) + M) % M;
	int iCur = j;
	int iNext = (j + 1) % M;
	int iNextNext = (j + 2) % M;

	// Interpolate to get the point (and upvector)
	p = Interpolate(m_controlPoints[iPrev], m_controlPoints[iCur], m_controlPoints[iNext], m_controlPoints[iNextNext], t);
	if (m_controlUpVectors.size() == m_controlPoints.size())
		up = glm::normalize(Interpolate(m_controlUpVectors[iPrev], m_controlUpVectors[iCur], m_controlUpVectors[iNext], m_controlUpVectors[iNextNext], t));

	return true;
}



// Sample a set of control points using an open Catmull-Rom spline, to produce a set of iNumSamples that are (roughly) equally spaced
void CCatmullRom::UniformlySampleControlPoints(int numSamples)
{
	glm::vec3 p, up;

	// Compute the lengths of each segment along the control polygon, and the total length
	ComputeLengthsAlongControlPoints();
	float fTotalLength = m_distances[m_distances.size() - 1];

	// The spacing will be based on the control polygon
	float fSpacing = fTotalLength / numSamples;

	// Call PointAt to sample the spline, to generate the points
	for (int i = 0; i < numSamples; i++) {
		Sample(i * fSpacing, p, up);
		m_centrelinePoints.push_back(p);
		if (m_controlUpVectors.size() > 0)
			m_centrelineUpVectors.push_back(up);

	}


	// Repeat once more for truly equidistant points
	m_controlPoints = m_centrelinePoints;
	m_controlUpVectors = m_centrelineUpVectors;
	m_centrelinePoints.clear();
	m_centrelineUpVectors.clear();
	m_distances.clear();
	ComputeLengthsAlongControlPoints();
	fTotalLength = m_distances[m_distances.size() - 1];
	fSpacing = fTotalLength / numSamples;
	for (int i = 0; i < numSamples; i++) {
		Sample(i * fSpacing, p, up);
		m_centrelinePoints.push_back(p);
		if (m_controlUpVectors.size() > 0)
			m_centrelineUpVectors.push_back(up);
	}


}



void CCatmullRom::CreateCentreline()
{
	// Call Set Control Points
	SetControlPoints();

	// Call UniformlySampleControlPoints with the number of samples required
	UniformlySampleControlPoints(1000);

	// Create a VAO called m_vaoCentreline and a VBO to get the points onto the graphics card
	glGenVertexArrays(1, &m_vaoCentreline);
	glBindVertexArray(m_vaoCentreline);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, m_centrelinePoints.size() * sizeof(glm::vec3), &m_centrelinePoints[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

}


void CCatmullRom::CreateOffsetCurves()
{
	// Compute the offset curves, one left, and one right.  Store the points in m_leftOffsetPoints and m_rightOffsetPoints respectively
	const float trackWidth = 10.0f; // Adjust this value for desired track width

	m_leftOffsetPoints.clear();
	m_rightOffsetPoints.clear();

	for (size_t i = 0; i < m_centrelinePoints.size(); i++) {
		glm::vec3 forward = (i < m_centrelinePoints.size() - 1) ?
			glm::normalize(m_centrelinePoints[i + 1] - m_centrelinePoints[i]) :
			glm::normalize(m_centrelinePoints[0] - m_centrelinePoints[i]);

		glm::vec3 right = glm::normalize(glm::cross(forward, m_centrelineUpVectors[i]));

		m_leftOffsetPoints.push_back(m_centrelinePoints[i] - right * trackWidth);
		m_rightOffsetPoints.push_back(m_centrelinePoints[i] + right * trackWidth);
	}

	// Generate two VAOs called m_vaoLeftOffsetCurve and m_vaoRightOffsetCurve, each with a VBO, and get the offset curve points on the graphics card
	// Note it is possible to only use one VAO / VBO with all the points instead.
	glGenVertexArrays(1, &m_vaoLeftOffsetCurve);
	glBindVertexArray(m_vaoLeftOffsetCurve);
	GLuint vboLeft;
	glGenBuffers(1, &vboLeft);
	glBindBuffer(GL_ARRAY_BUFFER, vboLeft);
	glBufferData(GL_ARRAY_BUFFER, m_leftOffsetPoints.size() * sizeof(glm::vec3), &m_leftOffsetPoints[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenVertexArrays(1, &m_vaoRightOffsetCurve);
	glBindVertexArray(m_vaoRightOffsetCurve);
	GLuint vboRight;
	glGenBuffers(1, &vboRight);
	glBindBuffer(GL_ARRAY_BUFFER, vboRight);
	glBufferData(GL_ARRAY_BUFFER, m_rightOffsetPoints.size() * sizeof(glm::vec3), &m_rightOffsetPoints[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

}

void CCatmullRom::CreateTrack()
{
	vector<glm::vec3> vertices;

	// Create vertices for track segments
	for (size_t i = 0; i < m_centrelinePoints.size() - 1; ++i) {
		// Add vertices for current segment
		vertices.push_back(m_leftOffsetPoints[i]);
		vertices.push_back(m_rightOffsetPoints[i]);
		vertices.push_back(m_rightOffsetPoints[i + 1]);
		vertices.push_back(m_leftOffsetPoints[i + 1]);
	}

	// Create and bind VAO
	glGenVertexArrays(1, &m_vaoTrack);
	glBindVertexArray(m_vaoTrack);

	// Create and bind VBO
	m_trackVBO.Create();
	m_trackVBO.Bind();

	// Add data to VBO
	for (size_t i = 0; i < vertices.size(); i++) {
		m_trackVBO.AddData(vertices.data() + i, sizeof(glm::vec3));
	}

	m_trackVBO.UploadDataToGPU(GL_STATIC_DRAW);

	// Set up vertex attributes
	glEnableVertexAttribArray(0); // position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	m_vertexCount = vertices.size();

	glBindVertexArray(0);
}



void CCatmullRom::RenderCentreline()
{
	// Bind the VAO m_vaoCentreline and render it
	glBindVertexArray(m_vaoCentreline);
	glDrawArrays(GL_LINE_STRIP, 0, m_centrelinePoints.size());
}


void CCatmullRom::RenderOffsetCurves()
{
	// Bind the VAO m_vaoLeftOffsetCurve and render it
	glBindVertexArray(m_vaoLeftOffsetCurve);
	glDrawArrays(GL_LINE_STRIP, 0, m_leftOffsetPoints.size());

	// Bind the VAO m_vaoRightOffsetCurve and render it
	glBindVertexArray(m_vaoRightOffsetCurve);
	glDrawArrays(GL_LINE_STRIP, 0, m_rightOffsetPoints.size());
}


void CCatmullRom::RenderTrack()
{
	glBindVertexArray(m_vaoTrack);
	glDrawArrays(GL_QUADS, 0, m_vertexCount);
	glBindVertexArray(0);
}


int CCatmullRom::CurrentLap(float d)
{

	return (int)(d / m_distances.back());

}

glm::vec3 CCatmullRom::_dummy_vector(0.0f, 0.0f, 0.0f);