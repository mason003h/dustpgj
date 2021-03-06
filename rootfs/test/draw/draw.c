#include <types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <kstring.h>
#include <graph/graph.h>
#include <graph/font.h>

void fbtest(void) {
	font_t* font = get_font_by_name("16x32");
	fb_t fb;
	if(fb_open("/dev/fb0", &fb) != 0)
		return;
	graph_t* g = graph_from_fb(&fb);
	if(g == NULL) {
		return;
	}

	int i = 0;
	char s[32];
	while(i<200) {
		clear(g, argb(0xff, 100, 100, 200));
		snprintf(s, 31, "Hello, MicroKernel OS! (%d)", i++);
		fill(g, 10, 10, font->w* strlen(s) + 20, font->h + 20, argb(0xff, 0, 0, 0));
		draw_text(g, 20, 20, s, font, argb(0xff, 255, 255, 255));
		fb_flush(&fb);
		sleep(0);
	}

	graph_free(g);
	fb_close(&fb);
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		fbtest();
	}
	else {
		int i = 0;
		while(i++ < atoi(argv[1])) {
			int pid = fork();
			if(pid == 0) {
				fbtest();
				return 0;
			}
		}
		while(true) {
			sleep(0);
		}
	}	
	return 0;
}

