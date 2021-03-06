#pragma once

#include "math/Vector3.h"
#include "math/Ray.h"
#include "iselectiontest.h"
#include "render/VertexNCb.h"
#include "render/VertexCb.h"

enum clipcull_t
{
    eClipCullNone,
    eClipCullCW,
    eClipCullCCW,
};

class Segment3D
{
public:
    // Constructor
    Segment3D(const Vector3& _p0, const Vector3& _p1) :
        p0(_p0),
        p1(_p1)
    {}

    Vector3 p0;
    Vector3 p1;
};

inline double triangle_signed_area_XY(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
    return ((p1[0] - p0[0]) * (p2[1] - p0[1])) - ((p2[0] - p0[0]) * (p1[1] - p0[1]));
}

// get the distance of a point to a segment.
inline Vector3 segment_closest_point_to_point(const Segment3D& segment, const Vector3& point)
{
    Vector3 v = segment.p1 - segment.p0;
    Vector3 w = point - segment.p0;

    double c1 = w.dot(v);
    if (c1 <= 0)
        return segment.p0;

    double c2 = v.dot(v);
    if (c2 <= c1)
        return segment.p1;

    return Vector3(segment.p0 + v * (c1 / c2));
}

// crossing number test for a point in a polygon
// This code is patterned after [Franklin, 2000]
inline bool point_test_polygon_2d(const Vector3& P, const Vector3* start, const Vector3* finish)
{
    std::size_t crossings = 0;

    // loop through all edges of the polygon
    const Vector3* cur = start;
    for (const Vector3* prev = finish - 1; cur != finish; prev = cur, ++cur)
    {    // edge from (*prev) to (*cur)
        if ((((*prev)[1] <= P[1]) && ((*cur)[1] > P[1]))    // an upward crossing
            || (((*prev)[1] > P[1]) && ((*cur)[1] <= P[1])))
        { // a downward crossing
          // compute the actual edge-ray intersect x-coordinate
            double vt = (double)(P[1] - (*prev)[1]) / ((*cur)[1] - (*prev)[1]);
            if (P[0] < (*prev)[0] + vt * ((*cur)[0] - (*prev)[0])) // P[0] < intersect
            {
                ++crossings;   // a valid crossing of y=P[1] right of P[0]
            }
        }
    }
    return (crossings & 0x1) != 0;    // 0 if even (out), and 1 if odd (in)
}

inline void BestPoint(std::size_t count, Vector4 clipped[9], SelectionIntersection& best, clipcull_t cull)
{
    Vector3 normalised[9];

    {
        for (std::size_t i = 0; i < count; ++i)
        {
            normalised[i][0] = clipped[i][0] / clipped[i][3];
            normalised[i][1] = clipped[i][1] / clipped[i][3];
            normalised[i][2] = clipped[i][2] / clipped[i][3];
        }
    }

    if (cull != eClipCullNone && count > 2)
    {
        double signed_area = triangle_signed_area_XY(normalised[0], normalised[1], normalised[2]);

        if ((cull == eClipCullCW && signed_area > 0)
            || (cull == eClipCullCCW && signed_area < 0))
            return;
    }

    if (count == 2)
    {
        Segment3D segment(normalised[0], normalised[1]);
        Vector3 point = segment_closest_point_to_point(segment, Vector3(0, 0, 0));
        best.assignIfCloser(SelectionIntersection(point.z(), 0));
    }
    else if (count > 2 && !point_test_polygon_2d(Vector3(0, 0, 0), normalised, normalised + count))
    {
        const Vector3* end = normalised + count;
        const Vector3* current = normalised;
        for (const Vector3* previous = end - 1; current != end; previous = current, ++current)
        {
            Segment3D segment(*previous, *current);
            Vector3 point = segment_closest_point_to_point(segment, Vector3(0, 0, 0));
            double depth = point.z();
            point.z() = 0;
            double distance = point.getLengthSquared();

            best.assignIfCloser(SelectionIntersection(depth, distance));
        }
    }
    else if (count > 2)
    {
        best.assignIfCloser(
            SelectionIntersection(
                static_cast<float>(
                    Ray(Vector3(0, 0, 0), Vector3(0, 0, 1)).getDistance(
                        Plane3(normalised[0], normalised[1], normalised[2])
                    )),
                0
            )
        );
    }
}

inline void LineStrip_BestPoint(const Matrix4& local2view, const VertexCb* vertices, const std::size_t size, SelectionIntersection& best)
{
    Vector4 clipped[2];
    for (std::size_t i = 0; (i + 1) < size; ++i)
    {
        const std::size_t count = local2view.clipLine(vertices[i].vertex, vertices[i + 1].vertex, clipped);
        BestPoint(count, clipped, best, eClipCullNone);
    }
}

inline void LineLoop_BestPoint(const Matrix4& local2view, const VertexCb* vertices, const std::size_t size, SelectionIntersection& best)
{
    Vector4 clipped[2];
    for (std::size_t i = 0; i < size; ++i)
    {
        const std::size_t count = local2view.clipLine(vertices[i].vertex, vertices[(i + 1) % size].vertex, clipped);
        BestPoint(count, clipped, best, eClipCullNone);
    }
}

inline void Line_BestPoint(const Matrix4& local2view, const VertexCb vertices[2], SelectionIntersection& best)
{
    Vector4 clipped[2];
    const std::size_t count = local2view.clipLine(vertices[0].vertex, vertices[1].vertex, clipped);
    BestPoint(count, clipped, best, eClipCullNone);
}

inline void Circle_BestPoint(const Matrix4& local2view, clipcull_t cull, const VertexCb* vertices, const std::size_t size, SelectionIntersection& best)
{
    Vector4 clipped[9];
    for (std::size_t i = 0; i < size; ++i)
    {
        const std::size_t count = local2view.clipTriangle(g_vector3_identity, vertices[i].vertex, vertices[(i + 1) % size].vertex, clipped);
        BestPoint(count, clipped, best, cull);
    }
}

inline void Quad_BestPoint(const Matrix4& local2view, clipcull_t cull, const VertexCb* vertices, SelectionIntersection& best)
{
    Vector4 clipped[9];
    {
        const std::size_t count = local2view.clipTriangle(vertices[0].vertex, vertices[1].vertex, vertices[3].vertex, clipped);
        BestPoint(count, clipped, best, cull);
    }
    {
        const std::size_t count = local2view.clipTriangle(vertices[1].vertex, vertices[2].vertex, vertices[3].vertex, clipped);
        BestPoint(count, clipped, best, cull);
    }
}

typedef VertexNCb* FlatShadedVertexIterator;
inline void Triangles_BestPoint(const Matrix4& local2view, clipcull_t cull, FlatShadedVertexIterator first, FlatShadedVertexIterator last, SelectionIntersection& best)
{
    for (FlatShadedVertexIterator x(first), y(first + 1), z(first + 2); x != last; x += 3, y += 3, z += 3)
    {
        Vector4 clipped[9];
        BestPoint(
            local2view.clipTriangle(
                x->vertex,
                y->vertex,
                z->vertex,
                clipped
            ),
            clipped,
            best,
            cull
        );
    }
}
