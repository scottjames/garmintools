/*
  Garmintools software package
  Copyright (C) 2006-2008 Dave Bailey
  Copyright (C) 2019 Jens Georg

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

#include <stdlib.h>
#include <math.h>
#include "garmin.h"
#include <locale.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static void
print_dtime ( uint32 t, FILE * fp )
{
  time_t     tval;
  struct tm  tmval;
  char       buf[128] = { 0 };
  int        len;

  /*
                                  012345678901234567890123
     This will make, for example, 2007-04-20T23:55:01-0700, but that date
     isn't quite ISO 8601 compliant.  We need to stick a ':' in the time
     zone between the hours and the minutes.
  */

  tval = t + TIME_OFFSET;
  localtime_r(&tval,&tmval);
  strftime(buf,sizeof(buf)-1,"%FT%T%z",&tmval);

  /*
     If the last character is a 'Z', don't do anything.  Otherwise, we
     need to move the last two characters out one and stick a colon in
     the vacated spot.  Let's not forget the trailing '\0' that needs to
     be moved as well.
  */

  len = strlen(buf);
  if ( len > 0 && buf[len-1] != 'Z' ) {
    memmove(buf+len-1,buf+len-2,3);
    buf[len-2] = ':';
  }

  /* OK.  Done. */

  fprintf(fp,"%s", buf);
}


static void
print_tcx_header(FILE *fp)
{
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp, "<TrainingCenterDatabase xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"\n"
    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
    "xmlns=\"http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2\">\n");
}

static const char* get_trigger(uint8 trigger_method) {
    switch (trigger_method) {
        case D1011_manual: return "Manual";
        case D1011_distance: return "Distance";
        case D1011_location: return "Location";
        case D1011_time: return "Time";
        case D1011_heart_rate: return "HeartRate";
        default: return "Manual";
    }
};

static void
print_lap(FILE *fn, garmin_data *lap_data, uint32_t end, garmin_list *points)
{
    if (lap_data->type != data_D1015) {
        fprintf(stderr, "Unsupported lap type %d\n", lap_data->type);
    }

    D1015 *lap = lap_data->data;
    fprintf(fn, "      <Lap StartTime=\"");
    print_dtime(lap->start_time, fn);
    fprintf(fn, "\">\n");
    fprintf(fn, "        <TotalTimeSeconds>%.2f</TotalTimeSeconds>\n", lap->total_time / 100.0);
    fprintf(fn, "        <DistanceMeters>%.2f</DistanceMeters>\n", lap->total_dist);
    fprintf(fn, "        <MaximumSpeed>%.8f</MaximumSpeed>\n", lap->max_speed);
    fprintf(fn, "        <Calories>%d</Calories>\n", lap->calories);
    if (lap->avg_heart_rate > 0) {
        fprintf(fn, "        <AverageHeartRateBpm>\n"
                    "          <Value>%d</Value>\n"
                    "        </AverageHeartRateBpm>\n", lap->avg_heart_rate);
    }
    if (lap->max_heart_rate > 0) {
        fprintf(fn, "        <MaximumHeartRateBpm>\n"
                    "          <Value>%d</Value>\n"
                    "        </MaximumHeartRateBpm>\n", lap->max_heart_rate);
    }
    fprintf(fn, "        <Intensity>%s</Intensity>\n", lap->intensity == 0 ? "Active" : "Rest");
    fprintf(fn, "        <TriggerMethod>%s</TriggerMethod>\n", get_trigger(lap->trigger_method));
    if (points != NULL) {
        fprintf(fn, "        <Track>\n");
        garmin_data *point;
        for (garmin_list_node *it = points->head; it != NULL; it = it->next) {
            point = it->data;
            if (point->type != data_D304) {
                continue;
            }
            D304 *d304 = point->data;
            if (d304->time < lap->start_time) {
                continue;
            }

            if (d304->time >= end) {
                break;
            }
            fprintf(fn, "          <Trackpoint>\n");
            fprintf(fn, "              <Time>");
            print_dtime (d304->time, fn);
            fprintf(fn, "</Time>\n");
            if (! (d304->posn.lat == INT32_MAX && d304->posn.lon == INT32_MAX)) {
                fprintf(fn, "               <Position>\n");
                fprintf(fn, "                   <LatitudeDegrees>%.8f</LatitudeDegrees>\n", SEMI2DEG (d304->posn.lat));
                fprintf(fn, "                   <LongitudeDegrees>%.8f</LongitudeDegrees>\n", SEMI2DEG (d304->posn.lon));
                fprintf(fn, "               </Position>\n");
                fprintf(fn, "               <AltitudeMeters>%.7f</AltitudeMeters>\n", d304->alt);
                fprintf(fn, "               <DistanceMeters>%.4f</DistanceMeters>\n", d304->distance);
                fprintf(fn, "               <HeartRateBpm>\n");
                fprintf(fn, "                   <Value>%u</Value>\n", d304->heart_rate);
                fprintf(fn, "               </HeartRateBpm>\n");
                if (d304->cadence != 0xff) {
                    fprintf(fn, "               <Cadence>%u</Cadence>\n", d304->cadence);
                }
            }
            fprintf(fn, "          </Trackpoint>\n");
        }
        fprintf(fn, "        </Track>\n");
    }
    fprintf(fn, "      </Lap>\n");
}

