#ifndef CLICK_TODEVICE_HH
#define CLICK_TODEVICE_HH
#include <click/element.hh>
#include <click/string.hh>
#include <click/task.hh>
#include <click/notifier.hh>
#include "elements/userlevel/fromdevice.hh"
CLICK_DECLS

/*
 * =title ToDevice.u
 * =c
 * ToDevice(DEVNAME [, I<KEYWORDS>])
 * =s devices
 * sends packets to network device (user-level)
 * =d
 *
 * This manual page describes the user-level version of the ToDevice element.
 * For the Linux kernel module element, read the ToDevice(n) manual page.
 *
 * Pulls packets and sends them out the named device using
 * Berkeley Packet Filters (or Linux equivalent).
 *
 * Keyword arguments are:
 *
 * =over 8
 *
 * =item USE_Q
 * 
 * Boolean.  If true, then writes that fail with errno ENOBUFS or EAGAIN
 * will be put on a single packet queue that is held until the next time
 * run_task is called.
 *
 * =item IGNORE_QUEUE_OVERFLOWS
 *
 * Boolean.  If true, don't print more than one error message when
 * there are queue overflows error when sending packets to the device
 * (e.g. send() or write() produced an ENOBUFS or EAGAIN error).
 * Default is false.
 *
 * =back
 *
 * This element is only available at user level.
 *
 * =n
 *
 * Packets sent via ToDevice should already have a link-level
 * header prepended. This means that ARP processing,
 * for example, must already have been done.
 *
 * Under Linux, a L<FromDevice(n)> element will not receive packets sent by a
 * ToDevice element for the same device. Under other operating systems, your
 * mileage may vary.
 *
 * Packets that are written successfully are sent on output 0, if it exists.
 * Packets that fail to be written are pushed out output 1, if it exists.

 * KernelTun lets you send IP packets to the host kernel's IP processing code,
 * sort of like the kernel module's ToHost element.
 *
 * =a
 * FromDevice.u, FromDump, ToDump, KernelTun, ToDevice(n) */

#if defined(__linux__)
# define TODEVICE_LINUX 1
# define TODEVICE_SEND 1
#elif defined(HAVE_PCAP)
extern "C" {
# include <pcap.h>
}
# if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
#  define TODEVICE_BSD_DEV_BPF 1
#  define TODEVICE_WRITE 1
# elif defined(__sun)
#  define TODEVICE_PCAP 1
#  define TODEVICE_WRITE 1
# endif
#endif

/*
 * Write packets to the ethernet via the bpf.
 * Expects packets that already have an ether header.
 * Can push or pull.
 */

class ToDevice : public Element { public:
  
  ToDevice();
  ~ToDevice();
  
  const char *class_name() const		{ return "ToDevice"; }
  const char *processing() const		{ return "a/h"; }
  const char *flags() const			{ return "S2"; }
  
  int configure_phase() const { return FromDevice::CONFIGURE_PHASE_TODEVICE; }
  int configure(Vector<String> &, ErrorHandler *);
  int initialize(ErrorHandler *);
  void cleanup(CleanupStage);
  void add_handlers();

  void notify_noutputs(int);

  String ifname() const				{ return _ifname; }
  int fd() const				{ return _fd; }

  void push(int port, Packet *);
  bool run_task();

protected:
  Task _task;
  void send_packet(Packet *);

private:

  String _ifname;
  int _fd;
  bool _my_fd;
  NotifierSignal _signal;
  
  bool _ignore_q_errs;
  bool _printed_err;


  bool _use_q;
  Packet *_q;
};

CLICK_ENDDECLS
#endif
