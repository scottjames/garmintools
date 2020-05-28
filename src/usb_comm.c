/*
  Garmintools software package
  Copyright (C) 2006-2008 Dave Bailey

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <libusb.h>
#include "garmin.h"

#include <stdbool.h>


#define INTR_TIMEOUT  3000
#define BULK_TIMEOUT  3000

static libusb_context *ctx = NULL;

/* Close the USB connection with the Garmin device. */

int
garmin_close ( garmin_unit * garmin )
{
  if ( garmin->usb.handle != NULL ) {
    libusb_release_interface(garmin->usb.handle,0);
    libusb_close(garmin->usb.handle);
    garmin->usb.handle = NULL;
  }

  return 0;
}

#if defined(__linux__)
static bool
check_for_kernel_module (void)
{
    FILE *f = NULL;
    bool result = false;

    f = fopen ("/proc/modules", "r");
    if (f != NULL) {
        char buffer[512] = { 0 };
        while (!feof (f) && !ferror(f)) {
            char *p = fgets (buffer, sizeof (buffer), f);
            if (p != NULL) {
                if (strstr(p, "garmin_gps") != NULL) {
                    result = true;
                    break;
                }
            }
        }
        fclose (f);
    }

    return result;
}
#else
static bool check_for_kernel_module (void) { return false; };
#endif

/*
   Open the USB connection with the first Garmin device we find.  Eventually,
   I'd like to add the ability to select a particular device.  Returns 1 on
   success, 0 on failure.  Prints diagnostic information and errors to stdout.
*/

int
garmin_open ( garmin_unit * garmin )
{
  libusb_device **     dl;
  libusb_device *      di;
  int                  cnt;
  int                  err = 0;
  int                  i;

  if (check_for_kernel_module ()) {
      printf("garmin_gps module is loaded; garmintools cannot work\n");
      return 0;
  }

  if ( garmin->usb.handle == NULL ) {
    if ( ctx == NULL ) {
      err = libusb_init(&ctx);
      if ( err ) {
        printf("libusb_init failed: %s\n", libusb_error_name(err));
        return ( garmin->usb.handle != NULL );
      } else if ( garmin->verbose != 0 ) {
        printf("[garmin] libusb_init succeeded\n");
      }
    }
    cnt = libusb_get_device_list(ctx,&dl);

    for (i = 0; i < cnt; ++i) {
        struct libusb_device_descriptor descriptor;
        struct libusb_config_descriptor *config = NULL;

        di = dl[i];
        err = libusb_get_device_descriptor (di, &descriptor);

        if ( !err &&
             descriptor.idVendor  == GARMIN_USB_VID &&
             descriptor.idProduct == GARMIN_USB_PID ) {

          if ( garmin->verbose != 0 ) {
            printf("[garmin] found VID %04x, PID %04x",
                   descriptor.idVendor,
                   descriptor.idProduct);
          }

          err = libusb_open(di,&garmin->usb.handle);
          garmin->usb.read_bulk = 0;

          if ( err ) {
            printf("libusb_open failed: %s\n",libusb_error_name(err));
            garmin->usb.handle = NULL;
          } else {
              if ( garmin->verbose != 0 ) {
                printf("[garmin] libusb_open = %p\n",(void*)garmin->usb.handle);
              }

            err = libusb_set_configuration(garmin->usb.handle,1);
            if ( err ) {
              printf("libusb_set_configuration failed: %s\n",
                     libusb_error_name(err));
            } else {
                if ( garmin->verbose != 0 ) {
                      printf("[garmin] libusb_set_configuration[1] succeeded\n");
                }

              err = libusb_claim_interface(garmin->usb.handle,0);
              if ( err ) {
                printf("libusb_claim_interface failed: %s\n",
                       libusb_error_name(err));
              } else {
                if ( garmin->verbose != 0 ) {
                     printf("[garmin] libusb_claim_interface[0] succeeded\n");
                }

                err = libusb_get_config_descriptor_by_value(di,1,&config);
                if ( err ) {
                  printf("libusb_get_config_descriptor_by_value failed: %s\n",
                         libusb_error_name(err));
                } else if ( garmin->verbose != 0 ) {
                  printf("[garmin] libusb_get_config_descriptor_by_value "
                         "succeeded\n");
                }
              }
            }
          }

          if ( !err ) {

            /*
               We've succeeded in opening and claiming the interface
               Let's set the bulk and interrupt in and out endpoints.
            */

            for ( i = 0;
                  i < config->interface->altsetting->bNumEndpoints;
                  i++ ) {
              const struct libusb_endpoint_descriptor * ep;

              ep = &config->interface->altsetting->endpoint[i];
              switch ( ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK ) {
              case LIBUSB_TRANSFER_TYPE_BULK:
                if ( ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK ) {
                  garmin->usb.bulk_in = ep->bEndpointAddress;
                  if ( garmin->verbose != 0 ) {
                    printf("[garmin] bulk IN  = 0x%02x\n",garmin->usb.bulk_in);
                  }
                } else {
                  garmin->usb.bulk_out = ep->bEndpointAddress;
                  if ( garmin->verbose != 0 ) {
                    printf("[garmin] bulk OUT = 0x%02x\n",garmin->usb.bulk_out);
                  }
                }
                break;
              case LIBUSB_TRANSFER_TYPE_INTERRUPT:
                if ( ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK ) {
                  garmin->usb.intr_in = ep->bEndpointAddress;
                  if ( garmin->verbose != 0 ) {
                    printf("[garmin] intr IN  = 0x%02x\n",garmin->usb.intr_in);
                  }
                }
                break;
              default:
                break;
              }
            }
          }

      if (config != NULL) {
          libusb_free_config_descriptor (config);
      }

          /* We've found what should be the Garmin interface. */

          break;
        }

      if ( garmin->usb.handle != NULL ) break;
    }
    libusb_free_device_list (dl, 1);
  }

  /*
     If the USB handle is open but we experienced an error in setting the
     configuration or claiming the interface, close the USB handle and set
     it to NULL.
  */

  if ( garmin->usb.handle != NULL && err != 0 ) {
    if ( garmin->verbose != 0 ) {
      printf("[garmin] (err = %d) libusb_close(%p)\n",err,(void *)garmin->usb.handle);
    }
    libusb_close(garmin->usb.handle);
    garmin->usb.handle = NULL;
  }

  return (garmin->usb.handle != NULL);
}


