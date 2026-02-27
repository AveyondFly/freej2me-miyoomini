/*
Anbu, an interface between FreeJ2ME emulator and SDL2
Authors:
	Anbu        Saket Dandawate (hex @ retropie)
	FreeJ2ME    D. Richardson (recompile @ retropie)
	
To compile : g++ -std=c++11 -lSDL2 -lpthread -lfreeimage -o anbu anbu.cpp

This file is part of FreeJ2ME.

FreeJ2ME is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FreeJ2ME is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FreeJ2ME.  If not, see http://www.gnu.org/licenses/

*/

#include <iostream>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "cJSON.h"
#include <FreeImage.h>

#define BYTES 3

using namespace std;

#define KEY_LEFT SDLK_w
#define KEY_RIGHT SDLK_q
#define KEY_OK SDLK_RETURN
#define KEY_STAR SDLK_e
#define KEY_POUND SDLK_r
#define KEY_1 SDLK_1
#define KEY_3 SDLK_3
#define KEY_7 SDLK_7
#define KEY_9 SDLK_9
#define KEY_0 SDLK_0
#define KEY_2 SDLK_2
#define KEY_4 SDLK_4
#define KEY_6 SDLK_6
#define KEY_8 SDLK_8
#define KEY_QUIT SDLK_F4
#define KEY_SNAP SDLK_z
#define KEY_ROTT SDLK_l
#define KEY_MOD SDLK_m
#define KEY_MOS SDLK_n
#define KEY_MENU SDLK_j

pthread_t t_capturing;

double additional_scale = 1;
double overlay_scale = 1;
string interpol = "nearest";
SDL_Texture *mOverlay;

int angle = 0;
//原始游戏画面大小
int source_width = 0, source_height = 0;
//miyoo的屏幕大小
int display_width = 640, display_height = 480;


unsigned int last_time = 0;

bool capturing = true;
int rotate=0;

SDL_Renderer *mRenderer;
SDL_Texture *mTexture;
SDL_Texture *romTexture;
SDL_Texture *mBackground;
SDL_Window *mWindow;

SDL_Joystick *g_joystick=NULL;

/* Settings menu */
#define MENU_ITEM_RESOLUTION  0
#define MENU_ITEM_PHONE       1
#define MENU_ITEM_ROTATE      2
#define MENU_ITEM_COUNT       3

static const char *menu_item_names[MENU_ITEM_COUNT] = {"Resolution", "Phone", "Rotate"};

static const int res_presets[][2] = {
	{240,320}, {176,220}, {128,160}, {320,240},
	{128,128}, {208,208}, {360,640},
};
#define RES_PRESET_COUNT 7

static const char *phone_names[] = {"Standard", "Nokia", "Ericsson", "Siemens", "Motorola"};
#define PHONE_MODE_COUNT 5

static const char *rotate_names[] = {"Off", "90", "180"};
#define ROTATE_MODE_COUNT 3

static TTF_Font *menu_font = NULL;
#define MENU_FONT_PATH "/usr/share/fonts/TTF/DejaVuSansMono.ttf"
static int menu_font_size;
static int menu_line_h;
static int menu_width;
static int menu_padding;
static int menu_border;

static struct {
	int visible;
	int selection;
	int need_redraw;
	SDL_Texture *tex;
	int res_index;
	int phone_index;
} settings_menu = {0, 0, 1, NULL, 0, 0};

static void settings_menu_adjust(int item, int dir);
static void settings_menu_draw(SDL_Renderer *renderer);