static void
print_tcx_data(garmin_data *data, char *device_information, FILE *fn, int spaces)
{
  //route_point **         laps = NULL;
  //route_point * points;
  //position_type  sw;
  //position_type  ne;
  //int i;

  {
      print_tcx_header(fn);
      fprintf(fn, "  <Activities>\n");
      garmin_data *d = garmin_list_data(data, 0);
      if (d->type != data_D1009) {
          fprintf(stderr, "Unsupported activity type %d\n", d->type);
          return;
      }

      D1009 *d1009 = d->data;
      fprintf(fn, "    <Activity Sport=\"%s\">\n",
              d1009->sport_type == D1000_running ? "Running" : (d1009->sport_type == D1000_biking ? "Biking" : "Other"));

      d = garmin_list_data(data, 1);
      if (d == NULL || d->type != data_Dlist) {
          fprintf(stderr, "No laps, exiting\n");
      }
      garmin_list *laps = d->data;

      d = garmin_list_data(data, 2);
      garmin_list *points = NULL;

      if (d != NULL && d->type == data_Dlist) {
          points = d->data;
      }

      for (garmin_list_node *lap_node = laps->head; lap_node != NULL; lap_node = lap_node->next) {
          // We need the start time of the first lap as the ID of the activity
          if (lap_node == laps->head &&
              lap_node->data->type == data_D1015) {
              D1015 *lap = lap_node->data->data;
              fprintf(fn, "      <Id>");
              print_dtime(lap->start_time, fn);
              fprintf(fn, "</Id>\n");
          }
         uint32_t end_time = UINT32_MAX;
          if (lap_node->next != NULL) {
              end_time = ((D1015 *)lap_node->next->data->data)->start_time;
          }
          print_lap(fn, lap_node->data, end_time, points);
      }

      fprintf(fn, "    </Activity>\n");
      fprintf(fn, "  </Activities>\n");
      if (device_information != NULL) {
          fprintf(fn, "  %s\n", device_information);
      }
      fprintf(fn, "</TrainingCenterDatabase>\n");
  }
}

char *read_device_file(const char *file_name)
{
    size_t length = strlen(file_name) + strlen(".device") + 1;
    char *buf = (char *)calloc(length, sizeof(char));
    snprintf(buf, length, "%s.device", file_name);
    struct stat info = {0};
    char *device_info = NULL;

    if (stat(buf, &info) != -1) {
        device_info = (char *)calloc(info.st_size + 1, 1);
        FILE *f = fopen(buf, "rb");
        if (f != NULL) {
            fread(device_info, info.st_size, 1, f);
            fclose(f);
        } else {
            free(device_info);
            device_info = NULL;
        }
    } else {
        perror(strerror(errno));
    }
    free(buf);

    return device_info;
}

int
main(int argc, char *argv[])
{
    char *old_lc_numeric = setlocale(LC_NUMERIC, NULL);
    setlocale(LC_NUMERIC, "C");
    garmin_data *data;

    for (int i = 1; i < argc; i++ ) {
        if ( (data = garmin_load(argv[i])) != NULL ) {
            char *device_info = read_device_file(argv[i]);
            print_tcx_data(data,device_info, stdout,0);
            free(device_info);

            garmin_free_data(data);
        }
    }
    setlocale(LC_NUMERIC, old_lc_numeric);

    return 0;
}
