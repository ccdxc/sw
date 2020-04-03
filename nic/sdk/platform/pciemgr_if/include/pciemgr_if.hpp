/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#ifndef __PCIEMGR_IF_HPP__
#define __PCIEMGR_IF_HPP__

#include "platform/evutils/include/evutils.h"

struct pciehdev_s;
typedef struct pciehdev_s pciehdev_t;

struct pciehdevice_resources_s;
typedef struct pciehdevice_resources_s pciehdevice_resources_t;

struct pciehdev_eventdata_s;
typedef struct pciehdev_eventdata_s pciehdev_eventdata_t;

struct pciehdev_memrw_notify_s;
typedef struct pciehdev_memrw_notify_s pciehdev_memrw_notify_t;

struct pmmsg_s;
typedef struct pmmsg_s pmmsg_t;

enum powermode {
    LOW_POWER,
    FULL_POWER,
};

class pciemgr {
public:
    class evhandler {
    public:
        evhandler(void) {}
        virtual ~evhandler(void) {}
        virtual void memrd(const int port,
                           const uint32_t lif,
                           const pciehdev_memrw_notify_t *n) {};
        virtual void memwr(const int port,
                           const uint32_t lif,
                           const pciehdev_memrw_notify_t *n) {};
        virtual void hostup(const int port) {};
        virtual void hostdn(const int port) {};
        virtual void sriov_numvfs(const int port,
                                  const uint32_t lif,
                                  const uint16_t numvfs) {};
        virtual void reset(const int port,
                           uint32_t rsttype,
                           const uint32_t lifb,
                           const uint32_t lifc) {};
    };

    pciemgr(const char *name);
    pciemgr(const char *name, evhandler &evhandlercb, EV_P);
    ~pciemgr(void);

    int initialize(const int port = 0);
    int finalize(const int port = 0);
    int add_devres(pciehdevice_resources_t *pres);
    int powermode(const int mode);

private:
    void connect(const char *name, const int receiver);
    void msghandler(pmmsg_t *m);
    void handle_event(const pciehdev_eventdata_t *evd);

    EV_P;
    int serverfd;
    evhandler &evhandlercb;
    static void msgrecv(void *arg);
};

#endif /* __PCIEMGR_IF_HPP__ */