short joymouseX = 0;
short joymouseY = 0;
bool use_mouse = 0;
// mouse cursor image
unsigned char joymouseImage[374] =
{
	0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,1,2,2,1,0,0,0,0,0,0,0,0,0,
	0,0,0,0,1,2,2,1,0,0,0,0,0,0,0,0,0,
	0,0,0,0,1,2,2,1,0,0,0,0,0,0,0,0,0,
	0,0,0,0,1,2,2,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,1,2,2,1,2,2,1,1,1,0,0,0,0,
	0,0,0,0,1,2,2,1,2,2,1,2,2,1,1,0,0,
	0,0,0,0,1,2,2,1,2,2,1,2,2,1,2,1,0,
	1,1,1,0,1,2,2,1,2,2,1,2,2,1,2,2,1,
	1,2,2,1,1,2,2,2,2,2,2,2,2,1,2,2,1,
	1,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,1,
	0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
	0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
	0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
	0,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,1,
	0,0,0,1,2,2,2,2,2,2,2,2,2,2,2,1,0,
	0,0,0,0,1,2,2,2,2,2,2,2,2,2,2,1,0,
	0,0,0,0,1,2,2,2,2,2,2,2,2,2,2,1,0,
	0,0,0,0,0,1,2,2,2,2,2,2,2,2,1,0,0,
	0,0,0,0,0,1,2,2,2,2,2,2,2,2,1,0,0,
	0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0
};

void sendKey(int key, bool pressed)
{
	unsigned char bytes [5];
	bytes[0] = (char) (use_mouse << 4) | pressed;
	
	if(use_mouse)
	{
		bytes[1] = (char) (joymouseX >> 8 & 0xFF);
		bytes[2] = (char) (joymouseX & 0xFF);
		bytes[3] = (char) (joymouseY >> 8 & 0xFF);
		bytes[4] = (char) (joymouseY & 0xFF);
	}
	else
	{
		bytes[1] = (char) (key >> 24 & 0xFF);
		bytes[2] = (char) (key >> 16 & 0xFF);
		bytes[3] = (char) (key >> 8 & 0xFF);
		bytes[4] = (char) (key & 0xFF);
	}
	
	//std::cout<<"sendkey:"<<key<<" pressed:"<<pressed<<std::endl;
	
	fwrite(&bytes, sizeof(char), 5, stderr);
}

void sendCommand(int cmd, int param1, int param2)
{
	unsigned char bytes[5];
	bytes[0] = (char)(0x20 | (cmd & 0x0F));
	bytes[1] = (char)((param1 >> 8) & 0xFF);
	bytes[2] = (char)(param1 & 0xFF);
	bytes[3] = (char)((param2 >> 8) & 0xFF);
	bytes[4] = (char)(param2 & 0xFF);
	fwrite(&bytes, sizeof(char), 5, stderr);
	fflush(stderr);
}

bool sendQuitEvent()
{
	SDL_Event* quit = new SDL_Event();
	quit->type = SDL_QUIT;
	SDL_PushEvent(quit);
	return true;
}


//实际显示区域
SDL_Rect getDestinationRect(int source_width,int source_height)
{
	double scale= std::min( (double) display_width/source_width, (double) display_height/source_height );

	int w = source_width * scale, h = source_height * scale;
	return { (display_width - w )/2, (display_height - h)/2, w, h };
}

bool updateFrame(size_t num_chars, unsigned char* buffer, FILE* input = stdin)
{
	int read_count = fread(buffer, sizeof(char), num_chars, input);
	return read_count == num_chars;
}


void rot(unsigned char* src,unsigned char*dest,int w,int h)
{
	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			dest[i*h*BYTES+j*BYTES]=src[j*w*BYTES+(w-i)*BYTES];
			dest[i*h*BYTES+j*BYTES+1]=src[j*w*BYTES+(w-i)*BYTES+1];
			dest[i*h*BYTES+j*BYTES+2]=src[j*w*BYTES+(w-i)*BYTES+2];
		}
	}
}

void rot2(unsigned char* src,unsigned char*dest,int w,int h)
{
	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			dest[(w-i-1)*h*BYTES+j*BYTES]=src[(h-j-1)*w*BYTES+(w-i)*BYTES];
			dest[(w-i-1)*h*BYTES+j*BYTES+1]=src[(h-j-1)*w*BYTES+(w-i)*BYTES+1];
			dest[(w-i-1)*h*BYTES+j*BYTES+2]=src[(h-j-1)*w*BYTES+(w-i)*BYTES+2];
		}
	}
}

