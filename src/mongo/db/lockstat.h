#include "util/timer.h"

namespace mongo { 

    class LockStat { 
        enum { N = 4 };
    public:
        Timer W_Timer;
        unsigned long long timeAcquiring[N];
        unsigned long long timeLocked[N];

        LockStat() { 
            for( int i = 0; i < N; i++ ) {
                timeAcquiring[i] = 0;
                timeLocked[i] = 0;
            }
        }

        struct Acquiring {
            Timer tmr;
            LockStat& ls;
            unsigned type;
            explicit Acquiring(LockStat&, char type);
            ~Acquiring();
        };

        void unlocking(char type);

        BSONObj report() const;

        static unsigned map(char type);
    };

    inline BSONObj LockStat::report() const { 
        BSONObjBuilder x;
        BSONObjBuilder y;
        //x.append("R", (long long) timeLocked[0]);
        x.append("W", (long long) timeLocked[1]);
        /*if( timeLocked[2] || timeLocked[3] ) {
            x.append("r", (long long) timeLocked[2]);
            x.append("w", (long long) timeLocked[3]);
        }*/
        y.append("R", (long long) timeAcquiring[0]);
        y.append("W", (long long) timeAcquiring[1]);
        if( timeAcquiring[2] || timeAcquiring[3] ) {
            y.append("r", (long long) timeAcquiring[2]);
            y.append("w", (long long) timeAcquiring[3]);
        }
        return BSON(
            "timeLocked" << x.obj() << 
            "timeAcquiring" << y.obj()
        );
    }

    inline unsigned LockStat::map(char type) {
        switch( type ) { 
        case 'R' : return 0;
        case 'W' : return 1;
        case 'r' : return 2;
        case 'w' : return 3;
        default: ;
        }
        fassert(16146,false);
        return 0;
    }

    inline LockStat::Acquiring::Acquiring(LockStat& _ls, char t) : ls(_ls) { 
        type = map(t);
        dassert( type < N );
    }

    // note: we have race conditions on the following += 
    // hmmm....

    inline LockStat::Acquiring::~Acquiring() { 
        ls.timeAcquiring[type] += tmr.micros();
        if( type == 1 ) 
            ls.W_Timer.reset();
    }

    inline void LockStat::unlocking(char tp) { 
        unsigned type = map(tp);
        if( type == 1 ) 
            timeLocked[type] += W_Timer.micros();
    }
}
