#include<iostream>
#include<fstream>
#include<cstring>

#include<ncurses.h>
#include"minmea.h"


#define GPS_DEVICE "/dev/ttyS4"

using namespace std;

class Display {
private:
  WINDOW *win {nullptr};
  int h, w;
  int current_message_number {1};
  int max_messages {1};
  char rotorstring[9] = "|/-\\|/-\\";
  size_t rotor_position {0};

public:
  Display();
  ~Display();
  void ForwardMessage();
  void rectangle(int,int,int,int);
  void GLL(const minmea_sentence_gll &frame);
  void GGA(const minmea_sentence_gga &frame);
  void RMC(const minmea_sentence_rmc &frame);
  void GSV(const minmea_sentence_gsv &frame);
  void DrawRotor();
};

  
Display::Display() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0); //?
  //win = newwin(20, 50, 2, 2);
  //wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);
  nodelay(stdscr,TRUE);
  getmaxyx(stdscr, this->h, this->w);

  this->rectangle(this->h-21,1,this->h-2,100);
  
  move(this->h-20,3);
  addstr("GPS READOUT [ ] (L80-39 + CP2102USB Serial): 'q'...quit 'SPACE'...next message");
  refresh();
}

Display::~Display() {
  endwin();
}

void Display::ForwardMessage() {
  this->current_message_number++;
  
  if ( (this->current_message_number) > this->max_messages) {this->current_message_number=1; }
}

void Display::rectangle(int y1, int x1, int y2, int x2)
{
    mvhline(y1, x1, 0, x2-x1);
    mvhline(y2, x1, 0, x2-x1);
    mvvline(y1, x1, 0, y2-y1);
    mvvline(y1, x2, 0, y2-y1);
    mvaddch(y1, x1, ACS_ULCORNER);
    mvaddch(y2, x1, ACS_LLCORNER);
    mvaddch(y1, x2, ACS_URCORNER);
    mvaddch(y2, x2, ACS_LRCORNER);
}

void Display::GLL(const minmea_sentence_gll &frame) {
  struct minmea_time t {frame.time};

  move(this->h-10,3);
  
  printw("$GLL latitude, longitude and time: (%d,%d) %d:%d:%dn (UTC)    ",
	 minmea_rescale(&frame.latitude, 1000),
	 minmea_rescale(&frame.longitude, 1000),
	 t.hours,t.minutes,t.seconds);
  refresh();
}

void Display::GGA(const minmea_sentence_gga &frame) {
  move(this->h-8,3);
  printw("$GGA: fix quality: %d", frame.fix_quality);
  refresh();
}

void Display::RMC(const minmea_sentence_rmc &frame) {

  move(this->h-6,3);

  printw("$RMC raw coordinates and speed: (%d/%d,%d/%d) %d/%d    ",
	 frame.latitude.value, frame.latitude.scale,
	 frame.longitude.value, frame.longitude.scale,
	 frame.speed.value, frame.speed.scale);

  move(this->h-5,3);
  
  printw("$RMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d    ",
	 minmea_rescale(&frame.latitude, 1000),
	 minmea_rescale(&frame.longitude, 1000),
	 minmea_rescale(&frame.speed, 1000));

  move(this->h-4,3);
  
  printw("$RMC floating point degree coordinates and speed: (%f,%f) %f    ",
	 minmea_tocoord(&frame.latitude),
	 minmea_tocoord(&frame.longitude),
	 minmea_tofloat(&frame.speed));
  refresh();
}

void Display::GSV(const minmea_sentence_gsv &frame) {
   
  move(this->h-17,3);
  printw("$GSV: satellites in view: %d    ", frame.total_sats);

  if (frame.msg_nr == this->current_message_number) {
    
    move(this->h-16,3);
    printw("$GSV: message %d of %d    ", frame.msg_nr, frame.total_msgs);

    this->max_messages=frame.total_msgs;
    if (this->current_message_number>this->max_messages) this->current_message_number=this->max_messages;
    
    for (int i = 0; i < 4; i++) {
      move(this->h-12-i,3);	      
      printw("$GSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm    ",
	     frame.sats[i].nr,
	     frame.sats[i].elevation,
	     frame.sats[i].azimuth,
	     frame.sats[i].snr);
    }
  }
  refresh();
}

void Display::DrawRotor() {
  rotor_position++; if (rotor_position>strlen(rotorstring)) rotor_position=0;
  move(this->h-20,16);
  printw("%c",rotorstring[rotor_position]);
  refresh();
}

    
int main() {

  ifstream fs;
  string filename {GPS_DEVICE};
  string line {};
  
  fs.open(filename.c_str());
  if (!fs.is_open()){
    char failstr [100] {"Failed to open device file "};
    strcat(failstr,GPS_DEVICE);
    perror(failstr);
    return 1;
  }

  Display D;
  
  int ch {0};
  
  while ( (!fs.eof()) & (ch != 'q' ) )
    {
      ch = getch();
      if (ch == 32) D.ForwardMessage();

      D.DrawRotor();
      
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
	if (minmea_parse_gsv(&frame, line.c_str())) { D.GSV(frame); 
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
