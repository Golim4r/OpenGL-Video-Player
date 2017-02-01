#include <iostream>
#include <thread>

#include "Renderer.h"
#include "Decoder.h"
#include "JUtils.h"


int main(int argc, char *argv[]) {
  Decoder d("western.mkv");
  
  Renderer r(d);
  JDurationManager dm;
  
  dm.start();
  //d.run();
  std::thread dt(&Decoder::run, &d);
  r.run();
  dt.join();  
  
  dm.stop();
  dm.print();
  
	return 0;
}
