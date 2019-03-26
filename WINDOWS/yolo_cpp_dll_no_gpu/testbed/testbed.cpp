#include "detector.h"

int main()
{
	char *cfgfile = "../modelnew/tiny-yolo-voc.cfg";
	char *weightfile = "../modelnew/tiny-yolo-voc_final.weights";
	network net;
	net = COM_parse_network_cfg(cfgfile);   //cfgÖÐµÄwidthºÍheight³ß´ç
	
	if (weightfile) {
		COM_load_weights(&(net), weightfile);
		
	}
	COM_set_batch_network(&(net), 1);

}