uint8
garmin_packet_type ( garmin_packet * p )
{
  return p->packet.type;
}


uint16
garmin_packet_id ( garmin_packet * p )
{
  return get_uint16(p->packet.id);
}


uint32
garmin_packet_size ( garmin_packet * p )
{
  return get_uint32(p->packet.size);
}


uint8 *
garmin_packet_data ( garmin_packet * p )
{
  return p->packet.data;
}


int
garmin_packetize ( garmin_packet *  p,
                   uint16           id,
                   uint32           size,
                   uint8 *          data )
{
  int ok = 0;

  if ( size + PACKET_HEADER_SIZE < sizeof(garmin_packet) ) {
    p->packet.type       = GARMIN_PROTOCOL_APP;
    p->packet.reserved1  = 0;
    p->packet.reserved2  = 0;
    p->packet.reserved3  = 0;
    p->packet.id[0]      = id;
    p->packet.id[1]      = id >> 8;
    p->packet.reserved4  = 0;
    p->packet.reserved5  = 0;
    p->packet.size[0]    = size;
    p->packet.size[1]    = size >> 8;
    p->packet.size[2]    = size >> 16;
    p->packet.size[3]    = size >> 24;
    if ( size > 0 && data != NULL ) {
      memcpy(p->packet.data,data,size);
    }
    ok = 1;
  }

  return ok;
}


