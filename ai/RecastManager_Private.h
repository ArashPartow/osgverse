#pragma once

#include <recastnavigation/Recast/Recast.h>
#include <recastnavigation/Detour/DetourCommon.h>
#include <recastnavigation/Detour/DetourNavMesh.h>
#include <recastnavigation/Detour/DetourNavMeshBuilder.h>
#include <recastnavigation/DetourTileCache/DetourTileCache.h>
#include <recastnavigation/DetourTileCache/DetourTileCacheBuilder.h>
#include <recastnavigation/DetourCrowd/DetourCrowd.h>
#include <modeling/Utilities.h>
#include <chrono>

namespace osgVerse
{

    static const int MAX_POLYS = 2048;
    struct FindPathData
    {
        dtPolyRef polygons[MAX_POLYS]{};       // Polygons
        dtPolyRef pathPolygons[MAX_POLYS]{};   // Polygons on the path
        osg::Vec3 pathPoints[MAX_POLYS];       // Points on the path
        unsigned char pathFlags[MAX_POLYS]{};  // Flags on the path
    };

    class BuildContext : public rcContext
    {
    public:
        BuildContext() : rcContext() {}

    protected:
        virtual void doResetLog() {}
        virtual void doResetTimers() { _timers.clear(); }

        virtual void doLog(const rcLogCategory category, const char* msg, const int len)
        {
            switch (category)
            {
            case RC_LOG_WARNING:
                OSG_WARN << "[BuildContext] Warning: " << std::string(msg, len) << std::endl;
            case RC_LOG_ERROR:
                OSG_FATAL << "[BuildContext] Error: " << std::string(msg, len) << std::endl;
            default:
                OSG_INFO << "[BuildContext] " << std::string(msg, len) << std::endl;
            }
        }

        virtual void doStartTimer(const rcTimerLabel label)
        { _timers[label].first = std::chrono::steady_clock::now(); }

        virtual void doStopTimer(const rcTimerLabel label)
        { _timers[label].second = std::chrono::steady_clock::now(); }

        virtual int doGetAccumulatedTime(const rcTimerLabel label) const
        {
            std::map<rcTimerLabel, TimePair>::const_iterator itr = _timers.find(label);
            if (itr == _timers.end()) return -1;
            return std::chrono::duration_cast<std::chrono::seconds>(itr->second.second - itr->second.first).count();
        }

        typedef std::pair<std::chrono::steady_clock::time_point, std::chrono::steady_clock::time_point> TimePair;
        std::map<rcTimerLabel, TimePair> _timers;
    };

    class NavData : public osg::Referenced
    {
    public:
        NavData() : navMesh(NULL), navQuery(NULL), crowd(NULL)
        { nearestReference = 0; context = new BuildContext; queryFilter = new dtQueryFilter; }

        static int calculateMaxTiles(const osg::BoundingBoxd& bb, osg::Vec2d& begin, osg::Vec2d& end,
                                     int tileSize, float cellSize)
        {
            if (!bb.valid()) return 0;
            const float tileEdgeLength = tileSize * cellSize;
            const osg::Vec2 beginTileIndex(bb.xMin() / tileEdgeLength, bb.yMin() / tileEdgeLength);
            const osg::Vec2 endTileIndex(bb.xMax() / tileEdgeLength, bb.yMax() / tileEdgeLength);
            const osg::Vec2 tileRange = endTileIndex - beginTileIndex + osg::Vec2();

            int numTiles = (int)tileRange.x() * (int)tileRange.y();
            begin = osg::Vec2d((int)beginTileIndex.x(), (int)beginTileIndex.y());
            end = osg::Vec2d((int)(endTileIndex.x() + 0.5f), (int)(endTileIndex.y() + 0.5f));
            return osg::Image::computeNearestPowerOfTwo(numTiles);
        }

        static unsigned int logBaseTwo(unsigned value)
        {
            // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogObvious
            unsigned ret = 0; while (value >>= 1) ++ret; return ret;
        }

        static void computeVelocity(float* vel, const float* pos, const float* tgt, const float speed)
        {
            dtVsub(vel, tgt, pos); vel[1] = 0.0;
            dtVnormalize(vel); dtVscale(vel, vel, speed);
        }

        void clear()
        {
            if (navMesh != NULL) dtFreeNavMesh(navMesh); navMesh = NULL;
            if (navQuery != NULL) dtFreeNavMeshQuery(navQuery); navQuery = NULL;
        }

        void clearCrowd()
        { if (!crowd) dtFreeCrowd(crowd); crowd = NULL; }

        dtNavMesh* navMesh;
        dtNavMeshQuery* navQuery;
        dtCrowd* crowd;
        dtQueryFilter* queryFilter;
        BuildContext* context;
        dtCrowdAgentDebugInfo agentDebugger;
        dtPolyRef nearestReference;
        FindPathData pathData;
        float nearestPointOnRef[3];

    protected:
        virtual ~NavData() { clear(); delete context; }
    };

}