void drawFrame(unsigned char *frame, SDL_Texture *mTexture,size_t pitch, SDL_Rect *dest, int interFrame = 16)
{
	//Cutoff rendering at 60fps，毫秒
	if (SDL_GetTicks() - last_time < interFrame) {
		return;
	}

	last_time = SDL_GetTicks();

	SDL_RenderClear(mRenderer);
	SDL_RenderCopy(mRenderer, mBackground, NULL, NULL);
	
	SDL_UpdateTexture(mTexture, NULL, frame, pitch);
	SDL_RenderCopy(mRenderer, mTexture, NULL, dest);

	if (settings_menu.visible)
		settings_menu_draw(mRenderer);

	SDL_RenderPresent(mRenderer);
	
}

void loadDisplayDimentions()
{
	SDL_DisplayMode dispMode;
	SDL_GetDesktopDisplayMode(0, &dispMode);
	display_width = dispMode.w;
	display_height = dispMode.h;
}

/******************************************************** Processing Function */
void init()
{
	if (source_width == 0 || source_height == 0)
	{
		std::cout << "anbu: Neither width nor height parameters can be 0." << std::endl;
		exit(0);
	}
	//if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0 )
	if(SDL_Init(SDL_INIT_VIDEO) < 0) 
	{
		std::cout<<"SDL无法初始化! SDL_Error: "<<SDL_GetError()<<std::endl;
		exit(0);
	}

	loadDisplayDimentions();

	SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &mWindow, &mRenderer);
	SDL_SetRenderDrawColor(mRenderer, 44, 62, 80, 255);
	SDL_RenderClear(mRenderer);
	SDL_RenderPresent(mRenderer);

	// Set scaling properties
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	SDL_RenderSetLogicalSize(mRenderer, display_width, display_height);

	menu_font_size = display_height / 30;
	if (menu_font_size < 12) menu_font_size = 12;
	menu_line_h = menu_font_size * 3 / 2;
	menu_width  = display_width * 2 / 3;
	menu_padding = menu_font_size / 2;
	menu_border = display_height / 240;
	if (menu_border < 1) menu_border = 1;

	if (TTF_Init() < 0)
		printf("TTF_Init failed: %s\n", TTF_GetError());
	else {
		menu_font = TTF_OpenFont(MENU_FONT_PATH, menu_font_size);
		if (!menu_font)
			printf("Failed to load font: %s\n", TTF_GetError());
	}
}

void loadBackground(string image)
{
	if (image.empty())
		return;

	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(image.c_str(), 0);
	FIBITMAP* imagen = FreeImage_Load(format, image.c_str());

	int w = FreeImage_GetWidth(imagen);
	int h = FreeImage_GetHeight(imagen);
	int scan_width = FreeImage_GetPitch(imagen);

	unsigned char* buffer = new unsigned char[w * h * 4];
	FreeImage_ConvertToRawBits(buffer, imagen, scan_width, 32, 0, 0, 0, TRUE);
	FreeImage_Unload(imagen);

	mBackground = SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
	SDL_UpdateTexture(mBackground, NULL, buffer, w * sizeof(char) * 4);
	delete[] buffer;
}

void loadOverlay(SDL_Rect &rect)
{
	int psize =  overlay_scale * rect.w / source_width;
	int size = rect.w * rect.h * 4;
	unsigned char *bytes = new unsigned char[size];

	for (int h = 0; h < rect.h; h++)
		for (int w = 0; w < rect.w; w++)
		{
			int c = (h * rect.w + w) * 4;
			bytes[c] = 0;
			bytes[c+1] = 0;
			bytes[c+2] = 0;
			bytes[c+3] = w % psize == 0 || h % psize == 0 ? 64 : 0;
		}

	mOverlay = SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, rect.w, rect.h);
	SDL_SetTextureBlendMode(mOverlay, SDL_BLENDMODE_BLEND);
	SDL_UpdateTexture(mOverlay, NULL, bytes, rect.w * sizeof(unsigned char) * 4);
	delete[] bytes;
}

