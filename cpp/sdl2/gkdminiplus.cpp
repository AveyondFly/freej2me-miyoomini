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
#include "cJSON.h"
#include <FreeImage.h>

#define BYTES 3

#define M_A 1
#define M_B 0
#define M_X 2
#define M_Y 3
#define M_start 9
#define M_select 8
#define M_L 4
#define M_L2 6
#define M_R 5
#define M_R2 7
#define M_UP 12
#define M_DOWN 13
#define M_LEFT 14
#define M_RIGHT 15
#define M_QUIT1 10
#define M_SNAP 11

using namespace std;

int KEY_LEFT=M_Y;
int KEY_RIGHT=M_A;
int KEY_OK=M_X;
int KEY_STAR=M_select;
int KEY_POUND=M_start;
int KEY_1=M_L;
int KEY_3=M_R;
int KEY_7=M_L2;
int KEY_9=M_R2;
int KEY_0=M_B;

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

	//更新到屏幕
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
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) 
	{
		std::cout<<"SDL无法初始化! SDL_Error: "<<SDL_GetError()<<std::endl;
		exit(0);
	}

	loadDisplayDimentions();
	if (SDL_NumJoysticks() >= 1)
	{
		g_joystick = SDL_JoystickOpen(0);
		if (g_joystick == NULL)
		{
			std::cout<<"Unable to open joystick."<<std::endl;
			exit(0);
		}
	}


	SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &mWindow, &mRenderer);
	SDL_SetRenderDrawColor(mRenderer, 44, 62, 80, 255);
	SDL_RenderClear(mRenderer);
	SDL_RenderPresent(mRenderer);

	// Set scaling properties
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	SDL_RenderSetLogicalSize(mRenderer, display_width, display_height);	
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

