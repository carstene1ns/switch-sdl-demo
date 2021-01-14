/* Mini SDL Demo
 * featuring SDL2 + SDL2_mixer + SDL2_image + SDL2_ttf
 * on Nintendo Switch using libnx
 *
 * (c) 2018-2021 carstene1ns, ISC license
 */

#include <time.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#ifdef __SWITCH__
#include <switch.h>
#endif

// some switch buttons
#define JOY_A     0
#define JOY_B     1
#define JOY_X     2
#define JOY_Y     3
#define JOY_PLUS  10
#define JOY_LEFT  12
#define JOY_UP    13
#define JOY_RIGHT 14
#define JOY_DOWN  15

#define SCREEN_W 1280
#define SCREEN_H 720

SDL_Texture *render_text(SDL_Renderer *renderer, const char *text, TTF_Font *font, SDL_Color color, SDL_Rect *rect) {
    SDL_Surface *surface;
    SDL_Texture *texture;

    surface = TTF_RenderText_Solid(font, text, color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect->w = surface->w;
    rect->h = surface->h;

    SDL_FreeSurface(surface);

    return texture;
}

int rand_range(int min, int max) {
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

int main(int argc, char** argv) {

    // find our data files
#ifdef __SWITCH__
    romfsInit();
    chdir("romfs:/");
#else
    chdir("romfs");
#endif

    int exit_requested = 0;
    int trail = 0;
    int wait = 25;

    SDL_Texture *switchlogo_tex = NULL, *sdllogo_tex = NULL, *helloworld_tex = NULL;
    SDL_Rect pos = { 0, 0, 0, 0 }, sdl_pos = { 0, 0, 0, 0 };
    Mix_Music *music = NULL;
    Mix_Chunk *sound[4] = { NULL };
    SDL_Event event;

    SDL_Color colors[] = {
        { 128, 128, 128, 0 }, // gray
        { 255, 255, 255, 0 }, // white
        { 255, 0, 0, 0 },     // red
        { 0, 255, 0, 0 },     // green
        { 0, 0, 255, 0 },     // blue
        { 255, 255, 0, 0 },   // brown
        { 0, 255, 255, 0 },   // cyan
        { 255, 0, 255, 0 },   // purple
    };
    int col = 0, snd = 0;

    srand(time(NULL));
    int vel_x = rand_range(1, 5);
    int vel_y = rand_range(1, 5);

    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    Mix_Init(MIX_INIT_OGG);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("sdl2+mixer+image+ttf demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    // load logos from file
    SDL_Surface *sdllogo = IMG_Load("data/sdl.png");
    if (sdllogo) {
        sdl_pos.w = sdllogo->w;
        sdl_pos.h = sdllogo->h;
        sdllogo_tex = SDL_CreateTextureFromSurface(renderer, sdllogo);
        SDL_FreeSurface(sdllogo);
    }

    SDL_Surface *switchlogo = IMG_Load("data/switch.png");
    if (switchlogo) {
        pos.x = SCREEN_W / 2 - switchlogo->w / 2;
        pos.y = SCREEN_H / 2 - switchlogo->h / 2;
        pos.w = switchlogo->w;
        pos.h = switchlogo->h;
        switchlogo_tex = SDL_CreateTextureFromSurface(renderer, switchlogo);
        SDL_FreeSurface(switchlogo);
    }

    col = rand_range(0, 7);

#ifdef __SWITCH__
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_JoystickOpen(0);
#endif

    SDL_InitSubSystem(SDL_INIT_AUDIO);
    Mix_AllocateChannels(5);
    Mix_OpenAudio(48000, AUDIO_S16, 2, 4096);

    // load music and sounds from files
    music = Mix_LoadMUS("data/background.ogg");
    sound[0] = Mix_LoadWAV("data/pop1.wav");
    sound[1] = Mix_LoadWAV("data/pop2.wav");
    sound[2] = Mix_LoadWAV("data/pop3.wav");
    sound[3] = Mix_LoadWAV("data/pop4.wav");
    if (music)
        Mix_PlayMusic(music, -1);

    // load font
    TTF_Font* font = TTF_OpenFont("data/LeroyLetteringLightBeta01.ttf", 36);

    // render text as texture
    SDL_Rect helloworld_rect = { 0, SCREEN_H - 36, 0, 0 };
    helloworld_tex = render_text(renderer, "Hello, world!", font, colors[1], &helloworld_rect);

    // no need to keep the font loaded
    TTF_CloseFont(font);

    while (!exit_requested
#ifdef __SWITCH__
        && appletMainLoop()
#endif
        ) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                exit_requested = 1;

#ifdef __SWITCH__
            // use joystick
            if (event.type == SDL_JOYBUTTONDOWN) {
                if (event.jbutton.button == JOY_UP)
                    if (wait > 0)
                        wait--;
                if (event.jbutton.button == JOY_DOWN)
                    if (wait < 100)
                        wait++;

                if (event.jbutton.button == JOY_PLUS)
                    exit_requested = 1;

                if (event.jbutton.button == JOY_B)
                    trail =! trail;
            }
#else
            // use keyboard
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_UP)
                    if (wait > 0)
                        wait--;
                if (event.key.keysym.sym == SDLK_DOWN)
                    if (wait < 100)
                        wait++;

                if (event.key.keysym.sym == SDLK_ESCAPE)
                    exit_requested = 1;

                if (event.key.keysym.sym == SDLK_b)
                    trail =! trail;
            }
#endif
        }

        // set position and bounce on the walls
        pos.y += vel_y;
        pos.x += vel_x;
        if (pos.x + pos.w > SCREEN_W) {
            pos.x = SCREEN_W - pos.w;
            vel_x = -rand_range(1, 5);
            col = rand_range(0, 4);
            snd = rand_range(0, 3);
            if (sound[snd])
                Mix_PlayChannel(-1, sound[snd], 0);
        }
        if (pos.x < 0) {
            pos.x = 0;
            vel_x = rand_range(1, 5);
            col = rand_range(0, 4);
            snd = rand_range(0, 3);
            if (sound[snd])
                Mix_PlayChannel(-1, sound[snd], 0);
        }
        if (pos.y + pos.h > SCREEN_H) {
            pos.y = SCREEN_H - pos.h;
            vel_y = -rand_range(1, 5);
            col = rand_range(0, 4);
            snd = rand_range(0, 3);
            if (sound[snd])
                Mix_PlayChannel(-1, sound[snd], 0);
        }
        if (pos.y < 0) {
            pos.y = 0;
            vel_y = rand_range(1, 5);
            col = rand_range(0, 4);
            snd = rand_range(0, 3);
            if (sound[snd])
                Mix_PlayChannel(-1, sound[snd], 0);
        }

        if (!trail) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
            SDL_RenderClear(renderer);
        }

        // put logos on screen
        if (sdllogo_tex)
            SDL_RenderCopy(renderer, sdllogo_tex, NULL, &sdl_pos);
        if (switchlogo_tex) {
            SDL_SetTextureColorMod(switchlogo_tex, colors[col].r, colors[col].g, colors[col].b);
            SDL_RenderCopy(renderer, switchlogo_tex, NULL, &pos);
        }

        // put text on screen
        if (helloworld_tex)
            SDL_RenderCopy(renderer, helloworld_tex, NULL, &helloworld_rect);

        SDL_RenderPresent(renderer);

        SDL_Delay(wait);
    }

    // clean up your textures when you are done with them
    if (sdllogo_tex)
        SDL_DestroyTexture(sdllogo_tex);

    if (switchlogo_tex)
        SDL_DestroyTexture(switchlogo_tex);

    if (helloworld_tex)
        SDL_DestroyTexture(helloworld_tex);

    // stop sounds and free loaded data
    Mix_HaltChannel(-1);
    Mix_FreeMusic(music);
    for (snd = 0; snd < 4; snd++)
        if (sound[snd])
            Mix_FreeChunk(sound[snd]);

    IMG_Quit();
    Mix_CloseAudio();
    TTF_Quit();
    Mix_Quit();
    SDL_Quit();

#ifdef __SWITCH__
    romfsExit();
#endif

    return 0;
}