int
garmin_read ( garmin_unit * garmin, garmin_packet * p )
{
  int r = -1;

  garmin_open(garmin);

  if ( garmin->usb.handle != NULL ) {
    if ( garmin->usb.read_bulk == 0 ) {
      libusb_interrupt_transfer(garmin->usb.handle,
                                garmin->usb.intr_in,
                                (unsigned char *) p->data,
                                sizeof(garmin_packet),
                                &r,
                                INTR_TIMEOUT);
      /*
         If the packet is a "Pid_Data_Available" packet, we need to read
         from the bulk endpoint until we get an empty packet.
      */

      if ( garmin_packet_type(p) == GARMIN_PROTOCOL_USB &&
           garmin_packet_id(p) == Pid_Data_Available ) {

        /* FIXME!!! */

        printf("Received a Pid_Data_Available from the unit!\n");
      }

    } else {
      libusb_bulk_transfer(garmin->usb.handle,
                           garmin->usb.bulk_in,
                           (unsigned char *) p->data,
                           sizeof(garmin_packet),
                           &r,
                           BULK_TIMEOUT);
    }
  }

  if ( garmin->verbose != 0 && r >= 0 ) {
    garmin_print_packet(p,GARMIN_DIR_READ,stdout);
  }

  return r;
}


int
garmin_write ( garmin_unit * garmin, garmin_packet * p )
{
  int err = 0;
  int r = -1;
  int s = garmin_packet_size(p) + PACKET_HEADER_SIZE;

  garmin_open(garmin);

  if ( garmin->usb.handle != NULL ) {

    if ( garmin->verbose != 0 ) {
      garmin_print_packet(p,GARMIN_DIR_WRITE,stdout);
    }

    err = libusb_bulk_transfer(garmin->usb.handle,
                               garmin->usb.bulk_out,
                               (unsigned char *) p->data,
                               s,
                               &r,
                               BULK_TIMEOUT);
    if ( r != s ) {
      printf("libusb_bulk_write failed: %s\n",libusb_error_name(err));
      garmin_close(garmin);
      exit(1);
    }
  }

  return r;
}


uint32
garmin_start_session ( garmin_unit * garmin )
{
  garmin_packet p;

  garmin_packetize(&p,Pid_Start_Session,0,NULL);
  p.packet.type = GARMIN_PROTOCOL_USB;

  garmin_write(garmin,&p);
  garmin_write(garmin,&p);
  garmin_write(garmin,&p);

  if ( garmin_read(garmin,&p) == 16 ) {
    garmin->id = get_uint32(p.packet.data);
  } else {
    garmin->id = 0;
  }

  return garmin->id;
}


void
garmin_print_packet ( garmin_packet * p, int dir, FILE * fp )
{
  uint32 i;
  int    j;
  uint32 s;
  char   hex[128] = { 0 };
  char   dec[128] = { 0 };

  s = garmin_packet_size(p);

  switch ( dir ) {
  case GARMIN_DIR_READ:   fprintf(fp,"<read");   break;
  case GARMIN_DIR_WRITE:  fprintf(fp,"<write");  break;
  default:                fprintf(fp,"<packet");        break;
  }

  fprintf(fp," type=\"0x%02x\" id=\"0x%04x\" size=\"%u\"",
          garmin_packet_type(p),garmin_packet_id(p),s);
  if ( s > 0 ) {
    fprintf(fp,">\n");
    for ( i = 0, j = 0; i < s; i++ ) {
      sprintf(&hex[(3*(i&0x0f))]," %02x",p->packet.data[i]);
      sprintf(&dec[(i&0x0f)],"%c",
              (isalnum(p->packet.data[i]) ||
               ispunct(p->packet.data[i]) ||
               p->packet.data[i] == ' ') ?
              p->packet.data[i] : '_');
      if ( (i & 0x0f) == 0x0f ) {
        j = 0;
        fprintf(fp,"[%04x] %-54s %s\n",i-15,hex,dec);
      } else {
        j++;
      }
    }
    if ( j > 0 ) {
      fprintf(fp,"[%04x] %-54s %s\n",s-(s & 0x0f),hex,dec);
    }
    switch ( dir ) {
    case GARMIN_DIR_READ:   fprintf(fp,"</read>\n");   break;
    case GARMIN_DIR_WRITE:  fprintf(fp,"</write>\n");  break;
    default:                fprintf(fp,"</packet>\n"); break;
    }
  } else {
    fprintf(fp,"/>\n");
  }
}

static void garmin_usb_shutdown(void) __attribute__((destructor));

static void
garmin_usb_shutdown(void) {
    if (ctx != NULL) {
        libusb_exit (ctx);
        ctx = NULL;
    }
}
