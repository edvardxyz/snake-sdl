#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define WIDTH 440
#define HEIGHT 300
#define SIZE 20
#define FPS 8
#define divX WIDTH / SIZE
#define divY HEIGHT / SIZE
#define APPLE_SCORE 120
#define NUM_WAVE 2
const char* wave_file_names[] = 
{
  "eat.wav",
  "die.wav"
};

Mix_Chunk* sample[NUM_WAVE];

enum direction { UP = 1, DOWN, LEFT, RIGHT } direction;

typedef struct Node
{
  SDL_Rect rect;
  struct Node *next;
} Node;

typedef struct Apple
{
  int score;
  SDL_Rect rect;
  struct Apple *next;
} Apple;

void MoveSnake(Node**, bool, int);

int Init(void)
{
  memset(sample, 0, sizeof(Mix_Chunk *) * NUM_WAVE);

  /* Set up the audio stream */
  int result = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
  if (result < 0)
  {
    fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
    exit(1);
  }

  result = Mix_AllocateChannels(4);
  if (result < 0)
  {
    fprintf(stderr, "Unable to allocate mixing channels %s\n", SDL_GetError());
    exit(1);
  }

  /* Load waveforms */
  for (int i = 0; i < NUM_WAVE; i++)
  {
    sample[i] = Mix_LoadWAV(wave_file_names[i]);
    if (sample[i] == NULL)
    {
      fprintf(stderr, "Unable to load wave file: %s Error: %s\n", 
      wave_file_names[i], Mix_GetError());
    }
  }
  return true;
}

int main(int argc, char *argv[])
{
    fprintf(stderr, "Init audio failed\n");
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
  {
    fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
    return 1;
  }
  /* Create a window */
  SDL_Window *wind = SDL_CreateWindow("Snake", SDL_WINDOWPOS_CENTERED, 
                                      SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
  if (!wind)
  {
    fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }
  /* Create a renderer */
  SDL_Renderer *rend = SDL_CreateRenderer(wind, -1, SDL_RENDERER_ACCELERATED);
  if (!rend)
  {
    fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(wind);
    SDL_Quit();
    return 1;
  }

  /* Init audio */
  if ( Init() == false){
    fprintf(stderr, "Init audio failed\n");
    return 1;
  }

  /* Init game vars */
  int score, highscore = 0;
  int dir = RIGHT;
  SDL_Event event;
  bool running = true;
  bool eaten = false;
  bool dead = false;
  bool btn_pressed = false;
  Uint8 apple_r = 255;
  Uint8 apple_g = 0;

  /* Get highscore */
  FILE *fptr = fopen("highscore.dat", "r");
  if (fptr != NULL)
  {
    fscanf(fptr, "%d", &highscore);
  }
  fclose(fptr);

  /* Create snake of 3 nodes */
  Node head, body, tail;
  /* Node pointer to keep track of head */
  Node *p_head = &head;

  head.rect.x = SIZE;
  head.rect.y = (HEIGHT / 2) + ((HEIGHT / 2) % SIZE);
  head.rect.h = head.rect.w = SIZE;
  head.next = &body;

  body.rect.x = head.rect.x - SIZE;
  body.rect.y = head.rect.y;
  body.rect.h = body.rect.w = SIZE;
  body.next = &tail;

  tail.rect.x = body.rect.x - SIZE;
  tail.rect.y = head.rect.y;
  tail.rect.h = tail.rect.w = SIZE;
  tail.next = NULL;

  /* Create apple */
  Apple apple;
  apple.rect.h = apple.rect.w = SIZE;
  apple.rect.x = rand() % divX * SIZE;
  apple.rect.y = rand() % divY * SIZE;
  apple.score = APPLE_SCORE;
  apple.next = NULL;


  /* Main loop */
  while (running)
  {
    btn_pressed = false;
    /* Process events */
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_KEYDOWN:

        if (!btn_pressed)
        {
          switch (event.key.keysym.scancode)
          {
          case SDL_SCANCODE_A:
          case SDL_SCANCODE_LEFT:
          case SDL_SCANCODE_H:
            if (dir != RIGHT)
              dir = LEFT;
            break;
          case SDL_SCANCODE_D:
          case SDL_SCANCODE_RIGHT:
          case SDL_SCANCODE_L:
            if (dir != LEFT)
              dir = RIGHT;
            break;
          case SDL_SCANCODE_W:
          case SDL_SCANCODE_UP:
          case SDL_SCANCODE_K:
            if (dir != DOWN)
              dir = UP;
            break;
          case SDL_SCANCODE_S:
          case SDL_SCANCODE_DOWN:
          case SDL_SCANCODE_J:
            if (dir != UP)
              dir = DOWN;
            break;
          default:
            break;
          }
          btn_pressed = true;
        }
        break;
      default:
        break;
      }
    }

    Node *p = p_head;
    /* Check if collision with apple */
    if (apple.rect.y == p_head->rect.y && apple.rect.x == p_head->rect.x)
    {
      Mix_PlayChannel(-1, sample[0], 0);
      eaten = true;
      score += apple.score;
    /* Find new apple pos not inside snake */
    notFound:
      p = p_head;
      int x = rand() % divX * SIZE;
      int y = rand() % divY * SIZE;
      while (p != NULL)
      {
        if (p->rect.x == x && p->rect.y == y)
        {
          goto notFound;
        }
        p = p->next;
      }
      apple.rect.y = y;
      apple.rect.x = x;
      apple.score = APPLE_SCORE;
    }
    /* Reduce score by 5 until eq 5 and blink the apple while higher score */
    else if ( apple.score != 5)
    {
      apple.score -= 5;
      if (apple_g == 255)
      {
        apple_g = 0;
        apple_r = 255;
      }
      else
      {
        apple_g = 255;
        apple_r = 0;
      }
    }

    /* Check collision with snake
     start loop 3 segments behind as snake cant turn and hit those segments */
    p = p_head->next->next->next; 
    while (p != NULL)
    {
      if (p->rect.x == p_head->rect.x && p->rect.y == p_head->rect.y)
      {
        dead = true;
        break;
      }
      p = p->next;
    }

    /* Check collision with edge */
    if (p_head->rect.x >= WIDTH || p_head->rect.x < 0 || 
    p_head->rect.y < 0 || p_head->rect.y >= HEIGHT)
    {
      dead = true;
    }

    if (dead){
      running = false;
      Mix_PlayChannel(-1, sample[1], 0);
      while(Mix_Playing(-1) !=0){
        SDL_Delay(300);
      }
      break;
    }

    MoveSnake(&p_head, eaten, dir);
    eaten = false;

    /* Print game info in window title */
    char score_str[256];
    sprintf(score_str, "Snake | Score: %d | Highscore: %d", score, highscore);
    SDL_SetWindowTitle(wind, score_str);

    /* Clear screen */
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderClear(rend);

    /* Draw the apple */
    SDL_SetRenderDrawColor(rend, apple_r, apple_g, 0, 255);
    SDL_RenderFillRect(rend, &apple.rect);

    /* Draw segments of snake */
    p = p_head;
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    while (p != NULL)
    {
      SDL_RenderFillRect(rend, &p->rect);
      p = p->next;
    }

    /* Draw to window and loop */
    SDL_RenderPresent(rend);
    SDL_Delay(1000 / FPS);
  }

  /* Save score if bigger than highscore */
  if (score > highscore)
  {
    FILE *fptr = fopen("highscore.dat", "w");
    if (fptr != NULL)
    {
      fprintf(fptr, "%d", score);
    }
    fclose(fptr);
  }

  /* Release resources */
  for ( int i = 0; i < NUM_WAVE; i++){
    Mix_FreeChunk(sample[i]);
  }
  Mix_CloseAudio();
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(wind);
  SDL_Quit();
  return 0;
}