void startStreaming(string image)
{
	size_t pitch,ropitch;
	SDL_Rect dest,rodest;
	
	dest= getDestinationRect(source_width,source_height);

	loadBackground(image);
	loadOverlay(dest);

	pitch= source_width * sizeof(char) * BYTES;
	//纹理大小是jar游戏大小
	mTexture = SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, source_width, source_height);
	
	rodest= getDestinationRect(source_height,source_width);
	ropitch= source_height * sizeof(char) * BYTES;
	romTexture = SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,  source_height,source_width);

 
	size_t num_chars = source_width * source_height * BYTES;
	unsigned char* frame = new unsigned char[num_chars];
	unsigned char* tmp_frame = new unsigned char[num_chars];
	
	//从输入流里读取到frame
	while (capturing && updateFrame(num_chars, frame) || !sendQuitEvent())
	{
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
	//SDL_DestroyTexture(mBackground);
	SDL_DestroyRenderer(mRenderer);
	
    SDL_DestroyWindow(mWindow);
	delete[] frame;
	delete[] tmp_frame;
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
				capturing = false;
				sendKey(-1, true);
				fflush(stderr);
				continue;

			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
			
				int key = event.jbutton.button;
				
				//printf("get key pressed:0x%x\n", key);
				if (key == M_QUIT1) {
					capturing = false;
					sendKey(-1, true);
					fflush(stderr);
					continue;
				}
				if (key == M_SNAP && event.jbutton.state == SDL_PRESSED) {
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
					
					
					if(mod && event.type == SDL_JOYBUTTONDOWN)
					{
						key=SDLK_x;//英文x，这里只要不冲突就行
						mod=0;
						rotate=(1+rotate)%3;//连续旋转
						ignore=1;//忽略下次0键的释放
					}
					
				}
				else if(key==KEY_OK) //X=ok
				{
					key=SDLK_RETURN;
					
					if(use_mouse)
					{
						sendKey(key, event.jbutton.state == SDL_PRESSED);
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
					
					
					if(mod && event.type == SDL_JOYBUTTONDOWN)
					{
						key=SDLK_x;//英文x，这里只要不冲突就行
						mod=0;
						use_mouse=1-use_mouse;//切换鼠标
						ignore=1;//忽略下次y键的释放
					}
					
					
				}
				else if(key==M_UP)//上
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
						if(use_mouse && event.jbutton.state == SDL_PRESSED)
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
				else if(key==M_DOWN)//下
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
						if(use_mouse && event.jbutton.state == SDL_PRESSED)
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
				else if(key==M_LEFT) //左
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
						if(use_mouse && event.jbutton.state == SDL_PRESSED)
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
				else if(key==M_RIGHT) //右
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
						if(use_mouse && event.jbutton.state == SDL_PRESSED)
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
					
					
					
					if(mod && event.type == SDL_JOYBUTTONDOWN)
					{
						key=SDLK_c;//英文c,发送到java,作为切换按键模式的信号
						mod=0;
						ignore=1;
					}
					
				}
				else if(key==KEY_STAR) //select=*
				{
					key=SDLK_e;
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
				
				
				//按住select键
				if(event.type == SDL_JOYBUTTONDOWN && event.jbutton.button == M_select){mod=1;}
				else if(event.type == SDL_JOYBUTTONUP && event.jbutton.button == M_select){mod=0;}
				
				if(!use_mouse)
				{
					sendKey(key, event.jbutton.state == SDL_PRESSED);
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

int keyname2keycode(const char * name)
{
	if(strcmp(name,"X")==0)
	{
		return M_X;
	}
	else if(strcmp(name,"B")==0)
	{
		return M_B;
	}
	else if(strcmp(name,"A")==0)
	{
		return M_A;
	}
	else if(strcmp(name,"SELECT")==0)
	{
		return M_select;
	}
	else if(strcmp(name,"START")==0)
	{
		return M_start;
	}
	else if(strcmp(name,"Y")==0)
	{
		return M_Y;
	}
	else if(strcmp(name,"L")==0)
	{
		return M_L;
	}
	else if(strcmp(name,"R")==0)
	{
		return M_R;
	}
	else if(strcmp(name,"L2")==0)
	{
		return M_L2;
	}
	else if(strcmp(name,"R2")==0)
	{
		return M_R2;
	}
	
	return 0;
	
}

void defaultKeymap()
{
	KEY_LEFT=M_Y;
	KEY_RIGHT=M_A;
	KEY_OK=M_X;
	KEY_STAR=M_select;
	KEY_POUND=M_start;
	KEY_1=M_L;
	KEY_3=M_R;
	KEY_7=M_L2;
	KEY_9=M_R2;
	KEY_0=M_B;
}

int loadConfig() {
    // 打开文件
    FILE *file = fopen("/storage/java/keymap.cfg", "r");
    if (file == NULL) {
		std::cout  << "打开文件失败" << std::endl;
        return -1;
    }
 
    // 确定文件长度
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
 
    // 读取文件内容到字符串
    char *data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);
 
    // 解析JSON字符串
    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stdout, "解析json错误：%s\n", error_ptr);
        }
        cJSON_Delete(json);
        free(data);
        return -1;
    }
	
	try{
		// 使用cJSON对象
		cJSON *name = cJSON_GetObjectItem(json, "左键");
		KEY_LEFT=keyname2keycode(name->valuestring);
		
		name = cJSON_GetObjectItem(json, "右键");
		KEY_RIGHT=keyname2keycode(name->valuestring);
		
		name = cJSON_GetObjectItem(json, "OK");
		KEY_OK=keyname2keycode(name->valuestring);
		
		name = cJSON_GetObjectItem(json, "*");
		KEY_STAR=keyname2keycode(name->valuestring);
		
		name = cJSON_GetObjectItem(json, "#");
		KEY_POUND=keyname2keycode(name->valuestring);
		
		name = cJSON_GetObjectItem(json, "0");
		KEY_0=keyname2keycode(name->valuestring);
		
		name = cJSON_GetObjectItem(json, "1");
		KEY_1=keyname2keycode(name->valuestring);
		
		name = cJSON_GetObjectItem(json, "3");
		KEY_3=keyname2keycode(name->valuestring);
		
		name = cJSON_GetObjectItem(json, "7");
		KEY_7=keyname2keycode(name->valuestring);
		
		name = cJSON_GetObjectItem(json, "9");
		KEY_9=keyname2keycode(name->valuestring);
		
	}
	catch(std::exception& e)
	{
		
		defaultKeymap();
		
		std::cout << "解析json出错:"<<e.what() << std::endl;
		return -1;
	}
 
 
    // 清理工作
    cJSON_Delete(json);
    free(data);
 
    return 0;
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

	loadConfig();
	init();
	
	if (pthread_create(&t_capturing, 0, &startCapturing, NULL))
	{
		std::cout << "Unable to start thread, exiting ..." << std::endl;
		SDL_Quit();
		return 1;
	}

	startStreaming(bg_image);
	pthread_join(t_capturing, NULL);
	SDL_ShowCursor(false);
	SDL_Quit();
	return 0;
}
