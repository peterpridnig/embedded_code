#include<iostream>
#include<fstream>
#include<cstring>
#include<iomanip>
#include<vector>

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>
#include<stdlib.h>
#include<sys/ioctl.h>
#include<unistd.h> //for usleep
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include <getopt.h>
#include<ncurses.h>

#include"minmea.h"

using namespace std;

class Display {
private:
  WINDOW *win {nullptr};
  int h, w;

public:
  Display();
  ~Display();
  void GLL(const minmea_sentence_gll &frame);
  void GGA(const minmea_sentence_gga &frame);
  void RMC(const minmea_sentence_rmc &frame);  
};

  
Display::Display() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  win = newwin(15, 50, 2, 2);
  wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);
  nodelay(stdscr,TRUE);
  getmaxyx(stdscr, this->h, this->w);

  move(this->h-12,3);
  addstr("GPS READOUT press SPACE to quit");
  refresh();
}

Display::~Display() {
  endwin();
}


void Display::GLL(const minmea_sentence_gll &frame) {
  struct minmea_time t {frame.time};

  move(this->h-10,3);
  
  printw("$GLL latitude, longitude and time: (%d,%d) %d:%d:%d\n",
	 minmea_rescale(&frame.latitude, 1000),
	 minmea_rescale(&frame.longitude, 1000),
	 t.hours,t.minutes,t.seconds);

  refresh();
}

void Display::GGA(const minmea_sentence_gga&frame) {

  move(this->h-8,3);
  
  printw("$GGA: fix quality: %d\n", frame.fix_quality);
  refresh();
  
}

void Display::RMC(const minmea_sentence_rmc&frame) {

  move(this->h-6,3);

  printw("$RMC raw coordinates and speed: (%d/%d,%d/%d) %d/%d\n",
	 frame.latitude.value, frame.latitude.scale,
	 frame.longitude.value, frame.longitude.scale,
	 frame.speed.value, frame.speed.scale);

  move(this->h-5,3);
  
  printw("$RMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d\n",
	 minmea_rescale(&frame.latitude, 1000),
	 minmea_rescale(&frame.longitude, 1000),
	 minmea_rescale(&frame.speed, 1000));

  move(this->h-4,3);
  
  printw("$RMC floating point degree coordinates and speed: (%f,%f) %f\n",
	 minmea_tocoord(&frame.latitude),
	 minmea_tocoord(&frame.longitude),
	 minmea_tofloat(&frame.speed));

  refresh();
}

int main() {

  ifstream fs;
  string filename {"/dev/ttyS4"};
  string line {};

  Display D;
  
  fs.open(filename.c_str());
  if (!fs.is_open()){
    perror("GPIO: read failed to open file ");
  }

  int ch {0};
  
  while ( (!fs.eof()) & (ch != 32 ) )
    {
      ch = getch();
      
      getline(fs,line);
      //cout << line << endl;

      switch (minmea_sentence_id(line.c_str(), false)) {
      case MINMEA_SENTENCE_RMC: {
	struct minmea_sentence_rmc frame;
	if (minmea_parse_rmc(&frame, line.c_str())) { D.RMC(frame);
	}
      } break;

      case MINMEA_SENTENCE_GGA: {
	struct minmea_sentence_gga frame;
	if (minmea_parse_gga(&frame, line.c_str())) { D.GGA(frame); }
      } break;

      case MINMEA_SENTENCE_GSV: {
	struct minmea_sentence_gsv frame;
	if (minmea_parse_gsv(&frame, line.c_str())) {
	  /*
	  printf("$GSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
	  printf("$GSV: satellites in view: %d\n", frame.total_sats);
	  for (int i = 0; i < 4; i++)
	    printf("$GSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
		   frame.sats[i].nr,
		   frame.sats[i].elevation,
		   frame.sats[i].azimuth,
		   frame.sats[i].snr);
	  */
	}
      } break;

      case MINMEA_SENTENCE_GLL: { 

	struct minmea_sentence_gll frame;
	if (minmea_parse_gll(&frame, line.c_str())) { D.GLL(frame); }
      } break;

	
      default: break;
      }

    }
  fs.close();

  return 0;
};
