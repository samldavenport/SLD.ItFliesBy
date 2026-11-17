#pragma once

#include "ifb-gui.hpp"

//--------------------------------------------------------------------
// PROFILER
//--------------------------------------------------------------------
// the tracy viewer is a separate program, so this starts it pointed at the running game
// IFB_TRACY_PROFILER_EXE comes from cmake when profiling is on and tracy-profiler.exe is installed

namespace ifb {

#if defined(IFB_PROFILE_ENABLED) && defined(IFB_TRACY_PROFILER_EXE)

    static HANDLE _gui_profiler_process = NULL;

    IFB_INTERNAL bool
    gui_engine_profiler_is_available(
        void) {

        return(true);
    }

    IFB_INTERNAL bool
    gui_engine_profiler_is_running(
        void) {

        if (_gui_profiler_process == NULL) return(false);

        // the viewer was closed, let go of it so it can be opened again
        if (WaitForSingleObject(_gui_profiler_process, 0) != WAIT_TIMEOUT) {
            CloseHandle(_gui_profiler_process);
            _gui_profiler_process = NULL;
            return(false);
        }
        return(true);
    }

    IFB_INTERNAL void
    gui_engine_profiler_open(
        void) {

        if (gui_engine_profiler_is_running()) return;

        // -a connects to the game as soon as the viewer opens
        // CreateProcessA can write to the command line, so it can't be a literal
        char cmd_line[1024];
        const int len = snprintf(cmd_line, sizeof(cmd_line), "\"%s\" -a 127.0.0.1", IFB_TRACY_PROFILER_EXE);
        assert(len > 0 && len < (int)sizeof(cmd_line));

        STARTUPINFOA        startup = {};
        PROCESS_INFORMATION process = {};
        startup.cb = sizeof(startup);

        const BOOL created = CreateProcessA(
            IFB_TRACY_PROFILER_EXE,
            cmd_line,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &startup,
            &process
        );
        if (!created) return;

        CloseHandle(process.hThread);
        _gui_profiler_process = process.hProcess;
    }

#else

    IFB_INTERNAL bool gui_engine_profiler_is_available (void) { return(false); }
    IFB_INTERNAL bool gui_engine_profiler_is_running   (void) { return(false); }
    IFB_INTERNAL void gui_engine_profiler_open         (void) { }

#endif

};
