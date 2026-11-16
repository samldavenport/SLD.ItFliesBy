#ifndef IFB_PROFILE_HPP
#define IFB_PROFILE_HPP

//--------------------------------------------------------------------
// PROFILING
//--------------------------------------------------------------------
// thin wrapper over tracy, so the rest of the code never includes it directly
// IFB_PROFILE_ENABLED comes from cmake (IFB_ENABLE_PROFILING), without it every macro compiles away
// connect with tracy-profiler.exe (vcpkg_installed/x64-windows/tools/tracy) while the game is running

#ifdef IFB_PROFILE_ENABLED

#   include <tracy/Tracy.hpp>

    // a zone named after the enclosing function, ends with the scope
#   define IFB_PROFILE_FUNCTION()           ZoneScoped
    // a zone with its own name, for a block inside a function
#   define IFB_PROFILE_SCOPE(name)          ZoneScopedN(name)
    // marks the end of a rendered frame
#   define IFB_PROFILE_FRAME()              FrameMark
    // names the calling thread in the profiler
#   define IFB_PROFILE_THREAD(name)         tracy::SetThreadName(name)
    // plots a value over time
#   define IFB_PROFILE_PLOT(name, value)    TracyPlot(name, value)

#else

#   define IFB_PROFILE_FUNCTION()
#   define IFB_PROFILE_SCOPE(name)
#   define IFB_PROFILE_FRAME()
#   define IFB_PROFILE_THREAD(name)
#   define IFB_PROFILE_PLOT(name, value)

#endif

#endif //IFB_PROFILE_HPP