/* Settings menu functions */
static void settings_menu_get_value_str(int item, char *buf, int buflen)
{
	switch (item) {
	case MENU_ITEM_RESOLUTION:
		snprintf(buf, buflen, "%dx%d",
			res_presets[settings_menu.res_index][0],
			res_presets[settings_menu.res_index][1]);
		break;
	case MENU_ITEM_PHONE:
		snprintf(buf, buflen, "%s", phone_names[settings_menu.phone_index]);
		break;
	case MENU_ITEM_ROTATE:
		snprintf(buf, buflen, "%s", rotate_names[rotate]);
		break;
	default:
		buf[0] = '\0';
	}
}

static void settings_menu_adjust(int item, int dir)
{
	switch (item) {
	case MENU_ITEM_RESOLUTION:
		settings_menu.res_index += dir;
		if (settings_menu.res_index < 0) settings_menu.res_index = RES_PRESET_COUNT - 1;
		if (settings_menu.res_index >= RES_PRESET_COUNT) settings_menu.res_index = 0;
		sendCommand(0, res_presets[settings_menu.res_index][0],
			res_presets[settings_menu.res_index][1]);
		break;
	case MENU_ITEM_PHONE:
		settings_menu.phone_index += dir;
		if (settings_menu.phone_index < 0) settings_menu.phone_index = PHONE_MODE_COUNT - 1;
		if (settings_menu.phone_index >= PHONE_MODE_COUNT) settings_menu.phone_index = 0;
		sendCommand(1, settings_menu.phone_index, 0);
		break;
	case MENU_ITEM_ROTATE:
		rotate += dir;
		if (rotate < 0) rotate = ROTATE_MODE_COUNT - 1;
		if (rotate >= ROTATE_MODE_COUNT) rotate = 0;
		break;
	}
	settings_menu.need_redraw = 1;
}

static void settings_menu_draw(SDL_Renderer *renderer)
{
	if (!menu_font) return;

	int menu_w = menu_width;
	int menu_h = MENU_ITEM_COUNT * menu_line_h + menu_padding * 2;
	int bd = menu_border;

	if (settings_menu.need_redraw || !settings_menu.tex) {
		SDL_Surface *surf = SDL_CreateRGBSurface(0, menu_w, menu_h, 32,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		if (!surf) return;

		SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 200));

		Uint32 white = SDL_MapRGBA(surf->format, 255, 255, 255, 255);
		SDL_Rect border;
		border = {0, 0, menu_w, bd};
		SDL_FillRect(surf, &border, white);
		border = {0, menu_h - bd, menu_w, bd};
		SDL_FillRect(surf, &border, white);
		border = {0, 0, bd, menu_h};
		SDL_FillRect(surf, &border, white);
		border = {menu_w - bd, 0, bd, menu_h};
		SDL_FillRect(surf, &border, white);

		int y = menu_padding;
		char buf[64], val_buf[80];
		for (int i = 0; i < MENU_ITEM_COUNT; i++) {
			SDL_Color name_color = (i == settings_menu.selection) ?
				SDL_Color{255, 255, 0, 255} : SDL_Color{255, 255, 255, 255};
			SDL_Color val_color = {0, 255, 0, 255};

			SDL_Surface *name_surf = TTF_RenderText_Blended(menu_font, menu_item_names[i], name_color);
			if (name_surf) {
				SDL_Rect dst = {menu_padding, y, name_surf->w, name_surf->h};
				SDL_BlitSurface(name_surf, NULL, surf, &dst);
				SDL_FreeSurface(name_surf);
			}

			settings_menu_get_value_str(i, buf, sizeof(buf));
			if (i == settings_menu.selection)
				snprintf(val_buf, sizeof(val_buf), "< %s >", buf);
			else
				snprintf(val_buf, sizeof(val_buf), "  %s  ", buf);

			SDL_Surface *val_surf = TTF_RenderText_Blended(menu_font, val_buf, val_color);
			if (val_surf) {
				SDL_Rect dst = {menu_w - val_surf->w - menu_padding, y, val_surf->w, val_surf->h};
				SDL_BlitSurface(val_surf, NULL, surf, &dst);
				SDL_FreeSurface(val_surf);
			}

			y += menu_line_h;
		}

		if (settings_menu.tex) SDL_DestroyTexture(settings_menu.tex);
		settings_menu.tex = SDL_CreateTextureFromSurface(renderer, surf);
		SDL_FreeSurface(surf);
		if (settings_menu.tex)
			SDL_SetTextureBlendMode(settings_menu.tex, SDL_BLENDMODE_BLEND);
		settings_menu.need_redraw = 0;
	}

	if (settings_menu.tex) {
		SDL_Rect dst;
		dst.w = menu_w;
		dst.h = menu_h;
		dst.x = (display_width - dst.w) / 2;
		dst.y = (display_height - dst.h) / 2;
		SDL_RenderCopy(renderer, settings_menu.tex, NULL, &dst);
	}
}