/*
 * Makes tail move to front then changes head pointer to new head
 * if eaten is true a new node will be added at the end
 */
void MoveSnake(Node **p_head, bool eaten, int dir)
{
  Node *prev = NULL, *cur = NULL;
  prev = *p_head;
  cur = prev->next;

  /* Go to last node and keep track of prev */
  while (cur->next != NULL)
  {
    prev = prev->next;
    cur = cur->next;
  }

  SDL_Rect tail_rect = cur->rect;


  /* Change tails position to heads with a diff depending on direction */
  switch (dir)
  {
  case UP:
    cur->rect.y = (*p_head)->rect.y - SIZE;
    cur->rect.x = (*p_head)->rect.x;
    break;
  case DOWN:
    cur->rect.y = (*p_head)->rect.y + SIZE;
    cur->rect.x = (*p_head)->rect.x;
    break;
  case LEFT:
    cur->rect.x = (*p_head)->rect.x - SIZE;
    cur->rect.y = (*p_head)->rect.y;
    break;
  case RIGHT:
    cur->rect.x = (*p_head)->rect.x + SIZE;
    cur->rect.y = (*p_head)->rect.y;
    break;
  default:
    break;
  }

  /* If eaten create new node and make it the current */
  if (eaten)
  {
    Node *new_segment = (Node *)malloc(sizeof(Node));
    if (new_segment == NULL)
    {
      fprintf(stderr, "Failed to allocate new node\n");
      exit(1);
    }
    new_segment->rect = tail_rect;
    new_segment->next = NULL;
    prev->next = new_segment;
  }
  else
  {
    /* Make 2nd last node point to NULL pointer as its the new tail */
    prev->next = NULL;
  }

  /* Make tail point to head */
  cur->next = *p_head;

  /* Make pointer to head point to new head */
  *p_head = cur;
}