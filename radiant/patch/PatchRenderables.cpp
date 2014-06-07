#include "PatchRenderables.h"

void RenderablePatchWireframe::render(const RenderInfo& info) const
{
    // No colour changing
    glDisableClientState(GL_COLOR_ARRAY);
    if (info.checkFlag(RENDER_VERTEX_COLOUR))
    {
        glColor3f(1, 1, 1);
    }

    if (m_tess.vertices.empty()) return;

    const render::VertexBuffer::Vertices& patchVerts = m_tess.vertices;

    // Vertex buffer to receive and render vertices
    render::VertexBuffer currentVBuf;

    std::size_t firstIndex = 0;
    for(std::size_t i = 0; i <= m_tess.curveTreeV.size(); ++i)
    {
        currentVBuf.addBatch(patchVerts.begin() + firstIndex,
                             m_tess.m_nArrayWidth);

        if(i == m_tess.curveTreeV.size()) break;

        if (!m_tess.curveTreeV[i]->isLeaf())
        {
            currentVBuf.addBatch(
                patchVerts.begin() + GLint(m_tess.curveTreeV[i]->index),
                m_tess.m_nArrayWidth
            );
        }

        firstIndex += (m_tess.arrayHeight[i]*m_tess.m_nArrayWidth);
    }

    const ArbitraryMeshVertex* p = &patchVerts.front();
    std::size_t uStride = m_tess.m_nArrayWidth;
    for(std::size_t i = 0; i <= m_tess.curveTreeU.size(); ++i)
    {
        currentVBuf.addBatch(p, m_tess.m_nArrayHeight, uStride);

        if(i == m_tess.curveTreeU.size()) break;

        if(!m_tess.curveTreeU[i]->isLeaf())
        {
            currentVBuf.addBatch(
                patchVerts.begin() + m_tess.curveTreeU[i]->index,
                m_tess.m_nArrayHeight, uStride
            );
        }

        p += m_tess.arrayWidth[i];
    }

    // Render all vertex batches
    _vertexBuf.replaceData(currentVBuf);
    _vertexBuf.renderAllBatches(GL_LINE_STRIP);
}

void RenderablePatchFixedWireframe::render(const RenderInfo& info) const
{
    if (m_tess.vertices.empty() || m_tess.indices.empty()) return;

    // No colour changing
    glDisableClientState(GL_COLOR_ARRAY);
    if (info.checkFlag(RENDER_VERTEX_COLOUR))
    {
        glColor3f(1, 1, 1);
    }

    // Create a VBO and add the vertex data
    render::IndexedVertexBuffer currentVBuf;
    currentVBuf.addVertices(m_tess.vertices.begin(), m_tess.vertices.end());

    // Submit index batches
    const RenderIndex* strip_indices = &m_tess.indices.front();
    for (std::size_t i = 0;
         i < m_tess.m_numStrips;
         i++, strip_indices += m_tess.m_lenStrips)
    {
        currentVBuf.addIndexBatch(strip_indices, m_tess.m_lenStrips);
    }

    // Render all index batches
    _vertexBuf.replaceData(currentVBuf);
    _vertexBuf.renderAllBatches(GL_QUAD_STRIP);
}

RenderablePatchSolid::RenderablePatchSolid(PatchTesselation& tess) :
    m_tess(tess),
    _vboData(0),
    _vboIdx(0)
{
#ifdef PATCHES_USE_VBO
    // Allocate a vertex buffer object
    glGenBuffersARB(1, &_vboData);
    glGenBuffers(1, &_vboIdx);
#endif
}

void RenderablePatchSolid::update()
{
#ifdef PATCHES_USE_VBO
    // Space needed for geometry
    std::size_t dataSize = sizeof(ArbitraryMeshVertex)*m_tess.vertices.size();
    std::size_t indexSize = sizeof(RenderIndex)*m_tess.indices.size();

    // Initialise the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, _vboData);

    // Allocate space for vertices
    glBufferData(GL_ARRAY_BUFFER, dataSize, NULL, GL_STATIC_DRAW);

    GlobalOpenGL().assertNoErrors();

    // Upload the data
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, &m_tess.vertices[0]);

    GlobalOpenGL().assertNoErrors();

    // Initialise the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vboIdx);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, NULL, GL_STATIC_DRAW);

    GlobalOpenGL().assertNoErrors();

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexSize, &m_tess.indices[0]);

    GlobalOpenGL().assertNoErrors();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GlobalOpenGL().assertNoErrors();
#endif
}

void RenderablePatchSolid::render(const RenderInfo& info) const
{
#ifdef PATCHES_USE_VBO
    glBindBuffer(GL_ARRAY_BUFFER, _vboData);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vboIdx);

    glNormalPointer(GL_DOUBLE, sizeof(ArbitraryMeshVertex), BUFFER_OFFSET(16));
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_DOUBLE, sizeof(ArbitraryMeshVertex), BUFFER_OFFSET(0));
    glVertexPointer(3, GL_DOUBLE, sizeof(ArbitraryMeshVertex), BUFFER_OFFSET(40));

    const RenderIndex* strip_indices = 0;

    for (std::size_t i = 0; i < m_tess.m_numStrips; i++, strip_indices += m_tess.m_lenStrips)
    {
        glDrawElements(GL_QUAD_STRIP, GLsizei(m_tess.m_lenStrips), RenderIndexTypeID, strip_indices);
    }

    GlobalOpenGL().assertNoErrors();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
    if (m_tess.vertices.empty() || m_tess.indices.empty()) return;

    if (info.checkFlag(RENDER_BUMP))
    {
        glVertexAttribPointerARB(11, 3, GL_DOUBLE, 0, sizeof(ArbitraryMeshVertex), &m_tess.vertices.front().normal);
        glVertexAttribPointerARB(8, 2, GL_DOUBLE, 0, sizeof(ArbitraryMeshVertex), &m_tess.vertices.front().texcoord);
        glVertexAttribPointerARB(9, 3, GL_DOUBLE, 0, sizeof(ArbitraryMeshVertex), &m_tess.vertices.front().tangent);
        glVertexAttribPointerARB(10, 3, GL_DOUBLE, 0, sizeof(ArbitraryMeshVertex), &m_tess.vertices.front().bitangent);
    }
    else
    {
        glNormalPointer(GL_DOUBLE, sizeof(ArbitraryMeshVertex), &m_tess.vertices.front().normal);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_DOUBLE, sizeof(ArbitraryMeshVertex), &m_tess.vertices.front().texcoord);
    }

    // No colour changing
    glDisableClientState(GL_COLOR_ARRAY);
    if (info.checkFlag(RENDER_VERTEX_COLOUR))
    {
        glColor3f(1, 1, 1);
    }

    glVertexPointer(3, GL_DOUBLE, sizeof(ArbitraryMeshVertex), &m_tess.vertices.front().vertex);

    const RenderIndex* strip_indices = &m_tess.indices.front();

    for(std::size_t i = 0; i<m_tess.m_numStrips; i++, strip_indices += m_tess.m_lenStrips)
    {
        glDrawElements(GL_QUAD_STRIP, GLsizei(m_tess.m_lenStrips), RenderIndexTypeID, strip_indices);
    }

#if defined(_DEBUG)
    //RenderNormals();
#endif

#endif
}