void rebuildTextures(size_t &pitch, size_t &ropitch, SDL_Rect &dest, SDL_Rect &rodest)
{
	if (mTexture) SDL_DestroyTexture(mTexture);
	if (romTexture) SDL_DestroyTexture(romTexture);

	dest = getDestinationRect(source_width, source_height);
	pitch = source_width * sizeof(char) * BYTES;
	mTexture = SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, source_width, source_height);

	rodest = getDestinationRect(source_height, source_width);
	ropitch = source_height * sizeof(char) * BYTES;
	romTexture = SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, source_height, source_width);
}

bool readFrameHeader(int &w, int &h)
{
	unsigned char header[4];
	if (fread(header, 1, 4, stdin) != 4) return false;
	w = (header[0] << 8) | header[1];
	h = (header[2] << 8) | header[3];
	return (w > 0 && h > 0);
}

void startStreaming(string image)
{
	size_t pitch, ropitch;
	SDL_Rect dest, rodest;

	loadBackground(image);

	rebuildTextures(pitch, ropitch, dest, rodest);
	loadOverlay(dest);

	for (int i = 0; i < RES_PRESET_COUNT; i++) {
		if (res_presets[i][0] == source_width && res_presets[i][1] == source_height) {
			settings_menu.res_index = i;
			break;
		}
	}

	size_t num_chars = source_width * source_height * BYTES;
	unsigned char* frame = new unsigned char[num_chars];
	unsigned char* tmp_frame = new unsigned char[num_chars];

	while (capturing)
	{
		int new_w, new_h;
		if (!readFrameHeader(new_w, new_h))
		{
			sendQuitEvent();
			break;
		}

		if (new_w != source_width || new_h != source_height)
		{
			source_width = new_w;
			source_height = new_h;
			num_chars = source_width * source_height * BYTES;
			delete[] frame;
			delete[] tmp_frame;
			frame = new unsigned char[num_chars];
			tmp_frame = new unsigned char[num_chars];
			rebuildTextures(pitch, ropitch, dest, rodest);
			printf("Resolution changed: %dx%d\n", source_width, source_height);
		}

		if (!updateFrame(num_chars, frame))
		{
			sendQuitEvent();
			break;
		}

		if(rotate==1)
		{
			rot(frame,tmp_frame,source_width, source_height);
			drawFrame(tmp_frame, romTexture,ropitch, &rodest);
		}
		else if(rotate==2)
		{
			rot2(frame,tmp_frame,source_width, source_height);
			drawFrame(tmp_frame, romTexture,ropitch, &rodest);
		}
		else
		{
			if(use_mouse)
			{
				int t=0;
				for(int i=0; i<22; i++)
				{
					for (int j=0; j<17; j++)
					{
						if((joymouseX+j>=source_width) || (joymouseY+i>=source_height))
						{
							continue;
						}
						t = ((joymouseY + i)*source_width+(joymouseX + j))*BYTES;

						switch (joymouseImage[(i*17)+j])
						{
							case 1:
								frame[t] = 0x00;
								frame[t+1] = 0x00;
								frame[t+2] = 0x00;
								break;
							case 2:
								frame[t] = 0xFF;
								frame[t+1] = 0xFF;
								frame[t+2] = 0xFF;
								break;
						}
					}
				}
			}

			drawFrame(frame, mTexture,pitch, &dest);
		}
	}

	if(g_joystick)
	{
		SDL_JoystickClose(g_joystick);
	}
	SDL_DestroyTexture(mTexture);
	SDL_DestroyTexture(romTexture);
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	delete[] frame;
	delete[] tmp_frame;
}

