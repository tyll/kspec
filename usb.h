#ifndef __USB_H__
#define __USB_H__

#include <vector>
#include <set>
#include <stdexcept>
#include <libusb-1.0/libusb.h>
#include <boost/shared_ptr.hpp>

#include "usbexception.h"

typedef boost::shared_ptr<libusb_context> libusb_context_ptr;

class USB;

class USBDevice
{
public:
  ~USBDevice();
  typedef boost::shared_ptr<USBDevice> Ptr;
  typedef boost::shared_ptr<std::vector<USBDevice::Ptr> > List;

  libusb_device *ptr() { return _device; }
  libusb_device_handle *handle() { return _handle; }

  void get_device_descriptor(libusb_device_descriptor *dev_desc);
  void open();
  void close();
  void claim_interface(int interface);
  void release_interface(int interface);
  void release_all_interfaces();

private:
  USBDevice(libusb_context_ptr ctx, libusb_device *device);

private:
  libusb_context_ptr _context;
  libusb_device *_device;
  libusb_device_handle *_handle;
  std::set<int> _claimed_interfaces;
  friend class USB;
};

class USBTransfer
{
public:
  void submit();
  void cancel();

  bool is_complete() { return _transfer->status == LIBUSB_TRANSFER_COMPLETED; }

protected:
  USBTransfer(int iso_packets = 0 /* default non-isochronous */);
  virtual ~USBTransfer();

  static void callback(libusb_transfer *tx);
  virtual void on_complete() = 0;
  virtual void on_cancel() {std::cerr << "Cancelled." << std::endl;}
  virtual void on_error() {std::cerr << "Error." << std::endl;}
  virtual void on_no_device() {std::cerr << "No device." << std::endl;}
  virtual void on_overflow() {std::cerr << "Overflow." << std::endl;}
  virtual void on_stall() {std::cerr << "Stalled." << std::endl;}
  virtual void on_time_out() {std::cerr << "Timed out." << std::endl;}

  libusb_transfer *_transfer;
};

class USBControlTransfer : public USBTransfer
{
public:
  USBControlTransfer(USBDevice::Ptr device, unsigned char endpoint, int buffer_size,
                     unsigned int timeout = 0);

  std::vector<unsigned char> &buffer() { return _buffer; }

//private:
  std::vector<unsigned char> _buffer;
};

class USBBulkTransfer : public USBTransfer
{
public:
  USBBulkTransfer(USBDevice::Ptr device, unsigned char endpoint, int buffer_size,
                  libusb_transfer_cb_fn callback, unsigned int timeout = 0);

  std::vector<unsigned char> &buffer() { return _buffer; }

private:
  std::vector<unsigned char> _buffer;
};

class USB
{
public:
  enum DebugLevel { DEBUG_OFF, DEBUG_LEVEL_1, DEBUG_LEVEL_2, DEBUG_LEVEL_3 };
  USB(DebugLevel debug_level = DEBUG_OFF);
  ~USB();

  USBDevice::List get_device_list();
  void handle_events();

private:
  boost::shared_ptr<libusb_context> _context;
};

#endif // __USB_H__