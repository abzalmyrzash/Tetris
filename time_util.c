#include "time_util.h"

/* Windows sleep in 100ns units */
BOOLEAN nanoSleep(LONGLONG ns, HANDLE timer){
    /* Declarations */
    LARGE_INTEGER li;   /* Time defintion */
    /* Set timer properties */
    li.QuadPart = -ns;
    if(!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)){
        return FALSE;
    }
    /* Start & wait for timer */
    WaitForSingleObject(timer, INFINITE);
    /* Slept without problems */
    return TRUE;
}