static void quit_process()
{
	capturing = false;
	sendKey(-1, true);
	fflush(stderr);
}

void *startCapturing(void *args)
{
	int mod=0;
	int ignore=0;
	//int isFirst=1;
	SDL_JoystickEventState(SDL_ENABLE);
	while (capturing)
	{
		SDL_Event event;
		if (SDL_WaitEvent(&event))
		// while(SDL_PollEvent(&event)!= 0)
		{
			switch (event.type)
			{
			case SDL_QUIT:
				quit_process();
				continue;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			
				int key = event.key.keysym.sym;
				
				printf("get key pressed:0x%x, mod:%d\n", key, mod);

				if (key == KEY_QUIT)
					quit_process();

				if (key == KEY_MENU && event.type == SDL_KEYDOWN) {
					settings_menu.visible = !settings_menu.visible;
					settings_menu.need_redraw = 1;
					break;
				}

				if (settings_menu.visible) {
					if (event.type == SDL_KEYDOWN) {
						if (key == KEY_2)  {
							settings_menu.selection = (settings_menu.selection - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
							settings_menu.need_redraw = 1;
						} else if (key == KEY_8) {
							settings_menu.selection = (settings_menu.selection + 1) % MENU_ITEM_COUNT;
							settings_menu.need_redraw = 1;
						} else if (key == KEY_4) {
							settings_menu_adjust(settings_menu.selection, -1);
						} else if (key == KEY_6) {
							settings_menu_adjust(settings_menu.selection, 1);
						} else if (key == KEY_0) {
							settings_menu.visible = 0;
						}
					}
					break;
				}

				if (key == KEY_SNAP && event.type == SDL_KEYDOWN) {
					sendKey(-2, true);
					fflush(stderr);
					continue;
				}
				else if(key==KEY_RIGHT)//A=右键
				{
					key=SDLK_w;
				}
				else if(key==KEY_0)//B=0
				{
					if(!ignore)
					{
						key=SDLK_0;
					}
					else
					{
						key=SDLK_x;//英文x，这里只要不冲突就行
						ignore=0;
					}
				}
				else if (key==KEY_ROTT)
				{
					key=SDLK_x;//英文x，这里只要不冲突就行
					if(event.type == SDL_KEYDOWN)
						rotate=(1+rotate)%3;//连续旋转
				}
				else if (key==KEY_MOD)
				{
					key=SDLK_x;//英文x，这里只要不冲突就行
					if(event.type == SDL_KEYDOWN)
						key=SDLK_c;//英文c,发送到java,作为切换按键模式的信号
				}
				else if (key==KEY_MOS)
				{
					key=SDLK_x;
					if(event.type == SDL_KEYDOWN)
						use_mouse=1-use_mouse;
				}
				else if(key==KEY_OK) //X=ok
				{
					key=SDLK_RETURN;
					
					if(use_mouse)
					{
						sendKey(key, event.type == SDL_KEYDOWN);
						fflush(stderr);
						continue;
					}
					
				}
				else if(key==KEY_LEFT) //Y=左键
				{
					
					if(!ignore)
					{
						key=SDLK_q;
					}
					else
					{
						key=SDLK_x;//英文x，这里只要不冲突就行
						ignore=0;
					}
				}
				else if(key==KEY_2)//上
				{
					key=SDLK_UP;
					if(rotate==1)
					{
						key=SDLK_RIGHT;
					}
					else if(rotate==2)
					{
						key=SDLK_LEFT;
					}
					else
					{
						if(use_mouse && event.type == SDL_KEYDOWN)
						{
							if(joymouseY<6)
							{
								joymouseY=0;
							}
							else
							{
								joymouseY-=6;
							}
						}
					}
				}
				else if(key==KEY_8)//下
				{
					key=SDLK_DOWN;
					if(rotate==1)
					{
						key=SDLK_LEFT;
					}
					else if(rotate==2)
					{
						key=SDLK_RIGHT;
					}
					else
					{
						if(use_mouse && event.type == SDL_KEYDOWN)
						{
							if(joymouseY+6>=source_height-11)
							{
								joymouseY=source_height-11;
							}
							else
							{
								joymouseY+=6;
							}
						}
					}
				}
				else if(key==KEY_4) //左
				{
					key=SDLK_LEFT;
					if(rotate==1)
					{
						key=SDLK_UP;
					}
					else if(rotate==2)
					{
						key=SDLK_DOWN;
					}
					else
					{
						if(use_mouse && event.type == SDL_KEYDOWN)
						{
							if(joymouseX<6)
							{
								joymouseX=0;
							}
							else
							{
								joymouseX-=6;
							}
						}
					}
				}
				else if(key==KEY_6) //右
				{
					key=SDLK_RIGHT;
					if(rotate==1)
					{
						key=SDLK_DOWN;
					}
					else if(rotate==2)
					{
						key=SDLK_UP;
					}
					else
					{
						if(use_mouse && event.type == SDL_KEYDOWN)
						{
							if(joymouseX+6>=source_width-8)
							{
								joymouseX=source_width-8;
							}
							else
							{
								joymouseX+=6;
							}
						}
					}
				}
				else if(key==KEY_POUND) //start=#
				{
					if(!ignore)
					{
						key=SDLK_r;
					}
					else
					{
						key=SDLK_x;//英文x，这里只要不冲突就行
						ignore=0;
					}
				}
				else if(key==KEY_STAR) //select=*
				{
					if(!ignore)
					{
						key=SDLK_e;
					}
					else
					{
						key=SDLK_x;//英文x，这里只要不冲突就行
						ignore=0;
					}
				}
				else if(key==KEY_1) //L1=1
				{
					key=SDLK_1;
				}
				else if(key==KEY_7) //L2=7
				{
					key=SDLK_7;
				}
				else if(key==KEY_3) //R1=3
				{
					key=SDLK_3;
				}
				else if(key==KEY_9) //R2=9
				{
					key=SDLK_9;
				}
				
				if(!use_mouse)
				{
					sendKey(key, event.type == SDL_KEYDOWN);
				}

				break;
			}
			fflush(stderr);
			
			//SDL_Delay(20);
		}
	}
	fflush(stderr);
	pthread_exit(NULL);
}

/*********************************************************************** Main */
int main(int argc, char* argv[])
{
	int c = 0;
	string bg_image = "";

	while (++c < argc)
	{
		if ( argc < 3 || string("--help") == argv[c] || string("-h") == argv[c]) {
			return 0;
		} else if (c == 1) {
			source_width = atoi(argv[c]);
			source_height = atoi(argv[++c]);
		} else if (c > 2 && string("-r") == argv[c] && argc > c + 1) {
			angle = atoi(argv[++c]) % 360;
		} else if (c > 2 && string("-i") == argv[c] && argc > c + 1) {
			interpol = argv[++c];
		} else if (c > 2 && string("-b") == argv[c] && argc > c + 1) {
			bg_image = argv[++c];
		} else if (c > 2 && string("-s") == argv[c] && argc > c + 1) {
			additional_scale = atof(argv[++c]);
		}
	}

	init();
	
	if (pthread_create(&t_capturing, 0, &startCapturing, NULL))
	{
		std::cout << "Unable to start thread, exiting ..." << std::endl;
		SDL_Quit();
		return 1;
	}

	startStreaming(bg_image);
	pthread_join(t_capturing, NULL);
	if (settings_menu.tex) SDL_DestroyTexture(settings_menu.tex);
	if (menu_font) TTF_CloseFont(menu_font);
	TTF_Quit();
	SDL_ShowCursor(false);
	SDL_Quit();
	return 0;
}
