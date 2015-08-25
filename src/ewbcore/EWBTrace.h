/*
 * EWBTrace.h
 *
 *  Created on: Aug 11, 2015
 *      Author: Benoit Rat (benoit<AT>sevensols.com)
 */

#ifndef EWBTRACE_H_
#define EWBTRACE_H_



#if defined(TRACE_STDERR)
#define errlogPrintf(...) fprintf(stderr,__VA_ARGS__)
#else
#include <epicsPrint.h>
#endif
//------------------------------------------------------------------------------
//         Global Definitions
//------------------------------------------------------------------------------

#define TRACE_LEVEL_DEBUG      5
#define TRACE_LEVEL_INFO       4
#define TRACE_LEVEL_WARNING    3
#define TRACE_LEVEL_ERROR      2
#define TRACE_LEVEL_FATAL      1
#define TRACE_LEVEL_NO_TRACE   0

// By default, all traces are output except the debug one.
#if !defined(TRACE_LEVEL)
#define TRACE_LEVEL TRACE_LEVEL_DEBUG
#endif

// By default, trace level is static (not dynamic)
#if !defined(DYN_TRACES)
#define DYN_TRACES 0
#endif

#if defined(NOTRACE)
#error "Error: NOTRACE has to be not defined !"
#endif

#undef NOTRACE
#if (DYN_TRACES==0)
    #if (TRACE_LEVEL == TRACE_LEVEL_NO_TRACE)
        #define NOTRACE
    #endif
#endif

//------------------------------------------------------------------------------
//         Global Macros
//------------------------------------------------------------------------------





//------------------------------------------------------------------------------
/// Outputs a formatted string using <printf> if the log level is high
/// enough. Can be disabled by defining TRACE_LEVEL=0 during compilation.
/// \param format  Formatted string to output.
/// \param ...  Additional parameters depending on formatted string.
//------------------------------------------------------------------------------
#if defined(NOTRACE)

// Empty macro
#define TRACE_P_DEBUG(...)      { }
#define TRACE_P_INFO(...)       { }
#define TRACE_P_WARNING(...)    { }
#define TRACE_P_ERROR(...)      { }
#define TRACE_P_FATAL(...)      { while(1); }

#else

#define TRACE_FILE stderr

#endif




#if defined(DYN_TRACES_EDES)

// Trace output depends on traceLevel value
#define TRACE(level,prefix,...) { if (traceLevel >= level) { errlogPrintf(prefix __VA_ARGS__); } }


#define TRACE_P_DEBUG(...)      TRACE(TRACE_LEVEL_DEBUG,   "-D- ", __VA_ARGS__)
#define TRACE_P_INFO(...)       TRACE(TRACE_LEVEL_INFO,    "-I- ", __VA_ARGS__)
#define TRACE_P_WARNING(...)    TRACE(TRACE_LEVEL_WARNING, "-W- ", __VA_ARGS__)
#define TRACE_P_ERROR(...)      TRACE(TRACE_LEVEL_ERROR,   "-E- ", __VA_ARGS__)
#define TRACE_P_FATAL(...)      { TRACE(TRACE_LEVEL_FATAL, "-F- ", __VA_ARGS__); while(1); }

#else

// Trace output depends on traceLevel value
#define TRACE(level,...) { if (TRACE_LEVEL >= level) { errlogPrintf(__VA_ARGS__); } }

#define END_PRINT errlogPrintf("\n")
#define S(x) #x
#define S_(x) S(x)
#if defined(WIN32)
	#define BEG_PRINT(lvlstr)
	#define MYLINE __FUNCTION__ "() #" S_(__LINE__) ": "
#else
	#define BEG_PRINT(lvlstr) errlogPrintf("%s#%03d %12s(): ",lvlstr, __LINE__, __func__)
	#define MYLINE
#endif

#define TRACE_P_PRINT(lvlstr,...) { BEG_PRINT(lvlstr); errlogPrintf(MYLINE __VA_ARGS__); END_PRINT; }

// Trace compilation depends on TRACE_LEVEL value
#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
#define TRACE_P_DEBUG(...) TRACE_P_PRINT("-D- ",__VA_ARGS__)
#else
#define TRACE_P_DEBUG(...)      { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_INFO)
#define TRACE_P_INFO(...)       TRACE_P_PRINT("-I- ",__VA_ARGS__)
#else
#define TRACE_P_INFO(...)       { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_WARNING)
#define TRACE_P_WARNING(...)    TRACE_P_PRINT("-W- ",__VA_ARGS__)
#else
#define TRACE_P_WARNING(...)    { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_ERROR)
#define TRACE_P_ERROR(...)     TRACE_P_PRINT("-E- ",__VA_ARGS__)
#else
#define TRACE_P_ERROR(...)      { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_FATAL)
#define TRACE_P_FATAL(...)      { TRACE_P_PRINT("-F- ",__VA_ARGS__) while(1); }
#else
#define TRACE_P_FATAL(...)      { while(1); }
#endif

#endif

#define TRACE_CHECK(assert_expr,ret,str) { if(!(assert_expr)) { TRACE_P_WARNING(str); return ret; } }
#define TRACE_CHECK_VA(assert_expr,ret,str,...) { if(!(assert_expr)) { TRACE_P_WARNING(str,__VA_ARGS__); return ret; } }
#define TRACE_CHECK_PTR(ptr, ret) { if(ptr==0) { TRACE_P_WARNING("%s is NULL",#ptr); return ret; }} //EFAULT

//------------------------------------------------------------------------------
//         Exported variables
//------------------------------------------------------------------------------
// Depending on DYN_TRACES, traceLevel is a modifable runtime variable
// or a define
#if !defined(NOTRACE) && (DYN_TRACES == 1)
    extern unsigned int traceLevel;
#endif

#include <sstream>


/**
 * Singleton class to handle trace message
 *
 * @ref: http://stackoverflow.com/questions/1008019/c-singleton-design-pattern
 */
class EWBTrace {
    public:
        static EWBTrace& getInstance()
        {
            static EWBTrace    instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
            return instance;
        }

        static std::string string_format(const std::string &fmt, ...);


    private:
        EWBTrace() {};                   // Forbidden Constructor

//#if __cplusplus <= 199711L
//        // C++ 11
//        // =======
//        // We can use the better technique of deleting the methods
//        // we don't want.
//        EWBTrace(EWBTrace const&)               = delete;
//        void operator=(EWBTrace const&)  = delete;
//#else
//        // C++ 03
//        // ========
//        // Dont forget to declare these two. You want to make sure they
//        // are unacceptable otherwise you may accidentally get copies of
//        // your singleton appearing.
//        EWBTrace(EWBTrace const&);              // Don't Implement
//        void operator=(EWBTrace const&); // Don't implement
//#endif

};

#endif /* EWBTRACE_H_ */
