/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PCIEMGR_IF_HPP__
#define __PCIEMGR_IF_HPP__

struct pciehdev_s;
typedef struct pciehdev_s pciehdev_t;

struct pciehdev_eventdata_s;
typedef struct pciehdev_eventdata_s pciehdev_eventdata_t;

struct pciehdev_memrw_notify_s;
typedef struct pciehdev_memrw_notify_s pciehdev_memrw_notify_t;

struct pmmsg_s;
typedef struct pmmsg_s pmmsg_t;

class pciemgr {
public:
    class evhandler {
    public:
        evhandler(void) {}
        virtual ~evhandler(void) {}
        virtual void memrd(const int port,
                           pciehdev_t *pdev,
                           const pciehdev_memrw_notify_t *n) {};
        virtual void memwr(const int port,
                           pciehdev_t *pdev,
                           const pciehdev_memrw_notify_t *n) {};
        virtual void hostup(const int port) {};
        virtual void hostdn(const int port) {};
    };

    pciemgr(const char *name);
    pciemgr(const char *name, evhandler &evhandlercb);
    ~pciemgr(void);

    int initialize(const int port = 0);
    int finalize(const int port = 0);
    int add_device(pciehdev_t *pdev);

private:
    void msghandler(pmmsg_t *m);
    void handle_event(const pciehdev_eventdata_t *evd);

    int serverfd;
    evhandler &evhandlercb;
    static void msgrecv(void *arg);
};

#endif /* __PCIEMGR_IF_HPP__ */
